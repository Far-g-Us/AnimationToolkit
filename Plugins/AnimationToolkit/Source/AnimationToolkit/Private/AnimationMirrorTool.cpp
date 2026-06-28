#include "AnimationMirrorTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"

UAnimSequence* UAnimationMirrorTool::MirrorAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const TArray<FMirrorBoneMapping>& BoneMappings,
	bool bMirrorTranslation,
	bool bMirrorRotation)
{
	if (!WorldContextObject || !SourceAnimation)
	{
		return nullptr;
	}

	const IAnimationDataModel* Model = SourceAnimation->GetDataModel();
	if (!Model)
	{
		return nullptr;
	}

	const FString NewName = SourceAnimation->GetName() + TEXT("_Mirrored");
	UAnimSequence* MirroredAnim = DuplicateObject<UAnimSequence>(SourceAnimation, SourceAnimation->GetOuter());
	if (!MirroredAnim)
	{
		return nullptr;
	}

	IAnimationDataController& Controller = MirroredAnim->GetController();

	const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		const FRawAnimSequenceTrack& Raw = Track.InternalTrackData;

		TArray<FVector3f> MirroredPosKeys = Raw.PosKeys;
		TArray<FQuat4f> MirroredRotKeys = Raw.RotKeys;
		TArray<FVector3f> MirroredScaleKeys = Raw.ScaleKeys;

		if (bMirrorTranslation)
		{
			for (FVector3f& Pos : MirroredPosKeys)
			{
				Pos.X = -Pos.X;
			}
		}

		if (bMirrorRotation)
		{
			for (FQuat4f& Rot : MirroredRotKeys)
			{
				Rot = FQuat4f(FQuat(-Rot.X, Rot.Y, Rot.Z, Rot.W));
			}
		}

		if (MirroredPosKeys.Num() > 0 || MirroredRotKeys.Num() > 0)
		{
			Controller.SetBoneTrackKeys(Track.Name, MirroredPosKeys, MirroredRotKeys, MirroredScaleKeys, false);
		}
	}

	MirroredAnim->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("AnimationMirrorTool: Mirrored '%s' → '%s' (%d tracks)"),
		*SourceAnimation->GetName(), *NewName, Tracks.Num());

	return MirroredAnim;
}

UAnimSequence* UAnimationMirrorTool::MirrorAnimationAxis(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	EMirrorFlags MirrorFlags)
{
	if (!WorldContextObject || !SourceAnimation)
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	TArray<FMirrorBoneMapping> EmptyMappings;
	return MirrorAnimation(WorldContextObject, SourceAnimation, EmptyMappings,
		(MirrorFlags & EMirrorFlags::Y) != EMirrorFlags::None,
		(MirrorFlags & EMirrorFlags::Y) != EMirrorFlags::None);
}

TArray<FMirrorBoneMapping> UAnimationMirrorTool::GenerateMirrorMappings(
	USkeleton* Skeleton,
	const FString& LeftSuffix,
	const FString& RightSuffix)
{
	TArray<FMirrorBoneMapping> Mappings;

	if (!Skeleton)
	{
		return Mappings;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FName BoneName = RefSkeleton.GetBoneName(i);
		FString BoneStr = BoneName.ToString();

		if (BoneStr.EndsWith(LeftSuffix))
		{
			FString RightBoneStr = BoneStr.LeftChop(LeftSuffix.Len()) + RightSuffix;
			int32 RightIdx = RefSkeleton.FindBoneIndex(FName(*RightBoneStr));

			if (RightIdx != INDEX_NONE)
			{
				FMirrorBoneMapping Mapping;
				Mapping.OriginalBone = BoneName;
				Mapping.MirroredBone = FName(*RightBoneStr);
				Mappings.Add(Mapping);
			}
		}
	}

	return Mappings;
}

TArray<FMirrorBoneMapping> UAnimationMirrorTool::GenerateMirrorMappingsFromPrefix(
	USkeleton* Skeleton,
	const FString& LeftPrefix,
	const FString& RightPrefix)
{
	TArray<FMirrorBoneMapping> Mappings;

	if (!Skeleton)
	{
		return Mappings;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FName BoneName = RefSkeleton.GetBoneName(i);
		FString BoneStr = BoneName.ToString();

		if (BoneStr.StartsWith(LeftPrefix))
		{
			FString RightBoneStr = RightPrefix + BoneStr.Mid(LeftPrefix.Len());
			int32 RightIdx = RefSkeleton.FindBoneIndex(FName(*RightBoneStr));

			if (RightIdx != INDEX_NONE)
			{
				FMirrorBoneMapping Mapping;
				Mapping.OriginalBone = BoneName;
				Mapping.MirroredBone = FName(*RightBoneStr);
				Mappings.Add(Mapping);
			}
		}
	}

	return Mappings;
}

void UAnimationMirrorTool::BatchMirrorAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	EMirrorFlags MirrorFlags)
{
	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			MirrorAnimationAxis(WorldContextObject, Anim, MirrorFlags);
		}
	}
}

bool UAnimationMirrorTool::IsMirroredBone(
	const TArray<FMirrorBoneMapping>& Mappings,
	FName BoneName)
{
	for (const FMirrorBoneMapping& Mapping : Mappings)
	{
		if (Mapping.OriginalBone == BoneName || Mapping.MirroredBone == BoneName)
		{
			return true;
		}
	}

	return false;
}

FName UAnimationMirrorTool::GetMirroredBone(
	const TArray<FMirrorBoneMapping>& Mappings,
	FName BoneName)
{
	for (const FMirrorBoneMapping& Mapping : Mappings)
	{
		if (Mapping.OriginalBone == BoneName)
		{
			return Mapping.MirroredBone;
		}
		else if (Mapping.MirroredBone == BoneName)
		{
			return Mapping.OriginalBone;
		}
	}

	return NAME_None;
}

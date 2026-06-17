#include "AnimationMirrorTool.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

UAnimSequence* UAnimationMirrorTool::MirrorAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const TArray<FMirrorBoneMapping>& BoneMappings,
	bool bMirrorTranslation,
	bool bMirrorRotation)
{
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	FString NewName = FString::Printf(TEXT("%s_Mirrored"), *SourceAnimation->GetName());

	UAnimSequence* MirroredAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!MirroredAnim)
	{
		return nullptr;
	}

	MirroredAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	MirroredAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);

		FName SourceBoneName = BoneName;
		for (const FMirrorBoneMapping& Mapping : BoneMappings)
		{
			if (Mapping.OriginalBone == BoneName)
			{
				SourceBoneName = Mapping.MirroredBone;
				break;
			}
			else if (Mapping.MirroredBone == BoneName)
			{
				SourceBoneName = Mapping.OriginalBone;
				break;
			}
		}

		int32 SourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(SourceBoneName);

		FRawAnimSequenceTrack& DestTrack = MirroredAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform SourceTransform = (SourceBoneIdx != INDEX_NONE) ?
				SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame) : FTransform::Identity;

			FVector Pos = SourceTransform.GetLocation();
			FQuat Rot = SourceTransform.GetRotation();

			if (bMirrorTranslation)
			{
				Pos.Y = -Pos.Y;
			}

			if (bMirrorRotation)
			{
				FRotator Rotator = Rot.Rotator();
				Rotator.Yaw = -Rotator.Yaw;
				Rotator.Roll = -Rotator.Roll;
				Rot = Rotator.Quaternion();
			}

			DestTrack.PosKeys[Frame] = Pos;
			DestTrack.RotKeys[Frame] = Rot;
			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();
		}
	}

	MirroredAnim->MarkPackageDirty();
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
		(MirrorFlags & EMirrorFlags::MirrorFlag_Y) != EMirrorFlags::None,
		(MirrorFlags & EMirrorFlags::MirrorFlag_Y) != EMirrorFlags::None);
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

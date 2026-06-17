#include "AnimationMirrorTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

UAnimSequence* UAnimationMirrorTool::MirrorAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const TArray<FMirrorBoneMapping>& BoneMappings,
	bool bMirrorTranslation,
	bool bMirrorRotation)
{
	return nullptr;
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

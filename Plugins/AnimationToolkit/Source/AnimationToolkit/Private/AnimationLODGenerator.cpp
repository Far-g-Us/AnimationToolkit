#include "AnimationLODGenerator.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

FAnimationLODSettings UAnimationLODGenerator::GetDefaultLODSettings()
{
	FAnimationLODSettings Settings;
	return Settings;
}

TArray<UAnimSequence*> UAnimationLODGenerator::GenerateAnimationLODs(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const FAnimationLODSettings& Settings,
	int32 NumLODs)
{
	TArray<UAnimSequence*> LODs;

	if (!WorldContextObject || !SourceAnimation)
	{
		return LODs;
	}

	for (int32 LODLevel = 0; LODLevel < NumLODs; LODLevel++)
	{
		UAnimSequence* LODAnim = GenerateLOD(WorldContextObject, SourceAnimation, LODLevel, Settings);
		if (LODAnim)
		{
			LODs.Add(LODAnim);
		}
	}

	return LODs;
}

UAnimSequence* UAnimationLODGenerator::GenerateLOD(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	int32 LODLevel,
	const FAnimationLODSettings& Settings)
{
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	float TranslationThreshold = Settings.LOD0TranslationThreshold;
	float RotationThreshold = Settings.LOD0RotationThreshold;

	if (LODLevel == 1)
	{
		TranslationThreshold = Settings.LOD1TranslationThreshold;
		RotationThreshold = Settings.LOD1RotationThreshold;
	}
	else if (LODLevel >= 2)
	{
		TranslationThreshold = Settings.LOD2TranslationThreshold;
		RotationThreshold = Settings.LOD2RotationThreshold;
	}

	FString NewName = FString::Printf(TEXT("%s_LOD%d"), *SourceAnimation->GetName(), LODLevel);

	UAnimSequence* LODAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!LODAnim)
	{
		return nullptr;
	}

	LODAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	LODAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);
		int32 SourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);

		FRawAnimSequenceTrack& DestTrack = LODAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		if (SourceBoneIdx == INDEX_NONE)
		{
			continue;
		}

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform SourceTransform = SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame);

			FVector Pos = SourceTransform.GetLocation();
			FQuat Rot = SourceTransform.GetRotation();

			if (Frame > 0)
			{
				FVector PrevPos = DestTrack.PosKeys[Frame - 1];
				FQuat PrevRot = DestTrack.RotKeys[Frame - 1];

				if (FVector::Dist(Pos, PrevPos) < TranslationThreshold)
				{
					Pos = PrevPos;
				}

				if (Rot.AngularDistance(PrevRot) < RotationThreshold)
				{
					Rot = PrevRot;
				}
			}

			DestTrack.PosKeys[Frame] = Pos;
			DestTrack.RotKeys[Frame] = Rot;
			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();
		}
	}

	LODAnim->MarkPackageDirty();
	return LODAnim;
}

int32 UAnimationLODGenerator::GetBoneCountForLOD(
	UAnimSequence* Animation,
	int32 LODLevel,
	const FAnimationLODSettings& Settings)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		return 0;
	}

	int32 TotalBones = Animation->GetSkeleton()->GetReferenceSkeleton().GetNum();

	if (LODLevel == 0) return TotalBones;
	if (LODLevel == 1) return FMath::Min(TotalBones, Settings.LOD1BoneThreshold);
	if (LODLevel == 2) return FMath::Min(TotalBones, Settings.LOD2BoneThreshold);

	return FMath::Min(TotalBones, Settings.LOD2BoneThreshold);
}

float UAnimationLODGenerator::EstimateLODMemoryUsage(
	UAnimSequence* Animation,
	int32 LODLevel,
	const FAnimationLODSettings& Settings)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		return 0.0f;
	}

	int32 BoneCount = GetBoneCountForLOD(Animation, LODLevel, Settings);
	int32 FrameCount = Animation->GetNumberOfSampledKeys();

	return (float)(BoneCount * FrameCount * (sizeof(FVector) + sizeof(FQuat) + sizeof(FVector)));
}

void UAnimationLODGenerator::BatchGenerateLODs(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	const FAnimationLODSettings& Settings,
	int32 NumLODs)
{
	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			GenerateAnimationLODs(WorldContextObject, Anim, Settings, NumLODs);
		}
	}
}

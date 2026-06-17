#include "AnimationLODGenerator.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

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
	return nullptr;
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

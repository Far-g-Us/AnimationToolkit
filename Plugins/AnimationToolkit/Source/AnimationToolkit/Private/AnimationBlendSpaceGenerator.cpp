#include "AnimationBlendSpaceGenerator.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Engine/Skeleton.h"

UBlendSpace* UAnimationBlendSpaceGenerator::GenerateBlendSpace1D(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSample>& Samples,
	float MinX,
	float MaxX)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	return nullptr;
}

UBlendSpace* UAnimationBlendSpaceGenerator::GenerateBlendSpace2D(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSample>& Samples,
	float MinX,
	float MaxX,
	float MinY,
	float MaxY)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	return nullptr;
}

TArray<FBlendSpaceSample> UAnimationBlendSpaceGenerator::ArrangeSamplesInGrid(
	const TArray<UAnimSequence*>& Animations,
	int32 GridSizeX,
	int32 GridSizeY)
{
	TArray<FBlendSpaceSample> Samples;

	for (int32 i = 0; i < Animations.Num(); i++)
	{
		int32 GridX = i % GridSizeX;
		int32 GridY = i / GridSizeX;

		FBlendSpaceSample Sample;
		Sample.Animation = Animations[i];
		Sample.X = (GridSizeX > 1) ? (float)GridX / (GridSizeX - 1) : 0.0f;
		Sample.Y = (GridSizeY > 1) ? (float)GridY / (GridSizeY - 1) : 0.0f;
		Samples.Add(Sample);
	}

	return Samples;
}

TArray<FBlendSpaceSample> UAnimationBlendSpaceGenerator::ArrangeSamplesInLine(
	const TArray<UAnimSequence*>& Animations)
{
	TArray<FBlendSpaceSample> Samples;

	for (int32 i = 0; i < Animations.Num(); i++)
	{
		FBlendSpaceSample Sample;
		Sample.Animation = Animations[i];
		Sample.X = (Animations.Num() > 1) ? (float)i / (Animations.Num() - 1) : 0.0f;
		Sample.Y = 0.0f;
		Samples.Add(Sample);
	}

	return Samples;
}

void UAnimationBlendSpaceGenerator::BatchGenerateBlendSpaces(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	const TArray<TArray<FBlendSpaceSample>>& SampleSets)
{
}

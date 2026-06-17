#include "AnimationBlendSpaceGenerator.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/Skeleton.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"

UBlendSpace* UAnimationBlendSpaceGenerator::GenerateBlendSpace1D(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSample>& Samples,
	float MinX,
	float MaxX)
{
	if (!WorldContextObject || !Skeleton || Samples.Num() == 0)
	{
		return nullptr;
	}

	const FString BSName = TEXT("ATK_BlendSpace1D_") + FGuid::NewGuid().ToString().Left(8);
	const FString PackagePath = TEXT("/Game/Animations/") + BSName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UBlendSpace* BlendSpace = NewObject<UBlendSpace>(Package, *BSName, RF_Public | RF_Standalone);
	if (!BlendSpace)
	{
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

	for (const FBlendSpaceSample& Sample : Samples)
	{
		if (Sample.Animation)
		{
			BlendSpace->AddSample(Sample.Animation, FVector(Sample.X, 0.0f, 0.0f));
		}
	}

	BlendSpace->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(BlendSpace);

	return BlendSpace;
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
	if (!WorldContextObject || !Skeleton || Samples.Num() == 0)
	{
		return nullptr;
	}

	const FString BSName = TEXT("ATK_BlendSpace2D_") + FGuid::NewGuid().ToString().Left(8);
	const FString PackagePath = TEXT("/Game/Animations/") + BSName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UBlendSpace* BlendSpace = NewObject<UBlendSpace>(Package, *BSName, RF_Public | RF_Standalone);
	if (!BlendSpace)
	{
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

	for (const FBlendSpaceSample& Sample : Samples)
	{
		if (Sample.Animation)
		{
			BlendSpace->AddSample(Sample.Animation, FVector(Sample.X, Sample.Y, 0.0f));
		}
	}

	BlendSpace->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(BlendSpace);

	return BlendSpace;
}

TArray<FBlendSpaceSample> UAnimationBlendSpaceGenerator::ArrangeSamplesInGrid(
	const TArray<UAnimSequence*>& Animations,
	int32 GridSizeX,
	int32 GridSizeY)
{
	TArray<FBlendSpaceSample> Samples;

	const int32 MaxSamples = FMath::Min(Animations.Num(), GridSizeX * GridSizeY);
	for (int32 i = 0; i < MaxSamples; i++)
	{
		const int32 GridX = i % GridSizeX;
		const int32 GridY = i / GridSizeX;

		FBlendSpaceSample Sample;
		Sample.Animation = Animations[i];
		Sample.X = (GridSizeX > 1) ? static_cast<float>(GridX) / static_cast<float>(GridSizeX - 1) : 0.0f;
		Sample.Y = (GridSizeY > 1) ? static_cast<float>(GridY) / static_cast<float>(GridSizeY - 1) : 0.0f;
		Samples.Add(Sample);
	}

	return Samples;
}

TArray<FBlendSpaceSample> UAnimationBlendSpaceGenerator::ArrangeSamplesInLine(
	const TArray<UAnimSequence*>& Animations)
{
	TArray<FBlendSpaceSample> Samples;

	const int32 NumAnimations = Animations.Num();
	for (int32 i = 0; i < NumAnimations; i++)
	{
		FBlendSpaceSample Sample;
		Sample.Animation = Animations[i];
		Sample.X = (NumAnimations > 1) ? static_cast<float>(i) / static_cast<float>(NumAnimations - 1) : 0.0f;
		Sample.Y = 0.0f;
		Samples.Add(Sample);
	}

	return Samples;
}

void UAnimationBlendSpaceGenerator::BatchGenerateBlendSpaces(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSample>& AllSamples,
	int32 SamplesPerBlendSpace)
{
	if (!WorldContextObject || !Skeleton || SamplesPerBlendSpace <= 0 || AllSamples.Num() == 0)
	{
		return;
	}

	const int32 NumBlendSpaces = FMath::Max(1, FMath::CeilToInt(static_cast<float>(AllSamples.Num()) / static_cast<float>(SamplesPerBlendSpace)));

	for (int32 BSIdx = 0; BSIdx < NumBlendSpaces; ++BSIdx)
	{
		const int32 StartIdx = BSIdx * SamplesPerBlendSpace;
		const int32 EndIdx = FMath::Min(StartIdx + SamplesPerBlendSpace, AllSamples.Num());

		TArray<FBlendSpaceSample> BatchSamples;
		BatchSamples.Reserve(EndIdx - StartIdx);
		for (int32 i = StartIdx; i < EndIdx; ++i)
		{
			BatchSamples.Add(AllSamples[i]);
		}

		if (BatchSamples.Num() > 0)
		{
			float BatchMinX = MAX_FLT;
			float BatchMaxX = -MAX_FLT;
			float BatchMinY = MAX_FLT;
			float BatchMaxY = -MAX_FLT;

			for (const FBlendSpaceSample& S : BatchSamples)
			{
				BatchMinX = FMath::Min(BatchMinX, S.X);
				BatchMaxX = FMath::Max(BatchMaxX, S.X);
				BatchMinY = FMath::Min(BatchMinY, S.Y);
				BatchMaxY = FMath::Max(BatchMaxY, S.Y);
			}

			if (BatchMinX == BatchMaxX)
			{
				BatchMinX -= 1.0f;
				BatchMaxX += 1.0f;
			}
			if (BatchMinY == BatchMaxY)
			{
				BatchMinY -= 1.0f;
				BatchMaxY += 1.0f;
			}

			GenerateBlendSpace2D(WorldContextObject, Skeleton, BatchSamples, BatchMinX, BatchMaxX, BatchMinY, BatchMaxY);
		}
	}
}

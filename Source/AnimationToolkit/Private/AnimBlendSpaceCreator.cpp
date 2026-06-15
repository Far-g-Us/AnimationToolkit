#include "AnimBlendSpaceCreator.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBlendSpace* UAnimBlendSpaceCreator::CreateBlendSpace1D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinAxisValue,
	float MaxAxisValue,
	FName AxisName,
	UPackage* Package)
{
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimBlendSpaceCreator: Skeleton is null"));
		return nullptr;
	}

	if (Samples.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimBlendSpaceCreator: No samples provided"));
		return nullptr;
	}

	FString AssetPath = FString::Printf(TEXT("/Game/Animations/%s"), *BlendSpaceName);
	if (!Package)
	{
		Package = CreatePackage(nullptr, *AssetPath);
	}

	UBlendSpace* BlendSpace = NewObject<UBlendSpace>(
		Package,
		*BlendSpaceName,
		RF_Public | RF_Standalone | RF_MarkAsRootSet
	);

	if (!BlendSpace)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimBlendSpaceCreator: Failed to create BlendSpace"));
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

	FPerBlendSpace BlendSpaceData;
	BlendSpaceData.Animations.Reserve(Samples.Num());

	for (const FBlendSpaceSampleData& Sample : Samples)
	{
		if (Sample.Animation)
		{
			FAlphaBlend Blend;
			Blend.SetAlpha(1.0f);

			FPerBlendSpace::FBlendSample SampleData;
			SampleData.Animation = Sample.Animation;
			SampleData.SampleValue = FVector2D(Sample.SampleValue.X, Sample.SampleValue.Y);
			SampleData.Weight = 1.0f;

			BlendSpaceData.Animations.Add(SampleData);
		}
	}

	BlendSpace->SetTargetValueRange(FVector2D(MaxAxisValue - MinAxisValue, 0.0f));
	BlendSpace->SetInterpolationType(EBPNotifyReturnMode::Immediate);

	ConfigureBlendSpace(BlendSpace, Samples);

	BlendSpace->MarkPackageDirty();
	BlendSpace->PostEditChange();

	return BlendSpace;
}

UBlendSpace* UAnimBlendSpaceCreator::CreateBlendSpace2D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinAxisX,
	float MaxAxisX,
	float MinAxisY,
	float MaxAxisY,
	FName AxisXName,
	FName AxisYName,
	UPackage* Package)
{
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimBlendSpaceCreator: Skeleton is null"));
		return nullptr;
	}

	if (Samples.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimBlendSpaceCreator: No samples provided"));
		return nullptr;
	}

	FString AssetPath = FString::Printf(TEXT("/Game/Animations/%s"), *BlendSpaceName);
	if (!Package)
	{
		Package = CreatePackage(nullptr, *AssetPath);
	}

	UBlendSpace* BlendSpace = NewObject<UBlendSpace>(
		Package,
		*BlendSpaceName,
		RF_Public | RF_Standalone | RF_MarkAsRootSet
	);

	if (!BlendSpace)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimBlendSpaceCreator: Failed to create BlendSpace"));
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

	FPerBlendSpace BlendSpaceData;
	BlendSpaceData.Animations.Reserve(Samples.Num());

	for (const FBlendSpaceSampleData& Sample : Samples)
	{
		if (Sample.Animation)
		{
			FPerBlendSpace::FBlendSample SampleData;
			SampleData.Animation = Sample.Animation;
			SampleData.SampleValue = FVector2D(Sample.SampleValue.X, Sample.SampleValue.Y);
			SampleData.Weight = 1.0f;

			BlendSpaceData.Animations.Add(SampleData);
		}
	}

	BlendSpace->SetTargetValueRange(FVector2D(
		MaxAxisX - MinAxisX,
		MaxAxisY - MinAxisY
	));
	BlendSpace->SetInterpolationType(EBPNotifyReturnMode::Immediate);

	ConfigureBlendSpace(BlendSpace, Samples);

	BlendSpace->MarkPackageDirty();
	BlendSpace->PostEditChange();

	return BlendSpace;
}

FBlendSpaceSampleData UAnimBlendSpaceCreator::CreateBlendSpaceSample(
	UAnimSequence* Animation,
	const FVector& SampleValue)
{
	FBlendSpaceSampleData SampleData;
	SampleData.Animation = Animation;
	SampleData.SampleValue = SampleValue;
	return SampleData;
}

bool UAnimBlendSpaceCreator::AddSampleToBlendSpace(
	UBlendSpace* BlendSpace,
	const FBlendSpaceSampleData& SampleData)
{
	if (!BlendSpace || !SampleData.Animation)
	{
		return false;
	}

	FPerBlendSpace BlendSpaceData = BlendSpace->GetBlendSamples();

	FPerBlendSpace::FBlendSample NewSample;
	NewSample.Animation = SampleData.Animation;
	NewSample.SampleValue = FVector2D(SampleData.SampleValue.X, SampleData.SampleValue.Y);
	NewSample.Weight = 1.0f;

	BlendSpaceData.Animations.Add(NewSample);
	BlendSpace->SetBlendSamples(BlendSpaceData);

	BlendSpace->MarkPackageDirty();
	return true;
}

bool UAnimBlendSpaceCreator::SetBlendSpaceInterpolation(
	UBlendSpace* BlendSpace,
	int32 InterpolationType)
{
	if (!BlendSpace)
	{
		return false;
	}

	BlendSpace->SetInterpolationType(static_cast<EBPNotifyReturnMode>(InterpolationType));
	BlendSpace->MarkPackageDirty();
	return true;
}

bool UAnimBlendSpaceCreator::SetBlendSpaceTargetWeight(
	UBlendSpace* BlendSpace,
	float TargetWeight)
{
	if (!BlendSpace)
	{
		return false;
	}

	BlendSpace->SetTargetWeight(TargetWeight);
	BlendSpace->MarkPackageDirty();
	return true;
}

void UAnimBlendSpaceCreator::ConfigureBlendSpace(
	UBlendSpace* BlendSpace,
	const TArray<FBlendSpaceSampleData>& Samples)
{
	if (!BlendSpace)
	{
		return;
	}

	FPerBlendSpace BlendSpaceData;
	BlendSpaceData.Animations.Reserve(Samples.Num());

	for (const FBlendSpaceSampleData& Sample : Samples)
	{
		if (Sample.Animation)
		{
			FPerBlendSpace::FBlendSample SampleData;
			SampleData.Animation = Sample.Animation;
			SampleData.SampleValue = FVector2D(Sample.SampleValue.X, Sample.SampleValue.Y);
			SampleData.Weight = 1.0f;

			BlendSpaceData.Animations.Add(SampleData);
		}
	}

	BlendSpace->SetBlendSamples(BlendSpaceData);
}

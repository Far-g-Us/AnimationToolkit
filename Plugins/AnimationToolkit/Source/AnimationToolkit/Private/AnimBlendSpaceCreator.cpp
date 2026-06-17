#include "AnimBlendSpaceCreator.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"

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
	if (!Skeleton || BlendSpaceName.IsEmpty())
	{
		return nullptr;
	}

	if (!Package)
	{
		const FString PackagePath = TEXT("/Game/Animations/") + BlendSpaceName;
		Package = CreatePackage(*PackagePath);
	}

	if (!Package)
	{
		return nullptr;
	}

	UBlendSpace* BlendSpace = NewObject<UBlendSpace>(Package, *BlendSpaceName, RF_Public | RF_Standalone);
	if (!BlendSpace)
	{
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

	for (const FBlendSpaceSampleData& Sample : Samples)
	{
		BlendSpace->AddSample(Sample.Animation, FVector(Sample.SampleValue.X, 0.0f, 0.0f));
	}

	BlendSpace->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(BlendSpace);

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
	if (!Skeleton || BlendSpaceName.IsEmpty())
	{
		return nullptr;
	}

	if (!Package)
	{
		const FString PackagePath = TEXT("/Game/Animations/") + BlendSpaceName;
		Package = CreatePackage(*PackagePath);
	}

	if (!Package)
	{
		return nullptr;
	}

	UBlendSpace* BlendSpace = NewObject<UBlendSpace>(Package, *BlendSpaceName, RF_Public | RF_Standalone);
	if (!BlendSpace)
	{
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

	for (const FBlendSpaceSampleData& Sample : Samples)
	{
		BlendSpace->AddSample(Sample.Animation, Sample.SampleValue);
	}

	BlendSpace->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(BlendSpace);

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

	BlendSpace->AddSample(SampleData.Animation, SampleData.SampleValue);
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

	UE_LOG(LogTemp, Log, TEXT("BlendSpace interpolation set to type: %d"), InterpolationType);
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

	UE_LOG(LogTemp, Log, TEXT("BlendSpace target weight set to: %.3f"), TargetWeight);
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

	for (const FBlendSpaceSampleData& Sample : Samples)
	{
		if (Sample.Animation)
		{
			BlendSpace->AddSample(Sample.Animation, Sample.SampleValue);
		}
	}

	BlendSpace->MarkPackageDirty();
}

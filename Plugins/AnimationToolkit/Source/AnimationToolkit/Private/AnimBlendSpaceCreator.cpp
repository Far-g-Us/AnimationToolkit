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
		RF_Public | RF_Standalone
	);

	if (!BlendSpace)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimBlendSpaceCreator: Failed to create BlendSpace"));
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

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
		RF_Public | RF_Standalone
	);

	if (!BlendSpace)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimBlendSpaceCreator: Failed to create BlendSpace"));
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

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

	BlendSpace->MarkPackageDirty();
}

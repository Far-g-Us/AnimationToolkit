#include "AnimBlendSpaceCreator.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
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
	return nullptr;
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
	return nullptr;
}

FBlendSpaceSampleData UAnimBlendSpaceCreator::CreateBlendSpaceSample(
	UAnimSequence* Animation,
	const FVector& SampleValue)
{
	FBlendSpaceSampleData SampleData;
	return SampleData;
}

bool UAnimBlendSpaceCreator::AddSampleToBlendSpace(
	UBlendSpace* BlendSpace,
	const FBlendSpaceSampleData& SampleData)
{
	return false;
}

bool UAnimBlendSpaceCreator::SetBlendSpaceInterpolation(
	UBlendSpace* BlendSpace,
	int32 InterpolationType)
{
	return false;
}

bool UAnimBlendSpaceCreator::SetBlendSpaceTargetWeight(
	UBlendSpace* BlendSpace,
	float TargetWeight)
{
	return false;
}

void UAnimBlendSpaceCreator::ConfigureBlendSpace(
	UBlendSpace* BlendSpace,
	const TArray<FBlendSpaceSampleData>& Samples)
{
}

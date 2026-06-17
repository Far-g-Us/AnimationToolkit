#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/BlendSpace.h"
#include "AnimationData.h"
#include "AnimBlendSpaceCreator.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimBlendSpaceCreator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BlendSpace", meta = (WorldContext = "WorldContextObject"))
	static UBlendSpace* CreateBlendSpace1D(
		UObject* WorldContextObject,
		const FString& BlendSpaceName,
		USkeleton* Skeleton,
		const TArray<FBlendSpaceSampleData>& Samples,
		float MinAxisValue = 0.0f,
		float MaxAxisValue = 100.0f,
		FName AxisName = FName("Speed"),
		UPackage* Package = nullptr
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BlendSpace", meta = (WorldContext = "WorldContextObject"))
	static UBlendSpace* CreateBlendSpace2D(
		UObject* WorldContextObject,
		const FString& BlendSpaceName,
		USkeleton* Skeleton,
		const TArray<FBlendSpaceSampleData>& Samples,
		float MinAxisX = 0.0f,
		float MaxAxisX = 100.0f,
		float MinAxisY = 0.0f,
		float MaxAxisY = 100.0f,
		FName AxisXName = FName("Speed"),
		FName AxisYName = FName("Direction"),
		UPackage* Package = nullptr
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|BlendSpace")
	static FBlendSpaceSampleData CreateBlendSpaceSample(
		UAnimSequence* Animation,
		const FVector& SampleValue
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BlendSpace")
	static bool AddSampleToBlendSpace(
		UBlendSpace* BlendSpace,
		const FBlendSpaceSampleData& SampleData
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BlendSpace")
	static bool SetBlendSpaceInterpolation(
		UBlendSpace* BlendSpace,
		int32 InterpolationType = 0
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BlendSpace")
	static bool SetBlendSpaceTargetWeight(
		UBlendSpace* BlendSpace,
		float TargetWeight = 0.5f
	);

private:
	static void ConfigureBlendSpace(
		UBlendSpace* BlendSpace,
		const TArray<FBlendSpaceSampleData>& Samples
	);
};

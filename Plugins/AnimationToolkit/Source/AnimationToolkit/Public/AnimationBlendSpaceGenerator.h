#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationBlendSpaceGenerator.generated.h"

USTRUCT(BlueprintType)
struct FBlendSpaceSample
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float X = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Y = 0.0f;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationBlendSpaceGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BlendSpace", meta = (WorldContext = "WorldContextObject"))
	static UBlendSpace* GenerateBlendSpace1D(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		const TArray<FBlendSpaceSample>& Samples,
		float MinX = -1.0f,
		float MaxX = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BlendSpace", meta = (WorldContext = "WorldContextObject"))
	static UBlendSpace* GenerateBlendSpace2D(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		const TArray<FBlendSpaceSample>& Samples,
		float MinX = -1.0f,
		float MaxX = 1.0f,
		float MinY = -1.0f,
		float MaxY = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|BlendSpace")
	static TArray<FBlendSpaceSample> ArrangeSamplesInGrid(
		const TArray<UAnimSequence*>& Animations,
		int32 GridSizeX,
		int32 GridSizeY);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|BlendSpace")
	static TArray<FBlendSpaceSample> ArrangeSamplesInLine(
		const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BlendSpace", meta = (WorldContext = "WorldContextObject"))
	static void BatchGenerateBlendSpaces(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		const TArray<TArray<FBlendSpaceSample>>& SampleSets);
};

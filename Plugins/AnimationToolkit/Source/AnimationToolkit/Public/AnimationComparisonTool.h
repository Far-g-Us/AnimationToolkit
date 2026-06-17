#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationComparisonTool.generated.h"

USTRUCT(BlueprintType)
struct FAnimationComparisonResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float SimilarityScore = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float PositionDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float RotationDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float ScaleDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsIdentical = false;

	UPROPERTY(BlueprintReadOnly)
	TArray<FName> DifferentBones;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationComparisonTool : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Comparison")
	static FAnimationComparisonResult CompareAnimations(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Comparison")
	static float CalculateSimilarity(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Comparison")
	static bool AreAnimationsIdentical(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB,
		float Tolerance = 0.001f);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Comparison")
	static TArray<FName> GetDifferentBones(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB,
		float Threshold = 0.01f);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Comparison")
	static FString GenerateComparisonReport(
		const FAnimationComparisonResult& Result);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Comparison")
	static TArray<int32> FindKeyframesWithLargestDifference(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB,
		FName BoneName,
		int32 TopN = 5);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Comparison")
	static float GetMaxBoneDifference(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB,
		FName& OutBoneName);
};

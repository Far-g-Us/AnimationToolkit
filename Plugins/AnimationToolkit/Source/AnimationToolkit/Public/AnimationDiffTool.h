#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationDiffTool.generated.h"

USTRUCT(BlueprintType)
struct FAnimationBoneDiff
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName BoneName;

	UPROPERTY(BlueprintReadOnly)
	float PositionDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float RotationDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float ScaleDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxPositionDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxRotationDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 FrameCount = 0;
};

USTRUCT(BlueprintType)
struct FAnimationDiffResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float TotalPositionDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float TotalRotationDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float TotalScaleDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxPositionDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxRotationDifference = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float SimilarityScore = 1.0f;

	UPROPERTY(BlueprintReadOnly)
	TArray<FAnimationBoneDiff> BoneDiffs;

	UPROPERTY(BlueprintReadOnly)
	bool bIsIdentical = false;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationDiffTool : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Diff")
	static FAnimationDiffResult CompareAnimations(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB,
		float PositionWeight = 1.0f,
		float RotationWeight = 1.0f,
		float ScaleWeight = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Diff")
	static TArray<FAnimationBoneDiff> CompareBoneTracks(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB,
		FName BoneName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Diff")
	static float CalculateSimilarity(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Diff")
	static bool AreAnimationsIdentical(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB,
		float Tolerance = 0.001f);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Diff")
	static FAnimationBoneDiff GetLargestDifference(
		const FAnimationDiffResult& DiffResult);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Diff")
	static TArray<FName> GetDifferentBones(
		const FAnimationDiffResult& DiffResult,
		float Threshold = 0.01f);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Diff")
	static FString GenerateDiffReport(const FAnimationDiffResult& DiffResult);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Diff")
	static TArray<int32> FindKeyframesWithLargestDifference(
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB,
		FName BoneName,
		int32 TopN = 5);
};

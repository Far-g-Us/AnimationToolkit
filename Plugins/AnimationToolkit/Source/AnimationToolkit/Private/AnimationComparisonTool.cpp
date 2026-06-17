#include "AnimationComparisonTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

FAnimationComparisonResult UAnimationComparisonTool::CompareAnimations(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB)
{
	FAnimationComparisonResult Result;
	return Result;
}

float UAnimationComparisonTool::CalculateSimilarity(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB)
{
	FAnimationComparisonResult Result = CompareAnimations(AnimationA, AnimationB);
	return Result.SimilarityScore;
}

bool UAnimationComparisonTool::AreAnimationsIdentical(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	float Tolerance)
{
	FAnimationComparisonResult Result = CompareAnimations(AnimationA, AnimationB);
	return Result.SimilarityScore >= (1.0f - Tolerance);
}

TArray<FName> UAnimationComparisonTool::GetDifferentBones(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	float Threshold)
{
	FAnimationComparisonResult Result = CompareAnimations(AnimationA, AnimationB);
	return Result.DifferentBones;
}

FString UAnimationComparisonTool::GenerateComparisonReport(
	const FAnimationComparisonResult& Result)
{
	FString Report;
	Report += FString::Printf(TEXT("Animation Comparison Report\n"));
	Report += FString::Printf(TEXT("==========================\n"));
	Report += FString::Printf(TEXT("Similarity Score: %.4f\n"), Result.SimilarityScore);
	Report += FString::Printf(TEXT("Is Identical: %s\n"), Result.bIsIdentical ? TEXT("Yes") : TEXT("No"));
	Report += FString::Printf(TEXT("Position Difference: %.4f\n"), Result.PositionDifference);
	Report += FString::Printf(TEXT("Rotation Difference: %.4f\n"), Result.RotationDifference);
	Report += FString::Printf(TEXT("Different Bones: %d\n"), Result.DifferentBones.Num());

	return Report;
}

TArray<int32> UAnimationComparisonTool::FindKeyframesWithLargestDifference(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	FName BoneName,
	int32 TopN)
{
	TArray<int32> Result;
	return Result;
}

float UAnimationComparisonTool::GetMaxBoneDifference(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	FName& OutBoneName)
{
	return 0.0f;
}

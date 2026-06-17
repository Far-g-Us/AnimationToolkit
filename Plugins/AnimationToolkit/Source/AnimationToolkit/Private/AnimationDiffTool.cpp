#include "AnimationDiffTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

FAnimationDiffResult UAnimationDiffTool::CompareAnimations(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	float PositionWeight,
	float RotationWeight,
	float ScaleWeight)
{
	FAnimationDiffResult Result;
	return Result;
}

TArray<FAnimationBoneDiff> UAnimationDiffTool::CompareBoneTracks(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	FName BoneName)
{
	TArray<FAnimationBoneDiff> Result;
	return Result;
}

float UAnimationDiffTool::CalculateSimilarity(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB)
{
	FAnimationDiffResult Result = CompareAnimations(AnimationA, AnimationB);
	return Result.SimilarityScore;
}

bool UAnimationDiffTool::AreAnimationsIdentical(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	float Tolerance)
{
	if (!AnimationA || !AnimationB)
	{
		return false;
	}

	FAnimationDiffResult Result = CompareAnimations(AnimationA, AnimationB);
	return Result.SimilarityScore >= (1.0f - Tolerance);
}

FAnimationBoneDiff UAnimationDiffTool::GetLargestDifference(
	const FAnimationDiffResult& DiffResult)
{
	FAnimationBoneDiff LargestDiff;
	float MaxDiff = 0.0f;

	for (const FAnimationBoneDiff& BoneDiff : DiffResult.BoneDiffs)
	{
		float Diff = BoneDiff.PositionDifference + BoneDiff.RotationDifference + BoneDiff.ScaleDifference;
		if (Diff > MaxDiff)
		{
			MaxDiff = Diff;
			LargestDiff = BoneDiff;
		}
	}

	return LargestDiff;
}

TArray<FName> UAnimationDiffTool::GetDifferentBones(
	const FAnimationDiffResult& DiffResult,
	float Threshold)
{
	TArray<FName> DifferentBones;

	for (const FAnimationBoneDiff& BoneDiff : DiffResult.BoneDiffs)
	{
		float TotalDiff = BoneDiff.PositionDifference + BoneDiff.RotationDifference + BoneDiff.ScaleDifference;
		if (TotalDiff > Threshold)
		{
			DifferentBones.Add(BoneDiff.BoneName);
		}
	}

	return DifferentBones;
}

FString UAnimationDiffTool::GenerateDiffReport(const FAnimationDiffResult& DiffResult)
{
	FString Report;

	Report += FString::Printf(TEXT("Animation Diff Report\n"));
	Report += FString::Printf(TEXT("====================\n"));
	Report += FString::Printf(TEXT("Similarity Score: %.4f\n"), DiffResult.SimilarityScore);
	Report += FString::Printf(TEXT("Is Identical: %s\n"), DiffResult.bIsIdentical ? TEXT("Yes") : TEXT("No"));
	Report += FString::Printf(TEXT("Avg Position Diff: %.4f\n"), DiffResult.TotalPositionDifference);
	Report += FString::Printf(TEXT("Avg Rotation Diff: %.4f\n"), DiffResult.TotalRotationDifference);
	Report += FString::Printf(TEXT("Avg Scale Diff: %.4f\n"), DiffResult.TotalScaleDifference);
	Report += FString::Printf(TEXT("Max Position Diff: %.4f\n"), DiffResult.MaxPositionDifference);
	Report += FString::Printf(TEXT("Max Rotation Diff: %.4f\n"), DiffResult.MaxRotationDifference);
	Report += FString::Printf(TEXT("\nBone Details:\n"));

	for (const FAnimationBoneDiff& BoneDiff : DiffResult.BoneDiffs)
	{
		Report += FString::Printf(TEXT("  %s: Pos=%.4f, Rot=%.4f, Scale=%.4f\n"),
			*BoneDiff.BoneName.ToString(),
			BoneDiff.PositionDifference,
			BoneDiff.RotationDifference,
			BoneDiff.ScaleDifference);
	}

	return Report;
}

TArray<int32> UAnimationDiffTool::FindKeyframesWithLargestDifference(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	FName BoneName,
	int32 TopN)
{
	TArray<int32> Result;
	return Result;
}

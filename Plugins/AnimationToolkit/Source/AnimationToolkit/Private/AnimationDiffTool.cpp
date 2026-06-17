#include "AnimationDiffTool.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

FAnimationDiffResult UAnimationDiffTool::CompareAnimations(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	float PositionWeight,
	float RotationWeight,
	float ScaleWeight)
{
	FAnimationDiffResult Result;

	if (!AnimationA || !AnimationB)
	{
		return Result;
	}

	if (AnimationA->GetSkeleton() != AnimationB->GetSkeleton())
	{
		return Result;
	}

	USkeleton* Skeleton = AnimationA->GetSkeleton();
	if (!Skeleton)
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 FrameCount = FMath::Min(
		AnimationA->GetNumberOfSampledKeys(),
		AnimationB->GetNumberOfSampledKeys()
	);

	if (FrameCount == 0)
	{
		return Result;
	}

	float TotalPositionDiff = 0.0f;
	float TotalRotationDiff = 0.0f;
	float TotalScaleDiff = 0.0f;
	float MaxPositionDiff = 0.0f;
	float MaxRotationDiff = 0.0f;

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);
		FAnimationBoneDiff BoneDiff;
		BoneDiff.BoneName = BoneName;
		BoneDiff.FrameCount = FrameCount;

		float BoneMaxPosDiff = 0.0f;
		float BoneMaxRotDiff = 0.0f;
		float BonePosSum = 0.0f;
		float BoneRotSum = 0.0f;
		float BoneScaleSum = 0.0f;

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform TransformA = AnimationA->GetBoneTransform(BoneIdx, Frame);
			FTransform TransformB = AnimationB->GetBoneTransform(BoneIdx, Frame);

			FVector PosA = TransformA.GetLocation();
			FVector PosB = TransformB.GetLocation();
			float PosDiff = FVector::Dist(PosA, PosB) * PositionWeight;
			BonePosSum += PosDiff;
			BoneMaxPosDiff = FMath::Max(BoneMaxPosDiff, PosDiff);

			FQuat RotA = TransformA.GetRotation();
			FQuat RotB = TransformB.GetRotation();
			float RotDiff = RotA.AngularDistance(RotB) * RotationWeight;
			BoneRotSum += RotDiff;
			BoneMaxRotDiff = FMath::Max(BoneMaxRotDiff, RotDiff);

			FVector ScaleA = TransformA.GetScale3D();
			FVector ScaleB = TransformB.GetScale3D();
			float ScaleDiff = FVector::Dist(ScaleA, ScaleB) * ScaleWeight;
			BoneScaleSum += ScaleDiff;
		}

		BoneDiff.PositionDifference = (FrameCount > 0) ? BonePosSum / FrameCount : 0.0f;
		BoneDiff.RotationDifference = (FrameCount > 0) ? BoneRotSum / FrameCount : 0.0f;
		BoneDiff.ScaleDifference = (FrameCount > 0) ? BoneScaleSum / FrameCount : 0.0f;
		BoneDiff.MaxPositionDifference = BoneMaxPosDiff;
		BoneDiff.MaxRotationDifference = BoneMaxRotDiff;

		Result.BoneDiffs.Add(BoneDiff);

		TotalPositionDiff += BoneDiff.PositionDifference;
		TotalRotationDiff += BoneDiff.RotationDifference;
		TotalScaleDiff += BoneDiff.ScaleDifference;
		MaxPositionDiff = FMath::Max(MaxPositionDiff, BoneMaxPosDiff);
		MaxRotationDiff = FMath::Max(MaxRotationDiff, BoneMaxRotDiff);
	}

	int32 BoneCount = RefSkeleton.GetNum();
	Result.TotalPositionDifference = (BoneCount > 0) ? TotalPositionDiff / BoneCount : 0.0f;
	Result.TotalRotationDifference = (BoneCount > 0) ? TotalRotationDiff / BoneCount : 0.0f;
	Result.TotalScaleDifference = (BoneCount > 0) ? TotalScaleDiff / BoneCount : 0.0f;
	Result.MaxPositionDifference = MaxPositionDiff;
	Result.MaxRotationDifference = MaxRotationDiff;

	float MaxDiff = FMath::Max(Result.MaxPositionDifference, Result.MaxRotationDifference);
	Result.SimilarityScore = FMath::Clamp(1.0f - MaxDiff, 0.0f, 1.0f);
	Result.bIsIdentical = (Result.SimilarityScore >= 0.999f);

	return Result;
}

TArray<FAnimationBoneDiff> UAnimationDiffTool::CompareBoneTracks(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	FName BoneName)
{
	TArray<FAnimationBoneDiff> Result;

	if (!AnimationA || !AnimationB)
	{
		return Result;
	}

	USkeleton* SkeletonA = AnimationA->GetSkeleton();
	USkeleton* SkeletonB = AnimationB->GetSkeleton();

	if (!SkeletonA || !SkeletonB)
	{
		return Result;
	}

	int32 BoneIdxA = SkeletonA->GetReferenceSkeleton().FindBoneIndex(BoneName);
	int32 BoneIdxB = SkeletonB->GetReferenceSkeleton().FindBoneIndex(BoneName);

	if (BoneIdxA == INDEX_NONE || BoneIdxB == INDEX_NONE)
	{
		return Result;
	}

	int32 FrameCount = FMath::Min(
		AnimationA->GetNumberOfSampledKeys(),
		AnimationB->GetNumberOfSampledKeys()
	);

	float TotalPosDiff = 0.0f;
	float TotalRotDiff = 0.0f;
	float TotalScaleDiff = 0.0f;
	float MaxPosDiff = 0.0f;
	float MaxRotDiff = 0.0f;

	for (int32 Frame = 0; Frame < FrameCount; Frame++)
	{
		FTransform TransformA = AnimationA->GetBoneTransform(BoneIdxA, Frame);
		FTransform TransformB = AnimationB->GetBoneTransform(BoneIdxB, Frame);

		float PosDiff = FVector::Dist(TransformA.GetLocation(), TransformB.GetLocation());
		float RotDiff = TransformA.GetRotation().AngularDistance(TransformB.GetRotation());
		float ScaleDiff = FVector::Dist(TransformA.GetScale3D(), TransformB.GetScale3D());

		TotalPosDiff += PosDiff;
		TotalRotDiff += RotDiff;
		TotalScaleDiff += ScaleDiff;

		MaxPosDiff = FMath::Max(MaxPosDiff, PosDiff);
		MaxRotDiff = FMath::Max(MaxRotDiff, RotDiff);
	}

	FAnimationBoneDiff BoneDiff;
	BoneDiff.BoneName = BoneName;
	BoneDiff.PositionDifference = (FrameCount > 0) ? TotalPosDiff / FrameCount : 0.0f;
	BoneDiff.RotationDifference = (FrameCount > 0) ? TotalRotDiff / FrameCount : 0.0f;
	BoneDiff.ScaleDifference = (FrameCount > 0) ? TotalScaleDiff / FrameCount : 0.0f;
	BoneDiff.MaxPositionDifference = MaxPosDiff;
	BoneDiff.MaxRotationDifference = MaxRotDiff;
	BoneDiff.FrameCount = FrameCount;

	Result.Add(BoneDiff);

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

	if (!AnimationA || !AnimationB)
	{
		return Result;
	}

	USkeleton* SkeletonA = AnimationA->GetSkeleton();
	USkeleton* SkeletonB = AnimationB->GetSkeleton();

	if (!SkeletonA || !SkeletonB)
	{
		return Result;
	}

	int32 BoneIdxA = SkeletonA->GetReferenceSkeleton().FindBoneIndex(BoneName);
	int32 BoneIdxB = SkeletonB->GetReferenceSkeleton().FindBoneIndex(BoneName);

	if (BoneIdxA == INDEX_NONE || BoneIdxB == INDEX_NONE)
	{
		return Result;
	}

	int32 FrameCount = FMath::Min(
		AnimationA->GetNumberOfSampledKeys(),
		AnimationB->GetNumberOfSampledKeys()
	);

	TArray<TPair<int32, float>> FrameDifferences;

	for (int32 Frame = 0; Frame < FrameCount; Frame++)
	{
		FTransform TransformA = AnimationA->GetBoneTransform(BoneIdxA, Frame);
		FTransform TransformB = AnimationB->GetBoneTransform(BoneIdxB, Frame);

		float PosDiff = FVector::Dist(TransformA.GetLocation(), TransformB.GetLocation());
		float RotDiff = TransformA.GetRotation().AngularDistance(TransformB.GetRotation());

		float TotalDiff = PosDiff + RotDiff;
		FrameDifferences.Add(TPair<int32, float>(Frame, TotalDiff));
	}

	FrameDifferences.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value > B.Value;
	});

	for (int32 i = 0; i < FMath::Min(TopN, FrameDifferences.Num()); i++)
	{
		Result.Add(FrameDifferences[i].Key);
	}

	return Result;
}

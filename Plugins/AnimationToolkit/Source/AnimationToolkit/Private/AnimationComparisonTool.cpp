#include "AnimationComparisonTool.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

FAnimationComparisonResult UAnimationComparisonTool::CompareAnimations(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB)
{
	FAnimationComparisonResult Result;

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

	float TotalPosDiff = 0.0f;
	float TotalRotDiff = 0.0f;
	float TotalScaleDiff = 0.0f;

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		float BonePosDiff = 0.0f;
		float BoneRotDiff = 0.0f;

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform TransformA = AnimationA->GetBoneTransform(BoneIdx, Frame);
			FTransform TransformB = AnimationB->GetBoneTransform(BoneIdx, Frame);

			BonePosDiff += FVector::Dist(TransformA.GetLocation(), TransformB.GetLocation());
			BoneRotDiff += TransformA.GetRotation().AngularDistance(TransformB.GetRotation());
		}

		if (FrameCount > 0)
		{
			BonePosDiff /= FrameCount;
			BoneRotDiff /= FrameCount;
		}

		TotalPosDiff += BonePosDiff;
		TotalRotDiff += BoneRotDiff;

		if (BonePosDiff > 0.01f || BoneRotDiff > 0.01f)
		{
			Result.DifferentBones.Add(RefSkeleton.GetBoneName(BoneIdx));
		}
	}

	int32 BoneCount = RefSkeleton.GetNum();
	Result.PositionDifference = (BoneCount > 0) ? TotalPosDiff / BoneCount : 0.0f;
	Result.RotationDifference = (BoneCount > 0) ? TotalRotDiff / BoneCount : 0.0f;
	Result.SimilarityScore = FMath::Clamp(1.0f - FMath::Max(Result.PositionDifference, Result.RotationDifference), 0.0f, 1.0f);
	Result.bIsIdentical = (Result.SimilarityScore >= 0.999f);

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

	TArray<TPair<int32, float>> FrameDiffs;

	for (int32 Frame = 0; Frame < FrameCount; Frame++)
	{
		FTransform TransformA = AnimationA->GetBoneTransform(BoneIdxA, Frame);
		FTransform TransformB = AnimationB->GetBoneTransform(BoneIdxB, Frame);

		float Diff = FVector::Dist(TransformA.GetLocation(), TransformB.GetLocation());
		FrameDiffs.Add(TPair<int32, float>(Frame, Diff));
	}

	FrameDiffs.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value > B.Value;
	});

	for (int32 i = 0; i < FMath::Min(TopN, FrameDiffs.Num()); i++)
	{
		Result.Add(FrameDiffs[i].Key);
	}

	return Result;
}

float UAnimationComparisonTool::GetMaxBoneDifference(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	FName& OutBoneName)
{
	if (!AnimationA || !AnimationB)
	{
		return 0.0f;
	}

	USkeleton* Skeleton = AnimationA->GetSkeleton();
	if (!Skeleton)
	{
		return 0.0f;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 FrameCount = FMath::Min(
		AnimationA->GetNumberOfSampledKeys(),
		AnimationB->GetNumberOfSampledKeys()
	);

	float MaxDiff = 0.0f;

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		float BoneDiff = 0.0f;

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform TransformA = AnimationA->GetBoneTransform(BoneIdx, Frame);
			FTransform TransformB = AnimationB->GetBoneTransform(BoneIdx, Frame);

			BoneDiff += FVector::Dist(TransformA.GetLocation(), TransformB.GetLocation());
		}

		if (FrameCount > 0)
		{
			BoneDiff /= FrameCount;
		}

		if (BoneDiff > MaxDiff)
		{
			MaxDiff = BoneDiff;
			OutBoneName = RefSkeleton.GetBoneName(BoneIdx);
		}
	}

	return MaxDiff;
}

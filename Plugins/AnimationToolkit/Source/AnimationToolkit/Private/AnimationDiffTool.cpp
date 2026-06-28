#include "AnimationDiffTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataModel.h"

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

	USkeleton* SkeletonA = AnimationA->GetSkeleton();
	USkeleton* SkeletonB = AnimationB->GetSkeleton();
	if (!SkeletonA || !SkeletonB)
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkelA = SkeletonA->GetReferenceSkeleton();
	const FReferenceSkeleton& RefSkelB = SkeletonB->GetReferenceSkeleton();

	const int32 NumFramesA = AnimationA->GetNumberOfSampledKeys();
	const int32 NumFramesB = AnimationB->GetNumberOfSampledKeys();
	const int32 NumFrames = FMath::Max(NumFramesA, NumFramesB);

	const int32 NumBonesA = RefSkelA.GetNum();
	const int32 NumBonesB = RefSkelB.GetNum();
	const int32 NumBones = FMath::Min(NumBonesA, NumBonesB);

	const IAnimationDataModel* ModelA = AnimationA->GetDataModel();
	const IAnimationDataModel* ModelB = AnimationB->GetDataModel();
	if (!ModelA || !ModelB)
	{
		return Result;
	}

	const TArray<FBoneAnimationTrack>& TracksA = ModelA->GetBoneAnimationTracks();
	const TArray<FBoneAnimationTrack>& TracksB = ModelB->GetBoneAnimationTracks();

	float TotalPosDiff = 0.0f;
	float TotalRotDiff = 0.0f;
	float TotalScaleDiff = 0.0f;
	float MaxPosDiff = 0.0f;
	float MaxRotDiff = 0.0f;
	int32 TotalSamples = 0;
	float TotalWeightedDiff = 0.0f;
	float MaxPossibleDiff = 0.0f;

	TMap<FName, int32> BoneNameToIndexA;
	TMap<FName, int32> BoneNameToIndexB;
	for (int32 i = 0; i < NumBonesA && i < TracksA.Num(); ++i)
	{
		BoneNameToIndexA.Add(TracksA[i].Name, i);
	}
	for (int32 i = 0; i < NumBonesB && i < TracksB.Num(); ++i)
	{
		BoneNameToIndexB.Add(TracksB[i].Name, i);
	}

	for (const auto& Pair : BoneNameToIndexA)
	{
		const FName& BoneName = Pair.Key;
		const int32* IndexB = BoneNameToIndexB.Find(BoneName);
		if (!IndexB)
		{
			continue;
		}

		const int32 IdxA = Pair.Value;
		const int32 IdxB = *IndexB;
		if (IdxA >= TracksA.Num() || IdxB >= TracksB.Num())
		{
			continue;
		}

		const FRawAnimSequenceTrack& RawA = TracksA[IdxA].InternalTrackData;
		const FRawAnimSequenceTrack& RawB = TracksB[IdxB].InternalTrackData;

		FAnimationBoneDiff BoneDiff;
		BoneDiff.BoneName = BoneName;
		BoneDiff.FrameCount = NumFrames;

		float BonePosSum = 0.0f;
		float BoneRotSum = 0.0f;
		float BoneScaleSum = 0.0f;
		float BoneMaxPos = 0.0f;
		float BoneMaxRot = 0.0f;

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float TimeA = (NumFramesA > 1) ? static_cast<float>(Frame) / static_cast<float>(NumFramesA - 1) : 0.0f;
			const float TimeB = (NumFramesB > 1) ? static_cast<float>(Frame) / static_cast<float>(NumFramesB - 1) : 0.0f;

			const float NormTimeA = FMath::Clamp(TimeA, 0.0f, 1.0f) * static_cast<float>(FMath::Max(0, RawA.PosKeys.Num() - 1));
			const float NormTimeB = FMath::Clamp(TimeB, 0.0f, 1.0f) * static_cast<float>(FMath::Max(0, RawB.PosKeys.Num() - 1));

			const int32 FrameAA = FMath::FloorToInt(NormTimeA);
			const int32 FrameAB = FMath::Min(FrameAA + 1, FMath::Max(0, RawA.PosKeys.Num() - 1));
			const float AlphaA = NormTimeA - static_cast<float>(FrameAA);

			const int32 FrameBA = FMath::FloorToInt(NormTimeB);
			const int32 FrameBB = FMath::Min(FrameBA + 1, FMath::Max(0, RawB.PosKeys.Num() - 1));
			const float AlphaB = NormTimeB - static_cast<float>(FrameBA);

			FVector3f PosA;
			if (RawA.PosKeys.Num() > 0)
			{
				PosA = (FrameAA < RawA.PosKeys.Num()) ? RawA.PosKeys[FrameAA] : FVector3f::ZeroVector;
			}
			else
			{
				PosA = FVector3f::ZeroVector;
			}

			FVector3f PosB;
			if (RawB.PosKeys.Num() > 0)
			{
				PosB = (FrameBA < RawB.PosKeys.Num()) ? RawB.PosKeys[FrameBA] : FVector3f::ZeroVector;
			}
			else
			{
				PosB = FVector3f::ZeroVector;
			}

			const float PosDiff = FVector::Dist(FVector(PosA), FVector(PosB));
			BonePosSum += PosDiff;
			BoneMaxPos = FMath::Max(BoneMaxPos, PosDiff);

			FQuat4f RotA;
			if (RawA.RotKeys.Num() > 0)
			{
				RotA = (FrameAA < RawA.RotKeys.Num()) ? RawA.RotKeys[FrameAA] : FQuat4f::Identity;
			}
			else
			{
				RotA = FQuat4f::Identity;
			}

			FQuat4f RotB;
			if (RawB.RotKeys.Num() > 0)
			{
				RotB = (FrameBA < RawB.RotKeys.Num()) ? RawB.RotKeys[FrameBA] : FQuat4f::Identity;
			}
			else
			{
				RotB = FQuat4f::Identity;
			}

			const FQuat DoubleRotA(RotA.X, RotA.Y, RotA.Z, RotA.W);
			const FQuat DoubleRotB(RotB.X, RotB.Y, RotB.Z, RotB.W);
			const float RotDiff = DoubleRotA.AngularDistance(DoubleRotB);
			BoneRotSum += RotDiff;
			BoneMaxRot = FMath::Max(BoneMaxRot, RotDiff);

			FVector3f ScaleA;
			if (RawA.ScaleKeys.Num() > 0)
			{
				ScaleA = (FrameAA < RawA.ScaleKeys.Num()) ? RawA.ScaleKeys[FrameAA] : FVector3f::OneVector;
			}
			else
			{
				ScaleA = FVector3f::OneVector;
			}

			FVector3f ScaleB;
			if (RawB.ScaleKeys.Num() > 0)
			{
				ScaleB = (FrameBA < RawB.ScaleKeys.Num()) ? RawB.ScaleKeys[FrameBA] : FVector3f::OneVector;
			}
			else
			{
				ScaleB = FVector3f::OneVector;
			}

			const float ScaleDiff = FVector::Dist(FVector(ScaleA), FVector(ScaleB));
			BoneScaleSum += ScaleDiff;

			TotalSamples++;
		}

		if (NumFrames > 0)
		{
			BoneDiff.PositionDifference = BonePosSum / static_cast<float>(NumFrames);
			BoneDiff.RotationDifference = BoneRotSum / static_cast<float>(NumFrames);
			BoneDiff.ScaleDifference = BoneScaleSum / static_cast<float>(NumFrames);
			BoneDiff.MaxPositionDifference = BoneMaxPos;
			BoneDiff.MaxRotationDifference = BoneMaxRot;
		}

		Result.BoneDiffs.Add(BoneDiff);

		TotalPosDiff += BoneDiff.PositionDifference;
		TotalRotDiff += BoneDiff.RotationDifference;
		TotalScaleDiff += BoneDiff.ScaleDifference;
		MaxPosDiff = FMath::Max(MaxPosDiff, BoneDiff.MaxPositionDifference);
		MaxRotDiff = FMath::Max(MaxRotDiff, BoneDiff.MaxRotationDifference);
	}

	const int32 BoneCount = Result.BoneDiffs.Num();
	if (BoneCount > 0)
	{
		Result.TotalPositionDifference = TotalPosDiff / static_cast<float>(BoneCount);
		Result.TotalRotationDifference = TotalRotDiff / static_cast<float>(BoneCount);
		Result.TotalScaleDifference = TotalScaleDiff / static_cast<float>(BoneCount);
		Result.MaxPositionDifference = MaxPosDiff;
		Result.MaxRotationDifference = MaxRotDiff;

		const float WeightedDiff = (Result.TotalPositionDifference * PositionWeight +
			Result.TotalRotationDifference * RotationWeight +
			Result.TotalScaleDifference * ScaleWeight) /
			(PositionWeight + RotationWeight + ScaleWeight);

		const float MaxPossible = 1000.0f;
		Result.SimilarityScore = FMath::Clamp(1.0f - (WeightedDiff / MaxPossible), 0.0f, 1.0f);
		Result.bIsIdentical = FMath::IsNearlyZero(WeightedDiff, 0.0001f);
	}

	return Result;
}

TArray<FAnimationBoneDiff> UAnimationDiffTool::CompareBoneTracks(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	FName BoneName)
{
	TArray<FAnimationBoneDiff> Result;

	if (!AnimationA || !AnimationB || BoneName.IsNone())
	{
		return Result;
	}

	const IAnimationDataModel* ModelA = AnimationA->GetDataModel();
	const IAnimationDataModel* ModelB = AnimationB->GetDataModel();
	if (!ModelA || !ModelB)
	{
		return Result;
	}

	const TArray<FBoneAnimationTrack>& TracksA = ModelA->GetBoneAnimationTracks();
	const TArray<FBoneAnimationTrack>& TracksB = ModelB->GetBoneAnimationTracks();

	int32 IdxA = INDEX_NONE;
	int32 IdxB = INDEX_NONE;

	for (int32 i = 0; i < TracksA.Num(); ++i)
	{
		if (TracksA[i].Name == BoneName)
		{
			IdxA = i;
			break;
		}
	}

	for (int32 i = 0; i < TracksB.Num(); ++i)
	{
		if (TracksB[i].Name == BoneName)
		{
			IdxB = i;
			break;
		}
	}

	if (IdxA == INDEX_NONE || IdxB == INDEX_NONE)
	{
		return Result;
	}

	const FRawAnimSequenceTrack& RawA = TracksA[IdxA].InternalTrackData;
	const FRawAnimSequenceTrack& RawB = TracksB[IdxB].InternalTrackData;

	const int32 NumFramesA = AnimationA->GetNumberOfSampledKeys();
	const int32 NumFramesB = AnimationB->GetNumberOfSampledKeys();
	const int32 NumFrames = FMath::Max(NumFramesA, NumFramesB);

	FAnimationBoneDiff BoneDiff;
	BoneDiff.BoneName = BoneName;
	BoneDiff.FrameCount = NumFrames;

	float BonePosSum = 0.0f;
	float BoneRotSum = 0.0f;
	float BoneScaleSum = 0.0f;
	float BoneMaxPos = 0.0f;
	float BoneMaxRot = 0.0f;

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const float TimeA = (NumFramesA > 1) ? static_cast<float>(Frame) / static_cast<float>(NumFramesA - 1) : 0.0f;
		const float TimeB = (NumFramesB > 1) ? static_cast<float>(Frame) / static_cast<float>(NumFramesB - 1) : 0.0f;

		const float NormTimeA = FMath::Clamp(TimeA, 0.0f, 1.0f) * static_cast<float>(FMath::Max(0, RawA.PosKeys.Num() - 1));
		const float NormTimeB = FMath::Clamp(TimeB, 0.0f, 1.0f) * static_cast<float>(FMath::Max(0, RawB.PosKeys.Num() - 1));

		const int32 FrameAA = FMath::FloorToInt(NormTimeA);
		const int32 FrameAB = FMath::Min(FrameAA + 1, FMath::Max(0, RawA.PosKeys.Num() - 1));

		const int32 FrameBA = FMath::FloorToInt(NormTimeB);
		const int32 FrameBB = FMath::Min(FrameBA + 1, FMath::Max(0, RawB.PosKeys.Num() - 1));

		FVector3f PosA = (RawA.PosKeys.Num() > 0 && FrameAA < RawA.PosKeys.Num()) ? RawA.PosKeys[FrameAA] : FVector3f::ZeroVector;
		FVector3f PosB = (RawB.PosKeys.Num() > 0 && FrameBA < RawB.PosKeys.Num()) ? RawB.PosKeys[FrameBA] : FVector3f::ZeroVector;

		const float PosDiff = FVector::Dist(FVector(PosA), FVector(PosB));
		BonePosSum += PosDiff;
		BoneMaxPos = FMath::Max(BoneMaxPos, PosDiff);

		FQuat4f RotA = (RawA.RotKeys.Num() > 0 && FrameAA < RawA.RotKeys.Num()) ? RawA.RotKeys[FrameAA] : FQuat4f::Identity;
		FQuat4f RotB = (RawB.RotKeys.Num() > 0 && FrameBA < RawB.RotKeys.Num()) ? RawB.RotKeys[FrameBA] : FQuat4f::Identity;

		const FQuat DoubleRotA(RotA.X, RotA.Y, RotA.Z, RotA.W);
		const FQuat DoubleRotB(RotB.X, RotB.Y, RotB.Z, RotB.W);
		const float RotDiff = DoubleRotA.AngularDistance(DoubleRotB);
		BoneRotSum += RotDiff;
		BoneMaxRot = FMath::Max(BoneMaxRot, RotDiff);

		FVector3f ScaleA = (RawA.ScaleKeys.Num() > 0 && FrameAA < RawA.ScaleKeys.Num()) ? RawA.ScaleKeys[FrameAA] : FVector3f::OneVector;
		FVector3f ScaleB = (RawB.ScaleKeys.Num() > 0 && FrameBA < RawB.ScaleKeys.Num()) ? RawB.ScaleKeys[FrameBA] : FVector3f::OneVector;

		const float ScaleDiff = FVector::Dist(FVector(ScaleA), FVector(ScaleB));
		BoneScaleSum += ScaleDiff;
	}

	if (NumFrames > 0)
	{
		BoneDiff.PositionDifference = BonePosSum / static_cast<float>(NumFrames);
		BoneDiff.RotationDifference = BoneRotSum / static_cast<float>(NumFrames);
		BoneDiff.ScaleDifference = BoneScaleSum / static_cast<float>(NumFrames);
		BoneDiff.MaxPositionDifference = BoneMaxPos;
		BoneDiff.MaxRotationDifference = BoneMaxRot;
	}

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

	if (!AnimationA || !AnimationB || BoneName.IsNone() || TopN <= 0)
	{
		return Result;
	}

	const IAnimationDataModel* ModelA = AnimationA->GetDataModel();
	const IAnimationDataModel* ModelB = AnimationB->GetDataModel();
	if (!ModelA || !ModelB)
	{
		return Result;
	}

	const TArray<FBoneAnimationTrack>& TracksA = ModelA->GetBoneAnimationTracks();
	const TArray<FBoneAnimationTrack>& TracksB = ModelB->GetBoneAnimationTracks();

	int32 IdxA = INDEX_NONE;
	int32 IdxB = INDEX_NONE;

	for (int32 i = 0; i < TracksA.Num(); ++i)
	{
		if (TracksA[i].Name == BoneName)
		{
			IdxA = i;
			break;
		}
	}

	for (int32 i = 0; i < TracksB.Num(); ++i)
	{
		if (TracksB[i].Name == BoneName)
		{
			IdxB = i;
			break;
		}
	}

	if (IdxA == INDEX_NONE || IdxB == INDEX_NONE)
	{
		return Result;
	}

	const FRawAnimSequenceTrack& RawA = TracksA[IdxA].InternalTrackData;
	const FRawAnimSequenceTrack& RawB = TracksB[IdxB].InternalTrackData;

	const int32 NumFramesA = AnimationA->GetNumberOfSampledKeys();
	const int32 NumFramesB = AnimationB->GetNumberOfSampledKeys();
	const int32 NumFrames = FMath::Max(NumFramesA, NumFramesB);

	TArray<float> FrameDiffs;
	FrameDiffs.SetNum(NumFrames);

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const float TimeA = (NumFramesA > 1) ? static_cast<float>(Frame) / static_cast<float>(NumFramesA - 1) : 0.0f;
		const float TimeB = (NumFramesB > 1) ? static_cast<float>(Frame) / static_cast<float>(NumFramesB - 1) : 0.0f;

		const float NormTimeA = FMath::Clamp(TimeA, 0.0f, 1.0f) * static_cast<float>(FMath::Max(0, RawA.PosKeys.Num() - 1));
		const float NormTimeB = FMath::Clamp(TimeB, 0.0f, 1.0f) * static_cast<float>(FMath::Max(0, RawB.PosKeys.Num() - 1));

		const int32 FrameAA = FMath::FloorToInt(NormTimeA);
		const int32 FrameBA = FMath::FloorToInt(NormTimeB);

		FVector3f PosA = (RawA.PosKeys.Num() > 0 && FrameAA < RawA.PosKeys.Num()) ? RawA.PosKeys[FrameAA] : FVector3f::ZeroVector;
		FVector3f PosB = (RawB.PosKeys.Num() > 0 && FrameBA < RawB.PosKeys.Num()) ? RawB.PosKeys[FrameBA] : FVector3f::ZeroVector;

		FQuat4f RotA = (RawA.RotKeys.Num() > 0 && FrameAA < RawA.RotKeys.Num()) ? RawA.RotKeys[FrameAA] : FQuat4f::Identity;
		FQuat4f RotB = (RawB.RotKeys.Num() > 0 && FrameBA < RawB.RotKeys.Num()) ? RawB.RotKeys[FrameBA] : FQuat4f::Identity;

		const float PosDiff = FVector::Dist(FVector(PosA), FVector(PosB));
		const float RotDiff = FQuat(RotA.X, RotA.Y, RotA.Z, RotA.W).AngularDistance(FQuat(RotB.X, RotB.Y, RotB.Z, RotB.W));

		FrameDiffs[Frame] = PosDiff + RotDiff;
	}

	TArray<TPair<float, int32>> Sorted;
	Sorted.Reserve(NumFrames);
	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		Sorted.Add(TPair<float, int32>(FrameDiffs[Frame], Frame));
	}

	Sorted.Sort([](const TPair<float, int32>& A, const TPair<float, int32>& B)
	{
		return A.Key > B.Key;
	});

	const int32 Count = FMath::Min(TopN, Sorted.Num());
	Result.Reserve(Count);
	for (int32 i = 0; i < Count; ++i)
	{
		Result.Add(Sorted[i].Value);
	}

	return Result;
}

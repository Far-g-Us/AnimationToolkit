#include "AnimationComparisonTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataModel.h"

FAnimationComparisonResult UAnimationComparisonTool::CompareAnimations(
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB)
{
	FAnimationComparisonResult Result;

	if (!AnimationA || !AnimationB)
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

	TMap<FName, int32> BoneIndexMapA;
	for (int32 i = 0; i < TracksA.Num(); ++i)
	{
		BoneIndexMapA.Add(TracksA[i].Name, i);
	}

	TMap<FName, int32> BoneIndexMapB;
	for (int32 i = 0; i < TracksB.Num(); ++i)
	{
		BoneIndexMapB.Add(TracksB[i].Name, i);
	}

	float TotalPositionDiff = 0.0f;
	float TotalRotationDiff = 0.0f;
	int32 CommonBoneCount = 0;

	for (const TPair<FName, int32>& Pair : BoneIndexMapA)
	{
		const FName& BoneName = Pair.Key;
		if (!BoneIndexMapB.Contains(BoneName))
		{
			continue;
		}

		const FRawAnimSequenceTrack& RawA = TracksA[Pair.Value].InternalTrackData;
		const FRawAnimSequenceTrack& RawB = TracksB[BoneIndexMapB[BoneName]].InternalTrackData;

		const int32 NumPosKeys = FMath::Min(RawA.PosKeys.Num(), RawB.PosKeys.Num());
		const int32 NumRotKeys = FMath::Min(RawA.RotKeys.Num(), RawB.RotKeys.Num());

		float BonePosDiff = 0.0f;
		for (int32 i = 0; i < NumPosKeys; ++i)
		{
			FVector PosA(RawA.PosKeys[i]);
			FVector PosB(RawB.PosKeys[i]);
			BonePosDiff += FVector::Dist(PosA, PosB);
		}
		if (NumPosKeys > 0)
		{
			BonePosDiff /= NumPosKeys;
		}

		float BoneRotDiff = 0.0f;
		for (int32 i = 0; i < NumRotKeys; ++i)
		{
			FQuat RotA(RawA.RotKeys[i]);
			FQuat RotB(RawB.RotKeys[i]);
			BoneRotDiff += FMath::Abs(RotA.AngularDistance(RotB));
		}
		if (NumRotKeys > 0)
		{
			BoneRotDiff /= NumRotKeys;
		}

		TotalPositionDiff += BonePosDiff;
		TotalRotationDiff += BoneRotDiff;
		CommonBoneCount++;

		if (BonePosDiff > 0.01f || BoneRotDiff > 0.01f)
		{
			Result.DifferentBones.Add(BoneName);
		}
	}

	if (CommonBoneCount > 0)
	{
		Result.PositionDifference = TotalPositionDiff / CommonBoneCount;
		Result.RotationDifference = TotalRotationDiff / CommonBoneCount;
	}

	const float NormPos = Result.PositionDifference / 100.0f;
	const float NormRot = Result.RotationDifference / PI;
	const float CombinedDiff = NormPos + NormRot;
	Result.SimilarityScore = FMath::Clamp(FMath::Exp(-CombinedDiff), 0.0f, 1.0f);
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

	const IAnimationDataModel* ModelA = AnimationA->GetDataModel();
	const IAnimationDataModel* ModelB = AnimationB->GetDataModel();

	if (!ModelA || !ModelB)
	{
		return Result;
	}

	const FBoneAnimationTrack* TrackA = ModelA->FindBoneTrackByName(BoneName);
	const FBoneAnimationTrack* TrackB = ModelB->FindBoneTrackByName(BoneName);

	if (!TrackA || !TrackB)
	{
		return Result;
	}

	const FRawAnimSequenceTrack& RawA = TrackA->InternalTrackData;
	const FRawAnimSequenceTrack& RawB = TrackB->InternalTrackData;

	const int32 NumPosKeys = FMath::Min(RawA.PosKeys.Num(), RawB.PosKeys.Num());

	TArray<TPair<int32, float>> FrameDiffs;
	FrameDiffs.Reserve(NumPosKeys);

	for (int32 i = 0; i < NumPosKeys; ++i)
	{
		const FVector PosA(RawA.PosKeys[i]);
		const FVector PosB(RawB.PosKeys[i]);
		const float Diff = FVector::Dist(PosA, PosB);
		FrameDiffs.Add(TPair<int32, float>(i, Diff));
	}

	FrameDiffs.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value > B.Value;
	});

	const int32 Count = FMath::Min(TopN, FrameDiffs.Num());
	for (int32 i = 0; i < Count; ++i)
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
	OutBoneName = NAME_None;

	if (!AnimationA || !AnimationB)
	{
		return 0.0f;
	}

	const IAnimationDataModel* ModelA = AnimationA->GetDataModel();
	const IAnimationDataModel* ModelB = AnimationB->GetDataModel();

	if (!ModelA || !ModelB)
	{
		return 0.0f;
	}

	const TArray<FBoneAnimationTrack>& TracksA = ModelA->GetBoneAnimationTracks();
	const TArray<FBoneAnimationTrack>& TracksB = ModelB->GetBoneAnimationTracks();

	TMap<FName, int32> BoneIndexMapB;
	for (int32 i = 0; i < TracksB.Num(); ++i)
	{
		BoneIndexMapB.Add(TracksB[i].Name, i);
	}

	float MaxDiff = 0.0f;

	for (const FBoneAnimationTrack& TrackA : TracksA)
	{
		if (!BoneIndexMapB.Contains(TrackA.Name))
		{
			continue;
		}

		const FRawAnimSequenceTrack& RawA = TrackA.InternalTrackData;
		const FRawAnimSequenceTrack& RawB = TracksB[BoneIndexMapB[TrackA.Name]].InternalTrackData;

		const int32 NumPosKeys = FMath::Min(RawA.PosKeys.Num(), RawB.PosKeys.Num());

		for (int32 i = 0; i < NumPosKeys; ++i)
		{
			const FVector PosA(RawA.PosKeys[i]);
			const FVector PosB(RawB.PosKeys[i]);
			const float Diff = FVector::Dist(PosA, PosB);

			if (Diff > MaxDiff)
			{
				MaxDiff = Diff;
				OutBoneName = TrackA.Name;
			}
		}
	}

	return MaxDiff;
}

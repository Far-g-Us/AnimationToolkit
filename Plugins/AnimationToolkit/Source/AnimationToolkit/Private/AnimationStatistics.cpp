#include "AnimationStatistics.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataModel.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

static void GetRawTracks(UAnimSequence* Animation, TArray<FRawAnimSequenceTrack*>& OutRawTracks)
{
#if WITH_EDITOR
	if (IAnimationDataModel* DataModel = Animation->GetDataModel())
	{
		const TArray<FBoneAnimationTrack>& BoneTracks = DataModel->GetBoneAnimationTracks();
		for (const FBoneAnimationTrack& BoneTrack : BoneTracks)
		{
			OutRawTracks.Add(const_cast<FRawAnimSequenceTrack*>(&BoneTrack.InternalTrackData));
		}
	}
#endif
}

FAnimationStats UAnimationStatistics::GetAnimationStatistics(UAnimSequence* Animation)
{
	FAnimationStats Stats;
	if (!Animation)
	{
		return Stats;
	}

	Stats.Duration = Animation->GetPlayLength();
	Stats.FrameCount = Animation->GetNumberOfSampledKeys();

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (Skeleton)
	{
		const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
		Stats.BoneCount = RefSkel.GetNum();
	}

	FVector TotalPosition = FVector::ZeroVector;
	int32 TotalPosKeys = 0;
	FVector MinPos(TNumericLimits<float>::Max());
	FVector MaxPos(TNumericLimits<float>::Lowest());
	bool bHasPositionKeys = false;

	TArray<FRawAnimSequenceTrack*> RawTracks;
	GetRawTracks(Animation, RawTracks);

	for (const FRawAnimSequenceTrack* TrackPtr : RawTracks)
	{
		if (!TrackPtr) continue;
		const FRawAnimSequenceTrack& Track = *TrackPtr;

		Stats.TranslationKeys += Track.PosKeys.Num();
		Stats.RotationKeys += Track.RotKeys.Num();
		Stats.ScaleKeys += Track.ScaleKeys.Num();

		for (const FVector3f& PosKey : Track.PosKeys)
		{
			FVector Pos(PosKey);
			TotalPosition += Pos;
			TotalPosKeys++;
			MinPos = MinPos.ComponentMin(Pos);
			MaxPos = MaxPos.ComponentMax(Pos);
			bHasPositionKeys = true;
		}
	}

	Stats.TotalKeyframes = Stats.TranslationKeys + Stats.RotationKeys + Stats.ScaleKeys;

	if (TotalPosKeys > 0)
	{
		Stats.AveragePosition = TotalPosition / static_cast<double>(TotalPosKeys);
	}

	if (bHasPositionKeys)
	{
		Stats.PositionRange = MaxPos - MinPos;
	}

	Stats.MemoryUsageKB = EstimateMemoryUsage(Animation);

	return Stats;
}

float UAnimationStatistics::EstimateMemoryUsage(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0.0f;
	}

	TArray<FRawAnimSequenceTrack*> RawTracks;
	GetRawTracks(Animation, RawTracks);
	float MemoryBytes = 0.0f;

	for (const FRawAnimSequenceTrack* TrackPtr : RawTracks)
	{
		if (!TrackPtr) continue;
		const FRawAnimSequenceTrack& Track = *TrackPtr;
		MemoryBytes += Track.PosKeys.Num() * 12.0f;
		MemoryBytes += Track.RotKeys.Num() * 16.0f;
		MemoryBytes += Track.ScaleKeys.Num() * 12.0f;
	}

	MemoryBytes += RawTracks.Num() * 24.0f;
	MemoryBytes += 1024.0f;

	return MemoryBytes / 1024.0f;
}

int32 UAnimationStatistics::GetTotalKeyframes(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0;
	}

	TArray<FRawAnimSequenceTrack*> RawTracks;
	GetRawTracks(Animation, RawTracks);
	int32 Total = 0;

	for (const FRawAnimSequenceTrack* TrackPtr : RawTracks)
	{
		if (!TrackPtr) continue;
		const FRawAnimSequenceTrack& Track = *TrackPtr;
		Total += Track.PosKeys.Num();
		Total += Track.RotKeys.Num();
		Total += Track.ScaleKeys.Num();
	}

	return Total;
}

FVector UAnimationStatistics::GetPositionRange(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return FVector::ZeroVector;
	}

	FVector MinPos(TNumericLimits<float>::Max());
	FVector MaxPos(TNumericLimits<float>::Lowest());
	bool bFoundKey = false;

	TArray<FRawAnimSequenceTrack*> RawTracks;
	GetRawTracks(Animation, RawTracks);

	for (const FRawAnimSequenceTrack* TrackPtr : RawTracks)
	{
		if (!TrackPtr) continue;
		const FRawAnimSequenceTrack& Track = *TrackPtr;

		for (const FVector3f& PosKey : Track.PosKeys)
		{
			FVector Pos(PosKey);
			MinPos = MinPos.ComponentMin(Pos);
			MaxPos = MaxPos.ComponentMax(Pos);
			bFoundKey = true;
		}
	}

	if (!bFoundKey)
	{
		return FVector::ZeroVector;
	}

	return MaxPos - MinPos;
}

FRotator UAnimationStatistics::GetRotationRange(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return FRotator::ZeroRotator;
	}

	FVector MinRot(TNumericLimits<float>::Max());
	FVector MaxRot(TNumericLimits<float>::Lowest());
	bool bFoundKey = false;

	TArray<FRawAnimSequenceTrack*> RawTracks;
	GetRawTracks(Animation, RawTracks);

	for (const FRawAnimSequenceTrack* TrackPtr : RawTracks)
	{
		if (!TrackPtr) continue;
		const FRawAnimSequenceTrack& Track = *TrackPtr;

		for (const FQuat4f& RotKey : Track.RotKeys)
		{
			FQuat DoubleQuat(RotKey.X, RotKey.Y, RotKey.Z, RotKey.W);
			FRotator Rot(DoubleQuat);
			FVector RotVec(Rot.Pitch, Rot.Yaw, Rot.Roll);
			MinRot = MinRot.ComponentMin(RotVec);
			MaxRot = MaxRot.ComponentMax(RotVec);
			bFoundKey = true;
		}
	}

	if (!bFoundKey)
	{
		return FRotator::ZeroRotator;
	}

	return FRotator(MaxRot.X - MinRot.X, MaxRot.Y - MinRot.Y, MaxRot.Z - MinRot.Z);
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS

FString UAnimationStatistics::GenerateReport(UAnimSequence* Animation)
{
	FAnimationStats Stats = GetAnimationStatistics(Animation);

	FString Report;
	Report += TEXT("========================================\n");
	Report += TEXT("      Animation Statistics Report       \n");
	Report += TEXT("========================================\n\n");

	Report += FString::Printf(TEXT("Duration:          %.3f seconds\n"), Stats.Duration);
	Report += FString::Printf(TEXT("Frame Count:       %d\n"), Stats.FrameCount);
	Report += FString::Printf(TEXT("Bone Count:        %d\n"), Stats.BoneCount);
	Report += TEXT("\n--- Key Counts ---\n");
	Report += FString::Printf(TEXT("Translation Keys:  %d\n"), Stats.TranslationKeys);
	Report += FString::Printf(TEXT("Rotation Keys:     %d\n"), Stats.RotationKeys);
	Report += FString::Printf(TEXT("Scale Keys:        %d\n"), Stats.ScaleKeys);
	Report += FString::Printf(TEXT("Total Keyframes:   %d\n"), Stats.TotalKeyframes);
	Report += TEXT("\n--- Position Data ---\n");
	Report += FString::Printf(TEXT("Average Position:  %s\n"), *Stats.AveragePosition.ToString());
	Report += FString::Printf(TEXT("Position Range:    %s\n"), *Stats.PositionRange.ToString());
	Report += TEXT("\n--- Memory ---\n");
	Report += FString::Printf(TEXT("Estimated Memory:  %.2f KB\n"), Stats.MemoryUsageKB);
	Report += TEXT("========================================\n");

	return Report;
}

TArray<FAnimationStats> UAnimationStatistics::BatchGetStatistics(
	const TArray<UAnimSequence*>& Animations)
{
	TArray<FAnimationStats> Results;
	Results.Reserve(Animations.Num());

	for (UAnimSequence* Anim : Animations)
	{
		Results.Add(GetAnimationStatistics(Anim));
	}

	return Results;
}

float UAnimationStatistics::CompareStatistics(
	const FAnimationStats& StatsA,
	const FAnimationStats& StatsB)
{
	float Score = 0.0f;
	float WeightSum = 0.0f;

	float MaxDuration = FMath::Max(StatsA.Duration, StatsB.Duration);
	if (MaxDuration > 0.0f)
	{
		Score += (FMath::Abs(StatsA.Duration - StatsB.Duration) / MaxDuration) * 0.25f;
	}
	WeightSum += 0.25f;

	float MaxFrames = FMath::Max(static_cast<float>(StatsA.FrameCount), static_cast<float>(StatsB.FrameCount));
	if (MaxFrames > 0.0f)
	{
		Score += (FMath::Abs(static_cast<float>(StatsA.FrameCount - StatsB.FrameCount)) / MaxFrames) * 0.20f;
	}
	WeightSum += 0.20f;

	float MaxBones = FMath::Max(static_cast<float>(StatsA.BoneCount), static_cast<float>(StatsB.BoneCount));
	if (MaxBones > 0.0f)
	{
		Score += (FMath::Abs(static_cast<float>(StatsA.BoneCount - StatsB.BoneCount)) / MaxBones) * 0.15f;
	}
	WeightSum += 0.15f;

	float MaxKeys = FMath::Max(static_cast<float>(StatsA.TotalKeyframes), static_cast<float>(StatsB.TotalKeyframes));
	if (MaxKeys > 0.0f)
	{
		Score += (FMath::Abs(static_cast<float>(StatsA.TotalKeyframes - StatsB.TotalKeyframes)) / MaxKeys) * 0.20f;
	}
	WeightSum += 0.20f;

	float RangeA = StatsA.PositionRange.Size();
	float RangeB = StatsB.PositionRange.Size();
	float MaxRange = FMath::Max(RangeA, RangeB);
	if (MaxRange > 0.0f)
	{
		Score += (FMath::Abs(RangeA - RangeB) / MaxRange) * 0.20f;
	}
	WeightSum += 0.20f;

	if (WeightSum > 0.0f)
	{
		Score /= WeightSum;
	}

	return FMath::Clamp(Score, 0.0f, 1.0f);
}

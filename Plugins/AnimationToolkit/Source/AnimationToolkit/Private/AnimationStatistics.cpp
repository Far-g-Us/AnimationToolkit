#include "AnimationStatistics.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

FAnimationStats UAnimationStatistics::GetAnimationStatistics(UAnimSequence* Animation)
{
	FAnimationStats Stats;

	if (!Animation || !Animation->GetSkeleton())
	{
		return Stats;
	}

	const FReferenceSkeleton& RefSkeleton = Animation->GetSkeleton()->GetReferenceSkeleton();
	Stats.BoneCount = RefSkeleton.GetNum();
	Stats.FrameCount = Animation->GetNumberOfSampledKeys();
	Stats.Duration = Animation->GetSequenceLength();
	Stats.TotalKeyframes = Stats.BoneCount * Stats.FrameCount;
	Stats.MemoryUsageKB = EstimateMemoryUsage(Animation) / 1024.0f;

	Stats.TranslationKeys = Stats.TotalKeyframes;
	Stats.RotationKeys = Stats.TotalKeyframes;
	Stats.ScaleKeys = Stats.TotalKeyframes;

	return Stats;
}

float UAnimationStatistics::EstimateMemoryUsage(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0.0f;
	}

	int32 Keyframes = GetTotalKeyframes(Animation);
	return (float)(Keyframes * (sizeof(FVector) + sizeof(FQuat) + sizeof(FVector)));
}

int32 UAnimationStatistics::GetTotalKeyframes(UAnimSequence* Animation)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		return 0;
	}

	int32 BoneCount = Animation->GetSkeleton()->GetReferenceSkeleton().GetNum();
	int32 FrameCount = Animation->GetNumberOfSampledKeys();

	return BoneCount * FrameCount;
}

FVector UAnimationStatistics::GetPositionRange(UAnimSequence* Animation)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		return FVector::ZeroVector;
	}

	const FReferenceSkeleton& RefSkeleton = Animation->GetSkeleton()->GetReferenceSkeleton();
	int32 FrameCount = Animation->GetNumberOfSampledKeys();

	FVector MinPos = FVector::ZeroVector;
	FVector MaxPos = FVector::ZeroVector;
	bool bFirst = true;

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform Transform = Animation->GetBoneTransform(BoneIdx, Frame);
			FVector Pos = Transform.GetLocation();

			if (bFirst)
			{
				MinPos = Pos;
				MaxPos = Pos;
				bFirst = false;
			}
			else
			{
				MinPos = MinPos.ComponentMin(Pos);
				MaxPos = MaxPos.ComponentMax(Pos);
			}
		}
	}

	return MaxPos - MinPos;
}

FRotator UAnimationStatistics::GetRotationRange(UAnimSequence* Animation)
{
	return FRotator::ZeroRotator;
}

FString UAnimationStatistics::GenerateReport(UAnimSequence* Animation)
{
	FAnimationStats Stats = GetAnimationStatistics(Animation);

	FString Report;
	Report += FString::Printf(TEXT("Animation Statistics Report\n"));
	Report += FString::Printf(TEXT("==========================\n"));
	Report += FString::Printf(TEXT("Bone Count: %d\n"), Stats.BoneCount);
	Report += FString::Printf(TEXT("Frame Count: %d\n"), Stats.FrameCount);
	Report += FString::Printf(TEXT("Duration: %.2f seconds\n"), Stats.Duration);
	Report += FString::Printf(TEXT("Total Keyframes: %d\n"), Stats.TotalKeyframes);
	Report += FString::Printf(TEXT("Memory Usage: %.2f KB\n"), Stats.MemoryUsageKB);

	return Report;
}

TArray<FAnimationStats> UAnimationStatistics::BatchGetStatistics(
	const TArray<UAnimSequence*>& Animations)
{
	TArray<FAnimationStats> Results;

	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			Results.Add(GetAnimationStatistics(Anim));
		}
	}

	return Results;
}

float UAnimationStatistics::CompareStatistics(
	const FAnimationStats& StatsA,
	const FAnimationStats& StatsB)
{
	float Diff = 0.0f;

	Diff += FMath::Abs((float)(StatsA.BoneCount - StatsB.BoneCount));
	Diff += FMath::Abs((float)(StatsA.FrameCount - StatsB.FrameCount));
	Diff += FMath::Abs(StatsA.Duration - StatsB.Duration);
	Diff += FMath::Abs(StatsA.MemoryUsageKB - StatsB.MemoryUsageKB);

	return Diff;
}

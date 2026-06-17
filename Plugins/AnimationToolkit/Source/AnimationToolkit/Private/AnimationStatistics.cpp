#include "AnimationStatistics.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

FAnimationStats UAnimationStatistics::GetAnimationStatistics(UAnimSequence* Animation)
{
	return FAnimationStats();
}

float UAnimationStatistics::EstimateMemoryUsage(UAnimSequence* Animation)
{
	return 0.0f;
}

int32 UAnimationStatistics::GetTotalKeyframes(UAnimSequence* Animation)
{
	return 0;
}

FVector UAnimationStatistics::GetPositionRange(UAnimSequence* Animation)
{
	return FVector::ZeroVector;
}

FRotator UAnimationStatistics::GetRotationRange(UAnimSequence* Animation)
{
	return FRotator::ZeroRotator;
}

FString UAnimationStatistics::GenerateReport(UAnimSequence* Animation)
{
	return FString();
}

TArray<FAnimationStats> UAnimationStatistics::BatchGetStatistics(
	const TArray<UAnimSequence*>& Animations)
{
	return TArray<FAnimationStats>();
}

float UAnimationStatistics::CompareStatistics(
	const FAnimationStats& StatsA,
	const FAnimationStats& StatsB)
{
	return 0.0f;
}

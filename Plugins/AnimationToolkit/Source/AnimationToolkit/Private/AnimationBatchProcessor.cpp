#include "AnimationBatchProcessor.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

static int32 GBatchProgress = 0;
static FString GBatchStatus = TEXT("Idle");

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchProcessAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	const FBatchProcessSettings& Settings)
{
	TArray<UAnimSequence*> Results;

	switch (Settings.Operation)
	{
	case EBatchOperation::Compress:
		return BatchCompress(WorldContextObject, Animations);
	case EBatchOperation::Optimize:
		return BatchOptimize(WorldContextObject, Animations);
	case EBatchOperation::Retarget:
		return BatchRetarget(WorldContextObject, Animations, Settings.TargetSkeleton);
	case EBatchOperation::Mirror:
		return BatchMirror(WorldContextObject, Animations, Settings.MirrorFlags);
	case EBatchOperation::Reverse:
		return BatchReverse(WorldContextObject, Animations);
	case EBatchOperation::ChangePlayRate:
		return BatchChangePlayRate(WorldContextObject, Animations, Settings.Parameter);
	}

	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchCompress(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations)
{
	TArray<UAnimSequence*> Results;
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchOptimize(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations)
{
	TArray<UAnimSequence*> Results;
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchRetarget(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	USkeleton* TargetSkeleton)
{
	TArray<UAnimSequence*> Results;
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchMirror(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	EMirrorFlags MirrorFlags)
{
	TArray<UAnimSequence*> Results;
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchReverse(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations)
{
	TArray<UAnimSequence*> Results;
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchChangePlayRate(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	float PlayRate)
{
	TArray<UAnimSequence*> Results;
	return Results;
}

int32 UAnimationBatchProcessor::GetBatchProgress()
{
	return GBatchProgress;
}

FString UAnimationBatchProcessor::GetBatchStatus()
{
	return GBatchStatus;
}

#include "AnimationRepairTool.h"
#include "BoneMappingTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

UAnimSequence* UAnimationRepairTool::RepairAnimationMissingBones(
	UObject* WorldContextObject,
	UAnimSequence* BrokenAnimation,
	USkeleton* TargetSkeleton,
	FAnimationRepairResult* OutResult)
{
	return nullptr;
}

UAnimSequence* UAnimationRepairTool::ReassignAnimationSkeleton(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	USkeleton* NewSkeleton,
	bool bAutoRemap,
	FAnimationRepairResult* OutResult)
{
	return nullptr;
}

FAnimationRepairResult UAnimationRepairTool::AnalyzeAnimationBones(
	UAnimSequence* Animation,
	USkeleton* ExpectedSkeleton)
{
	return FAnimationRepairResult();
}

TArray<UAnimSequence*> UAnimationRepairTool::BatchRepairAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& BrokenAnimations,
	USkeleton* TargetSkeleton,
	FAnimationRepairResult* OutResult)
{
	return TArray<UAnimSequence*>();
}

bool UAnimationRepairTool::IsAnimationBroken(UAnimSequence* Animation)
{
	return false;
}

TArray<FName> UAnimationRepairTool::GetMissingBones(
	UAnimSequence* Animation, USkeleton* ExpectedSkeleton)
{
	return TArray<FName>();
}

TArray<FName> UAnimationRepairTool::GetExtraBones(
	UAnimSequence* Animation, USkeleton* ExpectedSkeleton)
{
	return TArray<FName>();
}

void UAnimationRepairTool::FillMissingBonesWithRestPose(
	UAnimSequence* Animation,
	USkeleton* Skeleton,
	const TArray<FName>& MissingBones)
{
}

void UAnimationRepairTool::RemoveExtraTracks(
	UAnimSequence* Animation,
	const TArray<FName>& ExtraBones)
{
}

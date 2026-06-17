#include "AnimationTimingTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

UAnimSequence* UAnimationTimingTool::ChangePlayRate(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float PlayRate)
{
	return nullptr;
}

UAnimSequence* UAnimationTimingTool::ReverseAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation)
{
	return nullptr;
}

UAnimSequence* UAnimationTimingTool::TimeWarp(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float StartTime,
	float EndTime)
{
	return nullptr;
}

UAnimSequence* UAnimationTimingTool::BlendTiming(
	UObject* WorldContextObject,
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	float BlendFactor)
{
	return nullptr;
}

float UAnimationTimingTool::GetAnimationSpeed(
	UAnimSequence* Animation,
	float TargetDuration)
{
	return 0.0f;
}

float UAnimationTimingTool::GetAnimationDuration(UAnimSequence* Animation)
{
	return 0.0f;
}

int32 UAnimationTimingTool::GetFrameCount(UAnimSequence* Animation)
{
	return 0;
}

float UAnimationTimingTool::TimeToFrame(UAnimSequence* Animation, float Time)
{
	return 0.0f;
}

float UAnimationTimingTool::FrameToTime(UAnimSequence* Animation, int32 Frame)
{
	return 0.0f;
}

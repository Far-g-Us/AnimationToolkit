#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationTimingTool.generated.h"

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationTimingTool : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Timing", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ChangePlayRate(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		float PlayRate);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Timing", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ReverseAnimation(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Timing", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* TimeWarp(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		float StartTime,
		float EndTime);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Timing", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* BlendTiming(
		UObject* WorldContextObject,
		UAnimSequence* AnimationA,
		UAnimSequence* AnimationB,
		float BlendFactor);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Timing")
	static float GetAnimationSpeed(
		UAnimSequence* Animation,
		float TargetDuration);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Timing")
	static float GetAnimationDuration(
		UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Timing")
	static int32 GetFrameCount(
		UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Timing")
	static float TimeToFrame(
		UAnimSequence* Animation,
		float Time);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Timing")
	static float FrameToTime(
		UAnimSequence* Animation,
		int32 Frame);
};

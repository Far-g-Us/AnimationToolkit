#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationPreviewWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimationPreviewChanged, UAnimSequence*, Animation);

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationPreviewWidget : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	static void SetPreviewAnimation(
		UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	static UAnimSequence* GetPreviewAnimation();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	static void SetPreviewTime(float Time);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	static float GetPreviewTime();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	static void PlayPreview();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	static void StopPreview();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	static void PausePreview();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	static bool IsPreviewPlaying();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	static void SetPreviewPlayRate(float PlayRate);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	static float GetPreviewPlayRate();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	static void SetPreviewLoop(bool bLoop);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	static bool IsPreviewLooping();
};

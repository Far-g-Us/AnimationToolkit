#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationProceduralGeneration.generated.h"

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationProceduralGeneration : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* GenerateIdleAnimation(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		float Duration = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* GenerateWalkCycle(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		float StepLength = 50.0f,
		float StepHeight = 15.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* GenerateRunCycle(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		float StepLength = 80.0f,
		float StepHeight = 25.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* GenerateBreathing(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		float BreathRate = 15.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* GenerateHeadBob(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		float BobFrequency = 2.0f,
		float BobAmplitude = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CombineProceduralAnimations(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations,
		const TArray<float>& Weights);
};

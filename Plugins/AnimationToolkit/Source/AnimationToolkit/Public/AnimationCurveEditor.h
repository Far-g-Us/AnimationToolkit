#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationCurveEditor.generated.h"

USTRUCT(BlueprintType)
struct FAnimationCurveKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName InterpolationType = FName("Linear");
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationCurveEditor : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curve", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* AddCurveToAnimation(
		UObject* WorldContextObject,
		UAnimSequence* Animation,
		FName CurveName,
		const TArray<FAnimationCurveKey>& Keyframes);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curve", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* RemoveCurveFromAnimation(
		UObject* WorldContextObject,
		UAnimSequence* Animation,
		FName CurveName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curve")
	static TArray<FName> GetAnimationCurves(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curve")
	static TArray<FAnimationCurveKey> GetCurveKeyframes(
		UAnimSequence* Animation,
		FName CurveName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curve", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* SetCurveKeyframes(
		UObject* WorldContextObject,
		UAnimSequence* Animation,
		FName CurveName,
		const TArray<FAnimationCurveKey>& Keyframes);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curve")
	static float EvaluateCurve(
		UAnimSequence* Animation,
		FName CurveName,
		float Time);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curve")
	static float GetCurveMinValue(
		UAnimSequence* Animation,
		FName CurveName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curve")
	static float GetCurveMaxValue(
		UAnimSequence* Animation,
		FName CurveName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curve")
	static float GetCurveDuration(
		UAnimSequence* Animation,
		FName CurveName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curve", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* RenameCurve(
		UObject* WorldContextObject,
		UAnimSequence* Animation,
		FName OldCurveName,
		FName NewCurveName);
};

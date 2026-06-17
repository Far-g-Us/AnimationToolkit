#include "AnimationCurveEditor.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

UAnimSequence* UAnimationCurveEditor::AddCurveToAnimation(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	FName CurveName,
	const TArray<FAnimationCurveKey>& Keyframes)
{
	if (!WorldContextObject || !Animation)
	{
		return nullptr;
	}

	return Animation;
}

UAnimSequence* UAnimationCurveEditor::RemoveCurveFromAnimation(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	FName CurveName)
{
	if (!WorldContextObject || !Animation)
	{
		return nullptr;
	}

	return Animation;
}

TArray<FName> UAnimationCurveEditor::GetAnimationCurves(UAnimSequence* Animation)
{
	TArray<FName> Result;

	if (!Animation)
	{
		return Result;
	}

	return Result;
}

TArray<FAnimationCurveKey> UAnimationCurveEditor::GetCurveKeyframes(
	UAnimSequence* Animation,
	FName CurveName)
{
	TArray<FAnimationCurveKey> Result;

	if (!Animation)
	{
		return Result;
	}

	return Result;
}

UAnimSequence* UAnimationCurveEditor::SetCurveKeyframes(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	FName CurveName,
	const TArray<FAnimationCurveKey>& Keyframes)
{
	if (!WorldContextObject || !Animation)
	{
		return nullptr;
	}

	return Animation;
}

float UAnimationCurveEditor::EvaluateCurve(
	UAnimSequence* Animation,
	FName CurveName,
	float Time)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return 0.0f;
}

float UAnimationCurveEditor::GetCurveMinValue(
	UAnimSequence* Animation,
	FName CurveName)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return 0.0f;
}

float UAnimationCurveEditor::GetCurveMaxValue(
	UAnimSequence* Animation,
	FName CurveName)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return 0.0f;
}

float UAnimationCurveEditor::GetCurveDuration(
	UAnimSequence* Animation,
	FName CurveName)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return Animation->GetSequenceLength();
}

UAnimSequence* UAnimationCurveEditor::RenameCurve(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	FName OldCurveName,
	FName NewCurveName)
{
	if (!WorldContextObject || !Animation)
	{
		return nullptr;
	}

	return Animation;
}

#include "AnimationCurveEditor.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "AnimationCurveEditor"

UAnimSequence* UAnimationCurveEditor::AddCurveToAnimation(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	FName CurveName,
	const TArray<FAnimationCurveKey>& Keyframes)
{
	if (!Animation || CurveName.IsNone()) return Animation;

	IAnimationDataController& Controller = Animation->GetController();
	FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);
	Controller.OpenBracket(LOCTEXT("AddCurve", "Adding Curve"), false);
	Controller.AddCurve(CurveId);

	for (const FAnimationCurveKey& KF : Keyframes)
	{
		FRichCurveKey RichKey;
		RichKey.Time = KF.Time;
		RichKey.Value = KF.Value;
		RichKey.InterpMode = ERichCurveInterpMode::RCIM_Linear;
		Controller.SetCurveKey(CurveId, RichKey, false);
	}
	Controller.CloseBracket(false);
	Animation->MarkPackageDirty();
	return Animation;
}

UAnimSequence* UAnimationCurveEditor::RemoveCurveFromAnimation(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	FName CurveName)
{
	if (!Animation || CurveName.IsNone()) return Animation;

	IAnimationDataController& Controller = Animation->GetController();
	FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);
	Controller.RemoveCurve(CurveId, true);
	Animation->MarkPackageDirty();
	return Animation;
}

TArray<FName> UAnimationCurveEditor::GetAnimationCurves(UAnimSequence* Animation)
{
	TArray<FName> Result;
	if (!Animation) return Result;

	const TArray<FFloatCurve>& FloatCurves = Animation->GetController().GetModelInterface()->GetFloatCurves();
	for (const FFloatCurve& Curve : FloatCurves)
	{
		Result.Add(Curve.GetName());
	}
	return Result;
}

TArray<FAnimationCurveKey> UAnimationCurveEditor::GetCurveKeyframes(
	UAnimSequence* Animation,
	FName CurveName)
{
	TArray<FAnimationCurveKey> Result;
	if (!Animation || CurveName.IsNone()) return Result;

	const TScriptInterface<IAnimationDataModel>& Model = Animation->GetController().GetModelInterface();
	FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);
	const FFloatCurve* Curve = Model->FindFloatCurve(CurveId);
	if (!Curve) return Result;

	const float SampleRate = 30.0f;
	const float Duration = static_cast<float>(Model->GetNumberOfFrames()) / SampleRate;
	const int32 NumSamples = FMath::Max(2, FMath::CeilToInt(Duration * SampleRate));

	for (int32 i = 0; i < NumSamples; ++i)
	{
		const float Time = (NumSamples > 1)
			? Duration * static_cast<float>(i) / static_cast<float>(NumSamples - 1)
			: 0.0f;
		float Value = Curve->Evaluate(Time);

		FAnimationCurveKey KF;
		KF.Time = Time;
		KF.Value = Value;
		Result.Add(KF);
	}
	return Result;
}

UAnimSequence* UAnimationCurveEditor::SetCurveKeyframes(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	FName CurveName,
	const TArray<FAnimationCurveKey>& Keyframes)
{
	if (!Animation || CurveName.IsNone()) return Animation;

	IAnimationDataController& Controller = Animation->GetController();
	FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);
	Controller.RemoveCurve(CurveId, true);
	Controller.AddCurve(CurveId);

	for (const FAnimationCurveKey& KF : Keyframes)
	{
		FRichCurveKey RichKey;
		RichKey.Time = KF.Time;
		RichKey.Value = KF.Value;
		RichKey.InterpMode = ERichCurveInterpMode::RCIM_Linear;
		Controller.SetCurveKey(CurveId, RichKey, false);
	}
	Animation->MarkPackageDirty();
	return Animation;
}

float UAnimationCurveEditor::EvaluateCurve(
	UAnimSequence* Animation,
	FName CurveName,
	float Time)
{
	if (!Animation || CurveName.IsNone()) return 0.0f;

	const TScriptInterface<IAnimationDataModel>& Model = Animation->GetController().GetModelInterface();
	if (!Model) return 0.0f;

	FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);
	const FFloatCurve* Curve = Model->FindFloatCurve(CurveId);
	if (!Curve) return 0.0f;
	return Curve->Evaluate(Time);
}

float UAnimationCurveEditor::GetCurveMinValue(
	UAnimSequence* Animation,
	FName CurveName)
{
	if (!Animation || CurveName.IsNone()) return 0.0f;

	const TScriptInterface<IAnimationDataModel>& Model = Animation->GetController().GetModelInterface();
	if (!Model) return 0.0f;

	FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);
	const FFloatCurve* Curve = Model->FindFloatCurve(CurveId);
	if (!Curve) return 0.0f;

	const float SampleRate = 30.0f;
	const float Duration = static_cast<float>(Model->GetNumberOfFrames()) / SampleRate;
	const int32 NumSamples = FMath::Max(2, FMath::CeilToInt(Duration * SampleRate));

	float MinVal = TNumericLimits<float>::Max();
	for (int32 i = 0; i < NumSamples; ++i)
	{
		const float Time = (NumSamples > 1)
			? Duration * static_cast<float>(i) / static_cast<float>(NumSamples - 1)
			: 0.0f;
		MinVal = FMath::Min(MinVal, Curve->Evaluate(Time));
	}
	return MinVal;
}

float UAnimationCurveEditor::GetCurveMaxValue(
	UAnimSequence* Animation,
	FName CurveName)
{
	if (!Animation || CurveName.IsNone()) return 0.0f;

	const TScriptInterface<IAnimationDataModel>& Model = Animation->GetController().GetModelInterface();
	if (!Model) return 0.0f;

	FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);
	const FFloatCurve* Curve = Model->FindFloatCurve(CurveId);
	if (!Curve) return 0.0f;

	const float SampleRate = 30.0f;
	const float Duration = static_cast<float>(Model->GetNumberOfFrames()) / SampleRate;
	const int32 NumSamples = FMath::Max(2, FMath::CeilToInt(Duration * SampleRate));

	float MaxVal = TNumericLimits<float>::Lowest();
	for (int32 i = 0; i < NumSamples; ++i)
	{
		const float Time = (NumSamples > 1)
			? Duration * static_cast<float>(i) / static_cast<float>(NumSamples - 1)
			: 0.0f;
		MaxVal = FMath::Max(MaxVal, Curve->Evaluate(Time));
	}
	return MaxVal;
}

float UAnimationCurveEditor::GetCurveDuration(
	UAnimSequence* Animation,
	FName CurveName)
{
	if (!Animation || CurveName.IsNone()) return 0.0f;

	const TScriptInterface<IAnimationDataModel>& Model = Animation->GetController().GetModelInterface();
	if (!Model) return 0.0f;
	return static_cast<float>(Model->GetNumberOfFrames()) / 30.0f;
}

UAnimSequence* UAnimationCurveEditor::RenameCurve(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	FName OldCurveName,
	FName NewCurveName)
{
	if (!Animation || OldCurveName.IsNone() || NewCurveName.IsNone()) return Animation;

	TArray<FAnimationCurveKey> OldKeyframes = GetCurveKeyframes(Animation, OldCurveName);

	IAnimationDataController& Controller = Animation->GetController();
	FAnimationCurveIdentifier OldId(OldCurveName, ERawCurveTrackTypes::RCT_Float);
	FAnimationCurveIdentifier NewId(NewCurveName, ERawCurveTrackTypes::RCT_Float);

	Controller.RemoveCurve(OldId, true);
	Controller.AddCurve(NewId);

	for (const FAnimationCurveKey& KF : OldKeyframes)
	{
		FRichCurveKey RichKey;
		RichKey.Time = KF.Time;
		RichKey.Value = KF.Value;
		RichKey.InterpMode = ERichCurveInterpMode::RCIM_Linear;
		Controller.SetCurveKey(NewId, RichKey, false);
	}
	Animation->MarkPackageDirty();
	return Animation;
}

#undef LOCTEXT_NAMESPACE

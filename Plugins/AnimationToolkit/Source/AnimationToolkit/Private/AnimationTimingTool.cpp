#include "AnimationTimingTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "AnimationTimingTool"

static UAnimSequence* CreateTimingAnim(
	UAnimSequence* Source,
	const TCHAR* Suffix,
	const TArray<FBoneAnimationTrack>& NewTracks,
	float NewDuration)
{
	if (!Source)
	{
		return nullptr;
	}

	USkeleton* Skeleton = Source->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	const FString NewName = Source->GetName() + Suffix;
	const FString PackagePath = TEXT("/Game/Animations/") + NewName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UAnimSequence* NewAnim = NewObject<UAnimSequence>(Package, *NewName, RF_Public | RF_Standalone);
	if (!NewAnim)
	{
		return nullptr;
	}

	NewAnim->SetSkeleton(Skeleton);

	IAnimationDataController& Controller = NewAnim->GetController();
	const TScriptInterface<IAnimationDataModel> Model = Controller.GetModelInterface();

	Controller.OpenBracket(LOCTEXT("CreateTimingAnim", "Creating Timing Animation"), false);

	const FFrameRate FrameRate = Model->GetFrameRate();
	const int32 NumFrames = FMath::Max(1, FMath::CeilToInt(NewDuration * FrameRate.AsDecimal()));
	Controller.SetNumberOfFrames(FFrameNumber(NumFrames), false);

	for (const FBoneAnimationTrack& SrcTrack : NewTracks)
	{
		Controller.AddBoneCurve(SrcTrack.Name, false);
		Controller.SetBoneTrackKeys(
			SrcTrack.Name,
			SrcTrack.InternalTrackData.PosKeys,
			SrcTrack.InternalTrackData.RotKeys,
			SrcTrack.InternalTrackData.ScaleKeys,
			false);
	}

	Controller.CloseBracket(false);

	NewAnim->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewAnim);

	UE_LOG(LogTemp, Log, TEXT("CreateTimingAnim: Created '%s' duration=%.3f tracks=%d"),
		*NewName, NewDuration, NewTracks.Num());

	return NewAnim;
}

static void ResampleTrack(
	const FBoneAnimationTrack& SrcTrack,
	FBoneAnimationTrack& DstTrack,
	int32 SrcNumFrames,
	int32 DstNumFrames)
{
	DstTrack.Name = SrcTrack.Name;
	DstTrack.BoneTreeIndex = SrcTrack.BoneTreeIndex;

	const FRawAnimSequenceTrack& Src = SrcTrack.InternalTrackData;
	FRawAnimSequenceTrack& Dst = DstTrack.InternalTrackData;

	if (SrcNumFrames <= 0 || DstNumFrames <= 0)
	{
		return;
	}

	Dst.PosKeys.SetNum(DstNumFrames);
	Dst.RotKeys.SetNum(DstNumFrames);
	Dst.ScaleKeys.SetNum(DstNumFrames);

	for (int32 DstFrame = 0; DstFrame < DstNumFrames; DstFrame++)
	{
		const float NormalizedTime = (DstNumFrames > 1) ? static_cast<float>(DstFrame) / static_cast<float>(DstNumFrames - 1) : 0.0f;
		const float SrcFloat = NormalizedTime * static_cast<float>(SrcNumFrames - 1);
		const int32 SrcFrameA = FMath::Clamp(FMath::FloorToInt(SrcFloat), 0, SrcNumFrames - 1);
		const int32 SrcFrameB = FMath::Clamp(FMath::CeilToInt(SrcFloat), 0, SrcNumFrames - 1);
		const float Alpha = SrcFloat - static_cast<float>(SrcFrameA);

		const FVector3f& PosA = (SrcFrameA < Src.PosKeys.Num()) ? Src.PosKeys[SrcFrameA] : FVector3f::ZeroVector;
		const FVector3f& PosB = (SrcFrameB < Src.PosKeys.Num()) ? Src.PosKeys[SrcFrameB] : FVector3f::ZeroVector;
		Dst.PosKeys[DstFrame] = FMath::Lerp(PosA, PosB, Alpha);

		const FQuat4f& RotA = (SrcFrameA < Src.RotKeys.Num()) ? Src.RotKeys[SrcFrameA] : FQuat4f::Identity;
		const FQuat4f& RotB = (SrcFrameB < Src.RotKeys.Num()) ? Src.RotKeys[SrcFrameB] : FQuat4f::Identity;
		Dst.RotKeys[DstFrame] = FQuat4f::Slerp(RotA, RotB, Alpha);

		const FVector3f& ScaleA = (SrcFrameA < Src.ScaleKeys.Num()) ? Src.ScaleKeys[SrcFrameA] : FVector3f::OneVector;
		const FVector3f& ScaleB = (SrcFrameB < Src.ScaleKeys.Num()) ? Src.ScaleKeys[SrcFrameB] : FVector3f::OneVector;
		Dst.ScaleKeys[DstFrame] = FMath::Lerp(ScaleA, ScaleB, Alpha);
	}
}

UAnimSequence* UAnimationTimingTool::ChangePlayRate(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float PlayRate)
{
	if (!SourceAnimation || PlayRate <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChangePlayRate: Invalid input (anim=%p, rate=%.3f)"),
			SourceAnimation, PlayRate);
		return nullptr;
	}

	const IAnimationDataModel* SourceModel = SourceAnimation->GetDataModel();
	if (!SourceModel)
	{
		return nullptr;
	}

	const float OriginalDuration = SourceAnimation->GetPlayLength();
	const float NewDuration = OriginalDuration / PlayRate;
	const int32 SrcNumFrames = SourceModel->GetNumberOfFrames();
	const int32 DstNumFrames = FMath::Max(1, FMath::CeilToInt(NewDuration * SourceModel->GetFrameRate().AsDecimal()));

	TArray<FBoneAnimationTrack> NewTracks;
	const TArray<FBoneAnimationTrack>& SrcTracks = SourceModel->GetBoneAnimationTracks();
	NewTracks.SetNum(SrcTracks.Num());

	for (int32 i = 0; i < SrcTracks.Num(); i++)
	{
		ResampleTrack(SrcTracks[i], NewTracks[i], SrcNumFrames, DstNumFrames);
	}

	UE_LOG(LogTemp, Log, TEXT("ChangePlayRate: %.2f -> %.2f (rate=%.2f)"),
		OriginalDuration, NewDuration, PlayRate);

	return CreateTimingAnim(SourceAnimation, TEXT("_RateChanged"), NewTracks, NewDuration);
}

UAnimSequence* UAnimationTimingTool::ReverseAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation)
{
	if (!SourceAnimation)
	{
		return nullptr;
	}

	const IAnimationDataModel* SourceModel = SourceAnimation->GetDataModel();
	if (!SourceModel)
	{
		return nullptr;
	}

	const float Duration = SourceAnimation->GetPlayLength();
	const int32 NumFrames = SourceModel->GetNumberOfFrames();
	const TArray<FBoneAnimationTrack>& SrcTracks = SourceModel->GetBoneAnimationTracks();

	TArray<FBoneAnimationTrack> NewTracks;
	NewTracks.SetNum(SrcTracks.Num());

	for (int32 i = 0; i < SrcTracks.Num(); i++)
	{
		const FBoneAnimationTrack& SrcTrack = SrcTracks[i];
		FBoneAnimationTrack& DstTrack = NewTracks[i];
		DstTrack.Name = SrcTrack.Name;
		DstTrack.BoneTreeIndex = SrcTrack.BoneTreeIndex;

		const FRawAnimSequenceTrack& Src = SrcTrack.InternalTrackData;
		FRawAnimSequenceTrack& Dst = DstTrack.InternalTrackData;

		const int32 NumPos = Src.PosKeys.Num();
		Dst.PosKeys.SetNum(NumPos);
		for (int32 j = 0; j < NumPos; j++)
		{
			Dst.PosKeys[j] = Src.PosKeys[NumPos - 1 - j];
		}

		const int32 NumRot = Src.RotKeys.Num();
		Dst.RotKeys.SetNum(NumRot);
		for (int32 j = 0; j < NumRot; j++)
		{
			Dst.RotKeys[j] = Src.RotKeys[NumRot - 1 - j];
		}

		const int32 NumScale = Src.ScaleKeys.Num();
		Dst.ScaleKeys.SetNum(NumScale);
		for (int32 j = 0; j < NumScale; j++)
		{
			Dst.ScaleKeys[j] = Src.ScaleKeys[NumScale - 1 - j];
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ReverseAnimation: Duration=%.3f, tracks=%d"),
		Duration, NewTracks.Num());

	return CreateTimingAnim(SourceAnimation, TEXT("_Reversed"), NewTracks, Duration);
}

UAnimSequence* UAnimationTimingTool::TimeWarp(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float StartTime,
	float EndTime)
{
	if (!SourceAnimation || StartTime >= EndTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeWarp: Invalid input (start=%.3f, end=%.3f)"),
			StartTime, EndTime);
		return nullptr;
	}

	const IAnimationDataModel* SourceModel = SourceAnimation->GetDataModel();
	if (!SourceModel)
	{
		return nullptr;
	}

	const float OriginalDuration = SourceAnimation->GetPlayLength();
	StartTime = FMath::Clamp(StartTime, 0.0f, OriginalDuration);
	EndTime = FMath::Clamp(EndTime, 0.0f, OriginalDuration);

	if (StartTime >= EndTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeWarp: Clamped range is empty"));
		return nullptr;
	}

	const float NewDuration = EndTime - StartTime;
	const FFrameRate FrameRate = SourceModel->GetFrameRate();
	const int32 SrcNumFrames = SourceModel->GetNumberOfFrames();
	const int32 DstNumFrames = FMath::Max(1, FMath::CeilToInt(NewDuration * FrameRate.AsDecimal()));

	const int32 SrcStartFrame = FMath::Clamp(FMath::FloorToInt(StartTime * FrameRate.AsDecimal()), 0, SrcNumFrames - 1);
	const int32 SrcEndFrame = FMath::Clamp(FMath::CeilToInt(EndTime * FrameRate.AsDecimal()), 0, SrcNumFrames - 1);
	const int32 SrcRangeFrames = FMath::Max(1, SrcEndFrame - SrcStartFrame);

	const TArray<FBoneAnimationTrack>& SrcTracks = SourceModel->GetBoneAnimationTracks();
	TArray<FBoneAnimationTrack> NewTracks;
	NewTracks.SetNum(SrcTracks.Num());

	for (int32 i = 0; i < SrcTracks.Num(); i++)
	{
		const FBoneAnimationTrack& SrcTrack = SrcTracks[i];
		FBoneAnimationTrack& DstTrack = NewTracks[i];
		DstTrack.Name = SrcTrack.Name;
		DstTrack.BoneTreeIndex = SrcTrack.BoneTreeIndex;

		const FRawAnimSequenceTrack& Src = SrcTrack.InternalTrackData;
		FRawAnimSequenceTrack& Dst = DstTrack.InternalTrackData;

		Dst.PosKeys.SetNum(DstNumFrames);
		Dst.RotKeys.SetNum(DstNumFrames);
		Dst.ScaleKeys.SetNum(DstNumFrames);

		for (int32 DstFrame = 0; DstFrame < DstNumFrames; DstFrame++)
		{
			const float NormalizedTime = (DstNumFrames > 1) ? static_cast<float>(DstFrame) / static_cast<float>(DstNumFrames - 1) : 0.0f;
			const float SrcFloat = static_cast<float>(SrcStartFrame) + NormalizedTime * static_cast<float>(SrcRangeFrames);
			const int32 SrcFrameA = FMath::Clamp(FMath::FloorToInt(SrcFloat), 0, SrcNumFrames - 1);
			const int32 SrcFrameB = FMath::Clamp(FMath::CeilToInt(SrcFloat), 0, SrcNumFrames - 1);
			const float Alpha = SrcFloat - static_cast<float>(SrcFrameA);

			const FVector3f& PosA = (SrcFrameA < Src.PosKeys.Num()) ? Src.PosKeys[SrcFrameA] : FVector3f::ZeroVector;
			const FVector3f& PosB = (SrcFrameB < Src.PosKeys.Num()) ? Src.PosKeys[SrcFrameB] : FVector3f::ZeroVector;
			Dst.PosKeys[DstFrame] = FMath::Lerp(PosA, PosB, Alpha);

			const FQuat4f& RotA = (SrcFrameA < Src.RotKeys.Num()) ? Src.RotKeys[SrcFrameA] : FQuat4f::Identity;
			const FQuat4f& RotB = (SrcFrameB < Src.RotKeys.Num()) ? Src.RotKeys[SrcFrameB] : FQuat4f::Identity;
			Dst.RotKeys[DstFrame] = FQuat4f::Slerp(RotA, RotB, Alpha);

			const FVector3f& ScaleA = (SrcFrameA < Src.ScaleKeys.Num()) ? Src.ScaleKeys[SrcFrameA] : FVector3f::OneVector;
			const FVector3f& ScaleB = (SrcFrameB < Src.ScaleKeys.Num()) ? Src.ScaleKeys[SrcFrameB] : FVector3f::OneVector;
			Dst.ScaleKeys[DstFrame] = FMath::Lerp(ScaleA, ScaleB, Alpha);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TimeWarp: [%.3f, %.3f] -> [0, %.3f] (original=%.3f)"),
		StartTime, EndTime, NewDuration, OriginalDuration);

	return CreateTimingAnim(SourceAnimation, TEXT("_TimeWarped"), NewTracks, NewDuration);
}

UAnimSequence* UAnimationTimingTool::BlendTiming(
	UObject* WorldContextObject,
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	float BlendFactor)
{
	if (!AnimationA || !AnimationB)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlendTiming: Null animation (A=%p, B=%p)"),
			AnimationA, AnimationB);
		return nullptr;
	}

	const IAnimationDataModel* ModelA = AnimationA->GetDataModel();
	const IAnimationDataModel* ModelB = AnimationB->GetDataModel();
	if (!ModelA || !ModelB)
	{
		return nullptr;
	}

	BlendFactor = FMath::Clamp(BlendFactor, 0.0f, 1.0f);

	const float DurationA = AnimationA->GetPlayLength();
	const float DurationB = AnimationB->GetPlayLength();
	const float BlendedDuration = FMath::Lerp(DurationA, DurationB, BlendFactor);

	const TArray<FBoneAnimationTrack>& TracksA = ModelA->GetBoneAnimationTracks();
	const TArray<FBoneAnimationTrack>& TracksB = ModelB->GetBoneAnimationTracks();

	const int32 NumTracks = FMath::Max(TracksA.Num(), TracksB.Num());
	TArray<FBoneAnimationTrack> NewTracks;
	NewTracks.SetNum(NumTracks);

	const FFrameRate FrameRate = ModelA->GetFrameRate();
	const int32 DstNumFrames = FMath::Max(1, FMath::CeilToInt(BlendedDuration * FrameRate.AsDecimal()));

	const int32 SrcNumFramesA = ModelA->GetNumberOfFrames();
	const int32 SrcNumFramesB = ModelB->GetNumberOfFrames();

	for (int32 i = 0; i < NumTracks; i++)
	{
		const bool bHasA = (i < TracksA.Num());
		const bool bHasB = (i < TracksB.Num());

		FBoneAnimationTrack& DstTrack = NewTracks[i];
		DstTrack.Name = bHasA ? TracksA[i].Name : TracksB[i].Name;
		DstTrack.BoneTreeIndex = bHasA ? TracksA[i].BoneTreeIndex : TracksB[i].BoneTreeIndex;

		FRawAnimSequenceTrack& Dst = DstTrack.InternalTrackData;

		const FRawAnimSequenceTrack& SrcA = bHasA ? TracksA[i].InternalTrackData : TracksB[i].InternalTrackData;
		const FRawAnimSequenceTrack& SrcB = bHasB ? TracksB[i].InternalTrackData : TracksA[i].InternalTrackData;
		const int32 SrcFramesA = bHasA ? SrcNumFramesA : SrcNumFramesB;
		const int32 SrcFramesB = bHasB ? SrcNumFramesB : SrcNumFramesA;

		Dst.PosKeys.SetNum(DstNumFrames);
		Dst.RotKeys.SetNum(DstNumFrames);
		Dst.ScaleKeys.SetNum(DstNumFrames);

		for (int32 DstFrame = 0; DstFrame < DstNumFrames; DstFrame++)
		{
			const float NormalizedTime = (DstNumFrames > 1) ? static_cast<float>(DstFrame) / static_cast<float>(DstNumFrames - 1) : 0.0f;

			const float SrcFloatA = NormalizedTime * static_cast<float>(SrcFramesA - 1);
			const int32 SrcFrameAA = FMath::Clamp(FMath::FloorToInt(SrcFloatA), 0, SrcFramesA - 1);
			const int32 SrcFrameAB = FMath::Clamp(FMath::CeilToInt(SrcFloatA), 0, SrcFramesA - 1);
			const float AlphaA = SrcFloatA - static_cast<float>(SrcFrameAA);

			const FVector3f PosA = FMath::Lerp(
				(SrcFrameAA < SrcA.PosKeys.Num()) ? SrcA.PosKeys[SrcFrameAA] : FVector3f::ZeroVector,
				(SrcFrameAB < SrcA.PosKeys.Num()) ? SrcA.PosKeys[SrcFrameAB] : FVector3f::ZeroVector,
				AlphaA);
			const FQuat4f RotA = FQuat4f::Slerp(
				(SrcFrameAA < SrcA.RotKeys.Num()) ? SrcA.RotKeys[SrcFrameAA] : FQuat4f::Identity,
				(SrcFrameAB < SrcA.RotKeys.Num()) ? SrcA.RotKeys[SrcFrameAB] : FQuat4f::Identity,
				AlphaA);
			const FVector3f ScaleA = FMath::Lerp(
				(SrcFrameAA < SrcA.ScaleKeys.Num()) ? SrcA.ScaleKeys[SrcFrameAA] : FVector3f::OneVector,
				(SrcFrameAB < SrcA.ScaleKeys.Num()) ? SrcA.ScaleKeys[SrcFrameAB] : FVector3f::OneVector,
				AlphaA);

			const float SrcFloatB = NormalizedTime * static_cast<float>(SrcFramesB - 1);
			const int32 SrcFrameBA = FMath::Clamp(FMath::FloorToInt(SrcFloatB), 0, SrcFramesB - 1);
			const int32 SrcFrameBB = FMath::Clamp(FMath::CeilToInt(SrcFloatB), 0, SrcFramesB - 1);
			const float AlphaB = SrcFloatB - static_cast<float>(SrcFrameBA);

			const FVector3f PosB = FMath::Lerp(
				(SrcFrameBA < SrcB.PosKeys.Num()) ? SrcB.PosKeys[SrcFrameBA] : FVector3f::ZeroVector,
				(SrcFrameBB < SrcB.PosKeys.Num()) ? SrcB.PosKeys[SrcFrameBB] : FVector3f::ZeroVector,
				AlphaB);
			const FQuat4f RotB = FQuat4f::Slerp(
				(SrcFrameBA < SrcB.RotKeys.Num()) ? SrcB.RotKeys[SrcFrameBA] : FQuat4f::Identity,
				(SrcFrameBB < SrcB.RotKeys.Num()) ? SrcB.RotKeys[SrcFrameBB] : FQuat4f::Identity,
				AlphaB);
			const FVector3f ScaleB = FMath::Lerp(
				(SrcFrameBA < SrcB.ScaleKeys.Num()) ? SrcB.ScaleKeys[SrcFrameBA] : FVector3f::OneVector,
				(SrcFrameBB < SrcB.ScaleKeys.Num()) ? SrcB.ScaleKeys[SrcFrameBB] : FVector3f::OneVector,
				AlphaB);

			Dst.PosKeys[DstFrame] = FMath::Lerp(PosA, PosB, BlendFactor);
			Dst.RotKeys[DstFrame] = FQuat4f::Slerp(RotA, RotB, BlendFactor);
			Dst.ScaleKeys[DstFrame] = FMath::Lerp(ScaleA, ScaleB, BlendFactor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("BlendTiming: durA=%.3f durB=%.3f blend=%.2f -> dur=%.3f"),
		DurationA, DurationB, BlendFactor, BlendedDuration);

	return CreateTimingAnim(AnimationA, TEXT("_Blended"), NewTracks, BlendedDuration);
}

float UAnimationTimingTool::GetAnimationSpeed(
	UAnimSequence* Animation,
	float TargetDuration)
{
	if (!Animation || TargetDuration <= 0.0f)
	{
		return 0.0f;
	}

	const float CurrentDuration = Animation->GetPlayLength();
	return (CurrentDuration > 0.0f) ? (CurrentDuration / TargetDuration) : 0.0f;
}

float UAnimationTimingTool::GetAnimationDuration(UAnimSequence* Animation)
{
	return Animation ? Animation->GetPlayLength() : 0.0f;
}

int32 UAnimationTimingTool::GetFrameCount(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0;
	}

	const IAnimationDataModel* Model = Animation->GetDataModel();
	return Model ? Model->GetNumberOfFrames() : 0;
}

float UAnimationTimingTool::TimeToFrame(UAnimSequence* Animation, float Time)
{
	if (!Animation)
	{
		return 0.0f;
	}

	const float Duration = Animation->GetPlayLength();
	const IAnimationDataModel* Model = Animation->GetDataModel();
	const int32 FrameCount = Model ? Model->GetNumberOfFrames() : 0;

	if (Duration <= 0.0f || FrameCount <= 0)
	{
		return 0.0f;
	}

	return (FMath::Clamp(Time, 0.0f, Duration) / Duration) * static_cast<float>(FrameCount - 1);
}

float UAnimationTimingTool::FrameToTime(UAnimSequence* Animation, int32 Frame)
{
	if (!Animation)
	{
		return 0.0f;
	}

	const float Duration = Animation->GetPlayLength();
	const IAnimationDataModel* Model = Animation->GetDataModel();
	const int32 FrameCount = Model ? Model->GetNumberOfFrames() : 0;

	if (Duration <= 0.0f || FrameCount <= 1)
	{
		return 0.0f;
	}

	return (static_cast<float>(FMath::Clamp(Frame, 0, FrameCount - 1))
		/ static_cast<float>(FrameCount - 1)) * Duration;
}

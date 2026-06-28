#include "AnimSequenceCreator.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetRegistryModule.h"

UAnimSequence* UAnimSequenceCreator::CreateAnimSequence(
	UObject* WorldContextObject,
	const FString& SequenceName,
	USkeleton* Skeleton,
	const FAnimationClipData& ClipData,
	UPackage* Package)
{
	if (!Skeleton || SequenceName.IsEmpty())
	{
		return nullptr;
	}

	if (!Package)
	{
		const FString PackagePath = TEXT("/Game/Animations/") + SequenceName;
		Package = CreatePackage(*PackagePath);
	}

	if (!Package)
	{
		return nullptr;
	}

	UAnimSequence* AnimSequence = NewObject<UAnimSequence>(Package, *SequenceName, RF_Public | RF_Standalone);
	if (!AnimSequence)
	{
		return nullptr;
	}

	AnimSequence->SetSkeleton(Skeleton);

	IAnimationDataController& Controller = AnimSequence->GetController();

	const float SafeFrameRate = FMath::Max(0.01f, ClipData.FrameRate);

	for (const FAnimationTrackData& Track : ClipData.Tracks)
	{
		Controller.AddBoneCurve(Track.TrackName, false);
		FRawAnimSequenceTrack RawTrack = ConvertTrackData(Track, SafeFrameRate, ClipData.Duration);
		Controller.SetBoneTrackKeys(Track.TrackName, RawTrack.PosKeys, RawTrack.RotKeys, RawTrack.ScaleKeys, false);
	}

	AnimSequence->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(AnimSequence);

	return AnimSequence;
}

UAnimSequence* UAnimSequenceCreator::CreateAnimSequenceFromKeyframes(
	UObject* WorldContextObject,
	const FString& SequenceName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	float Duration,
	float FrameRate,
	UPackage* Package)
{
	if (!Skeleton || SequenceName.IsEmpty())
	{
		return nullptr;
	}

	FAnimationClipData ClipData;
	ClipData.ClipName = SequenceName;
	ClipData.Duration = Duration;
	ClipData.FrameRate = FrameRate;
	ClipData.Tracks = Tracks;

	return CreateAnimSequence(WorldContextObject, SequenceName, Skeleton, ClipData, Package);
}

FAnimationClipData UAnimSequenceCreator::CreateClipData(
	const FString& ClipName,
	float Duration,
	float FrameRate,
	const TArray<FAnimationTrackData>& Tracks)
{
	FAnimationClipData ClipData;
	ClipData.ClipName = ClipName;
	ClipData.Duration = Duration;
	ClipData.FrameRate = FrameRate;
	ClipData.Tracks = Tracks;
	return ClipData;
}

FAnimationTrackData UAnimSequenceCreator::CreateTrackData(
	const FName& TrackName,
	const TArray<FAnimationKeyframe>& Keyframes)
{
	FAnimationTrackData TrackData;
	TrackData.TrackName = TrackName;
	TrackData.Keyframes = Keyframes;
	return TrackData;
}

FAnimationKeyframe UAnimSequenceCreator::CreateKeyframe(
	float Time,
	const FTransform& Transform)
{
	FAnimationKeyframe Keyframe;
	Keyframe.Time = Time;
	Keyframe.Transform = Transform;
	return Keyframe;
}

void UAnimSequenceCreator::ApplyTracksToAnimSequence(
	UAnimSequence* AnimSequence,
	const TArray<FAnimationTrackData>& Tracks,
	USkeleton* Skeleton,
	float FrameRate)
{
	if (!AnimSequence || !Skeleton)
	{
		return;
	}

	const float SafeFrameRate = FMath::Max(0.01f, FrameRate);
	const float Duration = AnimSequence->GetPlayLength();

	IAnimationDataController& Controller = AnimSequence->GetController();

	for (const FAnimationTrackData& Track : Tracks)
	{
		Controller.AddBoneCurve(Track.TrackName, false);
		FRawAnimSequenceTrack RawTrack = ConvertTrackData(Track, SafeFrameRate, Duration);
		Controller.SetBoneTrackKeys(Track.TrackName, RawTrack.PosKeys, RawTrack.RotKeys, RawTrack.ScaleKeys, false);
		UE_LOG(LogTemp, Log, TEXT("ApplyTracksToAnimSequence: Applied Track '%s' with %d keyframes"),
			*Track.TrackName.ToString(), Track.Keyframes.Num());
	}

	AnimSequence->MarkPackageDirty();
}

FRawAnimSequenceTrack UAnimSequenceCreator::ConvertTrackData(
	const FAnimationTrackData& TrackData,
	float FrameRate,
	float Duration)
{
	FRawAnimSequenceTrack RawTrack;
	const int32 NumFrames = FMath::Max(1, FMath::RoundToInt(Duration * FrameRate));

	RawTrack.PosKeys.SetNum(NumFrames);
	RawTrack.RotKeys.SetNum(NumFrames);
	RawTrack.ScaleKeys.SetNum(NumFrames);

	for (int32 i = 0; i < NumFrames; ++i)
	{
		const float Time = (FrameRate > 0.0f) ? (static_cast<float>(i) / FrameRate) : 0.0f;

		FTransform InterpolatedTransform = FTransform::Identity;
		if (TrackData.Keyframes.Num() > 0)
		{
			if (TrackData.Keyframes.Num() == 1)
			{
				InterpolatedTransform = TrackData.Keyframes[0].Transform;
			}
			else
			{
				const float SafeDuration = FMath::Max(0.001f, Duration);
				const float NormalizedTime = FMath::Clamp(Time / SafeDuration, 0.0f, 1.0f);
				const float ScaledTime = NormalizedTime * (TrackData.Keyframes.Num() - 1);
				const int32 IndexA = FMath::FloorToInt(ScaledTime);
				const int32 IndexB = FMath::Min(IndexA + 1, TrackData.Keyframes.Num() - 1);
				const float Alpha = ScaledTime - static_cast<float>(IndexA);

				InterpolatedTransform = TrackData.Keyframes[IndexA].Transform;
				InterpolatedTransform.BlendWith(TrackData.Keyframes[IndexB].Transform, Alpha);
			}
		}

		RawTrack.PosKeys[i] = FVector3f(InterpolatedTransform.GetTranslation());
		RawTrack.RotKeys[i] = FQuat4f(InterpolatedTransform.GetRotation());
		RawTrack.ScaleKeys[i] = FVector3f(InterpolatedTransform.GetScale3D());
	}

	return RawTrack;
}

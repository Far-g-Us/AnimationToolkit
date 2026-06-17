#include "AnimSequenceCreator.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetData.h"

UAnimSequence* UAnimSequenceCreator::CreateAnimSequence(
	UObject* WorldContextObject,
	const FString& SequenceName,
	USkeleton* Skeleton,
	const FAnimationClipData& ClipData,
	UPackage* Package)
{
	return nullptr;
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
	return nullptr;
}

FAnimationClipData UAnimSequenceCreator::CreateClipData(
	const FString& ClipName,
	float Duration,
	float FrameRate,
	const TArray<FAnimationTrackData>& Tracks)
{
	FAnimationClipData ClipData;
	return ClipData;
}

FAnimationTrackData UAnimSequenceCreator::CreateTrackData(
	const FName& TrackName,
	const TArray<FAnimationKeyframe>& Keyframes)
{
	FAnimationTrackData TrackData;
	return TrackData;
}

FAnimationKeyframe UAnimSequenceCreator::CreateKeyframe(
	float Time,
	const FTransform& Transform)
{
	FAnimationKeyframe Keyframe;
	return Keyframe;
}

void UAnimSequenceCreator::ApplyTracksToAnimSequence(
	UAnimSequence* AnimSequence,
	const TArray<FAnimationTrackData>& Tracks,
	USkeleton* Skeleton,
	float FrameRate)
{
}

FRawAnimSequenceTrack UAnimSequenceCreator::ConvertTrackData(
	const FAnimationTrackData& TrackData,
	float FrameRate,
	float Duration)
{
	FRawAnimSequenceTrack RawTrack;
	return RawTrack;
}

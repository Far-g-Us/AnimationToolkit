#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimSequence.h"
#include "AnimationData.h"
#include "AnimSequenceCreator.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimSequenceCreator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|AnimSequence", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CreateAnimSequence(
		UObject* WorldContextObject,
		const FString& SequenceName,
		USkeleton* Skeleton,
		const FAnimationClipData& ClipData,
		UPackage* Package = nullptr
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|AnimSequence", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CreateAnimSequenceFromKeyframes(
		UObject* WorldContextObject,
		const FString& SequenceName,
		USkeleton* Skeleton,
		const TArray<FAnimationTrackData>& Tracks,
		float Duration,
		float FrameRate,
		UPackage* Package = nullptr
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|AnimSequence")
	static FAnimationClipData CreateClipData(
		const FString& ClipName,
		float Duration,
		float FrameRate,
		const TArray<FAnimationTrackData>& Tracks
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|AnimSequence")
	static FAnimationTrackData CreateTrackData(
		const FName& TrackName,
		const TArray<FAnimationKeyframe>& Keyframes
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|AnimSequence")
	static FAnimationKeyframe CreateKeyframe(
		float Time,
		const FTransform& Transform
	);

private:
	static void ApplyTracksToAnimSequence(
		UAnimSequence* AnimSequence,
		const TArray<FAnimationTrackData>& Tracks,
		USkeleton* Skeleton,
		float FrameRate
	);

	static FRawAnimSequenceTrack ConvertTrackData(
		const FAnimationTrackData& TrackData,
		float FrameRate,
		float Duration
	);
};

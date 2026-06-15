#include "AnimSequenceCreator.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"
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
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimSequenceCreator: Skeleton is null"));
		return nullptr;
	}

	if (ClipData.Tracks.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimSequenceCreator: No tracks provided"));
		return nullptr;
	}

	FString AssetPath;
	if (Package)
	{
		AssetPath = FString::Printf(TEXT("/Game/Animations/%s"), *SequenceName);
	}
	else
	{
		AssetPath = FString::Printf(TEXT("/Game/Animations/%s"), *SequenceName);
		Package = CreatePackage(nullptr, *AssetPath);
	}

	UAnimSequence* AnimSequence = NewObject<UAnimSequence>(
		Package,
		*SequenceName,
		RF_Public | RF_Standalone | RF_MarkAsRootSet
	);

	if (!AnimSequence)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimSequenceCreator: Failed to create AnimSequence"));
		return nullptr;
	}

	AnimSequence->SetSkeleton(Skeleton);
	AnimSequence->SetRawNumberOfFrames(FMath::Max(1, FMath::CeilToInt(ClipData.Duration * ClipData.FrameRate)));
	AnimSequence->SetRawCurveData(TMap<FRawCurveTracks::CurveType, FSmartName>());
	AnimSequence->SetSamplingFrameRate(FFrameRate(ClipData.FrameRate, 1));
	AnimSequence->SetNumberOfSampledKeys();

	ApplyTracksToAnimSequence(AnimSequence, ClipData.Tracks, Skeleton, ClipData.FrameRate);

	AnimSequence->MarkPackageDirty();
	AnimSequence->PostEditChange();

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

	const float Duration = AnimSequence->GetSequenceLength();
	const int32 NumFrames = AnimSequence->GetNumberOfSampledKeys();

	for (const FAnimationTrackData& TrackData : Tracks)
	{
		int32 BoneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(TrackData.TrackName);
		if (BoneIndex == INDEX_NONE)
		{
			continue;
		}

		FRawAnimSequenceTrack RawTrack = ConvertTrackData(TrackData, FrameRate, Duration);

		AnimSequence->AddTrack(TrackData.TrackName);
		AnimSequence->SetRawTrack(TrackData.TrackName, &RawTrack);
	}
}

FRawAnimSequenceTrack UAnimSequenceCreator::ConvertTrackData(
	const FAnimationTrackData& TrackData,
	float FrameRate,
	float Duration)
{
	FRawAnimSequenceTrack RawTrack;
	const int32 NumKeys = FMath::Max(1, FMath::CeilToInt(Duration * FrameRate));

	RawTrack.PosKeys.Reserve(NumKeys);
	RawTrack.RotKeys.Reserve(NumKeys);
	RawTrack.ScaleKeys.Reserve(NumKeys);

	for (const FAnimationKeyframe& Keyframe : TrackData.Keyframes)
	{
		const float Alpha = FMath::Clamp(Keyframe.Time / Duration, 0.0f, 1.0f);
		const int32 FrameIndex = FMath::Clamp(
			FMath::CeilToInt(Keyframe.Time * FrameRate),
			0,
			NumKeys - 1
		);

		while (RawTrack.PosKeys.Num() <= FrameIndex)
		{
			int32 LastIndex = FMath::Max(0, RawTrack.PosKeys.Num() - 1);
			FVector Position = (RawTrack.PosKeys.Num() > 0) ? RawTrack.PosKeys[LastIndex] : FVector::ZeroVector;
			RawTrack.PosKeys.Add(FVector(Position));
		}

		while (RawTrack.RotKeys.Num() <= FrameIndex)
		{
			int32 LastIndex = FMath::Max(0, RawTrack.RotKeys.Num() - 1);
			FQuat Rotation = (RawTrack.RotKeys.Num() > 0) ? RawTrack.RotKeys[LastIndex] : FQuat::Identity;
			RawTrack.RotKeys.Add(FQuat(Rotation));
		}

		while (RawTrack.ScaleKeys.Num() <= FrameIndex)
		{
			int32 LastIndex = FMath::Max(0, RawTrack.ScaleKeys.Num() - 1);
			FVector Scale = (RawTrack.ScaleKeys.Num() > 0) ? RawTrack.ScaleKeys[LastIndex] : FVector::OneVector;
			RawTrack.ScaleKeys.Add(FVector(Scale));
		}

		RawTrack.PosKeys[FrameIndex] = Keyframe.Transform.GetLocation();
		RawTrack.RotKeys[FrameIndex] = Keyframe.Transform.GetRotation();
		RawTrack.ScaleKeys[FrameIndex] = Keyframe.Transform.GetScale3D();
	}

	if (RawTrack.PosKeys.Num() == 0)
	{
		RawTrack.PosKeys.Add(FVector::ZeroVector);
	}
	if (RawTrack.RotKeys.Num() == 0)
	{
		RawTrack.RotKeys.Add(FQuat::Identity);
	}
	if (RawTrack.ScaleKeys.Num() == 0)
	{
		RawTrack.ScaleKeys.Add(FVector::OneVector);
	}

	return RawTrack;
}

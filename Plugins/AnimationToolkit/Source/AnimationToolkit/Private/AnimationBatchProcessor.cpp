#include "AnimationBatchProcessor.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimData/IAnimationDataController.h"

static int32 GBatchProgress = 0;
static FString GBatchStatus = TEXT("Idle");

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchProcessAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	const FBatchProcessSettings& Settings)
{
	TArray<UAnimSequence*> Results;

	switch (Settings.Operation)
	{
	case EBatchOperation::Compress:
		return BatchCompress(WorldContextObject, Animations);
	case EBatchOperation::Optimize:
		return BatchOptimize(WorldContextObject, Animations);
	case EBatchOperation::Retarget:
		return BatchRetarget(WorldContextObject, Animations, Settings.TargetSkeleton);
	case EBatchOperation::Mirror:
		return BatchMirror(WorldContextObject, Animations, Settings.MirrorFlags);
	case EBatchOperation::Reverse:
		return BatchReverse(WorldContextObject, Animations);
	case EBatchOperation::ChangePlayRate:
		return BatchChangePlayRate(WorldContextObject, Animations, Settings.Parameter);
	}

	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchCompress(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations)
{
	TArray<UAnimSequence*> Results;

	GBatchProgress = 0;
	GBatchStatus = TEXT("Compressing...");

	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		UAnimSequence* Animation = Animations[i];
		if (!Animation)
		{
			continue;
		}

		Animation->RequestAnimCompression(true);
		Results.Add(Animation);

		GBatchProgress = ((i + 1) * 100) / Animations.Num();
	}

	GBatchStatus = TEXT("Idle");
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchOptimize(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations)
{
	TArray<UAnimSequence*> Results;

	GBatchProgress = 0;
	GBatchStatus = TEXT("Optimizing...");

	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		UAnimSequence* Animation = Animations[i];
		if (!Animation)
		{
			continue;
		}

		const IAnimationDataModel* Model = Animation->GetDataModel();
		if (!Model)
		{
			continue;
		}

		IAnimationDataController& Controller = Animation->GetController();

		const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();

		for (const FBoneAnimationTrack& Track : Tracks)
		{
			const FRawAnimSequenceTrack& RawData = Track.InternalTrackData;

			if (RawData.PosKeys.Num() <= 2)
			{
				continue;
			}

			TArray<FVector3f> OptPosKeys;
			TArray<FQuat4f> OptRotKeys;
			TArray<FVector3f> OptScaleKeys;

			OptPosKeys.Add(RawData.PosKeys[0]);
			for (int32 k = 1; k < RawData.PosKeys.Num() - 1; k += 2)
			{
				OptPosKeys.Add(RawData.PosKeys[k]);
			}
			OptPosKeys.Add(RawData.PosKeys.Last());

			OptRotKeys.Add(RawData.RotKeys[0]);
			for (int32 k = 1; k < RawData.RotKeys.Num() - 1; k += 2)
			{
				OptRotKeys.Add(RawData.RotKeys[k]);
			}
			OptRotKeys.Add(RawData.RotKeys.Last());

			if (RawData.ScaleKeys.Num() > 0)
			{
				OptScaleKeys.Add(RawData.ScaleKeys[0]);
				for (int32 k = 1; k < RawData.ScaleKeys.Num() - 1; k += 2)
				{
					OptScaleKeys.Add(RawData.ScaleKeys[k]);
				}
				OptScaleKeys.Add(RawData.ScaleKeys.Last());
			}

			Controller.SetBoneTrackKeys(Track.Name, OptPosKeys, OptRotKeys, OptScaleKeys);
		}

		Results.Add(Animation);

		GBatchProgress = ((i + 1) * 100) / Animations.Num();
	}

	GBatchStatus = TEXT("Idle");
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchRetarget(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	USkeleton* TargetSkeleton)
{
	TArray<UAnimSequence*> Results;

	if (!TargetSkeleton)
	{
		return Results;
	}

	GBatchProgress = 0;
	GBatchStatus = TEXT("Retargeting...");

	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		UAnimSequence* Animation = Animations[i];
		if (!Animation)
		{
			continue;
		}

		const IAnimationDataModel* SourceModel = Animation->GetDataModel();
		if (!SourceModel)
		{
			continue;
		}

		UAnimSequence* NewAnim = DuplicateObject<UAnimSequence>(Animation, Animation->GetOuter());

		IAnimationDataController& Controller = NewAnim->GetController();

		const TArray<FBoneAnimationTrack>& ExistingTracks = SourceModel->GetBoneAnimationTracks();
		for (const FBoneAnimationTrack& Track : ExistingTracks)
		{
			Controller.RemoveBoneTrack(Track.Name);
		}

		const FReferenceSkeleton& TargetRefSkeleton = TargetSkeleton->GetReferenceSkeleton();
		const TArray<FBoneAnimationTrack>& SourceTracks = SourceModel->GetBoneAnimationTracks();

		for (const FBoneAnimationTrack& SourceTrack : SourceTracks)
		{
			if (TargetRefSkeleton.FindBoneIndex(SourceTrack.Name) != INDEX_NONE)
			{
				const FRawAnimSequenceTrack& RawData = SourceTrack.InternalTrackData;
				Controller.SetBoneTrackKeys(
					SourceTrack.Name,
					RawData.PosKeys,
					RawData.RotKeys,
					RawData.ScaleKeys);
			}
		}

		Results.Add(NewAnim);

		GBatchProgress = ((i + 1) * 100) / Animations.Num();
	}

	GBatchStatus = TEXT("Idle");
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchMirror(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	EMirrorFlags MirrorFlags)
{
	TArray<UAnimSequence*> Results;

	GBatchProgress = 0;
	GBatchStatus = TEXT("Mirroring...");

	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		UAnimSequence* Animation = Animations[i];
		if (!Animation)
		{
			continue;
		}

		const IAnimationDataModel* Model = Animation->GetDataModel();
		if (!Model)
		{
			continue;
		}

		IAnimationDataController& Controller = Animation->GetController();

		const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();

		for (const FBoneAnimationTrack& Track : Tracks)
		{
			const FRawAnimSequenceTrack& RawData = Track.InternalTrackData;

			TArray<FVector3f> MirroredPosKeys;
			TArray<FQuat4f> MirroredRotKeys;
			TArray<FVector3f> MirroredScaleKeys;

			MirroredPosKeys.Reserve(RawData.PosKeys.Num());
			MirroredRotKeys.Reserve(RawData.RotKeys.Num());
			MirroredScaleKeys.Reserve(RawData.ScaleKeys.Num());

			for (const FVector3f& PosKey : RawData.PosKeys)
			{
				MirroredPosKeys.Add(FVector3f(-PosKey.X, PosKey.Y, PosKey.Z));
			}

			for (const FQuat4f& RotKey : RawData.RotKeys)
			{
				MirroredRotKeys.Add(FQuat4f(-RotKey.X, RotKey.Y, RotKey.Z, RotKey.W));
			}

			for (const FVector3f& ScaleKey : RawData.ScaleKeys)
			{
				MirroredScaleKeys.Add(FVector3f(ScaleKey.X, ScaleKey.Y, ScaleKey.Z));
			}

			Controller.SetBoneTrackKeys(Track.Name, MirroredPosKeys, MirroredRotKeys, MirroredScaleKeys);
		}

		Results.Add(Animation);

		GBatchProgress = ((i + 1) * 100) / Animations.Num();
	}

	GBatchStatus = TEXT("Idle");
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchReverse(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations)
{
	TArray<UAnimSequence*> Results;

	GBatchProgress = 0;
	GBatchStatus = TEXT("Reversing...");

	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		UAnimSequence* Animation = Animations[i];
		if (!Animation)
		{
			continue;
		}

		const IAnimationDataModel* Model = Animation->GetDataModel();
		if (!Model)
		{
			continue;
		}

		IAnimationDataController& Controller = Animation->GetController();

		const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();

		for (const FBoneAnimationTrack& Track : Tracks)
		{
			const FRawAnimSequenceTrack& RawData = Track.InternalTrackData;

			TArray<FVector3f> ReversedPosKeys = RawData.PosKeys;
			TArray<FQuat4f> ReversedRotKeys = RawData.RotKeys;
			TArray<FVector3f> ReversedScaleKeys = RawData.ScaleKeys;

			for (int32 k = 0; k < ReversedPosKeys.Num() / 2; ++k)
			{
				Swap(ReversedPosKeys[k], ReversedPosKeys[ReversedPosKeys.Num() - 1 - k]);
			}
			for (int32 k = 0; k < ReversedRotKeys.Num() / 2; ++k)
			{
				Swap(ReversedRotKeys[k], ReversedRotKeys[ReversedRotKeys.Num() - 1 - k]);
			}
			for (int32 k = 0; k < ReversedScaleKeys.Num() / 2; ++k)
			{
				Swap(ReversedScaleKeys[k], ReversedScaleKeys[ReversedScaleKeys.Num() - 1 - k]);
			}

			Controller.SetBoneTrackKeys(Track.Name, ReversedPosKeys, ReversedRotKeys, ReversedScaleKeys);
		}

		Results.Add(Animation);

		GBatchProgress = ((i + 1) * 100) / Animations.Num();
	}

	GBatchStatus = TEXT("Idle");
	return Results;
}

TArray<UAnimSequence*> UAnimationBatchProcessor::BatchChangePlayRate(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	float PlayRate)
{
	TArray<UAnimSequence*> Results;

	GBatchProgress = 0;
	GBatchStatus = TEXT("Changing Play Rate...");

	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		UAnimSequence* Animation = Animations[i];
		if (!Animation)
		{
			continue;
		}

		IAnimationDataController& Controller = Animation->GetController();
		const FFrameRate CurrentFrameRate = Animation->GetSamplingFrameRate();
		const float NewRate = CurrentFrameRate.AsDecimal() * PlayRate;
		const FFrameRate NewFrameRate(FMath::Max(1, FMath::RoundToInt32(NewRate)), 1);
		Controller.SetFrameRate(NewFrameRate);
		Results.Add(Animation);

		GBatchProgress = ((i + 1) * 100) / Animations.Num();
	}

	GBatchStatus = TEXT("Idle");
	return Results;
}

int32 UAnimationBatchProcessor::GetBatchProgress()
{
	return GBatchProgress;
}

FString UAnimationBatchProcessor::GetBatchStatus()
{
	return GBatchStatus;
}

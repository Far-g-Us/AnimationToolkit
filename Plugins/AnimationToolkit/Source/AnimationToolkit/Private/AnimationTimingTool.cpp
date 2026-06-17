#include "AnimationTimingTool.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

UAnimSequence* UAnimationTimingTool::ChangePlayRate(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float PlayRate)
{
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	FString NewName = FString::Printf(TEXT("%s_Rate%.2f"), *SourceAnimation->GetName(), PlayRate);

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);
		int32 SourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);

		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		if (SourceBoneIdx == INDEX_NONE)
		{
			continue;
		}

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			float SourceTime = (float)Frame / (float)FMath::Max(FrameCount - 1, 1);
			float NewTime = SourceTime * PlayRate;
			NewTime = FMath::Clamp(NewTime, 0.0f, 1.0f);

			FTransform SourceTransform = SourceAnimation->GetBoneTransform(SourceBoneIdx, NewTime);

			DestTrack.PosKeys[Frame] = SourceTransform.GetLocation();
			DestTrack.RotKeys[Frame] = SourceTransform.GetRotation();
			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
}

UAnimSequence* UAnimationTimingTool::ReverseAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation)
{
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	FString NewName = FString::Printf(TEXT("%s_Reversed"), *SourceAnimation->GetName());

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);
		int32 SourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);

		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		if (SourceBoneIdx == INDEX_NONE)
		{
			continue;
		}

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			int32 ReversedFrame = FrameCount - 1 - Frame;
			FTransform SourceTransform = SourceAnimation->GetBoneTransform(SourceBoneIdx, ReversedFrame);

			DestTrack.PosKeys[Frame] = SourceTransform.GetLocation();
			DestTrack.RotKeys[Frame] = SourceTransform.GetRotation();
			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
}

UAnimSequence* UAnimationTimingTool::TimeWarp(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float StartTime,
	float EndTime)
{
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	FString NewName = FString::Printf(TEXT("%s_Warp"), *SourceAnimation->GetName());

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);
		int32 SourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);

		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		if (SourceBoneIdx == INDEX_NONE)
		{
			continue;
		}

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			float NormalizedTime = (float)Frame / (float)FMath::Max(FrameCount - 1, 1);
			float WarpedTime = FMath::Lerp(StartTime, EndTime, NormalizedTime);
			WarpedTime = FMath::Clamp(WarpedTime, 0.0f, 1.0f);

			FTransform SourceTransform = SourceAnimation->GetBoneTransform(SourceBoneIdx, WarpedTime);

			DestTrack.PosKeys[Frame] = SourceTransform.GetLocation();
			DestTrack.RotKeys[Frame] = SourceTransform.GetRotation();
			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
}

UAnimSequence* UAnimationTimingTool::BlendTiming(
	UObject* WorldContextObject,
	UAnimSequence* AnimationA,
	UAnimSequence* AnimationB,
	float BlendFactor)
{
	if (!WorldContextObject || !AnimationA || !AnimationB)
	{
		return nullptr;
	}

	return AnimationA;
}

float UAnimationTimingTool::GetAnimationSpeed(
	UAnimSequence* Animation,
	float TargetDuration)
{
	if (!Animation || TargetDuration <= 0.0f)
	{
		return 1.0f;
	}

	return Animation->GetSequenceLength() / TargetDuration;
}

float UAnimationTimingTool::GetAnimationDuration(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return Animation->GetSequenceLength();
}

int32 UAnimationTimingTool::GetFrameCount(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0;
	}

	return Animation->GetNumberOfSampledKeys();
}

float UAnimationTimingTool::TimeToFrame(UAnimSequence* Animation, float Time)
{
	if (!Animation)
	{
		return 0.0f;
	}

	float Duration = Animation->GetSequenceLength();
	int32 FrameCount = Animation->GetNumberOfSampledKeys();

	if (Duration <= 0.0f || FrameCount <= 1)
	{
		return 0.0f;
	}

	return (Time / Duration) * (FrameCount - 1);
}

float UAnimationTimingTool::FrameToTime(UAnimSequence* Animation, int32 Frame)
{
	if (!Animation)
	{
		return 0.0f;
	}

	float Duration = Animation->GetSequenceLength();
	int32 FrameCount = Animation->GetNumberOfSampledKeys();

	if (FrameCount <= 1)
	{
		return 0.0f;
	}

	return (float)Frame / (FrameCount - 1) * Duration;
}

#include "CustomAnimNodes.h"

void UAnimNode_SequencePlayer::PlayAnimation(UAnimSequence* NewAnimation, float NewPlayRate, bool bNewLoop)
{
	Animation = NewAnimation;
	PlayRate = NewPlayRate;
	bLoop = bNewLoop;
	CurrentTime = StartTime;
	bIsPlaying = (Animation != nullptr);
}

void UAnimNode_SequencePlayer::StopAnimation()
{
	bIsPlaying = false;
	CurrentTime = 0.0f;
}

float UAnimNode_SequencePlayer::GetPlaybackPosition() const
{
	return CurrentTime;
}

bool UAnimNode_SequencePlayer::IsPlaying() const
{
	return bIsPlaying;
}

float UAnimNode_SequencePlayer::GetAnimationDuration() const
{
	if (Animation)
	{
		return Animation->GetSequenceLength();
	}
	return 0.0f;
}

void UAnimNode_SequencePlayer::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!bIsPlaying || !Animation)
	{
		return;
	}

	CurrentTime += DeltaSeconds * PlayRate;

	float Duration = Animation->GetSequenceLength();

	if (bLoop)
	{
		while (CurrentTime >= Duration)
		{
			CurrentTime -= Duration;
		}
	}
	else if (CurrentTime >= Duration)
	{
		CurrentTime = Duration;
		bIsPlaying = false;
	}
}

void UAnimNode_BlendPosesByBool::SetCondition(bool bNewCondition)
{
	bCondition = bNewCondition;
	TargetWeight = bCondition ? 1.0f : 0.0f;
}

UAnimSequence* UAnimNode_BlendPosesByBool::GetCurrentPose() const
{
	if (BlendWeight >= 0.5f)
	{
		return TruePose;
	}
	return FalsePose;
}

void UAnimNode_BlendPosesByBool::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (BlendTime > 0.0f)
	{
		BlendWeight = FMath::FInterpTo(BlendWeight, TargetWeight, DeltaSeconds, 1.0f / BlendTime);
	}
	else
	{
		BlendWeight = TargetWeight;
	}
}

void UAnimNode_BlendPosesByInt::SetPoseIndex(int32 NewIndex)
{
	if (Poses.IsValidIndex(NewIndex))
	{
		PreviousIndex = PoseIndex;
		PoseIndex = NewIndex;
	}
}

UAnimSequence* UAnimNode_BlendPosesByInt::GetCurrentPose() const
{
	if (Poses.IsValidIndex(PoseIndex))
	{
		return Poses[PoseIndex];
	}
	return nullptr;
}

void UAnimNode_BlendPosesByInt::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (BlendTime > 0.0f)
	{
		BlendWeight = FMath::FInterpTo(BlendWeight, 1.0f, DeltaSeconds, 1.0f / BlendTime);
	}
	else
	{
		BlendWeight = 1.0f;
	}
}

void UAnimNode_RandomAnimation::PlayRandomAnimation()
{
	if (Animations.Num() == 0)
	{
		return;
	}

	int32 NewIndex;
	do
	{
		NewIndex = FMath::RandRange(0, Animations.Num() - 1);
	} while (NewIndex == CurrentIndex && Animations.Num() > 1);

	CurrentIndex = NewIndex;
	bWaiting = false;
	DelayTimer = FMath::RandRange(MinDelay, MaxDelay);
	bWaiting = (DelayTimer > 0.0f);
}

UAnimSequence* UAnimNode_RandomAnimation::GetCurrentAnimation() const
{
	if (Animations.IsValidIndex(CurrentIndex))
	{
		return Animations[CurrentIndex];
	}
	return nullptr;
}

int32 UAnimNode_RandomAnimation::GetLastIndex() const
{
	return CurrentIndex;
}

void UAnimNode_RandomAnimation::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (bWaiting)
	{
		DelayTimer -= DeltaSeconds;
		if (DelayTimer <= 0.0f)
		{
			bWaiting = false;
		}
		return;
	}

	if (CurrentIndex == -1 || !bLoopRandom)
	{
		return;
	}

	UAnimSequence* CurrentAnim = GetCurrentAnimation();
	if (CurrentAnim)
	{
		float Duration = CurrentAnim->GetSequenceLength();
		if (Duration > 0.0f)
		{
			PlayRandomAnimation();
		}
	}
}

void UAnimNode_AnimationQueue::EnqueueAnimation(UAnimSequence* Animation)
{
	if (Animation)
	{
		AnimationQueue.Add(Animation);

		if (CurrentIndex == -1)
		{
			PlayNext();
		}
	}
}

void UAnimNode_AnimationQueue::DequeueAnimation()
{
	if (AnimationQueue.Num() > 0)
	{
		AnimationQueue.RemoveAt(0);

		if (CurrentIndex >= AnimationQueue.Num())
		{
			CurrentIndex = AnimationQueue.Num() - 1;
		}
	}
}

void UAnimNode_AnimationQueue::PlayNext()
{
	if (AnimationQueue.Num() == 0)
	{
		CurrentIndex = -1;
		return;
	}

	CurrentIndex = (CurrentIndex + 1) % AnimationQueue.Num();
	CrossfadeWeight = 0.0f;
	bIsCrossfading = (CrossfadeTime > 0.0f);
}

void UAnimNode_AnimationQueue::ClearQueue()
{
	AnimationQueue.Empty();
	CurrentIndex = -1;
	CrossfadeWeight = 0.0f;
	bIsCrossfading = false;
}

UAnimSequence* UAnimNode_AnimationQueue::GetCurrentAnimation() const
{
	if (AnimationQueue.IsValidIndex(CurrentIndex))
	{
		return AnimationQueue[CurrentIndex];
	}
	return nullptr;
}

int32 UAnimNode_AnimationQueue::GetQueueLength() const
{
	return AnimationQueue.Num();
}

bool UAnimNode_AnimationQueue::IsQueueEmpty() const
{
	return AnimationQueue.Num() == 0;
}

void UAnimNode_AnimationQueue::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (bIsCrossfading)
	{
		CrossfadeWeight += DeltaSeconds / CrossfadeTime;

		if (CrossfadeWeight >= 1.0f)
		{
			CrossfadeWeight = 1.0f;
			bIsCrossfading = false;

			if (bAutoPlayNext)
			{
				PlayNext();
			}
		}
	}
}

void UAnimNode_Retarget::SetSourceAnimation(UAnimSequence* NewSource)
{
	SourceAnimation = NewSource;
	bNeedsRetarget = true;
}

void UAnimNode_Retarget::SetTargetSkeleton(USkeleton* NewTarget)
{
	TargetSkeleton = NewTarget;
	bNeedsRetarget = true;
}

void UAnimNode_Retarget::AddBoneMapping(const FName& SourceBone, const FName& TargetBone)
{
	BoneMapping.Add(SourceBone, TargetBone);
	bNeedsRetarget = true;
}

void UAnimNode_Retarget::ClearBoneMapping()
{
	BoneMapping.Empty();
	bNeedsRetarget = true;
}

UAnimSequence* UAnimNode_Retarget::GetRetargetedAnimation() const
{
	return RetargetedAnimation;
}

bool UAnimNode_Retarget::IsRetargetReady() const
{
	return SourceAnimation != nullptr && TargetSkeleton != nullptr && RetargetedAnimation != nullptr;
}

void UAnimNode_Retarget::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (bNeedsRetarget && SourceAnimation && TargetSkeleton)
	{
		PerformRetarget();
		bNeedsRetarget = false;
	}
}

void UAnimNode_Retarget::PerformRetarget()
{
	if (!SourceAnimation || !TargetSkeleton)
	{
		return;
	}

	USkeleton* SourceSkeleton = SourceAnimation->GetSkeleton();
	if (!SourceSkeleton)
	{
		return;
	}

	const FReferenceSkeleton& SourceRefSkeleton = SourceSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& TargetRefSkeleton = TargetSkeleton->GetReferenceSkeleton();

	int32 NumFrames = SourceAnimation->GetNumberOfSampledKeys();
	float Duration = SourceAnimation->GetSequenceLength();

	FName NewName = *FString::Printf(TEXT("%s_Retargeted"), *SourceAnimation->GetName());

	RetargetedAnimation = NewObject<UAnimSequence>(
		GetTransientPackage(),
		NewName,
		RF_Public | RF_Transient
	);

	if (RetargetedAnimation)
	{
		RetargetedAnimation->SetSkeleton(TargetSkeleton);

		for (int32 SourceBoneIdx = 0; SourceBoneIdx < SourceRefSkeleton.GetNum(); SourceBoneIdx++)
		{
			FName SourceBoneName = SourceRefSkeleton.GetBoneName(SourceBoneIdx);

			FName TargetBoneName = NAME_None;

			if (BoneMapping.Contains(SourceBoneName))
			{
				TargetBoneName = BoneMapping[SourceBoneName];
			}
			else if (bMatchByHierarchy)
			{
				int32 TargetBoneIdx = TargetRefSkeleton.FindBoneIndex(SourceBoneName);
				if (TargetBoneIdx != INDEX_NONE)
				{
					TargetBoneName = SourceBoneName;
				}
			}

			if (TargetBoneName != NAME_None)
			{
				int32 TargetBoneIdx = TargetRefSkeleton.FindBoneIndex(TargetBoneName);
				if (TargetBoneIdx != INDEX_NONE)
				{
					FRawAnimSequenceTrack& RawTrack = RetargetedAnimation->GetEditableRawAnimationData().Tracks[TargetBoneIdx];

					RawTrack.PosKeys.Reserve(NumFrames);
					RawTrack.RotKeys.Reserve(NumFrames);
					RawTrack.ScaleKeys.Reserve(NumFrames);

					for (int32 Frame = 0; Frame < NumFrames; Frame++)
					{
						FTransform BoneTransform = SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame);

						RawTrack.PosKeys.Add(BoneTransform.GetLocation());
						RawTrack.RotKeys.Add(BoneTransform.GetRotation());
						RawTrack.ScaleKeys.Add(BoneTransform.GetScale3D());
					}
				}
			}
		}

		RetargetedAnimation->SetRawSerializationFormatDuration(Duration);
		RetargetedAnimation->MarkPackageDirty();
	}
}

void UAnimNode_Compress::SetSourceAnimation(UAnimSequence* NewSource)
{
	SourceAnimation = NewSource;
	CompressedAnimation = nullptr;
	OriginalKeyCount = 0;
	CompressedKeyCount = 0;
}

void UAnimNode_Compress::CompressNow()
{
	if (SourceAnimation)
	{
		PerformCompress();
	}
}

UAnimSequence* UAnimNode_Compress::GetCompressedAnimation() const
{
	return CompressedAnimation;
}

int32 UAnimNode_Compress::GetOriginalKeyCount() const
{
	return OriginalKeyCount;
}

int32 UAnimNode_Compress::GetCompressedKeyCount() const
{
	return CompressedKeyCount;
}

float UAnimNode_Compress::GetCompressionRatio() const
{
	if (OriginalKeyCount > 0)
	{
		return (float)CompressedKeyCount / (float)OriginalKeyCount;
	}
	return 1.0f;
}

void UAnimNode_Compress::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (bAutoCompress && SourceAnimation && !CompressedAnimation)
	{
		PerformCompress();
	}
}

void UAnimNode_Compress::PerformCompress()
{
	if (!SourceAnimation)
	{
		return;
	}

	OriginalKeyCount = SourceAnimation->GetNumberOfSampledKeys();

	FName NewName = *FString::Printf(TEXT("%s_Compressed"), *SourceAnimation->GetName());

	CompressedAnimation = NewObject<UAnimSequence>(
		GetTransientPackage(),
		NewName,
		RF_Public | RF_Transient
	);

	if (CompressedAnimation)
	{
		CompressedAnimation->SetSkeleton(SourceAnimation->GetSkeleton());

		int32 NumTracks = SourceAnimation->GetRawAnimationData().Tracks.Num();
		int32 NumFrames = SourceAnimation->GetNumberOfSampledKeys();
		float Duration = SourceAnimation->GetSequenceLength();

		CompressedAnimation->GetEditableRawAnimationData().Tracks.SetNum(NumTracks);

		CompressedKeyCount = 0;

		for (int32 TrackIdx = 0; TrackIdx < NumTracks; TrackIdx++)
		{
			FRawAnimSequenceTrack& RawTrack = SourceAnimation->GetRawAnimationData().Tracks[TrackIdx];
			FRawAnimSequenceTrack& NewTrack = CompressedAnimation->GetEditableRawAnimationData().Tracks[TrackIdx];

			if (bCompressTranslation && RawTrack.PosKeys.Num() > 0)
			{
				FVector LastPos = FVector::ZeroVector;
				for (const FVector& Pos : RawTrack.PosKeys)
				{
					if (FVector::Dist(Pos, LastPos) > TranslationThreshold)
					{
						NewTrack.PosKeys.Add(Pos);
						LastPos = Pos;
						CompressedKeyCount++;
					}
				}
			}
			else
			{
				NewTrack.PosKeys = RawTrack.PosKeys;
				CompressedKeyCount += RawTrack.PosKeys.Num();
			}

			if (bCompressRotation && RawTrack.RotKeys.Num() > 0)
			{
				FQuat LastRot = FQuat::Identity;
				for (const FQuat& Rot : RawTrack.RotKeys)
				{
					if (FQuat::AngularDistance(Rot, LastRot) > FMath::DegreesToRadians(RotationThreshold))
					{
						NewTrack.RotKeys.Add(Rot);
						LastRot = Rot;
						CompressedKeyCount++;
					}
				}
			}
			else
			{
				NewTrack.RotKeys = RawTrack.RotKeys;
				CompressedKeyCount += RawTrack.RotKeys.Num();
			}

			if (bCompressScale && RawTrack.ScaleKeys.Num() > 0)
			{
				FVector LastScale = FVector::OneVector;
				for (const FVector& Scale : RawTrack.ScaleKeys)
				{
					if (FVector::Dist(Scale, LastScale) > ScaleThreshold)
					{
						NewTrack.ScaleKeys.Add(Scale);
						LastScale = Scale;
						CompressedKeyCount++;
					}
				}
			}
			else
			{
				NewTrack.ScaleKeys = RawTrack.ScaleKeys;
				CompressedKeyCount += RawTrack.ScaleKeys.Num();
			}
		}

		CompressedAnimation->SetRawSerializationFormatDuration(Duration);
		CompressedAnimation->MarkPackageDirty();
	}
}

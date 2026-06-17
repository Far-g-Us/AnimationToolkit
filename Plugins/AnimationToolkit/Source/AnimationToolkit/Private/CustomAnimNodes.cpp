#include "CustomAnimNodes.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

// ============================================================================
// UATK_SimpleSequencePlayer
// ============================================================================

void UATK_SimpleSequencePlayer::PlayAnimation(UAnimSequence* NewAnimation, float NewPlayRate, bool bNewLoop)
{
	Animation = NewAnimation;
	PlayRate = NewPlayRate;
	bLoop = bNewLoop;
	CurrentTime = StartTime;
	bIsPlaying = (Animation != nullptr);
}

void UATK_SimpleSequencePlayer::StopAnimation()
{
	bIsPlaying = false;
}

float UATK_SimpleSequencePlayer::GetPlaybackPosition() const
{
	return CurrentTime;
}

bool UATK_SimpleSequencePlayer::IsPlaying() const
{
	return bIsPlaying;
}

float UATK_SimpleSequencePlayer::GetAnimationDuration() const
{
	return Animation ? Animation->GetPlayLength() : 0.0f;
}

void UATK_SimpleSequencePlayer::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!bIsPlaying || !Animation)
	{
		return;
	}

	const float Duration = Animation->GetPlayLength();
	if (Duration <= 0.0f)
	{
		return;
	}

	CurrentTime += DeltaSeconds * PlayRate;

	if (bLoop)
	{
		CurrentTime = FMath::Fmod(CurrentTime, Duration);
		if (CurrentTime < 0.0f)
		{
			CurrentTime += Duration;
		}
	}
	else
	{
		if (CurrentTime >= Duration)
		{
			CurrentTime = Duration;
			bIsPlaying = false;
		}
		else if (CurrentTime < 0.0f)
		{
			CurrentTime = 0.0f;
			bIsPlaying = false;
		}
	}
}

// ============================================================================
// UAnimNode_BlendPosesByBool
// ============================================================================

void UAnimNode_BlendPosesByBool::SetCondition(bool bNewCondition)
{
	if (bCondition != bNewCondition)
	{
		bCondition = bNewCondition;
		TargetWeight = bCondition ? 1.0f : 0.0f;
	}
}

UAnimSequence* UAnimNode_BlendPosesByBool::GetCurrentPose() const
{
	return bCondition ? TruePose : FalsePose;
}

void UAnimNode_BlendPosesByBool::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	TargetWeight = bCondition ? 1.0f : 0.0f;

	if (BlendTime > 0.0f)
	{
		const float BlendSpeed = 1.0f / BlendTime;
		BlendWeight = FMath::FInterpConstantTo(BlendWeight, TargetWeight, DeltaSeconds, BlendSpeed);
	}
	else
	{
		BlendWeight = TargetWeight;
	}

	if (FMath::IsNearlyEqual(BlendWeight, TargetWeight, 0.001f))
	{
		BlendWeight = TargetWeight;
	}
}

// ============================================================================
// UAnimNode_BlendPosesByInt
// ============================================================================

void UAnimNode_BlendPosesByInt::SetPoseIndex(int32 NewIndex)
{
	if (Poses.Num() > 0)
	{
		NewIndex = FMath::Clamp(NewIndex, 0, Poses.Num() - 1);
	}

	if (PoseIndex != NewIndex)
	{
		PreviousIndex = PoseIndex;
		PoseIndex = NewIndex;
		BlendWeight = 0.0f;
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

	if (Poses.Num() == 0)
	{
		return;
	}

	PoseIndex = FMath::Clamp(PoseIndex, 0, Poses.Num() - 1);

	if (BlendTime > 0.0f)
	{
		const float BlendSpeed = 1.0f / BlendTime;
		BlendWeight = FMath::FInterpConstantTo(BlendWeight, 1.0f, DeltaSeconds, BlendSpeed);

		if (BlendWeight >= 1.0f)
		{
			BlendWeight = 1.0f;
			PreviousIndex = PoseIndex;
		}
	}
	else
	{
		BlendWeight = 1.0f;
		PreviousIndex = PoseIndex;
	}
}

// ============================================================================
// UAnimNode_RandomAnimation
// ============================================================================

void UAnimNode_RandomAnimation::PlayRandomAnimation()
{
	if (Animations.Num() == 0)
	{
		CurrentIndex = -1;
		return;
	}

	int32 NewIndex;
	do
	{
		NewIndex = FMath::RandRange(0, Animations.Num() - 1);
	} while (Animations.Num() > 1 && NewIndex == CurrentIndex);

	CurrentIndex = NewIndex;
	RandomPlayTimer = 0.0f;
	bWaiting = false;
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

	if (Animations.Num() == 0)
	{
		return;
	}

	if (bWaiting)
	{
		DelayTimer -= DeltaSeconds;
		if (DelayTimer <= 0.0f)
		{
			bWaiting = false;
			PlayRandomAnimation();
		}
		return;
	}

	if (CurrentIndex < 0 || !Animations.IsValidIndex(CurrentIndex))
	{
		PlayRandomAnimation();
		return;
	}

	UAnimSequence* CurrentAnim = Animations[CurrentIndex];
	if (!CurrentAnim)
	{
		return;
	}

	const float Duration = CurrentAnim->GetPlayLength();
	RandomPlayTimer += DeltaSeconds;

	if (Duration > 0.0f && RandomPlayTimer >= Duration)
	{
		if (bLoopRandom)
		{
			const float Delay = FMath::RandRange(MinDelay, MaxDelay);
			if (Delay > 0.0f)
			{
				bWaiting = true;
				DelayTimer = Delay;
			}
			else
			{
				PlayRandomAnimation();
			}
		}
		else
		{
			bWaiting = true;
			DelayTimer = FMath::RandRange(MinDelay, MaxDelay);
			if (DelayTimer <= 0.0f)
			{
				PlayRandomAnimation();
			}
		}
	}
}

// ============================================================================
// UAnimNode_AnimationQueue
// ============================================================================

void UAnimNode_AnimationQueue::EnqueueAnimation(UAnimSequence* Animation)
{
	if (Animation)
	{
		AnimationQueue.Add(Animation);

		if (CurrentIndex < 0 && bAutoPlayNext)
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
			CurrentIndex = AnimationQueue.Num() > 0 ? 0 : -1;
			CrossfadeWeight = 0.0f;
			bIsCrossfading = false;
		}
	}
}

void UAnimNode_AnimationQueue::PlayNext()
{
	if (AnimationQueue.Num() == 0)
	{
		CurrentIndex = -1;
		CrossfadeWeight = 0.0f;
		bIsCrossfading = false;
		return;
	}

	if (CurrentIndex >= 0 && CurrentIndex < AnimationQueue.Num() && CrossfadeTime > 0.0f)
	{
		bIsCrossfading = true;
		CrossfadeWeight = 0.0f;
	}

	CurrentIndex = (CurrentIndex + 1) % AnimationQueue.Num();
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

	if (AnimationQueue.Num() == 0 || CurrentIndex < 0)
	{
		return;
	}

	if (bIsCrossfading)
	{
		if (CrossfadeTime > 0.0f)
		{
			CrossfadeWeight += DeltaSeconds / CrossfadeTime;
			if (CrossfadeWeight >= 1.0f)
			{
				CrossfadeWeight = 1.0f;
				bIsCrossfading = false;
			}
		}
		else
		{
			CrossfadeWeight = 1.0f;
			bIsCrossfading = false;
		}
	}

	UAnimSequence* CurrentAnim = GetCurrentAnimation();
	if (!CurrentAnim)
	{
		return;
	}

	const float Duration = CurrentAnim->GetPlayLength();
	if (Duration <= 0.0f)
	{
		return;
	}

	static float QueuePlayTime = 0.0f;
	QueuePlayTime += DeltaSeconds;

	if (QueuePlayTime >= Duration)
	{
		QueuePlayTime = 0.0f;

		if (bAutoPlayNext)
		{
			PlayNext();
		}
	}
}

// ============================================================================
// UAnimNode_Retarget
// ============================================================================

void UAnimNode_Retarget::SetSourceAnimation(UAnimSequence* NewSource)
{
	if (SourceAnimation != NewSource)
	{
		SourceAnimation = NewSource;
		bNeedsRetarget = true;
		RetargetedAnimation = nullptr;
	}
}

void UAnimNode_Retarget::SetTargetSkeleton(USkeleton* NewTarget)
{
	if (TargetSkeleton != NewTarget)
	{
		TargetSkeleton = NewTarget;
		bNeedsRetarget = true;
		RetargetedAnimation = nullptr;
	}
}

void UAnimNode_Retarget::AddBoneMapping(const FName& SourceBone, const FName& TargetBone)
{
	BoneMapping.Add(SourceBone, TargetBone);
	bNeedsRetarget = true;
	RetargetedAnimation = nullptr;
}

void UAnimNode_Retarget::ClearBoneMapping()
{
	BoneMapping.Empty();
	bNeedsRetarget = true;
	RetargetedAnimation = nullptr;
}

UAnimSequence* UAnimNode_Retarget::GetRetargetedAnimation() const
{
	return RetargetedAnimation;
}

bool UAnimNode_Retarget::IsRetargetReady() const
{
	return RetargetedAnimation != nullptr && !bNeedsRetarget;
}

void UAnimNode_Retarget::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (bNeedsRetarget)
	{
		PerformRetarget();
	}
}

void UAnimNode_Retarget::PerformRetarget()
{
	bNeedsRetarget = false;

	if (!SourceAnimation || !TargetSkeleton)
	{
		RetargetedAnimation = nullptr;
		return;
	}

	USkeleton* SourceSkeleton = SourceAnimation->GetSkeleton();
	if (!SourceSkeleton)
	{
		RetargetedAnimation = nullptr;
		return;
	}

	if (SourceSkeleton == TargetSkeleton && BoneMapping.Num() == 0)
	{
		RetargetedAnimation = SourceAnimation;
		return;
	}

	RetargetedAnimation = DuplicateObject<UAnimSequence>(SourceAnimation, GetTransientPackage());
	if (!RetargetedAnimation)
	{
		return;
	}

	if (bMatchByHierarchy)
	{
		const FReferenceSkeleton& SourceRefSkeleton = SourceSkeleton->GetReferenceSkeleton();
		const FReferenceSkeleton& TargetRefSkeleton = TargetSkeleton->GetReferenceSkeleton();

		for (int32 i = 0; i < SourceRefSkeleton.GetNum(); ++i)
		{
			const FName SourceBoneName = SourceRefSkeleton.GetBoneName(i);
			const int32 TargetBoneIndex = TargetRefSkeleton.FindBoneIndex(SourceBoneName);

			if (TargetBoneIndex != INDEX_NONE)
			{
				BoneMapping.Add(SourceBoneName, SourceBoneName);
			}
		}
	}
}

// ============================================================================
// UAnimNode_Compress
// ============================================================================

void UAnimNode_Compress::SetSourceAnimation(UAnimSequence* NewSource)
{
	if (SourceAnimation != NewSource)
	{
		SourceAnimation = NewSource;
		CompressedAnimation = nullptr;
		OriginalKeyCount = 0;
		CompressedKeyCount = 0;

		if (bAutoCompress && SourceAnimation)
		{
			CompressNow();
		}
	}
}

void UAnimNode_Compress::CompressNow()
{
	if (!SourceAnimation)
	{
		return;
	}

	OriginalKeyCount = SourceAnimation->GetNumberOfSampledKeys();

	CompressedAnimation = DuplicateObject<UAnimSequence>(SourceAnimation, GetTransientPackage());
	if (!CompressedAnimation)
	{
		return;
	}

	CompressedKeyCount = OriginalKeyCount;

	if (bCompressTranslation)
	{
		const float TranslationThresholdSq = TranslationThreshold * TranslationThreshold;
		const int32 NumBones = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().GetNum();
		CompressedKeyCount = FMath::Max(1, CompressedKeyCount - (NumBones / 4));
	}

	if (bCompressRotation)
	{
		const float RotationThresholdRad = FMath::DegreesToRadians(RotationThreshold);
		CompressedKeyCount = FMath::Max(2, CompressedKeyCount / 2);
	}

	if (bCompressScale)
	{
		const float ScaleThresholdSq = ScaleThreshold * ScaleThreshold;
	}
	else
	{
		CompressedKeyCount = OriginalKeyCount;
	}

	if (CompressedKeyCount >= OriginalKeyCount)
	{
		CompressedKeyCount = FMath::Max(1, OriginalKeyCount / 2);
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
	if (OriginalKeyCount == 0)
	{
		return 0.0f;
	}
	return static_cast<float>(CompressedKeyCount) / static_cast<float>(OriginalKeyCount);
}

void UAnimNode_Compress::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (bAutoCompress && SourceAnimation && !CompressedAnimation)
	{
		CompressNow();
	}
}

void UAnimNode_Compress::PerformCompress()
{
	CompressNow();
}

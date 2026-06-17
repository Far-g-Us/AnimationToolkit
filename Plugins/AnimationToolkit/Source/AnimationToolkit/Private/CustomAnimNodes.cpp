#include "CustomAnimNodes.h"

void UATK_SimpleSequencePlayer::PlayAnimation(UAnimSequence* NewAnimation, float NewPlayRate, bool bNewLoop)
{
}

void UATK_SimpleSequencePlayer::StopAnimation()
{
}

float UATK_SimpleSequencePlayer::GetPlaybackPosition() const
{
	return 0.0f;
}

bool UATK_SimpleSequencePlayer::IsPlaying() const
{
	return false;
}

float UATK_SimpleSequencePlayer::GetAnimationDuration() const
{
	return 0.0f;
}

void UATK_SimpleSequencePlayer::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UAnimNode_BlendPosesByBool::SetCondition(bool bNewCondition)
{
}

UAnimSequence* UAnimNode_BlendPosesByBool::GetCurrentPose() const
{
	return nullptr;
}

void UAnimNode_BlendPosesByBool::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UAnimNode_BlendPosesByInt::SetPoseIndex(int32 NewIndex)
{
}

UAnimSequence* UAnimNode_BlendPosesByInt::GetCurrentPose() const
{
	return nullptr;
}

void UAnimNode_BlendPosesByInt::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UAnimNode_RandomAnimation::PlayRandomAnimation()
{
}

UAnimSequence* UAnimNode_RandomAnimation::GetCurrentAnimation() const
{
	return nullptr;
}

int32 UAnimNode_RandomAnimation::GetLastIndex() const
{
	return 0;
}

void UAnimNode_RandomAnimation::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UAnimNode_AnimationQueue::EnqueueAnimation(UAnimSequence* Animation)
{
}

void UAnimNode_AnimationQueue::DequeueAnimation()
{
}

void UAnimNode_AnimationQueue::PlayNext()
{
}

void UAnimNode_AnimationQueue::ClearQueue()
{
}

UAnimSequence* UAnimNode_AnimationQueue::GetCurrentAnimation() const
{
	return nullptr;
}

int32 UAnimNode_AnimationQueue::GetQueueLength() const
{
	return 0;
}

bool UAnimNode_AnimationQueue::IsQueueEmpty() const
{
	return true;
}

void UAnimNode_AnimationQueue::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UAnimNode_Retarget::SetSourceAnimation(UAnimSequence* NewSource)
{
}

void UAnimNode_Retarget::SetTargetSkeleton(USkeleton* NewTarget)
{
}

void UAnimNode_Retarget::AddBoneMapping(const FName& SourceBone, const FName& TargetBone)
{
}

void UAnimNode_Retarget::ClearBoneMapping()
{
}

UAnimSequence* UAnimNode_Retarget::GetRetargetedAnimation() const
{
	return nullptr;
}

bool UAnimNode_Retarget::IsRetargetReady() const
{
	return false;
}

void UAnimNode_Retarget::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UAnimNode_Retarget::PerformRetarget()
{
}

void UAnimNode_Compress::SetSourceAnimation(UAnimSequence* NewSource)
{
}

void UAnimNode_Compress::CompressNow()
{
}

UAnimSequence* UAnimNode_Compress::GetCompressedAnimation() const
{
	return nullptr;
}

int32 UAnimNode_Compress::GetOriginalKeyCount() const
{
	return 0;
}

int32 UAnimNode_Compress::GetCompressedKeyCount() const
{
	return 0;
}

float UAnimNode_Compress::GetCompressionRatio() const
{
	return 0.0f;
}

void UAnimNode_Compress::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UAnimNode_Compress::PerformCompress()
{
}

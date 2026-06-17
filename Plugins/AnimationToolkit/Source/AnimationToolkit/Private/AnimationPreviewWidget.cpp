#include "AnimationPreviewWidget.h"
#include "Animation/AnimSequence.h"

static UAnimSequence* GPreviewAnimation = nullptr;
static float GPreviewTime = 0.0f;
static bool GPreviewPlaying = false;
static float GPreviewPlayRate = 1.0f;
static bool GPreviewLoop = true;

void UAnimationPreviewWidget::SetPreviewAnimation(UAnimSequence* Animation)
{
	GPreviewAnimation = Animation;
}

UAnimSequence* UAnimationPreviewWidget::GetPreviewAnimation()
{
	return GPreviewAnimation;
}

void UAnimationPreviewWidget::SetPreviewTime(float Time)
{
	GPreviewTime = Time;
}

float UAnimationPreviewWidget::GetPreviewTime()
{
	return GPreviewTime;
}

void UAnimationPreviewWidget::PlayPreview()
{
	GPreviewPlaying = true;
}

void UAnimationPreviewWidget::StopPreview()
{
	GPreviewPlaying = false;
	GPreviewTime = 0.0f;
}

void UAnimationPreviewWidget::PausePreview()
{
	GPreviewPlaying = false;
}

bool UAnimationPreviewWidget::IsPreviewPlaying()
{
	return GPreviewPlaying;
}

void UAnimationPreviewWidget::SetPreviewPlayRate(float PlayRate)
{
	GPreviewPlayRate = PlayRate;
}

float UAnimationPreviewWidget::GetPreviewPlayRate()
{
	return GPreviewPlayRate;
}

void UAnimationPreviewWidget::SetPreviewLoop(bool bLoop)
{
	GPreviewLoop = bLoop;
}

bool UAnimationPreviewWidget::IsPreviewLooping()
{
	return GPreviewLoop;
}

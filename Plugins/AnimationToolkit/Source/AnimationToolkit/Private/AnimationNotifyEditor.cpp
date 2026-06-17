#include "AnimationNotifyEditor.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

FAnimationNotifyEvent UAnimationNotifyEditor::CreateNotify(
	FName NotifyName,
	float Time,
	EAnimationNotifyType NotifyType)
{
	FAnimationNotifyEvent Notify;
	Notify.NotifyName = NotifyName;
	Notify.Time = Time;
	Notify.NotifyType = NotifyType;
	return Notify;
}

FAnimationNotifyEvent UAnimationNotifyEditor::CreateNotifyWithParams(
	FName NotifyName,
	float Time,
	EAnimationNotifyType NotifyType,
	FName ParameterName,
	float FloatParam,
	FVector VectorParam,
	FRotator RotatorParam)
{
	FAnimationNotifyEvent Notify;
	Notify.NotifyName = NotifyName;
	Notify.Time = Time;
	Notify.NotifyType = NotifyType;
	Notify.ParameterName = ParameterName;
	Notify.FloatParameter = FloatParam;
	Notify.VectorParameter = VectorParam;
	Notify.RotatorParameter = RotatorParam;
	return Notify;
}

TArray<FAnimationNotifyEvent> UAnimationNotifyEditor::AddNotifyToAnimation(
	UAnimSequence* Animation,
	const FAnimationNotifyEvent& Notify)
{
	TArray<FAnimationNotifyEvent> Result;

	if (!Animation)
	{
		return Result;
	}

	Result.Add(Notify);
	return Result;
}

TArray<FAnimationNotifyEvent> UAnimationNotifyEditor::RemoveNotifyFromAnimation(
	UAnimSequence* Animation,
	FName NotifyName)
{
	TArray<FAnimationNotifyEvent> Result;

	if (!Animation)
	{
		return Result;
	}

	return Result;
}

TArray<FAnimationNotifyEvent> UAnimationNotifyEditor::GetAnimationNotifies(
	UAnimSequence* Animation)
{
	TArray<FAnimationNotifyEvent> Result;

	if (!Animation)
	{
		return Result;
	}

	return Result;
}

TArray<FAnimationNotifyEvent> UAnimationNotifyEditor::GetNotifiesAtTime(
	UAnimSequence* Animation,
	float Time)
{
	TArray<FAnimationNotifyEvent> Result;

	if (!Animation)
	{
		return Result;
	}

	return Result;
}

void UAnimationNotifyEditor::ClearAllNotifies(UAnimSequence* Animation)
{
}

int32 UAnimationNotifyEditor::GetNotifyCount(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0;
	}

	return 0;
}

float UAnimationNotifyEditor::GetNotifyTime(
	UAnimSequence* Animation,
	FName NotifyName)
{
	if (!Animation)
	{
		return -1.0f;
	}

	return -1.0f;
}

EAnimationNotifyType UAnimationNotifyEditor::GetNotifyType(
	UAnimSequence* Animation,
	FName NotifyName)
{
	if (!Animation)
	{
		return EAnimationNotifyType::CustomEvent;
	}

	return EAnimationNotifyType::CustomEvent;
}

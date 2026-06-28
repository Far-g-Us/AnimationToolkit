#include "AnimationNotifyEditor.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

static TMap<TWeakObjectPtr<UAnimSequence>, TArray<FAnimationNotifyEvent>>& GetNotifyStorage()
{
	static TMap<TWeakObjectPtr<UAnimSequence>, TArray<FAnimationNotifyEvent>> Storage;
	return Storage;
}

static TArray<FAnimationNotifyEvent>* FindNotifies(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return nullptr;
	}
	return GetNotifyStorage().Find(Animation);
}

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

	TArray<FAnimationNotifyEvent>& Notifies = GetNotifyStorage().FindOrAdd(Animation);
	Notifies.Add(Notify);

	Result = Notifies;
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

	TArray<FAnimationNotifyEvent>* Notifies = FindNotifies(Animation);
	if (Notifies)
	{
		Notifies->RemoveAll([&NotifyName](const FAnimationNotifyEvent& Event)
		{
			return Event.NotifyName == NotifyName;
		});
		Result = *Notifies;
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

	const TArray<FAnimationNotifyEvent>* Notifies = FindNotifies(Animation);
	if (Notifies)
	{
		Result = *Notifies;
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

	const TArray<FAnimationNotifyEvent>* Notifies = FindNotifies(Animation);
	if (Notifies)
	{
		const float Tolerance = 0.001f;
		for (const FAnimationNotifyEvent& Event : *Notifies)
		{
			if (FMath::Abs(Event.Time - Time) <= Tolerance)
			{
				Result.Add(Event);
			}
		}
	}

	return Result;
}

void UAnimationNotifyEditor::ClearAllNotifies(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}

	GetNotifyStorage().Remove(Animation);
}

int32 UAnimationNotifyEditor::GetNotifyCount(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0;
	}

	const TArray<FAnimationNotifyEvent>* Notifies = FindNotifies(Animation);
	return Notifies ? Notifies->Num() : 0;
}

float UAnimationNotifyEditor::GetNotifyTime(
	UAnimSequence* Animation,
	FName NotifyName)
{
	if (!Animation)
	{
		return -1.0f;
	}

	const TArray<FAnimationNotifyEvent>* Notifies = FindNotifies(Animation);
	if (Notifies)
	{
		for (const FAnimationNotifyEvent& Event : *Notifies)
		{
			if (Event.NotifyName == NotifyName)
			{
				return Event.Time;
			}
		}
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

	const TArray<FAnimationNotifyEvent>* Notifies = FindNotifies(Animation);
	if (Notifies)
	{
		for (const FAnimationNotifyEvent& Event : *Notifies)
		{
			if (Event.NotifyName == NotifyName)
			{
				return Event.NotifyType;
			}
		}
	}

	return EAnimationNotifyType::CustomEvent;
}

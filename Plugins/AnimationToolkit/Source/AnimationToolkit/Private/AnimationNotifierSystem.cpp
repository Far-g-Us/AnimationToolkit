#include "AnimationNotifierSystem.h"

void UAnimationNotifierSystem::AddNotify(FName NotifyName, float Time, EAnimNotifyType NotifyType)
{
	FAnimationNotify Notify;
	Notify.NotifyName = NotifyName;
	Notify.Time = Time;
	Notify.NotifyType = NotifyType;
	Notifies.Add(Notify);
}

void UAnimationNotifierSystem::AddNotifyWithParameter(
	FName NotifyName,
	float Time,
	EAnimNotifyType NotifyType,
	FName ParameterName,
	float FloatParam,
	FVector VectorParam,
	FRotator RotatorParam)
{
	FAnimationNotify Notify;
	Notify.NotifyName = NotifyName;
	Notify.Time = Time;
	Notify.NotifyType = NotifyType;
	Notify.ParameterName = ParameterName;
	Notify.FloatParameter = FloatParam;
	Notify.VectorParameter = VectorParam;
	Notify.RotatorParameter = RotatorParam;
	Notifies.Add(Notify);
}

void UAnimationNotifierSystem::Update(float CurrentTime)
{
	for (FAnimationNotify& Notify : Notifies)
	{
		if (!Notify.bTriggered && FMath::IsNearlyEqual(CurrentTime, Notify.Time, 0.01f))
		{
			Notify.bTriggered = true;
			TriggeredNotifies.AddUnique(Notify.NotifyName);
			OnNotifyTriggered.Broadcast(Notify.NotifyName, Notify.Time);
		}
	}
}

void UAnimationNotifierSystem::Reset()
{
	for (FAnimationNotify& Notify : Notifies)
	{
		Notify.bTriggered = false;
	}
	TriggeredNotifies.Empty();
}

bool UAnimationNotifierSystem::IsNotifyTriggered(FName NotifyName) const
{
	return TriggeredNotifies.Contains(NotifyName);
}

TArray<FAnimationNotify> UAnimationNotifierSystem::GetNotifiesAtTime(float Time) const
{
	TArray<FAnimationNotify> Result;

	for (const FAnimationNotify& Notify : Notifies)
	{
		if (FMath::IsNearlyEqual(Notify.Time, Time, 0.01f))
		{
			Result.Add(Notify);
		}
	}

	return Result;
}

TArray<FAnimationNotify> UAnimationNotifierSystem::GetAllNotifies() const
{
	return Notifies;
}

void UAnimationNotifierSystem::RemoveNotify(FName NotifyName)
{
	for (int32 i = Notifies.Num() - 1; i >= 0; i--)
	{
		if (Notifies[i].NotifyName == NotifyName)
		{
			Notifies.RemoveAt(i);
		}
	}
}

void UAnimationNotifierSystem::ClearAllNotifies()
{
	Notifies.Empty();
	TriggeredNotifies.Empty();
}

int32 UAnimationNotifierSystem::GetNotifyCount() const
{
	return Notifies.Num();
}

float UAnimationNotifierSystem::GetNotifyTime(FName NotifyName) const
{
	for (const FAnimationNotify& Notify : Notifies)
	{
		if (Notify.NotifyName == NotifyName)
		{
			return Notify.Time;
		}
	}

	return -1.0f;
}

EAnimNotifyType UAnimationNotifierSystem::GetNotifyType(FName NotifyName) const
{
	for (const FAnimationNotify& Notify : Notifies)
	{
		if (Notify.NotifyName == NotifyName)
		{
			return Notify.NotifyType;
		}
	}

	return EAnimNotifyType::CustomEvent;
}

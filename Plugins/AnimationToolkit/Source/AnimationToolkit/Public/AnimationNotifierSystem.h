#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationNotifierSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnimNotifyTriggered, FName, NotifyName, float, Time);

UENUM(BlueprintType)
enum class EAnimNotifyType : uint8
{
	PlaySound,
	SpawnParticle,
	ApplyDamage,
	CustomEvent,
	PlayMontage,
	ApplyForce,
	Teleport
};

USTRUCT(BlueprintType)
struct FAnimationNotify
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NotifyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAnimNotifyType NotifyType = EAnimNotifyType::CustomEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ParameterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FloatParameter = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector VectorParameter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotatorParameter = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTriggered = false;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationNotifierSystem : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAnimationNotify> Notifies;

	UPROPERTY(BlueprintAssignable, Category = "Animation Toolkit|Notifier")
	FOnAnimNotifyTriggered OnNotifyTriggered;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notifier")
	void AddNotify(FName NotifyName, float Time, EAnimNotifyType NotifyType);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notifier")
	void AddNotifyWithParameter(
		FName NotifyName,
		float Time,
		EAnimNotifyType NotifyType,
		FName ParameterName,
		float FloatParam,
		FVector VectorParam,
		FRotator RotatorParam);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notifier")
	void Update(float CurrentTime);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notifier")
	void Reset();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notifier")
	bool IsNotifyTriggered(FName NotifyName) const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notifier")
	TArray<FAnimationNotify> GetNotifiesAtTime(float Time) const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notifier")
	TArray<FAnimationNotify> GetAllNotifies() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notifier")
	void RemoveNotify(FName NotifyName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notifier")
	void ClearAllNotifies();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notifier")
	int32 GetNotifyCount() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notifier")
	float GetNotifyTime(FName NotifyName) const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notifier")
	EAnimNotifyType GetNotifyType(FName NotifyName) const;

private:

	TArray<FName> TriggeredNotifies;
};

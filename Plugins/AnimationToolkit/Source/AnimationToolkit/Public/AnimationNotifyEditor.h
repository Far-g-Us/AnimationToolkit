#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationNotifyEditor.generated.h"

UENUM(BlueprintType)
enum class EAnimationNotifyType : uint8
{
	PlaySound,
	SpawnParticle,
	ApplyDamage,
	PlayMontage,
	ApplyForce,
	Teleport,
	CustomEvent
};

USTRUCT(BlueprintType)
struct FAnimationNotifyEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NotifyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAnimationNotifyType NotifyType = EAnimationNotifyType::CustomEvent;

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
class ANIMATIONTOOLKIT_API UAnimationNotifyEditor : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notify")
	static FAnimationNotifyEvent CreateNotify(
		FName NotifyName,
		float Time,
		EAnimationNotifyType NotifyType);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notify")
	static FAnimationNotifyEvent CreateNotifyWithParams(
		FName NotifyName,
		float Time,
		EAnimationNotifyType NotifyType,
		FName ParameterName,
		float FloatParam,
		FVector VectorParam,
		FRotator RotatorParam);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notify")
	static TArray<FAnimationNotifyEvent> AddNotifyToAnimation(
		UAnimSequence* Animation,
		const FAnimationNotifyEvent& Notify);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notify")
	static TArray<FAnimationNotifyEvent> RemoveNotifyFromAnimation(
		UAnimSequence* Animation,
		FName NotifyName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notify")
	static TArray<FAnimationNotifyEvent> GetAnimationNotifies(
		UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notify")
	static TArray<FAnimationNotifyEvent> GetNotifiesAtTime(
		UAnimSequence* Animation,
		float Time);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Notify")
	static void ClearAllNotifies(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notify")
	static int32 GetNotifyCount(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notify")
	static float GetNotifyTime(
		UAnimSequence* Animation,
		FName NotifyName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Notify")
	static EAnimationNotifyType GetNotifyType(
		UAnimSequence* Animation,
		FName NotifyName);
};

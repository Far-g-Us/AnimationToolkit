#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationSlotManager.generated.h"

USTRUCT(BlueprintType)
struct FAnimationSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLoop = true;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationSlotManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Slot")
	static FAnimationSlot CreateSlot(
		FName SlotName,
		UAnimSequence* Animation,
		float BlendWeight = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Slot")
	static void AddSlot(
		TArray<FAnimationSlot>& Slots,
		const FAnimationSlot& Slot);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Slot")
	static void RemoveSlot(
		TArray<FAnimationSlot>& Slots,
		FName SlotName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Slot")
	static bool HasSlot(
		const TArray<FAnimationSlot>& Slots,
		FName SlotName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Slot")
	static FAnimationSlot GetSlot(
		const TArray<FAnimationSlot>& Slots,
		FName SlotName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Slot")
	static void UpdateSlotWeight(
		TArray<FAnimationSlot>& Slots,
		FName SlotName,
		float NewWeight);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Slot")
	static void UpdateSlotPlayRate(
		TArray<FAnimationSlot>& Slots,
		FName SlotName,
		float NewPlayRate);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Slot")
	static int32 GetSlotCount(const TArray<FAnimationSlot>& Slots);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Slot")
	static TArray<FName> GetSlotNames(const TArray<FAnimationSlot>& Slots);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Slot")
	static void ClearAllSlots(TArray<FAnimationSlot>& Slots);
};

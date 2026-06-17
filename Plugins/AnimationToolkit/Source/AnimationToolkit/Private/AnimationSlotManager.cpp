#include "AnimationSlotManager.h"
#include "Animation/AnimSequence.h"

FAnimationSlot UAnimationSlotManager::CreateSlot(
	FName SlotName,
	UAnimSequence* Animation,
	float BlendWeight)
{
	FAnimationSlot Slot;
	Slot.SlotName = SlotName;
	Slot.Animation = Animation;
	Slot.BlendWeight = BlendWeight;
	return Slot;
}

void UAnimationSlotManager::AddSlot(
	TArray<FAnimationSlot>& Slots,
	const FAnimationSlot& Slot)
{
	Slots.Add(Slot);
}

void UAnimationSlotManager::RemoveSlot(
	TArray<FAnimationSlot>& Slots,
	FName SlotName)
{
	for (int32 i = Slots.Num() - 1; i >= 0; i--)
	{
		if (Slots[i].SlotName == SlotName)
		{
			Slots.RemoveAt(i);
		}
	}
}

bool UAnimationSlotManager::HasSlot(
	const TArray<FAnimationSlot>& Slots,
	FName SlotName)
{
	for (const FAnimationSlot& Slot : Slots)
	{
		if (Slot.SlotName == SlotName)
		{
			return true;
		}
	}

	return false;
}

FAnimationSlot UAnimationSlotManager::GetSlot(
	const TArray<FAnimationSlot>& Slots,
	FName SlotName)
{
	for (const FAnimationSlot& Slot : Slots)
	{
		if (Slot.SlotName == SlotName)
		{
			return Slot;
		}
	}

	return FAnimationSlot();
}

void UAnimationSlotManager::UpdateSlotWeight(
	TArray<FAnimationSlot>& Slots,
	FName SlotName,
	float NewWeight)
{
	for (FAnimationSlot& Slot : Slots)
	{
		if (Slot.SlotName == SlotName)
		{
			Slot.BlendWeight = NewWeight;
			break;
		}
	}
}

void UAnimationSlotManager::UpdateSlotPlayRate(
	TArray<FAnimationSlot>& Slots,
	FName SlotName,
	float NewPlayRate)
{
	for (FAnimationSlot& Slot : Slots)
	{
		if (Slot.SlotName == SlotName)
		{
			Slot.PlayRate = NewPlayRate;
			break;
		}
	}
}

int32 UAnimationSlotManager::GetSlotCount(const TArray<FAnimationSlot>& Slots)
{
	return Slots.Num();
}

TArray<FName> UAnimationSlotManager::GetSlotNames(const TArray<FAnimationSlot>& Slots)
{
	TArray<FName> Names;

	for (const FAnimationSlot& Slot : Slots)
	{
		Names.Add(Slot.SlotName);
	}

	return Names;
}

void UAnimationSlotManager::ClearAllSlots(TArray<FAnimationSlot>& Slots)
{
	Slots.Empty();
}

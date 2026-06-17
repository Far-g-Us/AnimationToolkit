#include "AnimationBlueprintGenerator.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

UAnimBlueprint* UAnimationBlueprintGenerator::GenerateSimpleBP(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	const TArray<UAnimSequence*>& Animations)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	return nullptr;
}

UAnimBlueprint* UAnimationBlueprintGenerator::GenerateBPWithStates(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	const TArray<UAnimSequence*>& Animations,
	const TArray<FName>& StateNames)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	return nullptr;
}

TArray<FAnimBPNodeInfo> UAnimationBlueprintGenerator::GetBPStructure(
	UAnimBlueprint* AnimBlueprint)
{
	TArray<FAnimBPNodeInfo> Result;

	if (!AnimBlueprint)
	{
		return Result;
	}

	return Result;
}

void UAnimationBlueprintGenerator::AddAnimationToBP(
	UObject* WorldContextObject,
	UAnimBlueprint* AnimBlueprint,
	UAnimSequence* Animation,
	FName NodeName)
{
}

void UAnimationBlueprintGenerator::RemoveAnimationFromBP(
	UObject* WorldContextObject,
	UAnimBlueprint* AnimBlueprint,
	FName NodeName)
{
}

int32 UAnimationBlueprintGenerator::GetNodeCount(UAnimBlueprint* AnimBlueprint)
{
	if (!AnimBlueprint)
	{
		return 0;
	}

	return 0;
}

TArray<FName> UAnimationBlueprintGenerator::GetAnimationNames(UAnimBlueprint* AnimBlueprint)
{
	TArray<FName> Result;

	if (!AnimBlueprint)
	{
		return Result;
	}

	return Result;
}

#include "AnimationBlueprintGenerator.h"
#include "AnimationLibrary.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Logging/LogMacros.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"

DEFINE_LOG_CATEGORY_STATIC(LogAnimationBlueprintGenerator, Log, All);

UAnimBlueprint* UAnimationBlueprintGenerator::GenerateSimpleBP(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	const TArray<UAnimSequence*>& Animations)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const FString BPName = FString::Printf(TEXT("ATK_AnimBP_%s"), *FGuid::NewGuid().ToString().Left(8));
	const FString PackagePath = TEXT("/Game/AnimationBlueprints");
	const FString FullPath = PackagePath + TEXT("/") + BPName;

	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		return nullptr;
	}

	UAnimBlueprint* AnimBlueprint = NewObject<UAnimBlueprint>(Package, *BPName, RF_Public | RF_Standalone);
	if (!AnimBlueprint)
	{
		return nullptr;
	}

	AnimBlueprint->TargetSkeleton = Skeleton;

	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(AnimBlueprint);

	UE_LOG(LogAnimationBlueprintGenerator, Log, TEXT("Created AnimBlueprint '%s' with %d animations"), *BPName, Animations.Num());

	return AnimBlueprint;
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

	const FString BPName = FString::Printf(TEXT("ATK_AnimBP_%s"), *FGuid::NewGuid().ToString().Left(8));
	const FString PackagePath = TEXT("/Game/AnimationBlueprints");
	const FString FullPath = PackagePath + TEXT("/") + BPName;

	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		return nullptr;
	}

	UAnimBlueprint* AnimBlueprint = NewObject<UAnimBlueprint>(Package, *BPName, RF_Public | RF_Standalone);
	if (!AnimBlueprint)
	{
		return nullptr;
	}

	AnimBlueprint->TargetSkeleton = Skeleton;

	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(AnimBlueprint);

	const int32 NumStates = FMath::Max(StateNames.Num(), Animations.Num());
	UE_LOG(LogAnimationBlueprintGenerator, Log, TEXT("Created AnimBlueprint '%s' with %d states/animations"), *BPName, NumStates);

	return AnimBlueprint;
}

TArray<FAnimBPNodeInfo> UAnimationBlueprintGenerator::GetBPStructure(UAnimBlueprint* AnimBlueprint)
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
	if (!AnimBlueprint || !Animation)
	{
		return;
	}

	UE_LOG(LogAnimationBlueprintGenerator, Log, TEXT("AddAnimationToBP: AnimBlueprint '%s', Animation '%s' (node graph editing not supported in this build)"),
		*AnimBlueprint->GetName(), *Animation->GetName());

	AnimBlueprint->MarkPackageDirty();
}

void UAnimationBlueprintGenerator::RemoveAnimationFromBP(
	UObject* WorldContextObject,
	UAnimBlueprint* AnimBlueprint,
	FName NodeName)
{
	if (!AnimBlueprint)
	{
		return;
	}

	UE_LOG(LogAnimationBlueprintGenerator, Log, TEXT("RemoveAnimationFromBP: AnimBlueprint '%s', NodeName '%s' (node graph editing not supported in this build)"),
		*AnimBlueprint->GetName(), *NodeName.ToString());

	AnimBlueprint->MarkPackageDirty();
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

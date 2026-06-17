#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationBlueprintGenerator.generated.h"

USTRUCT(BlueprintType)
struct FAnimBPNodeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NodeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NodeIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParentIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendWeight = 1.0f;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationBlueprintGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Blueprint", meta = (WorldContext = "WorldContextObject"))
	static UAnimBlueprint* GenerateSimpleBP(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Blueprint", meta = (WorldContext = "WorldContextObject"))
	static UAnimBlueprint* GenerateBPWithStates(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		const TArray<UAnimSequence*>& Animations,
		const TArray<FName>& StateNames);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Blueprint")
	static TArray<FAnimBPNodeInfo> GetBPStructure(
		UAnimBlueprint* AnimBlueprint);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Blueprint", meta = (WorldContext = "WorldContextObject"))
	static void AddAnimationToBP(
		UObject* WorldContextObject,
		UAnimBlueprint* AnimBlueprint,
		UAnimSequence* Animation,
		FName NodeName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Blueprint", meta = (WorldContext = "WorldContextObject"))
	static void RemoveAnimationFromBP(
		UObject* WorldContextObject,
		UAnimBlueprint* AnimBlueprint,
		FName NodeName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Blueprint")
	static int32 GetNodeCount(UAnimBlueprint* AnimBlueprint);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Blueprint")
	static TArray<FName> GetAnimationNames(UAnimBlueprint* AnimBlueprint);
};

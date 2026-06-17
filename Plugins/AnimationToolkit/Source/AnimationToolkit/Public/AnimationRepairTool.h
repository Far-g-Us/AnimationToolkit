#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationRepairTool.generated.h"

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationRepairTool : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Repair", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* RepairAnimationMissingBones(
		UObject* WorldContextObject,
		UAnimSequence* BrokenAnimation,
		USkeleton* TargetSkeleton,
		FAnimationRepairResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Repair", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ReassignAnimationSkeleton(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		USkeleton* NewSkeleton,
		bool bAutoRemap = true,
		FAnimationRepairResult& OutResult);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Repair")
	static FAnimationRepairResult AnalyzeAnimationBones(
		UAnimSequence* Animation,
		USkeleton* ExpectedSkeleton);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Repair", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchRepairAnimations(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& BrokenAnimations,
		USkeleton* TargetSkeleton,
		FAnimationRepairResult& OutResult);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Repair")
	static bool IsAnimationBroken(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Repair")
	static TArray<FName> GetMissingBones(UAnimSequence* Animation, USkeleton* ExpectedSkeleton);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Repair")
	static TArray<FName> GetExtraBones(UAnimSequence* Animation, USkeleton* ExpectedSkeleton);

private:
	static void FillMissingBonesWithRestPose(
		UAnimSequence* Animation,
		USkeleton* Skeleton,
		const TArray<FName>& MissingBones);

	static void RemoveExtraTracks(
		UAnimSequence* Animation,
		const TArray<FName>& ExtraBones);
};

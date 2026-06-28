#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationIKRig.generated.h"

class UIKRigDefinition;
class UIKRetargeter;

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimationIKRig : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|IK Rig")
	static UIKRigDefinition* CreateIKRigFromSkeleton(USkeleton* Skeleton, const FString& AssetName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|IK Rig")
	static UIKRetargeter* AutoCreateIKRetargeter(USkeleton* SourceSkeleton, USkeleton* TargetSkeleton);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|IK Rig")
	static TArray<FString> GetBoneChains(USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|IK Rig")
	static UAnimSequence* AutoRetargetAnimation(UAnimSequence* Animation, USkeleton* TargetSkeleton);

private:
	static FString NormalizeChainName(const FString& BoneName);
	static FName FindBoneByPattern(USkeleton* Skeleton, const TArray<FString>& Patterns);
	static int32 FindBoneIndexByPattern(const FReferenceSkeleton& RefSkel, const TArray<FString>& Patterns);
	static void CollectBonesBetween(const FReferenceSkeleton& RefSkel, int32 StartIdx, int32 EndIdx, TArray<FName>& OutBones);
};

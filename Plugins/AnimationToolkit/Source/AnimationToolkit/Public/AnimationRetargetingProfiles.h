#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationRetargetingProfiles.generated.h"

USTRUCT(BlueprintType)
struct FRetargetingProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeleton* SourceSkeleton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeleton* TargetSkeleton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FName> BoneMapping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TranslationMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRetargetRoot = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RootBoneName = FName("root");
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationRetargetingProfiles : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Retargeting")
	static FRetargetingProfile CreateProfile(
		const FString& ProfileName,
		USkeleton* SourceSkeleton,
		USkeleton* TargetSkeleton);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Retargeting")
	static void AddBoneMapping(
		FRetargetingProfile& Profile,
		const FName& SourceBone,
		const FName& TargetBone);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Retargeting")
	static void RemoveBoneMapping(
		FRetargetingProfile& Profile,
		const FName& SourceBone);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Retargeting")
	static FName GetMappedBone(
		const FRetargetingProfile& Profile,
		const FName& SourceBone);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Retargeting")
	static bool HasMapping(
		const FRetargetingProfile& Profile,
		const FName& SourceBone);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Retargeting", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* RetargetWithProfile(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		const FRetargetingProfile& Profile);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Retargeting", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* AutoRetarget(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		USkeleton* TargetSkeleton);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Retargeting")
	static TArray<FName> GetUnmappedBones(const FRetargetingProfile& Profile);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Retargeting")
	static float CalculateRetargetCoverage(const FRetargetingProfile& Profile);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Retargeting")
	static FRetargetingProfile MergeProfiles(
		const FRetargetingProfile& ProfileA,
		const FRetargetingProfile& ProfileB);
};

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationMirrorTool.generated.h"

USTRUCT(BlueprintType)
struct FMirrorBoneMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OriginalBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MirroredBone;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationMirrorTool : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Mirror", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* MirrorAnimation(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		const TArray<FMirrorBoneMapping>& BoneMappings,
		bool bMirrorTranslation = true,
		bool bMirrorRotation = true);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Mirror", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* MirrorAnimationAxis(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		EMirrorFlags MirrorFlags);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Mirror")
	static TArray<FMirrorBoneMapping> GenerateMirrorMappings(
		USkeleton* Skeleton,
		const FString& LeftSuffix = TEXT("_l"),
		const FString& RightSuffix = TEXT("_r"));

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Mirror")
	static TArray<FMirrorBoneMapping> GenerateMirrorMappingsFromPrefix(
		USkeleton* Skeleton,
		const FString& LeftPrefix = TEXT("l_"),
		const FString& RightPrefix = TEXT("r_"));

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Mirror", meta = (WorldContext = "WorldContextObject"))
	static void BatchMirrorAnimations(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations,
		EMirrorFlags MirrorFlags);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Mirror")
	static bool IsMirroredBone(
		const TArray<FMirrorBoneMapping>& Mappings,
		FName BoneName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Mirror")
	static FName GetMirroredBone(
		const TArray<FMirrorBoneMapping>& Mappings,
		FName BoneName);
};

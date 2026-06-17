#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationLODGenerator.generated.h"

USTRUCT(BlueprintType)
struct FAnimationLODSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LOD0BoneThreshold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LOD1BoneThreshold = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LOD2BoneThreshold = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD0TranslationThreshold = 0.001f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD0RotationThreshold = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD1TranslationThreshold = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD1RotationThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD2TranslationThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD2RotationThreshold = 1.0f;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationLODGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LOD")
	static FAnimationLODSettings GetDefaultLODSettings();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LOD", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> GenerateAnimationLODs(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		const FAnimationLODSettings& Settings,
		int32 NumLODs = 3);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LOD", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* GenerateLOD(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		int32 LODLevel,
		const FAnimationLODSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LOD")
	static int32 GetBoneCountForLOD(
		UAnimSequence* Animation,
		int32 LODLevel,
		const FAnimationLODSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LOD")
	static float EstimateLODMemoryUsage(
		UAnimSequence* Animation,
		int32 LODLevel,
		const FAnimationLODSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LOD", meta = (WorldContext = "WorldContextObject"))
	static void BatchGenerateLODs(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations,
		const FAnimationLODSettings& Settings,
		int32 NumLODs = 3);
};

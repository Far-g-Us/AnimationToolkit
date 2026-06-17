#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationVersionConverter.generated.h"

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationVersionConverter : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Version Converter", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ConvertAnimation(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		const FEngineVersionProfile& Profile,
		FAnimationConversionResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Version Converter", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> ConvertAnimations(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& SourceAnimations,
		const FEngineVersionProfile& Profile,
		FAnimationConversionResult& OutResult);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Version Converter")
	static FString GetEngineVersionName(EEngineVersion Version);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Version Converter")
	static float GetRecommendedScale(EEngineVersion SourceVersion, EEngineVersion TargetVersion);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Version Converter")
	static TMap<FName, FName> GetBoneRenamesForVersion(EEngineVersion SourceVersion, EEngineVersion TargetVersion);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Version Converter")
	static bool IsVersionCompatible(EEngineVersion SourceVersion, EEngineVersion TargetVersion);

private:
	static TArray<FName> GetDeprecatedBones(EEngineVersion Version);
	static TArray<FName> GetAddedBones(EEngineVersion Version);
	static float GetUnitScale(EEngineVersion Version);
	static void ApplyScaleToAnimation(UAnimSequence* Animation, float ScaleMultiplier);
	static void RenameBonesInAnimation(UAnimSequence* Animation, const TMap<FName, FName>& Renames);
};

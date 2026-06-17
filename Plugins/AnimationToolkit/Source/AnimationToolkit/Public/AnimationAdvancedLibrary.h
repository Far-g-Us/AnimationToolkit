#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationAdvancedLibrary.generated.h"

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationAdvancedLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Advanced", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* AdditiveBlendAnimations(
		UObject* WorldContextObject,
		const FString& AnimationName,
		USkeleton* Skeleton,
		UAnimSequence* BaseAnimation,
		UAnimSequence* AdditiveAnimation,
		float AdditiveWeight = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Advanced", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ApplyAdditiveLayer(
		UObject* WorldContextObject,
		UAnimSequence* BaseAnimation,
		UAnimSequence* AdditiveLayer,
		float LayerWeight,
		const TArray<FName>& AffectedBones);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Advanced")
	static bool ExtractRootMotion(
		UAnimSequence* Animation,
		TArray<FVector>& OutPositions,
		TArray<FRotator>& OutRotations,
		TArray<float>& OutTimes);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Advanced", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ApplyRootMotion(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		const TArray<FVector>& RootPositions,
		const TArray<FRotator>& RootRotations);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Advanced")
	static float QueryAnimationCurve(
		UAnimSequence* Animation,
		FName CurveName,
		float Time);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Advanced")
	static TArray<FAnimationCurveKeyframe> QueryAnimationCurveAll(
		UAnimSequence* Animation,
		FName CurveName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Advanced")
	static FTransform GetBoneTransformAtTimeInterpolated(
		UAnimSequence* Animation,
		FName BoneName,
		float Time,
		bool bInterpolate = true);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Advanced")
	static TArray<FName> GetAnimationBoneNames(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Advanced")
	static float GetAnimationDuration(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Advanced")
	static int32 GetAnimationFrameCount(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Advanced", meta = (WorldContext = "WorldContextObject"))
	static bool SaveAnimationAsAsset(
		UObject* WorldContextObject,
		UAnimSequence* Animation,
		const FString& PackagePath,
		const FString& AssetName);
};

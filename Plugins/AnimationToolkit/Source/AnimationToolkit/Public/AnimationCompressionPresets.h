#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationCompressionPresets.generated.h"

UENUM(BlueprintType)
enum class ECompressionPreset : uint8
{
	HighQuality,
	Medium,
	Low,
	Mobile,
	Custom
};

USTRUCT(BlueprintType)
struct FCompressionPresetSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TranslationThreshold = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleThreshold = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompressTranslation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompressRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompressScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxKeysPerBone = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRemoveConstantKeys = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDetectSequenceLength = true;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationCompressionPresets : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Compression")
	static FCompressionPresetSettings GetPresetSettings(ECompressionPreset Preset);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Compression", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CompressWithPreset(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		ECompressionPreset Preset);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Compression", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CompressWithCustomSettings(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		const FCompressionPresetSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Compression", meta = (WorldContext = "WorldContextObject"))
	static void BatchCompressAnimations(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations,
		ECompressionPreset Preset);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Compression")
	static int32 EstimateCompressedSize(
		UAnimSequence* Animation,
		const FCompressionPresetSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Compression")
	static float GetCompressionRatio(
		UAnimSequence* Original,
		UAnimSequence* Compressed);
};

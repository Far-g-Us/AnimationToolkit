#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationExportPresets.generated.h"

UENUM(BlueprintType)
enum class EExportFormat : uint8
{
	FBX,
	glTF,
	BVH,
	COLLADA,
	USD
};

USTRUCT(BlueprintType)
struct FExportPresetSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EExportFormat Format = EExportFormat::FBX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExportMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExportSkeleton = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExportAnimation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConvertToYUp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExportMorphTargets = true;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationExportPresets : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Export")
	static FExportPresetSettings GetFBXPreset();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Export")
	static FExportPresetSettings GetGlTFPreset();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Export")
	static FExportPresetSettings GetBVHPreset();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Export")
	static FExportPresetSettings GetCOLLADAPreset();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Export")
	static FExportPresetSettings GetUSDPreset();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Export", meta = (WorldContext = "WorldContextObject"))
	static bool ExportWithPreset(
		UObject* WorldContextObject,
		UAnimSequence* Animation,
		const FString& FilePath,
		const FExportPresetSettings& Preset);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Export", meta = (WorldContext = "WorldContextObject"))
	static void BatchExportAnimations(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations,
		const FString& DirectoryPath,
		const FExportPresetSettings& Preset);
};

#include "AnimationExportPresets.h"
#include "Animation/AnimSequence.h"

FExportPresetSettings UAnimationExportPresets::GetFBXPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::FBX;
	Preset.bExportMesh = true;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = false;
	Preset.bExportMorphTargets = true;
	return Preset;
}

FExportPresetSettings UAnimationExportPresets::GetGlTFPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::glTF;
	Preset.bExportMesh = true;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = true;
	Preset.bExportMorphTargets = true;
	return Preset;
}

FExportPresetSettings UAnimationExportPresets::GetBVHPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::BVH;
	Preset.bExportMesh = false;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = false;
	Preset.bExportMorphTargets = false;
	return Preset;
}

FExportPresetSettings UAnimationExportPresets::GetCOLLADAPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::COLLADA;
	Preset.bExportMesh = true;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = true;
	Preset.bExportMorphTargets = true;
	return Preset;
}

FExportPresetSettings UAnimationExportPresets::GetUSDPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::USD;
	Preset.bExportMesh = true;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = true;
	Preset.bExportMorphTargets = true;
	return Preset;
}

bool UAnimationExportPresets::ExportWithPreset(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	const FString& FilePath,
	const FExportPresetSettings& Preset)
{
	if (!WorldContextObject || !Animation)
	{
		return false;
	}

	return false;
}

void UAnimationExportPresets::BatchExportAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	const FString& DirectoryPath,
	const FExportPresetSettings& Preset)
{
}

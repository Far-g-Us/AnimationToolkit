#include "AnimationCompressionPresets.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

FCompressionPresetSettings UAnimationCompressionPresets::GetPresetSettings(ECompressionPreset Preset)
{
	FCompressionPresetSettings Settings;

	switch (Preset)
	{
	case ECompressionPreset::HighQuality:
		Settings.TranslationThreshold = 0.001f;
		Settings.RotationThreshold = 0.01f;
		Settings.ScaleThreshold = 0.001f;
		Settings.bCompressTranslation = true;
		Settings.bCompressRotation = true;
		Settings.bCompressScale = true;
		Settings.MaxKeysPerBone = 0;
		Settings.bRemoveConstantKeys = true;
		break;

	case ECompressionPreset::Medium:
		Settings.TranslationThreshold = 0.01f;
		Settings.RotationThreshold = 0.1f;
		Settings.ScaleThreshold = 0.01f;
		Settings.bCompressTranslation = true;
		Settings.bCompressRotation = true;
		Settings.bCompressScale = false;
		Settings.MaxKeysPerBone = 0;
		Settings.bRemoveConstantKeys = true;
		break;

	case ECompressionPreset::Low:
		Settings.TranslationThreshold = 0.1f;
		Settings.RotationThreshold = 1.0f;
		Settings.ScaleThreshold = 0.1f;
		Settings.bCompressTranslation = true;
		Settings.bCompressRotation = true;
		Settings.bCompressScale = false;
		Settings.MaxKeysPerBone = 0;
		Settings.bRemoveConstantKeys = true;
		break;

	case ECompressionPreset::Mobile:
		Settings.TranslationThreshold = 0.1f;
		Settings.RotationThreshold = 2.0f;
		Settings.ScaleThreshold = 0.1f;
		Settings.bCompressTranslation = true;
		Settings.bCompressRotation = true;
		Settings.bCompressScale = false;
		Settings.MaxKeysPerBone = 32;
		Settings.bRemoveConstantKeys = true;
		break;

	case ECompressionPreset::Custom:
	default:
		break;
	}

	return Settings;
}

UAnimSequence* UAnimationCompressionPresets::CompressWithPreset(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	ECompressionPreset Preset)
{
	if (!WorldContextObject || !SourceAnimation)
	{
		return nullptr;
	}

	FCompressionPresetSettings Settings = GetPresetSettings(Preset);
	return CompressWithCustomSettings(WorldContextObject, SourceAnimation, Settings);
}

UAnimSequence* UAnimationCompressionPresets::CompressWithCustomSettings(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const FCompressionPresetSettings& Settings)
{
	return nullptr;
}

void UAnimationCompressionPresets::BatchCompressAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	ECompressionPreset Preset)
{
	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			CompressWithPreset(WorldContextObject, Anim, Preset);
		}
	}
}

int32 UAnimationCompressionPresets::EstimateCompressedSize(
	UAnimSequence* Animation,
	const FCompressionPresetSettings& Settings)
{
	return 0;
}

float UAnimationCompressionPresets::GetCompressionRatio(
	UAnimSequence* Original,
	UAnimSequence* Compressed)
{
	if (!Original || !Compressed)
	{
		return 1.0f;
	}

	if (!Original->GetSkeleton() || !Compressed->GetSkeleton())
	{
		return 1.0f;
	}

	int32 OriginalSize = Original->GetNumberOfSampledKeys() *
		Original->GetSkeleton()->GetReferenceSkeleton().GetNum() *
		(sizeof(FVector) + sizeof(FQuat) + sizeof(FVector));

	int32 CompressedSize = 0;
	if (Compressed->GetSkeleton())
	{
		CompressedSize = Compressed->GetNumberOfSampledKeys() *
			Compressed->GetSkeleton()->GetReferenceSkeleton().GetNum() *
			(sizeof(FVector) + sizeof(FQuat) + sizeof(FVector));
	}

	if (OriginalSize == 0)
	{
		return 1.0f;
	}

	return (float)CompressedSize / (float)OriginalSize;
}

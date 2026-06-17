#include "AnimationCompressionPresets.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/Engine.h"
#include "Serialization/MemoryWriter.h"

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
		Settings.bAutoDetectSequenceLength = true;
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
		Settings.bAutoDetectSequenceLength = true;
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
		Settings.bAutoDetectSequenceLength = true;
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
		Settings.bAutoDetectSequenceLength = true;
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
	if (!SourceAnimation)
	{
		return nullptr;
	}

	const USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();
	if (NumBones == 0)
	{
		return nullptr;
	}

	UE_LOG(LogAnimation, Log, TEXT("Compressing animation '%s' (%d bones, preset thresholds: T=%.4f R=%.4f S=%.4f)"),
		*SourceAnimation->GetName(), NumBones,
		Settings.TranslationThreshold, Settings.RotationThreshold, Settings.ScaleThreshold);

	SourceAnimation->RequestAnimCompression(true);

	SourceAnimation->MarkPackageDirty();
	return SourceAnimation;
}

void UAnimationCompressionPresets::BatchCompressAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	ECompressionPreset Preset)
{
	FCompressionPresetSettings Settings = GetPresetSettings(Preset);

	int32 CompressedCount = 0;
	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			const USkeleton* AnimSkel = Anim->GetSkeleton();
			if (AnimSkel && AnimSkel->GetReferenceSkeleton().GetNum() > 0)
			{
				Anim->RequestAnimCompression(true);
				Anim->MarkPackageDirty();
				CompressedCount++;
			}
		}
	}

	UE_LOG(LogAnimation, Log, TEXT("BatchCompressAnimations: Requested compression for %d animations (preset thresholds: T=%.4f R=%.4f S=%.4f)"),
		CompressedCount, Settings.TranslationThreshold, Settings.RotationThreshold, Settings.ScaleThreshold);
}

int32 UAnimationCompressionPresets::EstimateCompressedSize(
	UAnimSequence* Animation,
	const FCompressionPresetSettings& Settings)
{
	if (!Animation)
	{
		return 0;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return 0;
	}

	const int32 NumFrames = Animation->GetNumberOfSampledKeys();
	const int32 NumBones = Skeleton->GetReferenceSkeleton().GetNum();

	constexpr int32 TranslationSize = sizeof(FVector);
	constexpr int32 RotationSize = sizeof(FQuat);
	constexpr int32 ScaleSize = sizeof(FVector);
	constexpr int32 FullTrackSize = TranslationSize + RotationSize + ScaleSize;

	const int64 UncompressedSize = static_cast<int64>(NumFrames) * NumBones * FullTrackSize;

	float ReductionFactor = 1.0f;

	if (Settings.bRemoveConstantKeys)
	{
		ReductionFactor *= 0.65f;
	}

	if (Settings.MaxKeysPerBone > 0 && NumFrames > Settings.MaxKeysPerBone)
	{
		ReductionFactor *= static_cast<float>(Settings.MaxKeysPerBone) / static_cast<float>(NumFrames);
	}

	const float AvgThreshold = (Settings.TranslationThreshold + Settings.RotationThreshold + Settings.ScaleThreshold) / 3.0f;
	const float ThresholdFactor = FMath::Clamp(1.0f - FMath::Loge(1.0f + AvgThreshold * 3.0f) * 0.25f, 0.2f, 1.0f);
	ReductionFactor *= ThresholdFactor;

	const int32 CodecOverhead = NumBones * 48 + 256;

	return FMath::Max(1, static_cast<int32>(UncompressedSize * ReductionFactor) + CodecOverhead);
}

float UAnimationCompressionPresets::GetCompressionRatio(
	UAnimSequence* Original,
	UAnimSequence* Compressed)
{
	if (!Original || !Compressed)
	{
		return 1.0f;
	}

	const int32 OrigFrames = Original->GetNumberOfSampledKeys();
	const int32 CompFrames = Compressed->GetNumberOfSampledKeys();

	if (OrigFrames == 0)
	{
		return 1.0f;
	}

	const int32 OrigSize = OrigFrames * (sizeof(float) * 10);
	const int32 CompSize = CompFrames * (sizeof(float) * 10);

	if (OrigSize == 0)
	{
		return 1.0f;
	}

	return static_cast<float>(CompSize) / static_cast<float>(OrigSize);
}

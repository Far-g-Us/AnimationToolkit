#include "AnimationCompressionPresets.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

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
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	FString NewName = FString::Printf(TEXT("%s_Compressed"), *SourceAnimation->GetName());

	UAnimSequence* CompressedAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!CompressedAnim)
	{
		return nullptr;
	}

	CompressedAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	CompressedAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);
		int32 SourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);

		FRawAnimSequenceTrack& DestTrack = CompressedAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		if (SourceBoneIdx == INDEX_NONE)
		{
			continue;
		}

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform SourceTransform = SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame);

			FVector Pos = SourceTransform.GetLocation();
			FQuat Rot = SourceTransform.GetRotation();
			FVector Scale = SourceTransform.GetScale3D();

			if (Settings.bCompressTranslation)
			{
				if (Frame > 0 && FVector::Dist(Pos, DestTrack.PosKeys[Frame - 1]) < Settings.TranslationThreshold)
				{
					Pos = DestTrack.PosKeys[Frame - 1];
				}
			}

			if (Settings.bCompressRotation)
			{
				if (Frame > 0 && Rot.AngularDistance(DestTrack.RotKeys[Frame - 1]) < Settings.RotationThreshold)
				{
					Rot = DestTrack.RotKeys[Frame - 1];
				}
			}

			if (Settings.bCompressScale)
			{
				if (Frame > 0 && FVector::Dist(Scale, DestTrack.ScaleKeys[Frame - 1]) < Settings.ScaleThreshold)
				{
					Scale = DestTrack.ScaleKeys[Frame - 1];
				}
			}

			DestTrack.PosKeys[Frame] = Pos;
			DestTrack.RotKeys[Frame] = Rot;
			DestTrack.ScaleKeys[Frame] = Scale;
		}
	}

	CompressedAnim->MarkPackageDirty();
	return CompressedAnim;
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
	if (!Animation || !Animation->GetSkeleton())
	{
		return 0;
	}

	const FReferenceSkeleton& RefSkeleton = Animation->GetSkeleton()->GetReferenceSkeleton();
	int32 BoneCount = RefSkeleton.GetNum();
	int32 FrameCount = Animation->GetNumberOfSampledKeys();

	int32 EstimatedKeys = 0;

	for (int32 BoneIdx = 0; BoneIdx < BoneCount; BoneIdx++)
	{
		int32 SourceBoneIdx = Animation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(
			RefSkeleton.GetBoneName(BoneIdx));

		if (SourceBoneIdx == INDEX_NONE)
		{
			continue;
		}

		int32 KeptKeys = 0;
		FVector LastPos = FVector::ZeroVector;
		FQuat LastRot = FQuat::Identity;

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform SourceTransform = Animation->GetBoneTransform(SourceBoneIdx, Frame);

			bool bKeepKey = true;

			if (Frame > 0 && Settings.bCompressTranslation)
			{
				if (FVector::Dist(SourceTransform.GetLocation(), LastPos) < Settings.TranslationThreshold)
				{
					bKeepKey = false;
				}
			}

			if (Frame > 0 && Settings.bCompressRotation)
			{
				if (SourceTransform.GetRotation().AngularDistance(LastRot) < Settings.RotationThreshold)
				{
					bKeepKey = false;
				}
			}

			if (bKeepKey)
			{
				KeptKeys++;
			}

			LastPos = SourceTransform.GetLocation();
			LastRot = SourceTransform.GetRotation();
		}

		EstimatedKeys += KeptKeys;
	}

	return EstimatedKeys * (sizeof(FVector) + sizeof(FQuat) + sizeof(FVector));
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

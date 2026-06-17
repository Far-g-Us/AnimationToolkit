#include "AnimationLibrary.h"
#include "AnimSequenceCreator.h"
#include "AnimMontageCreator.h"
#include "AnimBlendSpaceCreator.h"
#include "AnimationImporter.h"
#include "AsyncImportHelper.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Skeleton.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

static TMap<TWeakObjectPtr<UObject>, TArray<FName>> AnimationTagsMap;
static FOnAnimationProgress GlobalProgressCallback;

UAnimSequence* UAnimationLibrary::CreateAnimation(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	float Duration,
	float FrameRate)
{
	return UAnimSequenceCreator::CreateAnimSequenceFromKeyframes(
		WorldContextObject,
		AnimationName,
		Skeleton,
		Tracks,
		Duration,
		FrameRate
	);
}

UAnimMontage* UAnimationLibrary::CreateMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections)
{
	return UAnimMontageCreator::CreateAnimMontage(
		WorldContextObject,
		MontageName,
		Skeleton,
		Sections
	);
}

UBlendSpace* UAnimationLibrary::CreateBlendSpace1D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinValue,
	float MaxValue)
{
	return UAnimBlendSpaceCreator::CreateBlendSpace1D(
		WorldContextObject,
		BlendSpaceName,
		Skeleton,
		Samples,
		MinValue,
		MaxValue,
		FName("Value")
	);
}

UBlendSpace* UAnimationLibrary::CreateBlendSpace2D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinX,
	float MaxX,
	float MinY,
	float MaxY)
{
	return UAnimBlendSpaceCreator::CreateBlendSpace2D(
		WorldContextObject,
		BlendSpaceName,
		Skeleton,
		Samples,
		MinX,
		MaxX,
		MinY,
		MaxY,
		FName("X"),
		FName("Y")
	);
}

UAnimSequence* UAnimationLibrary::ImportAnimationAsSequence(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	return UAnimationImporter::ImportAndCreateAnimSequence(
		WorldContextObject,
		FilePath,
		AnimationName,
		Skeleton,
		Format
	);
}

FAnimationImportResult UAnimationLibrary::ImportAnimationFile(
	UObject* WorldContextObject,
	const FString& FilePath,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	return UAnimationImporter::ImportAnimation(
		WorldContextObject,
		FilePath,
		Format,
		Skeleton
	);
}

FAnimationTrackData UAnimationLibrary::CreateTrack(
	const FName& BoneName,
	const TArray<FTransform>& Transforms,
	float FrameRate)
{
	FAnimationTrackData TrackData;
	TrackData.TrackName = BoneName;

	for (int32 i = 0; i < Transforms.Num(); i++)
	{
		FAnimationKeyframe Keyframe;
		Keyframe.Time = (float)i / FrameRate;
		Keyframe.Transform = Transforms[i];
		TrackData.Keyframes.Add(Keyframe);
	}

	return TrackData;
}

FAnimationKeyframe UAnimationLibrary::MakeKeyframe(
	float Time,
	const FTransform& Transform)
{
	return UAnimSequenceCreator::CreateKeyframe(Time, Transform);
}

FAnimMontageSectionData UAnimationLibrary::MakeMontageSection(
	const FName& SectionName,
	UAnimSequence* Animation,
	float StartTime,
	float EndTime)
{
	return UAnimMontageCreator::CreateMontageSectionData(
		SectionName,
		Animation,
		StartTime,
		EndTime
	);
}

FBlendSpaceSampleData UAnimationLibrary::MakeBlendSpaceSample(
	UAnimSequence* Animation,
	const FVector& SampleValue)
{
	return UAnimBlendSpaceCreator::CreateBlendSpaceSample(
		Animation,
		SampleValue
	);
}

EAnimationImportFormat UAnimationLibrary::DetectAnimationFormat(const FString& FilePath)
{
	return UAnimationImporter::DetectFormat(FilePath);
}

bool UAnimationLibrary::IsAnimationFormatSupported(const FString& FilePath)
{
	EAnimationImportFormat Format;
	return UAnimationImporter::IsSupportedFormat(FilePath, Format);
}

FString UAnimationLibrary::GetAnimationFormatName(EAnimationImportFormat Format)
{
	return UAnimationImporter::GetFormatExtension(Format);
}

TArray<EAnimationImportFormat> UAnimationLibrary::GetAllSupportedFormats()
{
	return UAnimationImporter::GetSupportedFormats();
}

TArray<FString> UAnimationLibrary::GetSkeletonBoneNames(USkeleton* Skeleton)
{
	TArray<FString> BoneNames;

	if (Skeleton)
	{
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
		const int32 NumBones = RefSkeleton.GetNum();

		for (int32 i = 0; i < NumBones; i++)
		{
			BoneNames.Add(RefSkeleton.GetBoneName(i).ToString());
		}
	}

	return BoneNames;
}

float UAnimationLibrary::GetAnimSequenceDuration(UAnimSequence* AnimSequence)
{
	if (AnimSequence)
	{
		return AnimSequence->GetSequenceLength();
	}
	return 0.0f;
}

int32 UAnimationLibrary::GetAnimSequenceFrameCount(UAnimSequence* AnimSequence)
{
	if (AnimSequence)
	{
		return AnimSequence->GetNumberOfSampledKeys();
	}
	return 0;
}

FAnimationCurveData UAnimationLibrary::CreateCurveData(
	const FName& CurveName,
	const TArray<FAnimationCurveKeyframe>& Keyframes)
{
	FAnimationCurveData CurveData;
	CurveData.CurveName = CurveName;
	CurveData.Keyframes = Keyframes;
	return CurveData;
}

FAnimationCurveKeyframe UAnimationLibrary::MakeCurveKeyframe(float Time, float Value)
{
	FAnimationCurveKeyframe Keyframe;
	Keyframe.Time = Time;
	Keyframe.Value = Value;
	return Keyframe;
}

UAnimSequence* UAnimationLibrary::CreateAnimationWithCurves(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	const TArray<FAnimationCurveData>& Curves,
	float Duration,
	float FrameRate)
{
	UAnimSequence* AnimSequence = UAnimSequenceCreator::CreateAnimSequenceFromKeyframes(
		WorldContextObject,
		AnimationName,
		Skeleton,
		Tracks,
		Duration,
		FrameRate
	);

	if (AnimSequence)
	{
		for (const FAnimationCurveData& CurveData : Curves)
		{
			AddCurveToAnimSequence(AnimSequence, CurveData);
		}
	}

	return AnimSequence;
}

bool UAnimationLibrary::AddCurveToAnimSequence(UAnimSequence* AnimSequence, const FAnimationCurveData& CurveData)
{
	if (!AnimSequence || CurveData.Keyframes.Num() == 0)
	{
		return false;
	}

	AnimSequence->MarkPackageDirty();
	return true;
}

FAnimationBatchImportResult UAnimationLibrary::BatchImportAnimations(
	UObject* WorldContextObject,
	const TArray<FAnimationBatchImportItem>& Items,
	USkeleton* Skeleton)
{
	FAnimationBatchImportResult BatchResult;
	BatchResult.bSuccess = true;

	for (const FAnimationBatchImportItem& Item : Items)
	{
		FAnimationImportResult Result = UAnimationImporter::ImportAnimation(
			WorldContextObject,
			Item.FilePath,
			Item.Format,
			Skeleton
		);

		BatchResult.Results.Add(Result);

		if (Result.bSuccess)
		{
			BatchResult.SuccessCount++;
		}
		else
		{
			BatchResult.FailCount++;
		}
	}

	BatchResult.bSuccess = (BatchResult.FailCount == 0);
	return BatchResult;
}

TArray<UAnimSequence*> UAnimationLibrary::BatchImportAsSequences(
	UObject* WorldContextObject,
	const TArray<FAnimationBatchImportItem>& Items,
	USkeleton* Skeleton)
{
	TArray<UAnimSequence*> Sequences;

	for (const FAnimationBatchImportItem& Item : Items)
	{
		UAnimSequence* AnimSequence = UAnimationImporter::ImportAndCreateAnimSequence(
			WorldContextObject,
			Item.FilePath,
			Item.AnimationName,
			Skeleton,
			Item.Format
		);

		if (AnimSequence)
		{
			Sequences.Add(AnimSequence);
		}
	}

	return Sequences;
}

FAnimationRetargetMapping UAnimationLibrary::MakeRetargetMapping(
	const FName& SourceBone,
	const FName& TargetBone)
{
	FAnimationRetargetMapping Mapping;
	Mapping.SourceBone = SourceBone;
	Mapping.TargetBone = TargetBone;
	return Mapping;
}

FAnimationRetargetSettings UAnimationLibrary::MakeRetargetSettings(
	EAnimationRetargetType RetargetType,
	const TArray<FAnimationRetargetMapping>& CustomMappings)
{
	FAnimationRetargetSettings Settings;
	Settings.RetargetType = RetargetType;
	Settings.CustomMappings = CustomMappings;
	Settings.bMatchByHierarchy = true;
	Settings.bAdjustRootMotion = true;
	return Settings;
}

UAnimSequence* UAnimationLibrary::RetargetAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	USkeleton* TargetSkeleton,
	const FAnimationRetargetSettings& Settings)
{
	if (!SourceAnimation || !TargetSkeleton)
	{
		return nullptr;
	}

	USkeleton* SourceSkeleton = SourceAnimation->GetSkeleton();
	if (!SourceSkeleton)
	{
		return nullptr;
	}

	const FReferenceSkeleton& SourceRefSkeleton = SourceSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& TargetRefSkeleton = TargetSkeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> RetargetedTracks;

	for (int32 i = 0; i < SourceRefSkeleton.GetNum(); i++)
	{
		FName SourceBoneName = SourceRefSkeleton.GetBoneName(i);

		FName TargetBoneName = NAME_None;

		if (Settings.RetargetType == EAnimationRetargetType::OneToOne)
		{
			TargetBoneName = SourceBoneName;
		}
		else if (Settings.RetargetType == EAnimationRetargetType::ByName)
		{
			TargetBoneName = SourceBoneName;
		}
		else if (Settings.RetargetType == EAnimationRetargetType::ByIndex)
		{
			if (i < TargetRefSkeleton.GetNum())
			{
				TargetBoneName = TargetRefSkeleton.GetBoneName(i);
			}
		}

		for (const FAnimationRetargetMapping& Mapping : Settings.CustomMappings)
		{
			if (Mapping.SourceBone == SourceBoneName)
			{
				TargetBoneName = Mapping.TargetBone;
				break;
			}
		}

		if (TargetBoneName != NAME_None)
		{
			int32 TargetBoneIndex = TargetRefSkeleton.FindBoneIndex(TargetBoneName);
			if (TargetBoneIndex != INDEX_NONE)
			{
				FAnimationTrackData Track;
				Track.TrackName = TargetBoneName;

				FAnimationKeyframe Keyframe;
				Keyframe.Time = 0.0f;
				Keyframe.Transform = SourceRefSkeleton.GetBoneTransform(i);
				Track.Keyframes.Add(Keyframe);

				RetargetedTracks.Add(Track);
			}
		}
	}

	return CreateAnimation(
		WorldContextObject,
		SourceAnimation->GetName() + TEXT("_Retargeted"),
		TargetSkeleton,
		RetargetedTracks,
		SourceAnimation->GetSequenceLength(),
		30.0f
	);
}

TArray<UAnimSequence*> UAnimationLibrary::BatchRetargetAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& SourceAnimations,
	USkeleton* TargetSkeleton,
	const FAnimationRetargetSettings& Settings)
{
	TArray<UAnimSequence*> RetargetedAnimations;

	for (UAnimSequence* SourceAnimation : SourceAnimations)
	{
		UAnimSequence* Retargeted = RetargetAnimation(
			WorldContextObject,
			SourceAnimation,
			TargetSkeleton,
			Settings
		);

		if (Retargeted)
		{
			RetargetedAnimations.Add(Retargeted);
		}
	}

	return RetargetedAnimations;
}

FAnimationCompressionSettings UAnimationLibrary::MakeCompressionSettings(
	bool bCompressTranslation,
	bool bCompressRotation,
	bool bCompressScale,
	float TranslationThreshold,
	float RotationThreshold,
	float ScaleThreshold)
{
	FAnimationCompressionSettings Settings;
	Settings.bCompressTranslation = bCompressTranslation;
	Settings.bCompressRotation = bCompressRotation;
	Settings.bCompressScale = bCompressScale;
	Settings.TranslationThreshold = TranslationThreshold;
	Settings.RotationThreshold = RotationThreshold;
	Settings.ScaleThreshold = ScaleThreshold;
	Settings.MaxKeyReduction = 4;
	return Settings;
}

bool UAnimationLibrary::CompressAnimation(
	UAnimSequence* AnimSequence,
	const FAnimationCompressionSettings& Settings)
{
	if (!AnimSequence)
	{
		return false;
	}

	if (AnimSequence->GetRawAnimationData().Tracks.Num() == 0)
	{
		return false;
	}

	FRawAnimSequenceTrack& RawTrack = AnimSequence->GetRawAnimationData().Tracks[0];

	if (Settings.bCompressTranslation)
	{
		TArray<FVector> CompressedPositions;
		FVector LastPos = FVector::ZeroVector;

		for (const FVector& Pos : RawTrack.PosKeys)
		{
			if (FVector::Dist(Pos, LastPos) > Settings.TranslationThreshold)
			{
				CompressedPositions.Add(Pos);
				LastPos = Pos;
			}
		}

		if (CompressedPositions.Num() > 0)
		{
			RawTrack.PosKeys = CompressedPositions;
		}
	}

	if (Settings.bCompressRotation)
	{
		TArray<FQuat> CompressedRotations;
		FQuat LastRot = FQuat::Identity;

		for (const FQuat& Rot : RawTrack.RotKeys)
		{
			if (FQuat::AngularDistance(Rot, LastRot) > FMath::DegreesToRadians(Settings.RotationThreshold))
			{
				CompressedRotations.Add(Rot);
				LastRot = Rot;
			}
		}

		if (CompressedRotations.Num() > 0)
		{
			RawTrack.RotKeys = CompressedRotations;
		}
	}

	if (Settings.bCompressScale)
	{
		TArray<FVector> CompressedScales;
		FVector LastScale = FVector::OneVector;

		for (const FVector& Scale : RawTrack.ScaleKeys)
		{
			if (FVector::Dist(Scale, LastScale) > Settings.ScaleThreshold)
			{
				CompressedScales.Add(Scale);
				LastScale = Scale;
			}
		}

		if (CompressedScales.Num() > 0)
		{
			RawTrack.ScaleKeys = CompressedScales;
		}
	}

	AnimSequence->MarkPackageDirty();
	return true;
}

int32 UAnimationLibrary::BatchCompressAnimations(
	const TArray<UAnimSequence*>& AnimSequences,
	const FAnimationCompressionSettings& Settings)
{
	int32 SuccessCount = 0;

	for (UAnimSequence* AnimSequence : AnimSequences)
	{
		if (CompressAnimation(AnimSequence, Settings))
		{
			SuccessCount++;
		}
	}

	return SuccessCount;
}

float UAnimationLibrary::GetAnimationCompressionRatio(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return 1.0f;
	}

	int32 OriginalSize = AnimSequence->GetNumberOfSampledKeys() * sizeof(FVector) * 3;
	int32 CompressedSize = AnimSequence->GetApproxSize();

	if (OriginalSize > 0)
	{
		return (float)CompressedSize / (float)OriginalSize;
	}

	return 1.0f;
}

int32 UAnimationLibrary::GetAnimationKeyCount(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return 0;
	}

	return AnimSequence->GetNumberOfSampledKeys();
}

UAsyncImportHelper* UAnimationLibrary::StartAsyncImport(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	return UAsyncImportHelper::StartAsyncImport(
		WorldContextObject,
		FilePath,
		AnimationName,
		Skeleton,
		Format
	);
}

UAnimSequence* UAnimationLibrary::BlendAnimations(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<UAnimSequence*>& Animations,
	const TArray<float>& Weights)
{
	if (!WorldContextObject || !Skeleton || Animations.Num() == 0)
	{
		return nullptr;
	}

	if (Animations.Num() != Weights.Num())
	{
		return nullptr;
	}

	if (!Animations[0])
	{
		return nullptr;
	}

	int32 FrameCount = Animations[0]->GetNumberOfSampledKeys();
	float Duration = Animations[0]->GetSequenceLength();

	TArray<FAnimationTrackData> BlendedTracks;

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); BoneIndex++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
		FAnimationTrackData Track;
		Track.TrackName = BoneName;

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			float Time = (float)Frame / 30.0f;

			FVector BlendedLocation = FVector::ZeroVector;
			FQuat BlendedRotation = FQuat::Identity;
			FVector BlendedScale = FVector::OneVector;

			float TotalWeight = 0.0f;

			for (int32 i = 0; i < Animations.Num(); i++)
			{
				float Weight = Weights[i];
				float AnimDuration = Animations[i] ? Animations[i]->GetSequenceLength() : 0.0f;
				if (Weight > 0.0f && Animations[i] && AnimDuration > 0.0f)
				{
					float NormalizedTime = FMath::Fmod(Time, AnimDuration);
					float Alpha = NormalizedTime / AnimDuration;
					int32 NumKeys = Animations[i]->GetNumberOfSampledKeys();
					int32 SampleIndex = FMath::Clamp((int32)(Alpha * (NumKeys - 1)), 0, FMath::Max(NumKeys - 1, 0));

					int32 AnimBoneIndex = Animations[i]->GetSkeleton() ?
						Animations[i]->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName) : INDEX_NONE;
					if (AnimBoneIndex == INDEX_NONE)
					{
						continue;
					}

					FTransform SampleTransform = Animations[i]->GetBoneTransform(AnimBoneIndex, SampleIndex);

					BlendedLocation += SampleTransform.GetLocation() * Weight;
					FQuat SampleQuat = SampleTransform.GetRotation();
					BlendedRotation = FQuat::Slerp(BlendedRotation, SampleQuat, Weight / (TotalWeight + Weight));
					BlendedScale *= FMath::Lerp(FVector::OneVector, SampleTransform.GetScale3D(), Weight);
					TotalWeight += Weight;
				}
			}

			if (TotalWeight > 0.0f)
			{
				BlendedLocation /= TotalWeight;
			}

			FAnimationKeyframe Keyframe;
			Keyframe.Time = Time;
			Keyframe.Transform = FTransform(BlendedRotation, BlendedLocation, BlendedScale);
			Track.Keyframes.Add(Keyframe);
		}

		BlendedTracks.Add(Track);
	}

	return CreateAnimation(WorldContextObject, AnimationName, Skeleton, BlendedTracks, Duration, 30.0f);
}

UAnimSequence* UAnimationLibrary::CrossfadeAnimations(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* From,
	UAnimSequence* To,
	float CrossfadeDuration,
	float FromEndTime,
	float ToStartTime)
{
	if (!WorldContextObject || !Skeleton || !From || !To)
	{
		return nullptr;
	}

	float FromDuration = (FromEndTime > 0.0f) ? FromEndTime : From->GetSequenceLength();
	float ToDuration = To->GetSequenceLength();
	float TotalDuration = FMath::Max(0.0f, FromDuration + ToDuration - CrossfadeDuration);

	TArray<FAnimationTrackData> CrossfadeTracks;

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	int32 FromBoneCount = From->GetSkeleton() ? From->GetSkeleton()->GetReferenceSkeleton().GetNum() : 0;
	int32 ToBoneCount = To->GetSkeleton() ? To->GetSkeleton()->GetReferenceSkeleton().GetNum() : 0;

	for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); BoneIndex++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
		FAnimationTrackData Track;
		Track.TrackName = BoneName;

		int32 FrameCount = FMath::CeilToInt(TotalDuration * 30.0f);

		int32 FromBoneIdx = From->GetSkeleton() ?
			From->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName) : INDEX_NONE;
		int32 ToBoneIdx = To->GetSkeleton() ?
			To->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName) : INDEX_NONE;

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			float Time = (float)Frame / 30.0f;

			FTransform FromTransform = FTransform::Identity;
			FTransform ToTransform = FTransform::Identity;

			if (FromBoneIdx != INDEX_NONE)
			{
				float FromAlpha = FMath::Clamp(Time / FromDuration, 0.0f, 1.0f);
				int32 FromSampleIndex = FMath::Clamp((int32)(FromAlpha * (From->GetNumberOfSampledKeys() - 1)), 0, From->GetNumberOfSampledKeys() - 1);
				FromTransform = From->GetBoneTransform(FromBoneIdx, FromSampleIndex);
			}

			if (Time >= FromDuration - CrossfadeDuration && ToBoneIdx != INDEX_NONE)
			{
				float ToTime = Time - (FromDuration - CrossfadeDuration);
				float ToAlpha = FMath::Clamp(ToTime / ToDuration, 0.0f, 1.0f);
				int32 ToSampleIndex = FMath::Clamp((int32)(ToAlpha * (To->GetNumberOfSampledKeys() - 1)), 0, To->GetNumberOfSampledKeys() - 1);
				ToTransform = To->GetBoneTransform(ToBoneIdx, ToSampleIndex);
			}

			float BlendAlpha = 0.0f;
			if (Time > FromDuration - CrossfadeDuration)
			{
				BlendAlpha = FMath::Clamp((Time - (FromDuration - CrossfadeDuration)) / CrossfadeDuration, 0.0f, 1.0f);
			}

			FVector BlendedLocation = FMath::Lerp(FromTransform.GetLocation(), ToTransform.GetLocation(), BlendAlpha);
			FQuat BlendedRotation = FQuat::Slerp(FromTransform.GetRotation(), ToTransform.GetRotation(), BlendAlpha);
			FVector BlendedScale = FMath::Lerp(FromTransform.GetScale3D(), ToTransform.GetScale3D(), BlendAlpha);

			FAnimationKeyframe Keyframe;
			Keyframe.Time = Time;
			Keyframe.Transform = FTransform(BlendedRotation, BlendedLocation, BlendedScale);
			Track.Keyframes.Add(Keyframe);
		}

		CrossfadeTracks.Add(Track);
	}

	return CreateAnimation(WorldContextObject, AnimationName, Skeleton, CrossfadeTracks, TotalDuration, 30.0f);
}

UAnimSequence* UAnimationLibrary::MirrorAnimation(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* SourceAnimation,
	EMirrorFlags MirrorFlags)
{
	if (!WorldContextObject || !Skeleton || !SourceAnimation)
	{
		return nullptr;
	}

	TArray<FAnimationTrackData> MirroredTracks;

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	float Duration = SourceAnimation->GetSequenceLength();

	for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); BoneIndex++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
		FAnimationTrackData Track;
		Track.TrackName = BoneName;

		int32 SourceBoneIdx = SourceAnimation->GetSkeleton() ?
			SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName) : INDEX_NONE;

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			float Alpha = (FrameCount > 1) ? (float)Frame / (FrameCount - 1) : 0.0f;

			FTransform SampleTransform = FTransform::Identity;
			if (SourceBoneIdx != INDEX_NONE)
			{
				SampleTransform = SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame);
			}

			FVector Location = SampleTransform.GetLocation();
			FQuat Rotation = SampleTransform.GetRotation();
			FVector Scale = SampleTransform.GetScale3D();

			if (MirrorFlags == EMirrorFlags::X || MirrorFlags == EMirrorFlags::XY || MirrorFlags == EMirrorFlags::XZ || MirrorFlags == EMirrorFlags::XYZ)
			{
				Location.X = -Location.X;
				Rotation.Y = -Rotation.Y;
				Rotation.Z = -Rotation.Z;
			}

			if (MirrorFlags == EMirrorFlags::Y || MirrorFlags == EMirrorFlags::XY || MirrorFlags == EMirrorFlags::YZ || MirrorFlags == EMirrorFlags::XYZ)
			{
				Location.Y = -Location.Y;
				Rotation.X = -Rotation.X;
				Rotation.Z = -Rotation.Z;
			}

			if (MirrorFlags == EMirrorFlags::Z || MirrorFlags == EMirrorFlags::XZ || MirrorFlags == EMirrorFlags::YZ || MirrorFlags == EMirrorFlags::XYZ)
			{
				Location.Z = -Location.Z;
				Rotation.X = -Rotation.X;
				Rotation.Y = -Rotation.Y;
			}

			FAnimationKeyframe Keyframe;
			Keyframe.Time = Alpha * Duration;
			Keyframe.Transform = FTransform(Rotation, Location, Scale);
			Track.Keyframes.Add(Keyframe);
		}

		MirroredTracks.Add(Track);
	}

	return CreateAnimation(WorldContextObject, AnimationName, Skeleton, MirroredTracks, Duration, 30.0f);
}

UAnimSequence* UAnimationLibrary::MirrorAnimationXY(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* SourceAnimation,
	bool bMirrorX,
	bool bMirrorY)
{
	EMirrorFlags Flags = EMirrorFlags::None;

	if (bMirrorX && bMirrorY)
	{
		Flags = EMirrorFlags::XY;
	}
	else if (bMirrorX)
	{
		Flags = EMirrorFlags::X;
	}
	else if (bMirrorY)
	{
		Flags = EMirrorFlags::Y;
	}

	return MirrorAnimation(WorldContextObject, AnimationName, Skeleton, SourceAnimation, Flags);
}

bool UAnimationLibrary::ExportAnimationFBX(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	return ExportAnimation(AnimSequence, OutputPath, EAnimationImportFormat::FBX);
}

bool UAnimationLibrary::ExportAnimationBVH(UAnimSequence* AnimSequence, const FString& OutputPath, const FString& CharacterName)
{
	if (!AnimSequence)
	{
		return false;
	}

	USkeleton* Skeleton = AnimSequence->GetSkeleton();
	if (!Skeleton)
	{
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 NumFrames = AnimSequence->GetNumberOfSampledKeys();
	float Duration = AnimSequence->GetSequenceLength();
	float FrameRate = (NumFrames > 1 && Duration > 0.0f) ? (NumFrames - 1) / Duration : 30.0f;

	FString BVHContent;

	BVHContent += FString::Printf(TEXT("HIERARCHY\n"));

	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FName BoneName = RefSkeleton.GetBoneName(i);
		int32 ParentIndex = RefSkeleton.GetParentIndex(i);
		FTransform BoneTransform = RefSkeleton.GetBoneTransform(i);
		FVector LocalPos = BoneTransform.GetLocation();

		FString Indent = FString::ChrN(RefSkeleton.GetBoneDepth(i), '\t');

		if (ParentIndex == INDEX_NONE)
		{
			BVHContent += FString::Printf(TEXT("%sROOT %s\n"), *Indent, *BoneName.ToString());
		}
		else
		{
			BVHContent += FString::Printf(TEXT("%sJOINT %s\n"), *Indent, *BoneName.ToString());
		}

		BVHContent += FString::Printf(TEXT("%s{\n"), *Indent);
		BVHContent += FString::Printf(TEXT("%s\tOFFSET %.4f %.4f %.4f\n"), *Indent, LocalPos.X, LocalPos.Y, LocalPos.Z);

		if (RefSkeleton.GetNum() - 1 == i)
		{
			BVHContent += FString::Printf(TEXT("%s\tCHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation\n"), *Indent);
			BVHContent += FString::Printf(TEXT("%s\tEnd Site\n"), *Indent);
			BVHContent += FString::Printf(TEXT("%s\t{\n"), *Indent);
			BVHContent += FString::Printf(TEXT("%s\t\tOFFSET 0.0 0.0 0.0\n"), *Indent);
			BVHContent += FString::Printf(TEXT("%s\t}\n"), *Indent);
		}
		else
		{
			BVHContent += FString::Printf(TEXT("%s\tCHANNELS 3 Zrotation Xrotation Yrotation\n"), *Indent);
		}

		BVHContent += FString::Printf(TEXT("%s}\n"), *Indent);
	}

	BVHContent += FString::Printf(TEXT("MOTION\n"));
	BVHContent += FString::Printf(TEXT("Frames: %d\n"), NumFrames);
	BVHContent += FString::Printf(TEXT("Frame Time: %.6f\n"), 1.0f / FrameRate);

	for (int32 Frame = 0; Frame < NumFrames; Frame++)
	{
		for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
		{
			FTransform BoneTransform = AnimSequence->GetBoneTransform(i, Frame);
			FVector Location = BoneTransform.GetLocation();
			FQuat Rotation = BoneTransform.GetRotation();
			FVector EulerRot = Rotation.Euler();

			if (RefSkeleton.GetNum() - 1 == i)
			{
				BVHContent += FString::Printf(TEXT("%.4f %.4f %.4f %.4f %.4f %.4f "), Location.X, Location.Y, Location.Z, EulerRot.Z, EulerRot.X, EulerRot.Y);
			}
			else
			{
				BVHContent += FString::Printf(TEXT("%.4f %.4f %.4f "), EulerRot.Z, EulerRot.X, EulerRot.Y);
			}
		}
		BVHContent += TEXT("\n");
	}

	return FFileHelper::SaveStringToFile(BVHContent, *OutputPath);
}

bool UAnimationLibrary::ExportAnimationglTF(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	return ExportAnimation(AnimSequence, OutputPath, EAnimationImportFormat::glTF);
}

bool UAnimationLibrary::ExportAnimationCollada(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	if (!AnimSequence)
	{
		return false;
	}

	USkeleton* Skeleton = AnimSequence->GetSkeleton();
	if (!Skeleton)
	{
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 NumFrames = AnimSequence->GetNumberOfSampledKeys();

	FString ColladaContent;

	ColladaContent += TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	ColladaContent += TEXT("<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n");
	ColladaContent += TEXT("<asset>\n");
	ColladaContent += TEXT("\t<contributor>\n");
	ColladaContent += TEXT("\t\t<authoring_tool>AnimationToolkit</authoring_tool>\n");
	ColladaContent += TEXT("\t</contributor>\n");
	ColladaContent += TEXT("\t<created>2024-01-01</created>\n");
	ColladaContent += TEXT("\t<modified>2024-01-01</modified>\n");
	ColladaContent += TEXT("\t<unit name=\"meter\" meter=\"1\"/>\n");
	ColladaContent += TEXT("\t<up_axis>Z_UP</up_axis>\n");
	ColladaContent += TEXT("</asset>\n");

	ColladaContent += TEXT("<library_animations>\n");

	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FName BoneName = RefSkeleton.GetBoneName(i);

		ColladaContent += FString::Printf(TEXT("\t<animation id=\"%s_anim\" name=\"%s\">\n"), *BoneName.ToString(), *BoneName.ToString());

		ColladaContent += FString::Printf(TEXT("\t\t<source id=\"%s_input\">\n"), *BoneName.ToString());
		ColladaContent += FString::Printf(TEXT("\t\t\t<accessor source=\"#%s\" count=\"%d\" stride=\"1\">\n"), *BoneName.ToString(), NumFrames);
		ColladaContent += TEXT("\t\t\t\t<param name=\"TIME\" type=\"float\"/>\n");
		ColladaContent += TEXT("\t\t\t</accessor>\n");
		ColladaContent += TEXT("\t\t</source>\n");

		ColladaContent += FString::Printf(TEXT("\t\t<float_array id=\"%s\" count=\"%d\">"), *BoneName.ToString(), NumFrames);
		for (int32 Frame = 0; Frame < NumFrames; Frame++)
		{
			float Time = (float)Frame / 30.0f;
			ColladaContent += FString::Printf(TEXT("%.4f "), Time);
		}
		ColladaContent += TEXT("</float_array>\n");

		ColladaContent += TEXT("\t\t<technique_common>\n");
		ColladaContent += TEXT("\t\t\t<accessor>\n");
		ColladaContent += TEXT("\t\t\t</accessor>\n");
		ColladaContent += TEXT("\t\t</technique_common>\n");

		ColladaContent += TEXT("\t</animation>\n");
	}

	ColladaContent += TEXT("</library_animations>\n");
	ColladaContent += TEXT("</COLLADA>\n");

	return FFileHelper::SaveStringToFile(ColladaContent, *OutputPath);
}

bool UAnimationLibrary::ExportAnimation(UAnimSequence* AnimSequence, const FString& OutputPath, EAnimationImportFormat Format)
{
	if (!AnimSequence)
	{
		return false;
	}

	switch (Format)
	{
	case EAnimationImportFormat::BVH:
		return ExportAnimationBVH(AnimSequence, OutputPath);

	case EAnimationImportFormat::glTF:
		return ExportAnimationglTF(AnimSequence, OutputPath);

	default:
		return false;
	}
}

void UAnimationLibrary::SetAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	if (AnimSequence)
	{
		TArray<FName>& Tags = AnimationTagsMap.FindOrAdd(AnimSequence);
		if (!Tags.Contains(Tag))
		{
			Tags.Add(Tag);
		}
	}
}

void UAnimationLibrary::SetAnimationTags(UAnimSequence* AnimSequence, const TArray<FName>& Tags)
{
	if (AnimSequence)
	{
		TArray<FName>& ExistingTags = AnimationTagsMap.FindOrAdd(AnimSequence);
		for (const FName& Tag : Tags)
		{
			if (!ExistingTags.Contains(Tag))
			{
				ExistingTags.Add(Tag);
			}
		}
	}
}

void UAnimationLibrary::RemoveAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	if (AnimSequence)
	{
		TArray<FName>* Tags = AnimationTagsMap.Find(AnimSequence);
		if (Tags)
		{
			Tags->Remove(Tag);
			if (Tags->Num() == 0)
			{
				AnimationTagsMap.Remove(AnimSequence);
			}
		}
	}
}

bool UAnimationLibrary::HasAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	if (AnimSequence)
	{
		const TArray<FName>* Tags = AnimationTagsMap.Find(AnimSequence);
		if (Tags)
		{
			return Tags->Contains(Tag);
		}
	}
	return false;
}

TArray<FName> UAnimationLibrary::GetAnimationTags(UAnimSequence* AnimSequence)
{
	if (AnimSequence)
	{
		const TArray<FName>* Tags = AnimationTagsMap.Find(AnimSequence);
		if (Tags)
		{
			return *Tags;
		}
	}
	return TArray<FName>();
}

TArray<UAnimSequence*> UAnimationLibrary::FindAnimationsByTag(const TArray<UAnimSequence*>& Animations, const FName& Tag)
{
	TArray<UAnimSequence*> Result;

	for (UAnimSequence* Anim : Animations)
	{
		if (HasAnimationTag(Anim, Tag))
		{
			Result.Add(Anim);
		}
	}

	return Result;
}

void UAnimationLibrary::ClearAnimationTags(UAnimSequence* AnimSequence)
{
	if (AnimSequence)
	{
		AnimationTagsMap.Remove(AnimSequence);
	}
}

void UAnimationLibrary::SetGlobalProgressCallback(FOnAnimationProgress Callback)
{
	GlobalProgressCallback = Callback;
}

void UAnimationLibrary::ClearGlobalProgressCallback()
{
	GlobalProgressCallback.Unbind();
}

// ─── GUIDE API ─────────────────────────────────────────────────

UAnimSequence* UAnimationLibrary::CreateEmptyAnimation(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float Duration,
	int32 FrameRate)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	FString AnimName = FString::Printf(TEXT("EmptyAnim_%d"), FMath::RandRange(0, 99999));

	return UAnimSequenceCreator::CreateAnimSequenceFromKeyframes(
		WorldContextObject,
		AnimName,
		Skeleton,
		TArray<FAnimationTrackData>(),
		Duration,
		(float)FrameRate
	);
}

bool UAnimationLibrary::AddBoneKeyframe(
	UAnimSequence* Animation,
	FName BoneName,
	float Time,
	FTransform Transform,
	bool bUpdateExistingKey)
{
	if (!Animation)
	{
		return false;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 BoneIdx = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIdx == INDEX_NONE)
	{
		return false;
	}

	if (BoneIdx < Animation->GetEditableRawAnimationData().Tracks.Num())
	{
		FRawAnimSequenceTrack& Track = Animation->GetEditableRawAnimationData().Tracks[BoneIdx];

		int32 InsertIdx = 0;
		for (int32 i = 0; i < Track.PosKeys.Num(); i++)
		{
			if (Time > (float)i / (float)FMath::Max(1, Track.PosKeys.Num() - 1))
			{
				InsertIdx = i + 1;
			}
		}

		Track.PosKeys.Insert(Transform.GetLocation(), InsertIdx);
		Track.RotKeys.Insert(Transform.GetRotation(), InsertIdx);
		Track.ScaleKeys.Insert(Transform.GetScale3D(), InsertIdx);

		Animation->MarkPackageDirty();
		return true;
	}

	return false;
}

int32 UAnimationLibrary::AddKeyframesBatch(
	UAnimSequence* Animation,
	const TArray<FAnimationKeyframe>& Keyframes)
{
	if (!Animation || Keyframes.Num() == 0)
	{
		return 0;
	}

	return Keyframes.Num();
}

bool UAnimationLibrary::RemoveBoneKeyframe(
	UAnimSequence* Animation,
	FName BoneName,
	float Time)
{
	if (!Animation)
	{
		return false;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 BoneIdx = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIdx == INDEX_NONE)
	{
		return false;
	}

	if (BoneIdx < Animation->GetEditableRawAnimationData().Tracks.Num())
	{
		FRawAnimSequenceTrack& Track = Animation->GetEditableRawAnimationData().Tracks[BoneIdx];

		if (Track.PosKeys.Num() > 1)
		{
			int32 RemoveIdx = FMath::Clamp(
				FMath::RoundToInt(Time * (Track.PosKeys.Num() - 1)),
				0, Track.PosKeys.Num() - 1);

			Track.PosKeys.RemoveAt(RemoveIdx);
			Track.RotKeys.RemoveAt(RemoveIdx);
			Track.ScaleKeys.RemoveAt(RemoveIdx);

			Animation->MarkPackageDirty();
			return true;
		}
	}

	return false;
}

FTransform UAnimationLibrary::GetBoneTransformAtTime(
	UAnimSequence* Animation,
	FName BoneName,
	float Time,
	bool bExtractRootMotion)
{
	if (!Animation)
	{
		return FTransform::Identity;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return FTransform::Identity;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 BoneIdx = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIdx == INDEX_NONE)
	{
		return FTransform::Identity;
	}

	int32 NumFrames = Animation->GetNumberOfSampledKeys();
	float Duration = Animation->GetSequenceLength();

	if (NumFrames <= 1 || Duration <= 0.0f)
	{
		return Animation->GetBoneTransform(BoneIdx, 0);
	}

	float NormalizedTime = FMath::Clamp(Time / Duration, 0.0f, 1.0f);
	int32 SampleIndex = FMath::Clamp(
		FMath::RoundToInt(NormalizedTime * (NumFrames - 1)),
		0, NumFrames - 1);

	return Animation->GetBoneTransform(BoneIdx, SampleIndex);
}

UAnimSequence* UAnimationLibrary::RetargetSpeed(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float SpeedMultiplier)
{
	if (!WorldContextObject || !SourceAnimation || SpeedMultiplier <= 0.0f)
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	float NewDuration = SourceAnimation->GetSequenceLength() / SpeedMultiplier;
	int32 SourceFrameCount = SourceAnimation->GetNumberOfSampledKeys();
	int32 NewFrameCount = FMath::Max(1, FMath::CeilToInt(NewDuration * 30.0f));
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> Tracks;

	for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); BoneIndex++)
	{
		FAnimationTrackData Track;
		Track.TrackName = RefSkeleton.GetBoneName(BoneIndex);

		for (int32 Frame = 0; Frame < NewFrameCount; Frame++)
		{
			float NormalizedTime = (NewFrameCount > 1) ? (float)Frame / (NewFrameCount - 1) : 0.0f;
			float SourceTime = NormalizedTime * SourceAnimation->GetSequenceLength();
			int32 SourceSampleIdx = FMath::Clamp(
				FMath::RoundToInt(NormalizedTime * (SourceFrameCount - 1)),
				0, FMath::Max(SourceFrameCount - 1, 0));

			FTransform SampleTransform = SourceAnimation->GetBoneTransform(BoneIndex, SourceSampleIdx);

			FAnimationKeyframe Keyframe;
			Keyframe.Time = (float)Frame / 30.0f;
			Keyframe.Transform = SampleTransform;
			Track.Keyframes.Add(Keyframe);
		}

		Tracks.Add(Track);
	}

	return CreateAnimation(
		WorldContextObject,
		SourceAnimation->GetName() + TEXT("_Speed"),
		Skeleton,
		Tracks,
		NewDuration,
		30.0f
	);
}

UAnimSequence* UAnimationLibrary::CropAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float StartTime,
	float EndTime)
{
	if (!WorldContextObject || !SourceAnimation)
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	float AnimDuration = SourceAnimation->GetSequenceLength();
	StartTime = FMath::Clamp(StartTime, 0.0f, AnimDuration);
	EndTime = FMath::Clamp(EndTime, StartTime, AnimDuration);
	float NewDuration = EndTime - StartTime;

	if (NewDuration <= 0.0f)
	{
		return nullptr;
	}

	int32 SourceFrameCount = SourceAnimation->GetNumberOfSampledKeys();
	int32 NewFrameCount = FMath::Max(1, FMath::CeilToInt(NewDuration * 30.0f));
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> Tracks;

	for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); BoneIndex++)
	{
		FAnimationTrackData Track;
		Track.TrackName = RefSkeleton.GetBoneName(BoneIndex);

		for (int32 Frame = 0; Frame < NewFrameCount; Frame++)
		{
			float NormalizedTime = (NewFrameCount > 1) ? (float)Frame / (NewFrameCount - 1) : 0.0f;
			float SourceTime = StartTime + NormalizedTime * NewDuration;
			float SourceNormalized = (AnimDuration > 0.0f) ? SourceTime / AnimDuration : 0.0f;
			int32 SourceSampleIdx = FMath::Clamp(
				FMath::RoundToInt(SourceNormalized * (SourceFrameCount - 1)),
				0, FMath::Max(SourceFrameCount - 1, 0));

			FTransform SampleTransform = SourceAnimation->GetBoneTransform(BoneIndex, SourceSampleIdx);

			FAnimationKeyframe Keyframe;
			Keyframe.Time = (float)Frame / 30.0f;
			Keyframe.Transform = SampleTransform;
			Track.Keyframes.Add(Keyframe);
		}

		Tracks.Add(Track);
	}

	return CreateAnimation(
		WorldContextObject,
		SourceAnimation->GetName() + TEXT("_Cropped"),
		Skeleton,
		Tracks,
		NewDuration,
		30.0f
	);
}

bool UAnimationLibrary::ApplyRootOffset(
	UAnimSequence* Animation,
	FVector LocationOffset,
	FRotator RotationOffset)
{
	if (!Animation)
	{
		return false;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 RootBoneIdx = RefSkeleton.FindBoneIndex(FName("root"));
	if (RootBoneIdx == INDEX_NONE && RefSkeleton.GetNum() > 0)
	{
		RootBoneIdx = 0;
	}

	if (RootBoneIdx == INDEX_NONE || RootBoneIdx >= Animation->GetEditableRawAnimationData().Tracks.Num())
	{
		return false;
	}

	FRawAnimSequenceTrack& RootTrack = Animation->GetEditableRawAnimationData().Tracks[RootBoneIdx];
	FQuat RotationQuat = RotationOffset.Quaternion();

	for (int32 i = 0; i < RootTrack.PosKeys.Num(); i++)
	{
		RootTrack.PosKeys[i] = RootTrack.PosKeys[i] + LocationOffset;
		RootTrack.RotKeys[i] = RotationQuat * RootTrack.RotKeys[i];
	}

	Animation->MarkPackageDirty();
	return true;
}

bool UAnimationLibrary::SaveAnimationAsAsset(
	UAnimSequence* Animation,
	const FString& PackagePath,
	const FString& AssetName)
{
	if (!Animation)
	{
		return false;
	}

	FString FullPath = PackagePath / AssetName;
	UPackage* Package = CreatePackage(nullptr, *FullPath);
	if (!Package)
	{
		return false;
	}

	UAnimSequence* SavedAnim = DuplicateObject<UAnimSequence>(Animation, Package, *AssetName);
	if (!SavedAnim)
	{
		return false;
	}

	SavedAnim->SetFlags(RF_Public | RF_Standalone);
	Package->MarkPackageDirty();

	return FAssetData(SavedAnim).IsValid();
}

UAnimSequence* UAnimationLibrary::DuplicateAnimation(
	UAnimSequence* SourceAnimation)
{
	if (!SourceAnimation)
	{
		return nullptr;
	}

	return DuplicateObject<UAnimSequence>(SourceAnimation, SourceAnimation->GetOuter(),
		*FString::Printf(TEXT("%s_Duplicate"), *SourceAnimation->GetName()));
}

bool UAnimationLibrary::IsAnimationCompatibleWithSkeleton(
	UAnimSequence* Animation,
	USkeleton* Skeleton)
{
	if (!Animation || !Skeleton)
	{
		return false;
	}

	USkeleton* AnimSkeleton = Animation->GetSkeleton();
	if (!AnimSkeleton)
	{
		return false;
	}

	return AnimSkeleton->IsCompatible(Skeleton);
}

FTransform UAnimationLibrary::GetRuntimeBoneTransform(
	USkeletalMeshComponent* SkelMesh,
	FName BoneName,
	EBoneSpaces::Type Space)
{
	if (!SkelMesh)
	{
		return FTransform::Identity;
	}

	int32 BoneIdx = SkelMesh->GetBoneIndex(BoneName);
	if (BoneIdx == INDEX_NONE)
	{
		return FTransform::Identity;
	}

	return SkelMesh->GetBoneTransform(BoneIdx, Space);
}

TArray<FName> UAnimationLibrary::GetAnimationBoneNames(
	UAnimSequence* Animation)
{
	TArray<FName> Result;

	if (!Animation)
	{
		return Result;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		Result.Add(RefSkeleton.GetBoneName(i));
	}

	return Result;
}

float UAnimationLibrary::GetAnimationDuration(
	UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return Animation->GetSequenceLength();
}

int32 UAnimationLibrary::GetAnimationFrameCount(
	UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0;
	}

	return Animation->GetNumberOfSampledKeys();
}

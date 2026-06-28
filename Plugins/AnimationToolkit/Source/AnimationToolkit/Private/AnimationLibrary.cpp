#include "AnimationLibrary.h"
#include "AnimSequenceCreator.h"
#include "AnimMontageCreator.h"
#include "AnimBlendSpaceCreator.h"
#include "AnimationImporter.h"
#include "AsyncImportHelper.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Curves/RichCurve.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"

static TMap<TWeakObjectPtr<UObject>, TArray<FName>> AnimationTagsMap;

static void CleanupStaleTags()
{
	TArray<TWeakObjectPtr<UObject>> StaleKeys;
	for (auto& Pair : AnimationTagsMap)
	{
		if (!Pair.Key.IsValid())
		{
			StaleKeys.Add(Pair.Key);
		}
	}
	for (const TWeakObjectPtr<UObject>& Key : StaleKeys)
	{
		AnimationTagsMap.Remove(Key);
	}
}

UAnimSequence* UAnimationLibrary::CreateAnimation(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	float Duration,
	float FrameRate)
{
	if (!Skeleton || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimSequenceCreator::CreateAnimSequenceFromKeyframes(
		WorldContextObject, AnimationName, Skeleton, Tracks, Duration, FrameRate);
}

UAnimMontage* UAnimationLibrary::CreateMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections)
{
	if (!Skeleton || MontageName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimMontageCreator::CreateAnimMontage(WorldContextObject, MontageName, Skeleton, Sections);
}

UBlendSpace* UAnimationLibrary::CreateBlendSpace1D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinValue,
	float MaxValue)
{
	if (!Skeleton || BlendSpaceName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimBlendSpaceCreator::CreateBlendSpace1D(
		WorldContextObject, BlendSpaceName, Skeleton, Samples, MinValue, MaxValue);
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
	if (!Skeleton || BlendSpaceName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimBlendSpaceCreator::CreateBlendSpace2D(
		WorldContextObject, BlendSpaceName, Skeleton, Samples, MinX, MaxX, MinY, MaxY);
}

UAnimSequence* UAnimationLibrary::ImportAnimationAsSequence(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	if (!Skeleton || FilePath.IsEmpty() || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimationImporter::ImportAndCreateAnimSequence(
		WorldContextObject, FilePath, AnimationName, Skeleton, Format);
}

FAnimationImportResult UAnimationLibrary::ImportAnimationFile(
	UObject* WorldContextObject,
	const FString& FilePath,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	if (!FPaths::FileExists(FilePath))
	{
		FAnimationImportResult Result;
		Result.ErrorMessage = TEXT("File not found: ") + FilePath;
		return Result;
	}

	if (!Skeleton)
	{
		FAnimationImportResult Result;
		Result.ErrorMessage = TEXT("Invalid skeleton");
		return Result;
	}

	return UAnimationImporter::ImportAnimation(WorldContextObject, FilePath, Format, Skeleton);
}

FAnimationTrackData UAnimationLibrary::CreateTrack(
	const FName& BoneName,
	const TArray<FTransform>& Transforms,
	float FrameRate)
{
	FAnimationTrackData TrackData;
	TrackData.TrackName = BoneName;
	const float SafeFrameRate = FMath::Max(0.01f, FrameRate);
	for (const FTransform& T : Transforms)
	{
		FAnimationKeyframe KF;
		KF.Time = TrackData.Keyframes.Num() / SafeFrameRate;
		KF.Transform = T;
		TrackData.Keyframes.Add(KF);
	}
	return TrackData;
}

FAnimationKeyframe UAnimationLibrary::MakeKeyframe(
	float Time,
	const FTransform& Transform)
{
	FAnimationKeyframe Keyframe;
	Keyframe.Time = Time;
	Keyframe.Transform = Transform;
	return Keyframe;
}

FAnimMontageSectionData UAnimationLibrary::MakeMontageSection(
	const FName& SectionName,
	UAnimSequence* Animation,
	float StartTime,
	float EndTime)
{
	FAnimMontageSectionData SectionData;
	SectionData.SectionName = SectionName;
	SectionData.Animation = Animation;
	SectionData.StartTime = StartTime;
	SectionData.EndTime = EndTime;
	return SectionData;
}

FBlendSpaceSampleData UAnimationLibrary::MakeBlendSpaceSample(
	UAnimSequence* Animation,
	const FVector& SampleValue)
{
	FBlendSpaceSampleData SampleData;
	SampleData.Animation = Animation;
	SampleData.SampleValue = SampleValue;
	return SampleData;
}

EAnimationImportFormat UAnimationLibrary::DetectAnimationFormat(const FString& FilePath)
{
	const FString Ext = FPaths::GetExtension(FilePath).ToLower();

	if (Ext == TEXT("fbx")) return EAnimationImportFormat::FBX;
	if (Ext == TEXT("gltf") || Ext == TEXT("glb")) return EAnimationImportFormat::glTF;
	if (Ext == TEXT("bvh")) return EAnimationImportFormat::BVH;
	if (Ext == TEXT("dae")) return EAnimationImportFormat::COLLADA;

	return EAnimationImportFormat::FBX;
}

bool UAnimationLibrary::IsAnimationFormatSupported(const FString& FilePath)
{
	const FString Ext = FPaths::GetExtension(FilePath).ToLower();
	return Ext == TEXT("fbx") || Ext == TEXT("gltf") || Ext == TEXT("glb") ||
		Ext == TEXT("bvh") || Ext == TEXT("dae");
}

FString UAnimationLibrary::GetAnimationFormatName(EAnimationImportFormat Format)
{
	switch (Format)
	{
	case EAnimationImportFormat::FBX: return TEXT("FBX");
	case EAnimationImportFormat::glTF: return TEXT("glTF");
	case EAnimationImportFormat::BVH: return TEXT("BVH");
	case EAnimationImportFormat::COLLADA: return TEXT("COLLADA");
	default: return TEXT("Unknown");
	}
}

TArray<EAnimationImportFormat> UAnimationLibrary::GetAllSupportedFormats()
{
	TArray<EAnimationImportFormat> Formats;
	Formats.Add(EAnimationImportFormat::FBX);
	Formats.Add(EAnimationImportFormat::glTF);
	Formats.Add(EAnimationImportFormat::BVH);
	Formats.Add(EAnimationImportFormat::COLLADA);
	return Formats;
}

TArray<FString> UAnimationLibrary::GetSkeletonBoneNames(USkeleton* Skeleton)
{
	TArray<FString> BoneNames;
	if (!Skeleton)
	{
		return BoneNames;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();

	for (int32 i = 0; i < NumBones; ++i)
	{
		BoneNames.Add(RefSkeleton.GetBoneName(i).ToString());
	}

	return BoneNames;
}

float UAnimationLibrary::GetAnimSequenceDuration(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return 0.0f;
	}

	return AnimSequence->GetPlayLength();
}

int32 UAnimationLibrary::GetAnimSequenceFrameCount(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return 0;
	}

	return AnimSequence->GetNumberOfSampledKeys();
}

FATK_AnimationCurveData UAnimationLibrary::CreateCurveData(
	const FName& CurveName,
	const TArray<FAnimationCurveKeyframe>& Keyframes)
{
	FATK_AnimationCurveData CurveData;
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
	const TArray<FATK_AnimationCurveData>& Curves,
	float Duration,
	float FrameRate)
{
	UAnimSequence* Anim = CreateAnimation(WorldContextObject, AnimationName, Skeleton, Tracks, Duration, FrameRate);
	if (Anim && Curves.Num() > 0)
	{
		IAnimationDataController& Controller = Anim->GetController();
		for (const FATK_AnimationCurveData& Curve : Curves)
		{
			FAnimationCurveIdentifier CurveId(Curve.CurveName, ERawCurveTrackTypes::RCT_Float);
			Controller.AddCurve(CurveId, false);
			for (const FAnimationCurveKeyframe& KF : Curve.Keyframes)
			{
				Controller.SetCurveKey(CurveId, FRichCurveKey(KF.Time, KF.Value));
			}
		}
		Anim->MarkPackageDirty();
	}
	return Anim;
}

bool UAnimationLibrary::AddCurveToAnimSequence(UAnimSequence* AnimSequence, const FATK_AnimationCurveData& CurveData)
{
	if (!AnimSequence)
	{
		return false;
	}

	IAnimationDataController& Controller = AnimSequence->GetController();
	FAnimationCurveIdentifier CurveId(CurveData.CurveName, ERawCurveTrackTypes::RCT_Float);
	Controller.AddCurve(CurveId, false);
	for (const FAnimationCurveKeyframe& KF : CurveData.Keyframes)
	{
		Controller.SetCurveKey(CurveId, FRichCurveKey(KF.Time, KF.Value));
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

	if (!Skeleton)
	{
		return BatchResult;
	}

	for (const FAnimationBatchImportItem& Item : Items)
	{
		FAnimationImportResult ItemResult;
		ItemResult.bSuccess = FPaths::FileExists(Item.FilePath);
		if (!ItemResult.bSuccess)
		{
			ItemResult.ErrorMessage = TEXT("File not found: ") + Item.FilePath;
		}
		BatchResult.Results.Add(ItemResult);
	}

	for (const FAnimationImportResult& R : BatchResult.Results)
	{
		if (R.bSuccess) BatchResult.SuccessCount++;
		else BatchResult.FailCount++;
	}

	BatchResult.bSuccess = BatchResult.FailCount == 0;
	return BatchResult;
}

TArray<UAnimSequence*> UAnimationLibrary::BatchImportAsSequences(
	UObject* WorldContextObject,
	const TArray<FAnimationBatchImportItem>& Items,
	USkeleton* Skeleton)
{
	TArray<UAnimSequence*> Sequences;

	if (!WorldContextObject || !Skeleton)
	{
		return Sequences;
	}

	for (const FAnimationBatchImportItem& Item : Items)
	{
		UAnimSequence* Seq = ImportAnimationAsSequence(
			WorldContextObject, Item.FilePath, Item.AnimationName, Skeleton, Item.Format);
		if (Seq)
		{
			Sequences.Add(Seq);
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

	const IAnimationDataModel* SourceModel = SourceAnimation->GetDataModel();
	if (!SourceModel)
	{
		return nullptr;
	}

	const FString NewName = SourceAnimation->GetName() + TEXT("_Retargeted");

	TArray<FAnimationTrackData> RetargetedTracks;
	const TArray<FBoneAnimationTrack>& SourceTracks = SourceModel->GetBoneAnimationTracks();
	const FReferenceSkeleton& TargetRefSkel = TargetSkeleton->GetReferenceSkeleton();

	for (const FBoneAnimationTrack& SrcTrack : SourceTracks)
	{
		FName MappedBoneName = SrcTrack.Name;

		if (Settings.RetargetType == EAnimationRetargetType::ByName)
		{
			bool bFoundCustom = false;
			for (const FAnimationRetargetMapping& Mapping : Settings.CustomMappings)
			{
				if (Mapping.SourceBone == SrcTrack.Name)
				{
					MappedBoneName = Mapping.TargetBone;
					bFoundCustom = true;
					break;
				}
			}

			if (!bFoundCustom)
			{
				if (TargetRefSkel.FindBoneIndex(MappedBoneName) == INDEX_NONE)
				{
					continue;
				}
			}
		}

		const FRawAnimSequenceTrack& Raw = SrcTrack.InternalTrackData;
		FAnimationTrackData TrackData;
		TrackData.TrackName = MappedBoneName;

		const int32 NumPosKeys = Raw.PosKeys.Num();
		const int32 NumRotKeys = Raw.RotKeys.Num();
		const int32 NumScaleKeys = Raw.ScaleKeys.Num();
		const int32 NumFrames = FMath::Max3(NumPosKeys, NumRotKeys, NumScaleKeys);

		if (NumFrames == 0)
		{
			continue;
		}

		const float Duration = SourceModel->GetPlayLength();
		const float SafeDuration = FMath::Max(Duration, 0.01f);

		for (int32 FrameIdx = 0; FrameIdx < NumFrames; ++FrameIdx)
		{
			FAnimationKeyframe KF;
			KF.Time = (NumFrames > 1) ? (static_cast<float>(FrameIdx) / (NumFrames - 1)) * SafeDuration : 0.0f;

			FVector Pos = (FrameIdx < NumPosKeys) ? FVector(Raw.PosKeys[FrameIdx]) : FVector::ZeroVector;
			FQuat Rot = (FrameIdx < NumRotKeys) ? FQuat(Raw.RotKeys[FrameIdx]) : FQuat::Identity;
			FVector Scale = (FrameIdx < NumScaleKeys) ? FVector(Raw.ScaleKeys[FrameIdx]) : FVector::OneVector;

			KF.Transform = FTransform(Rot, Pos, Scale);
			TrackData.Keyframes.Add(KF);
		}

		RetargetedTracks.Add(TrackData);
	}

	return CreateAnimation(WorldContextObject, NewName, TargetSkeleton, RetargetedTracks,
		SourceModel->GetPlayLength(), 30.0f);
}

TArray<UAnimSequence*> UAnimationLibrary::BatchRetargetAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& SourceAnimations,
	USkeleton* TargetSkeleton,
	const FAnimationRetargetSettings& Settings)
{
	TArray<UAnimSequence*> RetargetedAnimations;

	if (!TargetSkeleton)
	{
		return RetargetedAnimations;
	}

	for (UAnimSequence* Source : SourceAnimations)
	{
		if (Source)
		{
			UAnimSequence* Retargeted = RetargetAnimation(WorldContextObject, Source, TargetSkeleton, Settings);
			if (Retargeted)
			{
				RetargetedAnimations.Add(Retargeted);
			}
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

	const IAnimationDataModel* Model = AnimSequence->GetDataModel();
	if (!Model)
	{
		return false;
	}

	IAnimationDataController& Controller = AnimSequence->GetController();
	const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();
	const float Duration = Model->GetPlayLength();
	const float SafeDuration = FMath::Max(Duration, 0.01f);

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		const FRawAnimSequenceTrack& Raw = Track.InternalTrackData;
		const FName& BoneName = Track.Name;

		TArray<FVector3f> ReducedPosKeys;
		TArray<FQuat4f> ReducedRotKeys;
		TArray<FVector3f> ReducedScaleKeys;

		if (Raw.PosKeys.Num() > 0 && Settings.bCompressTranslation)
		{
			ReducedPosKeys.Add(Raw.PosKeys[0]);
			for (int32 i = 1; i < Raw.PosKeys.Num(); ++i)
			{
				const float Delta = FVector::Distance(FVector(Raw.PosKeys[i - 1]), FVector(Raw.PosKeys[i]));
				if (Delta > Settings.TranslationThreshold)
				{
					ReducedPosKeys.Add(Raw.PosKeys[i]);
				}
			}
			if (ReducedPosKeys.Num() == 1 && Raw.PosKeys.Num() > 1)
			{
				ReducedPosKeys.Add(Raw.PosKeys.Last());
			}
		}
		else
		{
			ReducedPosKeys = Raw.PosKeys;
		}

		if (Raw.RotKeys.Num() > 0 && Settings.bCompressRotation)
		{
			ReducedRotKeys.Add(Raw.RotKeys[0]);
			for (int32 i = 1; i < Raw.RotKeys.Num(); ++i)
			{
				const float Angle = FMath::RadiansToDegrees(
					FQuat(FQuat4f(Raw.RotKeys[i - 1])).AngularDistance(FQuat(FQuat4f(Raw.RotKeys[i]))));
				if (Angle > Settings.RotationThreshold)
				{
					ReducedRotKeys.Add(Raw.RotKeys[i]);
				}
			}
			if (ReducedRotKeys.Num() == 1 && Raw.RotKeys.Num() > 1)
			{
				ReducedRotKeys.Add(Raw.RotKeys.Last());
			}
		}
		else
		{
			ReducedRotKeys = Raw.RotKeys;
		}

		if (Raw.ScaleKeys.Num() > 0 && Settings.bCompressScale)
		{
			ReducedScaleKeys.Add(Raw.ScaleKeys[0]);
			for (int32 i = 1; i < Raw.ScaleKeys.Num(); ++i)
			{
				const float Delta = FVector::Distance(FVector(Raw.ScaleKeys[i - 1]), FVector(Raw.ScaleKeys[i]));
				if (Delta > Settings.ScaleThreshold)
				{
					ReducedScaleKeys.Add(Raw.ScaleKeys[i]);
				}
			}
			if (ReducedScaleKeys.Num() == 1 && Raw.ScaleKeys.Num() > 1)
			{
				ReducedScaleKeys.Add(Raw.ScaleKeys.Last());
			}
		}
		else
		{
			ReducedScaleKeys = Raw.ScaleKeys;
		}

		Controller.SetBoneTrackKeys(BoneName, ReducedPosKeys, ReducedRotKeys, ReducedScaleKeys, false);
	}

	AnimSequence->MarkPackageDirty();
	return true;
}

int32 UAnimationLibrary::BatchCompressAnimations(
	const TArray<UAnimSequence*>& AnimSequences,
	const FAnimationCompressionSettings& Settings)
{
	int32 CompressedCount = 0;
	for (UAnimSequence* Anim : AnimSequences)
	{
		if (Anim && CompressAnimation(Anim, Settings))
		{
			CompressedCount++;
		}
	}
	return CompressedCount;
}

float UAnimationLibrary::GetAnimationCompressionRatio(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return 1.0f;
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
	if (!Skeleton)
	{
		return nullptr;
	}

	UAsyncImportHelper* Helper = NewObject<UAsyncImportHelper>();
	return Helper;
}

UAnimSequence* UAnimationLibrary::BlendAnimations(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<UAnimSequence*>& Animations,
	const TArray<float>& Weights)
{
	if (!Skeleton || Animations.Num() == 0 || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	for (UAnimSequence* Anim : Animations)
	{
		if (!Anim)
		{
			return nullptr;
		}
	}

	float MaxDuration = 0.0f;
	float TotalWeight = 0.0f;
	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		const float Weight = (i < Weights.Num()) ? Weights[i] : 1.0f;
		TotalWeight += Weight;
		MaxDuration = FMath::Max(MaxDuration, Animations[i]->GetPlayLength());
	}
	if (TotalWeight <= 0.0f)
	{
		TotalWeight = 1.0f;
	}

	TMap<FName, int32> CommonBones;
	const int32 FirstNumTracks = Animations[0]->GetDataModel() ? Animations[0]->GetDataModel()->GetBoneAnimationTracks().Num() : 0;

	if (FirstNumTracks > 0)
	{
		const TArray<FBoneAnimationTrack>& FirstTracks = Animations[0]->GetDataModel()->GetBoneAnimationTracks();
		for (int32 i = 0; i < FirstTracks.Num(); ++i)
		{
			const FName& BoneName = FirstTracks[i].Name;
			bool bAllHaveBone = true;
			for (int32 a = 1; a < Animations.Num(); ++a)
			{
				const IAnimationDataModel* OtherModel = Animations[a]->GetDataModel();
				if (!OtherModel || !OtherModel->FindBoneTrackByName(BoneName))
				{
					bAllHaveBone = false;
					break;
				}
			}
			if (bAllHaveBone)
			{
				CommonBones.Add(BoneName, i);
			}
		}
	}

	const float BlendFrameRate = 30.0f;
	const int32 NumBlendFrames = FMath::Max(2, FMath::CeilToInt(MaxDuration * BlendFrameRate) + 1);

	TArray<FAnimationTrackData> BlendTracks;
	for (const TPair<FName, int32>& BonePair : CommonBones)
	{
		FAnimationTrackData TrackData;
		TrackData.TrackName = BonePair.Key;

		for (int32 Frame = 0; Frame < NumBlendFrames; ++Frame)
		{
			const float Time = (NumBlendFrames > 1) ? static_cast<float>(Frame) / (NumBlendFrames - 1) * MaxDuration : 0.0f;

			FVector BlendedPos = FVector::ZeroVector;
			FQuat BlendedRot = FQuat::Identity;
			FVector BlendedScale = FVector::OneVector;

			for (int32 a = 0; a < Animations.Num(); ++a)
			{
				const float Weight = ((a < Weights.Num()) ? Weights[a] : 1.0f) / TotalWeight;
				const IAnimationDataModel* OtherModel = Animations[a]->GetDataModel();
				const FBoneAnimationTrack* OtherTrack = OtherModel->FindBoneTrackByName(BonePair.Key);
				if (!OtherTrack)
				{
					continue;
				}

				const FRawAnimSequenceTrack& Raw = OtherTrack->InternalTrackData;
				const int32 NumPos = Raw.PosKeys.Num();
				const int32 NumRot = Raw.RotKeys.Num();
				const int32 NumScale = Raw.ScaleKeys.Num();
				const float OtherDuration = FMath::Max(OtherModel->GetPlayLength(), 0.01f);
				const float NormalizedTime = FMath::Clamp(Time / OtherDuration, 0.0f, 1.0f);

				if (NumPos > 0)
				{
					const float PosAlpha = NormalizedTime * (NumPos - 1);
					const int32 PosIdx0 = FMath::Clamp(FMath::FloorToInt(PosAlpha), 0, NumPos - 2);
					const int32 PosIdx1 = PosIdx0 + 1;
					const float PosLerp = PosAlpha - PosIdx0;
					BlendedPos += Weight * FMath::Lerp(FVector(Raw.PosKeys[PosIdx0]), FVector(Raw.PosKeys[PosIdx1]), PosLerp);
				}

				if (NumRot > 0)
				{
					const float RotAlpha = NormalizedTime * (NumRot - 1);
					const int32 RotIdx0 = FMath::Clamp(FMath::FloorToInt(RotAlpha), 0, NumRot - 2);
					const int32 RotIdx1 = RotIdx0 + 1;
					const float RotLerp = RotAlpha - RotIdx0;
					FQuat R0 = FQuat(Raw.RotKeys[RotIdx0]);
					FQuat R1 = FQuat(Raw.RotKeys[RotIdx1]);
					if (R0.SizeSquared() > 0.0f && R1.SizeSquared() > 0.0f)
					{
						BlendedRot = Weight * FQuat::Slerp(R0, R1, RotLerp);
					}
				}

				if (NumScale > 0)
				{
					const float ScaleAlpha = NormalizedTime * (NumScale - 1);
					const int32 ScaleIdx0 = FMath::Clamp(FMath::FloorToInt(ScaleAlpha), 0, NumScale - 2);
					const int32 ScaleIdx1 = ScaleIdx0 + 1;
					const float ScaleLerp = ScaleAlpha - ScaleIdx0;
					BlendedScale *= FMath::Lerp(FVector(Raw.ScaleKeys[ScaleIdx0]), FVector(Raw.ScaleKeys[ScaleIdx1]), ScaleLerp);
				}
			}

			FAnimationKeyframe KF;
			KF.Time = Time;
			KF.Transform = FTransform(BlendedRot, BlendedPos, BlendedScale);
			TrackData.Keyframes.Add(KF);
		}

		BlendTracks.Add(TrackData);
	}

	return CreateAnimation(WorldContextObject, AnimationName, Skeleton, BlendTracks, MaxDuration, BlendFrameRate);
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
	if (!Skeleton || !From || !To || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	const float FromDuration = FromEndTime < 0.0f ? From->GetPlayLength() : FromEndTime;
	const float ClampedToStart = FMath::Clamp(ToStartTime, 0.0f, To->GetPlayLength());
	const float TotalDuration = FMath::Max(0.0f, FromDuration + (To->GetPlayLength() - ClampedToStart));

	return CreateAnimation(WorldContextObject, AnimationName, Skeleton,
		TArray<FAnimationTrackData>(), TotalDuration, 30.0f);
}

UAnimSequence* UAnimationLibrary::MirrorAnimation(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* SourceAnimation,
	EMirrorFlags MirrorFlags)
{
	if (!Skeleton || !SourceAnimation || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	if (MirrorFlags == EMirrorFlags::None)
	{
		return nullptr;
	}

	const IAnimationDataModel* SourceModel = SourceAnimation->GetDataModel();
	if (!SourceModel)
	{
		return nullptr;
	}

	const bool bMirrorX = (static_cast<uint8>(MirrorFlags) & static_cast<uint8>(EMirrorFlags::X)) != 0;
	const bool bMirrorY = (static_cast<uint8>(MirrorFlags) & static_cast<uint8>(EMirrorFlags::Y)) != 0;
	const bool bMirrorZ = (static_cast<uint8>(MirrorFlags) & static_cast<uint8>(EMirrorFlags::Z)) != 0;

	TArray<FAnimationTrackData> MirroredTracks;
	const TArray<FBoneAnimationTrack>& SourceTracks = SourceModel->GetBoneAnimationTracks();
	const float Duration = SourceModel->GetPlayLength();
	const float SafeDuration = FMath::Max(Duration, 0.01f);

	const USkeleton* SourceSkeleton = SourceAnimation->GetSkeleton();

	for (const FBoneAnimationTrack& SrcTrack : SourceTracks)
	{
		FAnimationTrackData TrackData;
		TrackData.TrackName = SrcTrack.Name;

		const FRawAnimSequenceTrack& Raw = SrcTrack.InternalTrackData;
		const int32 NumPosKeys = Raw.PosKeys.Num();
		const int32 NumRotKeys = Raw.RotKeys.Num();
		const int32 NumScaleKeys = Raw.ScaleKeys.Num();
		const int32 NumFrames = FMath::Max3(NumPosKeys, NumRotKeys, NumScaleKeys);

		if (NumFrames == 0)
		{
			continue;
		}

		bool bIsLeftBone = false;
		bool bIsRightBone = false;
		const FString BoneNameStr = SrcTrack.Name.ToString();

		if (SourceSkeleton)
		{
			const FReferenceSkeleton& RefSkel = SourceSkeleton->GetReferenceSkeleton();
			const int32 BoneIdx = RefSkel.FindBoneIndex(SrcTrack.Name);
			if (BoneIdx != INDEX_NONE)
			{
				bIsLeftBone = RefSkel.GetBoneName(BoneIdx).ToString().Contains(TEXT("Left")) ||
					RefSkel.GetBoneName(BoneIdx).ToString().Contains(TEXT("L_")) ||
					RefSkel.GetBoneName(BoneIdx).ToString().StartsWith(TEXT("l_"));
				bIsRightBone = RefSkel.GetBoneName(BoneIdx).ToString().Contains(TEXT("Right")) ||
					RefSkel.GetBoneName(BoneIdx).ToString().Contains(TEXT("R_")) ||
					RefSkel.GetBoneName(BoneIdx).ToString().StartsWith(TEXT("r_"));
			}
		}

		for (int32 FrameIdx = 0; FrameIdx < NumFrames; ++FrameIdx)
		{
			FVector Pos = (FrameIdx < NumPosKeys) ? FVector(Raw.PosKeys[FrameIdx]) : FVector::ZeroVector;
			FQuat Rot = (FrameIdx < NumRotKeys) ? FQuat(Raw.RotKeys[FrameIdx]) : FQuat::Identity;
			FVector Scale = (FrameIdx < NumScaleKeys) ? FVector(Raw.ScaleKeys[FrameIdx]) : FVector::OneVector;

			FVector MirrorScale(bMirrorX ? -1.0f : 1.0f, bMirrorY ? -1.0f : 1.0f, bMirrorZ ? -1.0f : 1.0f);

			if (bIsLeftBone)
			{
				Pos.Y = -Pos.Y;
			}
			else if (bIsRightBone)
			{
				Pos.Y = -Pos.Y;
			}
			else
			{
				Pos *= MirrorScale;
			}

			if (bMirrorX)
			{
				Rot.Y = -Rot.Y;
				Rot.Z = -Rot.Z;
			}
			if (bMirrorY)
			{
				Rot.X = -Rot.X;
				Rot.Z = -Rot.Z;
			}
			if (bMirrorZ)
			{
				Rot.X = -Rot.X;
				Rot.Y = -Rot.Y;
			}

			FAnimationKeyframe KF;
			KF.Time = (NumFrames > 1) ? (static_cast<float>(FrameIdx) / (NumFrames - 1)) * SafeDuration : 0.0f;
			KF.Transform = FTransform(Rot, Pos, Scale);
			TrackData.Keyframes.Add(KF);
		}

		MirroredTracks.Add(TrackData);
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
	if (!Skeleton || !SourceAnimation || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	EMirrorFlags Flags = EMirrorFlags::None;
	if (bMirrorX) Flags = Flags | EMirrorFlags::X;
	if (bMirrorY) Flags = Flags | EMirrorFlags::Y;

	return MirrorAnimation(WorldContextObject, AnimationName, Skeleton, SourceAnimation, Flags);
}

bool UAnimationLibrary::ExportAnimationFBX(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	FString Content = FString::Printf(TEXT("; AnimationToolkit FBX Export\n; Animation: %s\n; Duration: %.3f\n"),
		*AnimSequence->GetName(), AnimSequence->GetPlayLength());

	const USkeleton* Skeleton = AnimSequence->GetSkeleton();
	if (Skeleton)
	{
		const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
		for (int32 i = 0; i < RefSkel.GetNum(); ++i)
		{
			Content += FString::Printf(TEXT("; Bone: %s\n"), *RefSkel.GetBoneName(i).ToString());
		}
	}

	return FFileHelper::SaveStringToFile(Content, *OutputPath);
}

bool UAnimationLibrary::ExportAnimationBVH(UAnimSequence* AnimSequence, const FString& OutputPath, const FString& CharacterName)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	const float Duration = AnimSequence->GetPlayLength();
	const int32 NumFrames = AnimSequence->GetNumberOfSampledKeys();
	const float FrameTime = (NumFrames > 1) ? Duration / (NumFrames - 1) : 0.0333f;

	FString Content;
	Content += TEXT("HIERARCHY\n");

	const USkeleton* Skeleton = AnimSequence->GetSkeleton();
	if (Skeleton)
	{
		const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
		const int32 NumBones = RefSkel.GetNum();

		TArray<int32> BoneDepths;
		BoneDepths.SetNum(NumBones);
		for (int32 i = 0; i < NumBones; ++i)
		{
			int32 Depth = 0;
			int32 ParentIdx = RefSkel.GetParentIndex(i);
			while (ParentIdx != INDEX_NONE)
			{
				Depth++;
				ParentIdx = RefSkel.GetParentIndex(ParentIdx);
			}
			BoneDepths[i] = Depth;
		}

		for (int32 i = 0; i < NumBones; ++i)
		{
			const int32 ParentIdx = RefSkel.GetParentIndex(i);
			const FName BoneName = RefSkel.GetBoneName(i);

			if (ParentIdx == INDEX_NONE)
			{
				Content += FString::Printf(TEXT("ROOT %s\n"), *BoneName.ToString());
			}
			else
			{
				Content += FString::Printf(TEXT("JOINT %s\n"), *BoneName.ToString());
			}
			Content += TEXT("{\n");
			Content += FString::Printf(TEXT("\tOFFSET 0.00 0.00 0.00\n"));
			Content += TEXT("\tCHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation\n");

			bool bIsLeaf = true;
			for (int32 j = 0; j < NumBones; ++j)
			{
				if (RefSkel.GetParentIndex(j) == i)
				{
					bIsLeaf = false;
					break;
				}
			}

			if (bIsLeaf)
			{
				Content += TEXT("\tEnd Site\n\t{\n\t\tOFFSET 0.00 0.00 0.00\n\t}\n");
			}

			int32 NextDepth = (i + 1 < NumBones) ? BoneDepths[i + 1] : 0;
			int32 ClosesNeeded = BoneDepths[i] - NextDepth;
			if (i == NumBones - 1)
			{
				ClosesNeeded = BoneDepths[i];
			}

			for (int32 c = 0; c < ClosesNeeded; ++c)
			{
				Content += TEXT("}\n");
			}
		}
	}

	Content += TEXT("MOTION\n");
	Content += FString::Printf(TEXT("Frames: %d\n"), NumFrames);
	Content += FString::Printf(TEXT("Frame Time: %.6f\n"), FrameTime);

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const float Time = Frame * FrameTime;
		Content += FString::Printf(TEXT("%.6f %.6f %.6f %.6f %.6f %.6f\n"),
			Time, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	}

	return FFileHelper::SaveStringToFile(Content, *OutputPath);
}

bool UAnimationLibrary::ExportAnimationglTF(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	FString Content;
	Content += TEXT("{\n");
	Content += TEXT("\t\"asset\": {\n");
	Content += TEXT("\t\t\"version\": \"2.0\",\n");
	Content += TEXT("\t\t\"generator\": \"AnimationToolkit\"\n");
	Content += TEXT("\t},\n");
	Content += TEXT("\t\"animations\": [],\n");
	Content += TEXT("\t\"nodes\": [],\n");
	Content += TEXT("\t\"accessors\": [],\n");
	Content += TEXT("\t\"bufferViews\": [],\n");
	Content += TEXT("\t\"buffers\": []\n");
	Content += TEXT("}\n");

	return FFileHelper::SaveStringToFile(Content, *OutputPath);
}

bool UAnimationLibrary::ExportAnimationCollada(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	FString Content;
	Content += TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	Content += TEXT("<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n");
	Content += TEXT("\t<asset>\n");
	Content += FString::Printf(TEXT("\t\t<created>AnimationToolkit Export</created>\n"));
	Content += TEXT("\t</asset>\n");
	Content += TEXT("\t<library_animations/>\n");
	Content += TEXT("\t<library_visual_scenes/>\n");
	Content += TEXT("\t<scene>\n");
	Content += TEXT("\t\t<instance_visual_scene url=\"#Scene\"/>\n");
	Content += TEXT("\t</scene>\n");
	Content += TEXT("</COLLADA>\n");

	return FFileHelper::SaveStringToFile(Content, *OutputPath);
}

bool UAnimationLibrary::ExportAnimation(UAnimSequence* AnimSequence, const FString& OutputPath, EAnimationImportFormat Format)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	switch (Format)
	{
	case EAnimationImportFormat::FBX: return ExportAnimationFBX(AnimSequence, OutputPath);
	case EAnimationImportFormat::BVH: return ExportAnimationBVH(AnimSequence, OutputPath, TEXT("Character"));
	case EAnimationImportFormat::glTF: return ExportAnimationglTF(AnimSequence, OutputPath);
	case EAnimationImportFormat::COLLADA: return ExportAnimationCollada(AnimSequence, OutputPath);
	default: return false;
	}
}

void UAnimationLibrary::SetAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	if (!AnimSequence)
	{
		return;
	}

	CleanupStaleTags();
	TArray<FName>& Tags = AnimationTagsMap.FindOrAdd(AnimSequence);
	if (!Tags.Contains(Tag))
	{
		Tags.Add(Tag);
	}
}

void UAnimationLibrary::SetAnimationTags(UAnimSequence* AnimSequence, const TArray<FName>& Tags)
{
	if (!AnimSequence)
	{
		return;
	}

	CleanupStaleTags();
	TArray<FName>& ExistingTags = AnimationTagsMap.FindOrAdd(AnimSequence);
	for (const FName& Tag : Tags)
	{
		if (!ExistingTags.Contains(Tag))
		{
			ExistingTags.Add(Tag);
		}
	}
}

void UAnimationLibrary::RemoveAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	if (!AnimSequence)
	{
		return;
	}

	CleanupStaleTags();
	TArray<FName>* Tags = AnimationTagsMap.Find(AnimSequence);
	if (Tags)
	{
		Tags->Remove(Tag);
	}
}

bool UAnimationLibrary::HasAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	if (!AnimSequence)
	{
		return false;
	}

	CleanupStaleTags();
	const TArray<FName>* Tags = AnimationTagsMap.Find(AnimSequence);
	return Tags && Tags->Contains(Tag);
}

TArray<FName> UAnimationLibrary::GetAnimationTags(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return TArray<FName>();
	}

	CleanupStaleTags();
	const TArray<FName>* Tags = AnimationTagsMap.Find(AnimSequence);
	return Tags ? *Tags : TArray<FName>();
}

TArray<UAnimSequence*> UAnimationLibrary::FindAnimationsByTag(const TArray<UAnimSequence*>& Animations, const FName& Tag)
{
	TArray<UAnimSequence*> Result;

	for (UAnimSequence* Anim : Animations)
	{
		if (Anim && HasAnimationTag(Anim, Tag))
		{
			Result.Add(Anim);
		}
	}

	return Result;
}

void UAnimationLibrary::ClearAnimationTags(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return;
	}

	AnimationTagsMap.Remove(AnimSequence);
}

void UAnimationLibrary::ClearGlobalProgressCallback()
{
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

	const FString Name = TEXT("EmptyAnim_") + FGuid::NewGuid().ToString().Left(8);
	return CreateAnimation(WorldContextObject, Name, Skeleton, TArray<FAnimationTrackData>(),
		Duration, static_cast<float>(FrameRate));
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

	IAnimationDataController& Controller = Animation->GetController();
	const IAnimationDataModel* Model = Animation->GetDataModel();
	Controller.AddBoneCurve(BoneName, false);

	const FBoneAnimationTrack* ExistingTrack = Model ? Model->FindBoneTrackByName(BoneName) : nullptr;

	TArray<FVector3f> PosKeys;
	TArray<FQuat4f> RotKeys;
	TArray<FVector3f> ScaleKeys;

	if (ExistingTrack)
	{
		const FRawAnimSequenceTrack& Raw = ExistingTrack->InternalTrackData;
		PosKeys = Raw.PosKeys;
		RotKeys = Raw.RotKeys;
		ScaleKeys = Raw.ScaleKeys;
	}

	const float Duration = Model ? Model->GetPlayLength() : 1.0f;
	const float NormalizedTime = FMath::Clamp(Time / FMath::Max(Duration, 0.01f), 0.0f, 1.0f);
	const int32 InsertIndex = FMath::Clamp(FMath::RoundToInt(NormalizedTime * FMath::Max(1, PosKeys.Num() - 1)), 0, PosKeys.Num());

	FVector3f Pos = FVector3f(Transform.GetLocation());
	FQuat4f Rot = FQuat4f(Transform.GetRotation());
	FVector3f Scale = FVector3f(Transform.GetScale3D());

	if (bUpdateExistingKey && PosKeys.Num() > 0)
	{
		const int32 KeyIndex = FMath::Clamp(FMath::RoundToInt(NormalizedTime * (PosKeys.Num() - 1)), 0, PosKeys.Num() - 1);
		if (KeyIndex < PosKeys.Num()) PosKeys[KeyIndex] = Pos;
		if (KeyIndex < RotKeys.Num()) RotKeys[KeyIndex] = Rot;
		if (KeyIndex < ScaleKeys.Num()) ScaleKeys[KeyIndex] = Scale;
		else if (ScaleKeys.Num() > 0) ScaleKeys.Add(Scale);
		else ScaleKeys.SetNum(PosKeys.Num());
	}
	else
	{
		PosKeys.Insert(Pos, InsertIndex);
		RotKeys.Insert(Rot, InsertIndex);
		ScaleKeys.Insert(Scale, InsertIndex);
	}

	Controller.SetBoneTrackKeys(BoneName, PosKeys, RotKeys, ScaleKeys, false);
	Animation->MarkPackageDirty();
	return true;
}

int32 UAnimationLibrary::AddKeyframesBatch(
	UAnimSequence* Animation,
	const TArray<FAnimationKeyframe>& Keyframes)
{
	if (!Animation || Keyframes.Num() == 0)
	{
		return 0;
	}

	IAnimationDataController& Controller = Animation->GetController();
	const IAnimationDataModel* Model = Animation->GetDataModel();
	if (!Model)
	{
		return 0;
	}

	const TArray<FBoneAnimationTrack>& BoneTracks = Model->GetBoneAnimationTracks();
	if (BoneTracks.Num() == 0)
	{
		return 0;
	}

	int32 KeysAdded = 0;

	for (const FBoneAnimationTrack& BoneTrack : BoneTracks)
	{
		const FName BoneName = BoneTrack.Name;
		Controller.AddBoneCurve(BoneName, false);

		const FRawAnimSequenceTrack& Raw = BoneTrack.InternalTrackData;
		TArray<FVector3f> PosKeys(Raw.PosKeys);
		TArray<FQuat4f> RotKeys(Raw.RotKeys);
		TArray<FVector3f> ScaleKeys(Raw.ScaleKeys);

		for (const FAnimationKeyframe& KF : Keyframes)
		{
			const float Duration = Model->GetPlayLength();
			const float SafeDuration = FMath::Max(Duration, 0.01f);
			const float NormalizedTime = FMath::Clamp(KF.Time / SafeDuration, 0.0f, 1.0f);
			const int32 NumFrames = FMath::Max3(PosKeys.Num(), RotKeys.Num(), ScaleKeys.Num());

			if (NumFrames > 0)
			{
				const int32 KeyIndex = FMath::Clamp(
					FMath::RoundToInt(NormalizedTime * FMath::Max(1, NumFrames - 1)),
					0, NumFrames - 1);

			if (KeyIndex < PosKeys.Num())
			{
				PosKeys[KeyIndex] = FVector3f(KF.Transform.GetLocation());
			}
			if (KeyIndex < RotKeys.Num())
			{
				RotKeys[KeyIndex] = FQuat4f(KF.Transform.GetRotation());
			}
			if (KeyIndex < ScaleKeys.Num())
			{
				ScaleKeys[KeyIndex] = FVector3f(KF.Transform.GetScale3D());
			}
		}
		else
		{
			PosKeys.Add(FVector3f(KF.Transform.GetLocation()));
			RotKeys.Add(FQuat4f(KF.Transform.GetRotation()));
			ScaleKeys.Add(FVector3f(KF.Transform.GetScale3D()));
			}

			KeysAdded++;
		}

		Controller.SetBoneTrackKeys(BoneName, PosKeys, RotKeys, ScaleKeys, false);
	}

	Animation->MarkPackageDirty();
	return KeysAdded;
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

	const IAnimationDataModel* Model = Animation->GetDataModel();
	if (!Model)
	{
		return false;
	}

	const FBoneAnimationTrack* Track = Model->FindBoneTrackByName(BoneName);
	if (!Track)
	{
		return false;
	}

	IAnimationDataController& Controller = Animation->GetController();
	const FRawAnimSequenceTrack& Raw = Track->InternalTrackData;
	const float Duration = Model->GetPlayLength();
	const float SafeDuration = FMath::Max(Duration, 0.01f);

	TArray<FVector3f> PosKeys(Raw.PosKeys);
	TArray<FQuat4f> RotKeys(Raw.RotKeys);
	TArray<FVector3f> ScaleKeys(Raw.ScaleKeys);

	const int32 NumPosKeys = PosKeys.Num();
	const int32 NumRotKeys = RotKeys.Num();
	const int32 NumScaleKeys = ScaleKeys.Num();
	const int32 NumFrames = FMath::Max3(NumPosKeys, NumRotKeys, NumScaleKeys);

	if (NumFrames <= 1)
	{
		return false;
	}

	const float NormalizedTime = FMath::Clamp(Time / SafeDuration, 0.0f, 1.0f);
	const int32 RemoveIndex = FMath::Clamp(FMath::RoundToInt(NormalizedTime * (NumFrames - 1)), 0, NumFrames - 1);

	if (RemoveIndex == 0 && NumFrames <= 1)
	{
		return false;
	}

	if (RemoveIndex < PosKeys.Num() && PosKeys.Num() > 1)
	{
		PosKeys.RemoveAt(RemoveIndex);
	}
	if (RemoveIndex < RotKeys.Num() && RotKeys.Num() > 1)
	{
		RotKeys.RemoveAt(RemoveIndex);
	}
	if (RemoveIndex < ScaleKeys.Num() && ScaleKeys.Num() > 1)
	{
		ScaleKeys.RemoveAt(RemoveIndex);
	}

	Controller.SetBoneTrackKeys(BoneName, PosKeys, RotKeys, ScaleKeys, false);
	Animation->MarkPackageDirty();
	return true;
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

	const IAnimationDataModel* Model = Animation->GetDataModel();
	if (!Model)
	{
		return FTransform::Identity;
	}

	const FBoneAnimationTrack* Track = Model->FindBoneTrackByName(BoneName);
	if (!Track)
	{
		return FTransform::Identity;
	}

	const FRawAnimSequenceTrack& Raw = Track->InternalTrackData;
	const float Duration = Model->GetPlayLength();
	const float SafeDuration = FMath::Max(Duration, 0.01f);

	FVector ResultPos = FVector::ZeroVector;
	FQuat ResultRot = FQuat::Identity;
	FVector ResultScale = FVector::OneVector;

	const int32 NumPosKeys = Raw.PosKeys.Num();
	const int32 NumRotKeys = Raw.RotKeys.Num();
	const int32 NumScaleKeys = Raw.ScaleKeys.Num();

	if (NumPosKeys > 0)
	{
		if (NumPosKeys == 1)
		{
			ResultPos = FVector(Raw.PosKeys[0]);
		}
		else
		{
			const float NormalizedTime = FMath::Clamp(Time / SafeDuration, 0.0f, 1.0f);
			const float PosAlpha = NormalizedTime * (NumPosKeys - 1);
			const int32 PosIdx0 = FMath::Clamp(FMath::FloorToInt(PosAlpha), 0, NumPosKeys - 2);
			const int32 PosIdx1 = PosIdx0 + 1;
			const float PosLerp = PosAlpha - PosIdx0;
			ResultPos = FMath::Lerp(FVector(Raw.PosKeys[PosIdx0]), FVector(Raw.PosKeys[PosIdx1]), PosLerp);
		}
	}

	if (NumRotKeys > 0)
	{
		if (NumRotKeys == 1)
		{
			ResultRot = FQuat(Raw.RotKeys[0]);
		}
		else
		{
			const float NormalizedTime = FMath::Clamp(Time / SafeDuration, 0.0f, 1.0f);
			const float RotAlpha = NormalizedTime * (NumRotKeys - 1);
			const int32 RotIdx0 = FMath::Clamp(FMath::FloorToInt(RotAlpha), 0, NumRotKeys - 2);
			const int32 RotIdx1 = RotIdx0 + 1;
			const float RotLerp = RotAlpha - RotIdx0;
			FQuat R0 = FQuat(Raw.RotKeys[RotIdx0]);
			FQuat R1 = FQuat(Raw.RotKeys[RotIdx1]);
			if (R0.SizeSquared() > 0.0f && R1.SizeSquared() > 0.0f)
			{
				ResultRot = FQuat::Slerp(R0, R1, RotLerp);
			}
		}
	}

	if (NumScaleKeys > 0)
	{
		if (NumScaleKeys == 1)
		{
			ResultScale = FVector(Raw.ScaleKeys[0]);
		}
		else
		{
			const float NormalizedTime = FMath::Clamp(Time / SafeDuration, 0.0f, 1.0f);
			const float ScaleAlpha = NormalizedTime * (NumScaleKeys - 1);
			const int32 ScaleIdx0 = FMath::Clamp(FMath::FloorToInt(ScaleAlpha), 0, NumScaleKeys - 2);
			const int32 ScaleIdx1 = ScaleIdx0 + 1;
			const float ScaleLerp = ScaleAlpha - ScaleIdx0;
			ResultScale = FMath::Lerp(FVector(Raw.ScaleKeys[ScaleIdx0]), FVector(Raw.ScaleKeys[ScaleIdx1]), ScaleLerp);
		}
	}

	return FTransform(ResultRot, ResultPos, ResultScale);
}

UAnimSequence* UAnimationLibrary::RetargetSpeed(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float SpeedMultiplier)
{
	if (!SourceAnimation)
	{
		return nullptr;
	}

	const IAnimationDataModel* SourceModel = SourceAnimation->GetDataModel();
	if (!SourceModel)
	{
		const float NewDuration = SourceAnimation->GetPlayLength() / FMath::Max(0.01f, SpeedMultiplier);
		const FString NewName = SourceAnimation->GetName() + TEXT("_Speed");
		return CreateAnimation(WorldContextObject, NewName, SourceAnimation->GetSkeleton(),
			TArray<FAnimationTrackData>(), NewDuration, 30.0f);
	}

	const float NewDuration = SourceModel->GetPlayLength() / FMath::Max(0.01f, SpeedMultiplier);
	TArray<FAnimationTrackData> ResampledTracks;

	for (const FBoneAnimationTrack& SrcTrack : SourceModel->GetBoneAnimationTracks())
	{
		FAnimationTrackData TrackData;
		TrackData.TrackName = SrcTrack.Name;

		const FRawAnimSequenceTrack& Raw = SrcTrack.InternalTrackData;
		const int32 NumPosKeys = Raw.PosKeys.Num();
		const int32 NumRotKeys = Raw.RotKeys.Num();
		const int32 NumScaleKeys = Raw.ScaleKeys.Num();
		const int32 NumFrames = FMath::Max3(NumPosKeys, NumRotKeys, NumScaleKeys);

		if (NumFrames == 0) continue;

		const float SourceDuration = SourceModel->GetPlayLength();
		const float SafeSourceDuration = FMath::Max(SourceDuration, 0.01f);
		const float SafeNewDuration = FMath::Max(NewDuration, 0.01f);
		const int32 NewNumFrames = FMath::Max(1, FMath::RoundToInt(SafeNewDuration * 30.0f));

		for (int32 NewFrame = 0; NewFrame < NewNumFrames; ++NewFrame)
		{
			const float NewTime = static_cast<float>(NewFrame) / 30.0f;
			const float SourceTime = NewTime * FMath::Max(0.01f, SpeedMultiplier);
			const float NormalizedSourceTime = FMath::Clamp(SourceTime / SafeSourceDuration, 0.0f, 1.0f);

			const float PosAlpha = NormalizedSourceTime * FMath::Max(1, NumPosKeys - 1);
			const int32 PosIdx0 = FMath::Clamp(FMath::FloorToInt(PosAlpha), 0, NumPosKeys - 2);
			const int32 PosIdx1 = PosIdx0 + 1;
			const float PosLerp = PosAlpha - PosIdx0;
			FVector Pos = (NumPosKeys > 1) ? FMath::Lerp(FVector(Raw.PosKeys[PosIdx0]), FVector(Raw.PosKeys[PosIdx1]), PosLerp) : (NumPosKeys > 0 ? FVector(Raw.PosKeys[0]) : FVector::ZeroVector);

			const float RotAlpha = NormalizedSourceTime * FMath::Max(1, NumRotKeys - 1);
			const int32 RotIdx0 = FMath::Clamp(FMath::FloorToInt(RotAlpha), 0, NumRotKeys - 2);
			const int32 RotIdx1 = RotIdx0 + 1;
			const float RotLerp = RotAlpha - RotIdx0;
			FQuat Rot = (NumRotKeys > 1) ? FQuat::Slerp(FQuat(Raw.RotKeys[RotIdx0]), FQuat(Raw.RotKeys[RotIdx1]), RotLerp) : (NumRotKeys > 0 ? FQuat(Raw.RotKeys[0]) : FQuat::Identity);

			const float ScaleAlpha = NormalizedSourceTime * FMath::Max(1, NumScaleKeys - 1);
			const int32 ScaleIdx0 = FMath::Clamp(FMath::FloorToInt(ScaleAlpha), 0, NumScaleKeys - 2);
			const int32 ScaleIdx1 = ScaleIdx0 + 1;
			const float ScaleLerp = ScaleAlpha - ScaleIdx0;
			FVector Scale = (NumScaleKeys > 1) ? FMath::Lerp(FVector(Raw.ScaleKeys[ScaleIdx0]), FVector(Raw.ScaleKeys[ScaleIdx1]), ScaleLerp) : (NumScaleKeys > 0 ? FVector(Raw.ScaleKeys[0]) : FVector::OneVector);

			FAnimationKeyframe KF;
			KF.Time = NewTime;
			KF.Transform = FTransform(Rot, Pos, Scale);
			TrackData.Keyframes.Add(KF);
		}

		ResampledTracks.Add(TrackData);
	}

	const FString NewName = SourceAnimation->GetName() + TEXT("_Speed");
	return CreateAnimation(WorldContextObject, NewName, SourceAnimation->GetSkeleton(),
		ResampledTracks, NewDuration, 30.0f);
}

UAnimSequence* UAnimationLibrary::CropAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float StartTime,
	float EndTime)
{
	if (!SourceAnimation)
	{
		return nullptr;
	}

	const IAnimationDataModel* SourceModel = SourceAnimation->GetDataModel();
	if (!SourceModel)
	{
		const float NewDuration = FMath::Max(0.0f, EndTime - StartTime);
		const FString NewName = SourceAnimation->GetName() + TEXT("_Cropped");
		return CreateAnimation(WorldContextObject, NewName, SourceAnimation->GetSkeleton(),
			TArray<FAnimationTrackData>(), NewDuration, 30.0f);
	}

	const TArray<FBoneAnimationTrack>& SourceTracks = SourceModel->GetBoneAnimationTracks();
	const float SourceDuration = SourceModel->GetPlayLength();
	const float SafeDuration = FMath::Max(SourceDuration, 0.01f);
	const float NewDuration = FMath::Max(0.0f, EndTime - StartTime);
	const float SafeNewDuration = FMath::Max(NewDuration, 0.01f);

	TArray<FAnimationTrackData> CroppedTracks;

	for (const FBoneAnimationTrack& SrcTrack : SourceTracks)
	{
		FAnimationTrackData TrackData;
		TrackData.TrackName = SrcTrack.Name;

		const FRawAnimSequenceTrack& Raw = SrcTrack.InternalTrackData;
		const int32 NumPosKeys = Raw.PosKeys.Num();
		const int32 NumRotKeys = Raw.RotKeys.Num();
		const int32 NumScaleKeys = Raw.ScaleKeys.Num();
		const int32 NumFrames = FMath::Max3(NumPosKeys, NumRotKeys, NumScaleKeys);

		if (NumFrames == 0)
		{
			continue;
		}

		for (int32 FrameIdx = 0; FrameIdx < NumFrames; ++FrameIdx)
		{
			const float FrameTime = (NumFrames > 1)
				? (static_cast<float>(FrameIdx) / (NumFrames - 1)) * SafeDuration
				: 0.0f;

			if (FrameTime < StartTime || FrameTime > EndTime)
			{
				continue;
			}

			FVector Pos = (FrameIdx < NumPosKeys) ? FVector(Raw.PosKeys[FrameIdx]) : FVector::ZeroVector;
			FQuat Rot = (FrameIdx < NumRotKeys) ? FQuat(Raw.RotKeys[FrameIdx]) : FQuat::Identity;
			FVector Scale = (FrameIdx < NumScaleKeys) ? FVector(Raw.ScaleKeys[FrameIdx]) : FVector::OneVector;

			FAnimationKeyframe KF;
			KF.Time = FrameTime - StartTime;
			KF.Transform = FTransform(Rot, Pos, Scale);
			TrackData.Keyframes.Add(KF);
		}

		if (TrackData.Keyframes.Num() > 0)
		{
			CroppedTracks.Add(TrackData);
		}
	}

	const FString NewName = SourceAnimation->GetName() + TEXT("_Cropped");
	return CreateAnimation(WorldContextObject, NewName, SourceAnimation->GetSkeleton(),
		CroppedTracks, NewDuration, 30.0f);
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

	IAnimationDataController& Controller = Animation->GetController();
	const IAnimationDataModel* Model = Animation->GetDataModel();
	if (!Model)
	{
		return false;
	}

	const TArray<FBoneAnimationTrack>& BoneTracks = Model->GetBoneAnimationTracks();
	if (BoneTracks.Num() == 0)
	{
		return false;
	}

	const FQuat RotQuat = RotationOffset.Quaternion();

	for (const FBoneAnimationTrack& BoneTrack : BoneTracks)
	{
		const FName BoneName = BoneTrack.Name;
		const FRawAnimSequenceTrack& Raw = BoneTrack.InternalTrackData;

		TArray<FVector3f> PosKeys(Raw.PosKeys);
		TArray<FQuat4f> RotKeys(Raw.RotKeys);
		TArray<FVector3f> ScaleKeys(Raw.ScaleKeys);

		const int32 NumPosKeys = PosKeys.Num();
		const int32 NumRotKeys = RotKeys.Num();

	const FVector3f LocOffset3f(LocationOffset);
	const FQuat4f RotQuat4f(RotQuat);

	for (int32 i = 0; i < NumPosKeys; ++i)
	{
		PosKeys[i] += LocOffset3f;
	}

	for (int32 i = 0; i < NumRotKeys; ++i)
	{
		RotKeys[i] = RotQuat4f * RotKeys[i];
	}

		Controller.SetBoneTrackKeys(BoneName, PosKeys, RotKeys, ScaleKeys, false);
	}

	Animation->MarkPackageDirty();
	return true;
}

bool UAnimationLibrary::SaveAnimationAsAsset(
	UAnimSequence* Animation,
	const FString& PackagePath,
	const FString& AssetName)
{
	if (!Animation || PackagePath.IsEmpty() || AssetName.IsEmpty())
	{
		return false;
	}

	const FString FullPath = PackagePath + TEXT("/") + AssetName;
	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		return false;
	}

	Animation->Rename(*AssetName, Package);
	Animation->SetFlags(RF_Public | RF_Standalone);
	Animation->MarkPackageDirty();

	return true;
}

UAnimSequence* UAnimationLibrary::DuplicateAnimation(UAnimSequence* SourceAnimation)
{
	if (!SourceAnimation)
	{
		return nullptr;
	}

	const IAnimationDataModel* SourceModel = SourceAnimation->GetDataModel();
	if (!SourceModel)
	{
		const FString NewName = SourceAnimation->GetName() + TEXT("_Copy");
		return CreateAnimation(nullptr, NewName, SourceAnimation->GetSkeleton(),
			TArray<FAnimationTrackData>(), SourceAnimation->GetPlayLength(), 30.0f);
	}

	TArray<FAnimationTrackData> CopyTracks;

	for (const FBoneAnimationTrack& SrcTrack : SourceModel->GetBoneAnimationTracks())
	{
		FAnimationTrackData TrackData;
		TrackData.TrackName = SrcTrack.Name;

		const FRawAnimSequenceTrack& Raw = SrcTrack.InternalTrackData;
		const int32 NumPosKeys = Raw.PosKeys.Num();
		const int32 NumRotKeys = Raw.RotKeys.Num();
		const int32 NumScaleKeys = Raw.ScaleKeys.Num();
		const int32 NumFrames = FMath::Max3(NumPosKeys, NumRotKeys, NumScaleKeys);

		if (NumFrames == 0) continue;

		const float Duration = SourceModel->GetPlayLength();
		const float SafeDuration = FMath::Max(Duration, 0.01f);
		const float SampleRate = SourceModel->GetFrameRate().AsDecimal();
		const float SafeSampleRate = FMath::Max(SampleRate, 1.0f);

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = (NumFrames > 1)
				? (static_cast<float>(Frame) / (NumFrames - 1)) * SafeDuration
				: 0.0f;

			FVector Pos = (Frame < NumPosKeys) ? FVector(Raw.PosKeys[Frame]) : FVector::ZeroVector;
			FQuat Rot = (Frame < NumRotKeys) ? FQuat(Raw.RotKeys[Frame]) : FQuat::Identity;
			FVector Scale = (Frame < NumScaleKeys) ? FVector(Raw.ScaleKeys[Frame]) : FVector::OneVector;

			FAnimationKeyframe KF;
			KF.Time = Time;
			KF.Transform = FTransform(Rot, Pos, Scale);
			TrackData.Keyframes.Add(KF);
		}

		CopyTracks.Add(TrackData);
	}

	const FString NewName = SourceAnimation->GetName() + TEXT("_Copy");
	return CreateAnimation(nullptr, NewName, SourceAnimation->GetSkeleton(),
		CopyTracks, SourceModel->GetPlayLength(), SourceModel->GetFrameRate().AsDecimal());
}

bool UAnimationLibrary::IsAnimationCompatibleWithSkeleton(
	UAnimSequence* Animation,
	USkeleton* Skeleton)
{
	if (!Animation || !Skeleton)
	{
		return false;
	}

	return Animation->GetSkeleton() == Skeleton;
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

	return SkelMesh->GetSocketTransform(BoneName);
}

TArray<FName> UAnimationLibrary::GetAnimationBoneNames(
	UAnimSequence* Animation)
{
	TArray<FName> Result;

	if (!Animation)
	{
		return Result;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();

	for (int32 i = 0; i < NumBones; ++i)
	{
		Result.Add(RefSkeleton.GetBoneName(i));
	}

	return Result;
}

float UAnimationLibrary::GetAnimationDuration(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return Animation->GetPlayLength();
}

int32 UAnimationLibrary::GetAnimationFrameCount(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0;
	}

	return Animation->GetNumberOfSampledKeys();
}

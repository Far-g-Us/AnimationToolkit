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
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

static TMap<TWeakObjectPtr<UObject>, TArray<FName>> AnimationTagsMap;

UAnimSequence* UAnimationLibrary::CreateAnimation(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	float Duration,
	float FrameRate)
{
	return nullptr;
}

UAnimMontage* UAnimationLibrary::CreateMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections)
{
	return nullptr;
}

UBlendSpace* UAnimationLibrary::CreateBlendSpace1D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinValue,
	float MaxValue)
{
	return nullptr;
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
	return nullptr;
}

UAnimSequence* UAnimationLibrary::ImportAnimationAsSequence(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	return nullptr;
}

FAnimationImportResult UAnimationLibrary::ImportAnimationFile(
	UObject* WorldContextObject,
	const FString& FilePath,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	FAnimationImportResult Result;
	return Result;
}

FAnimationTrackData UAnimationLibrary::CreateTrack(
	const FName& BoneName,
	const TArray<FTransform>& Transforms,
	float FrameRate)
{
	FAnimationTrackData TrackData;
	return TrackData;
}

FAnimationKeyframe UAnimationLibrary::MakeKeyframe(
	float Time,
	const FTransform& Transform)
{
	FAnimationKeyframe Keyframe;
	return Keyframe;
}

FAnimMontageSectionData UAnimationLibrary::MakeMontageSection(
	const FName& SectionName,
	UAnimSequence* Animation,
	float StartTime,
	float EndTime)
{
	FAnimMontageSectionData SectionData;
	return SectionData;
}

FBlendSpaceSampleData UAnimationLibrary::MakeBlendSpaceSample(
	UAnimSequence* Animation,
	const FVector& SampleValue)
{
	FBlendSpaceSampleData SampleData;
	return SampleData;
}

EAnimationImportFormat UAnimationLibrary::DetectAnimationFormat(const FString& FilePath)
{
	return EAnimationImportFormat::FBX;
}

bool UAnimationLibrary::IsAnimationFormatSupported(const FString& FilePath)
{
	return false;
}

FString UAnimationLibrary::GetAnimationFormatName(EAnimationImportFormat Format)
{
	return FString();
}

TArray<EAnimationImportFormat> UAnimationLibrary::GetAllSupportedFormats()
{
	TArray<EAnimationImportFormat> Formats;
	return Formats;
}

TArray<FString> UAnimationLibrary::GetSkeletonBoneNames(USkeleton* Skeleton)
{
	TArray<FString> BoneNames;
	return BoneNames;
}

float UAnimationLibrary::GetAnimSequenceDuration(UAnimSequence* AnimSequence)
{
	return 0.0f;
}

int32 UAnimationLibrary::GetAnimSequenceFrameCount(UAnimSequence* AnimSequence)
{
	return 0;
}

FATK_AnimationCurveData UAnimationLibrary::CreateCurveData(
	const FName& CurveName,
	const TArray<FAnimationCurveKeyframe>& Keyframes)
{
	FATK_AnimationCurveData CurveData;
	return CurveData;
}

FAnimationCurveKeyframe UAnimationLibrary::MakeCurveKeyframe(float Time, float Value)
{
	FAnimationCurveKeyframe Keyframe;
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
	return nullptr;
}

bool UAnimationLibrary::AddCurveToAnimSequence(UAnimSequence* AnimSequence, const FATK_AnimationCurveData& CurveData)
{
	return false;
}

FAnimationBatchImportResult UAnimationLibrary::BatchImportAnimations(
	UObject* WorldContextObject,
	const TArray<FAnimationBatchImportItem>& Items,
	USkeleton* Skeleton)
{
	FAnimationBatchImportResult BatchResult;
	return BatchResult;
}

TArray<UAnimSequence*> UAnimationLibrary::BatchImportAsSequences(
	UObject* WorldContextObject,
	const TArray<FAnimationBatchImportItem>& Items,
	USkeleton* Skeleton)
{
	TArray<UAnimSequence*> Sequences;
	return Sequences;
}

FAnimationRetargetMapping UAnimationLibrary::MakeRetargetMapping(
	const FName& SourceBone,
	const FName& TargetBone)
{
	FAnimationRetargetMapping Mapping;
	return Mapping;
}

FAnimationRetargetSettings UAnimationLibrary::MakeRetargetSettings(
	EAnimationRetargetType RetargetType,
	const TArray<FAnimationRetargetMapping>& CustomMappings)
{
	FAnimationRetargetSettings Settings;
	return Settings;
}

UAnimSequence* UAnimationLibrary::RetargetAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	USkeleton* TargetSkeleton,
	const FAnimationRetargetSettings& Settings)
{
	return nullptr;
}

TArray<UAnimSequence*> UAnimationLibrary::BatchRetargetAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& SourceAnimations,
	USkeleton* TargetSkeleton,
	const FAnimationRetargetSettings& Settings)
{
	TArray<UAnimSequence*> RetargetedAnimations;
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
	return Settings;
}

bool UAnimationLibrary::CompressAnimation(
	UAnimSequence* AnimSequence,
	const FAnimationCompressionSettings& Settings)
{
	return false;
}

int32 UAnimationLibrary::BatchCompressAnimations(
	const TArray<UAnimSequence*>& AnimSequences,
	const FAnimationCompressionSettings& Settings)
{
	return 0;
}

float UAnimationLibrary::GetAnimationCompressionRatio(UAnimSequence* AnimSequence)
{
	return 1.0f;
}

int32 UAnimationLibrary::GetAnimationKeyCount(UAnimSequence* AnimSequence)
{
	return 0;
}

UAsyncImportHelper* UAnimationLibrary::StartAsyncImport(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	return nullptr;
}

UAnimSequence* UAnimationLibrary::BlendAnimations(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<UAnimSequence*>& Animations,
	const TArray<float>& Weights)
{
	return nullptr;
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
	return nullptr;
}

UAnimSequence* UAnimationLibrary::MirrorAnimation(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* SourceAnimation,
	EMirrorFlags MirrorFlags)
{
	return nullptr;
}

UAnimSequence* UAnimationLibrary::MirrorAnimationXY(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* SourceAnimation,
	bool bMirrorX,
	bool bMirrorY)
{
	return nullptr;
}

bool UAnimationLibrary::ExportAnimationFBX(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	return false;
}

bool UAnimationLibrary::ExportAnimationBVH(UAnimSequence* AnimSequence, const FString& OutputPath, const FString& CharacterName)
{
	return false;
}

bool UAnimationLibrary::ExportAnimationglTF(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	return false;
}

bool UAnimationLibrary::ExportAnimationCollada(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	return false;
}

bool UAnimationLibrary::ExportAnimation(UAnimSequence* AnimSequence, const FString& OutputPath, EAnimationImportFormat Format)
{
	return false;
}

void UAnimationLibrary::SetAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
}

void UAnimationLibrary::SetAnimationTags(UAnimSequence* AnimSequence, const TArray<FName>& Tags)
{
}

void UAnimationLibrary::RemoveAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
}

bool UAnimationLibrary::HasAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	return false;
}

TArray<FName> UAnimationLibrary::GetAnimationTags(UAnimSequence* AnimSequence)
{
	TArray<FName> Tags;
	return Tags;
}

TArray<UAnimSequence*> UAnimationLibrary::FindAnimationsByTag(const TArray<UAnimSequence*>& Animations, const FName& Tag)
{
	TArray<UAnimSequence*> Result;
	return Result;
}

void UAnimationLibrary::ClearAnimationTags(UAnimSequence* AnimSequence)
{
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
	return nullptr;
}

bool UAnimationLibrary::AddBoneKeyframe(
	UAnimSequence* Animation,
	FName BoneName,
	float Time,
	FTransform Transform,
	bool bUpdateExistingKey)
{
	return false;
}

int32 UAnimationLibrary::AddKeyframesBatch(
	UAnimSequence* Animation,
	const TArray<FAnimationKeyframe>& Keyframes)
{
	return 0;
}

bool UAnimationLibrary::RemoveBoneKeyframe(
	UAnimSequence* Animation,
	FName BoneName,
	float Time)
{
	return false;
}

FTransform UAnimationLibrary::GetBoneTransformAtTime(
	UAnimSequence* Animation,
	FName BoneName,
	float Time,
	bool bExtractRootMotion)
{
	return FTransform::Identity;
}

UAnimSequence* UAnimationLibrary::RetargetSpeed(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float SpeedMultiplier)
{
	return nullptr;
}

UAnimSequence* UAnimationLibrary::CropAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float StartTime,
	float EndTime)
{
	return nullptr;
}

bool UAnimationLibrary::ApplyRootOffset(
	UAnimSequence* Animation,
	FVector LocationOffset,
	FRotator RotationOffset)
{
	return false;
}

bool UAnimationLibrary::SaveAnimationAsAsset(
	UAnimSequence* Animation,
	const FString& PackagePath,
	const FString& AssetName)
{
	return false;
}

UAnimSequence* UAnimationLibrary::DuplicateAnimation(
	UAnimSequence* SourceAnimation)
{
	return nullptr;
}

bool UAnimationLibrary::IsAnimationCompatibleWithSkeleton(
	UAnimSequence* Animation,
	USkeleton* Skeleton)
{
	return false;
}

FTransform UAnimationLibrary::GetRuntimeBoneTransform(
	USkeletalMeshComponent* SkelMesh,
	FName BoneName,
	EBoneSpaces::Type Space)
{
	return FTransform::Identity;
}

TArray<FName> UAnimationLibrary::GetAnimationBoneNames(
	UAnimSequence* Animation)
{
	TArray<FName> Result;
	return Result;
}

float UAnimationLibrary::GetAnimationDuration(
	UAnimSequence* Animation)
{
	return 0.0f;
}

int32 UAnimationLibrary::GetAnimationFrameCount(
	UAnimSequence* Animation)
{
	return 0;
}

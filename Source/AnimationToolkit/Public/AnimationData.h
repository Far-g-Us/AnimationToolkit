#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "AnimationData.generated.h"

UENUM(BlueprintType)
enum class EAnimationImportFormat : uint8
{
	FBX,
	glTF,
	BVH
};

USTRUCT(BlueprintType)
struct FAnimationBoneData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	int32 ParentIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FTransform LocalTransform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct FAnimationKeyframe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FTransform Transform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct FAnimationTrackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName TrackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<FAnimationKeyframe> Keyframes;
};

USTRUCT(BlueprintType)
struct FAnimationClipData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString ClipName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Duration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float FrameRate = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<FAnimationTrackData> Tracks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<FAnimationBoneData> Skeleton;
};

USTRUCT(BlueprintType)
struct FAnimationImportResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FAnimationClipData ClipData;
};

USTRUCT(BlueprintType)
struct FAnimMontageSectionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName SectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimSequence* Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float EndTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FBlendSpaceSampleData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimSequence* Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FVector SampleValue = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FAnimationCurveKeyframe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Curve")
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Curve")
	float Value = 0.0f;
};

USTRUCT(BlueprintType)
struct FAnimationCurveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Curve")
	FName CurveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Curve")
	TArray<FAnimationCurveKeyframe> Keyframes;
};

USTRUCT(BlueprintType)
struct FAnimationBatchImportItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Batch")
	FString FilePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Batch")
	FString AnimationName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Batch")
	EAnimationImportFormat Format = EAnimationImportFormat::FBX;
};

USTRUCT(BlueprintType)
struct FAnimationBatchImportResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Batch")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Batch")
	TArray<FAnimationImportResult> Results;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Batch")
	int32 SuccessCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Batch")
	int32 FailCount = 0;
};

UENUM(BlueprintType)
enum class EAnimationRetargetType : uint8
{
	OneToOne,
	ByName,
	ByIndex
};

USTRUCT(BlueprintType)
struct FAnimationRetargetMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Retarget")
	FName SourceBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Retarget")
	FName TargetBone;
};

USTRUCT(BlueprintType)
struct FAnimationRetargetSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Retarget")
	EAnimationRetargetType RetargetType = EAnimationRetargetType::ByName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Retarget")
	TArray<FAnimationRetargetMapping> CustomMappings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Retarget")
	bool bMatchByHierarchy = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Retarget")
	bool bAdjustRootMotion = true;
};

USTRUCT(BlueprintType)
struct FAnimationCompressionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Compression")
	bool bRemoveLinearKeys = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Compression")
	bool bCompressTranslation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Compression")
	bool bCompressRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Compression")
	bool bCompressScale = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Compression")
	float TranslationThreshold = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Compression")
	float RotationThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Compression")
	float ScaleThreshold = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Compression")
	int32 MaxKeyReduction = 4;
};

UENUM(BlueprintType)
enum class EMirrorFlags : uint8
{
	None,
	X,
	Y,
	Z,
	XY,
	XZ,
	YZ,
	XYZ
};

USTRUCT(BlueprintType)
struct FAnimationExportSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Export")
	EAnimationImportFormat Format = EAnimationImportFormat::FBX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Export")
	bool bExportMetadata = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Export")
	bool bExportCurves = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Export")
	float FrameRate = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Export")
	FString CharacterName = TEXT("Character");
};

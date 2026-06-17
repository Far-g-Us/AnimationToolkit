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
	BVH,
	COLLADA
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
struct FATK_AnimationCurveData
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

inline EMirrorFlags operator|(EMirrorFlags A, EMirrorFlags B)
{
	return static_cast<EMirrorFlags>(static_cast<uint8>(A) | static_cast<uint8>(B));
}

inline EMirrorFlags operator&(EMirrorFlags A, EMirrorFlags B)
{
	return static_cast<EMirrorFlags>(static_cast<uint8>(A) & static_cast<uint8>(B));
}

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

UENUM(BlueprintType)
enum class EEngineVersion : uint8
{
	UE4_0,
	UE4_1,
	UE4_2,
	UE4_3,
	UE4_4,
	UE4_5,
	UE4_6,
	UE4_7,
	UE4_8,
	UE4_9,
	UE4_10,
	UE4_11,
	UE4_12,
	UE4_13,
	UE4_14,
	UE4_15,
	UE4_16,
	UE4_17,
	UE4_18,
	UE4_19,
	UE4_20,
	UE4_21,
	UE4_22,
	UE4_23,
	UE4_24,
	UE4_25,
	UE4_26,
	UE4_27,
	UE5_0,
	UE5_1,
	UE5_2,
	UE5_3,
	UE5_4,
	UE5_5,
	UE5_6,
	UE5_7,
	UE5_8,
	Custom
};

USTRUCT(BlueprintType)
struct FEngineVersionProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Version")
	EEngineVersion SourceVersion = EEngineVersion::UE4_27;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Version")
	EEngineVersion TargetVersion = EEngineVersion::UE5_5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Version")
	float ScaleMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Version")
	bool bConvertBoneNames = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Version")
	bool bFixRootMotion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Version")
	bool bNormalizeRotations = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Version")
	TMap<FName, FName> CustomBoneRenames;
};

USTRUCT(BlueprintType)
struct FAnimationRepairResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Repair")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Repair")
	int32 BonesRepaired = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Repair")
	int32 BonesStillMissing = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Repair")
	TArray<FName> RepairedBones;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Repair")
	TArray<FName> StillMissingBones;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Repair")
	FString Message;
};

USTRUCT(BlueprintType)
struct FAnimationConversionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Convert")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Convert")
	int32 AnimationsConverted = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Convert")
	int32 BonesRenamed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Convert")
	TArray<FString> Warnings;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Convert")
	FString Message;
};

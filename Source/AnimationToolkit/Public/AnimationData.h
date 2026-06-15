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

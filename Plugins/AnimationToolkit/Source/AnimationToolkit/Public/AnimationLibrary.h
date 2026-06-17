#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AnimationData.h"
#include "AnimationLibrary.generated.h"

UCLASS()
class ANIMATIONTOOLKIT_API UAnimationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Create", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CreateAnimation(
		UObject* WorldContextObject,
		const FString& AnimationName,
		USkeleton* Skeleton,
		const TArray<FAnimationTrackData>& Tracks,
		float Duration,
		float FrameRate
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Create", meta = (WorldContext = "WorldContextObject"))
	static UAnimMontage* CreateMontage(
		UObject* WorldContextObject,
		const FString& MontageName,
		USkeleton* Skeleton,
		const TArray<FAnimMontageSectionData>& Sections
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Create", meta = (WorldContext = "WorldContextObject"))
	static UBlendSpace* CreateBlendSpace1D(
		UObject* WorldContextObject,
		const FString& BlendSpaceName,
		USkeleton* Skeleton,
		const TArray<FBlendSpaceSampleData>& Samples,
		float MinValue,
		float MaxValue
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Create", meta = (WorldContext = "WorldContextObject"))
	static UBlendSpace* CreateBlendSpace2D(
		UObject* WorldContextObject,
		const FString& BlendSpaceName,
		USkeleton* Skeleton,
		const TArray<FBlendSpaceSampleData>& Samples,
		float MinX,
		float MaxX,
		float MinY,
		float MaxY
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Import", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ImportAnimationAsSequence(
		UObject* WorldContextObject,
		const FString& FilePath,
		const FString& AnimationName,
		USkeleton* Skeleton,
		EAnimationImportFormat Format
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Import", meta = (WorldContext = "WorldContextObject"))
	static FAnimationImportResult ImportAnimationFile(
		UObject* WorldContextObject,
		const FString& FilePath,
		USkeleton* Skeleton,
		EAnimationImportFormat Format
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static FAnimationTrackData CreateTrack(
		const FName& BoneName,
		const TArray<FTransform>& Transforms,
		float FrameRate
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static FAnimationKeyframe MakeKeyframe(
		float Time,
		const FTransform& Transform
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static FAnimMontageSectionData MakeMontageSection(
		const FName& SectionName,
		UAnimSequence* Animation,
		float StartTime,
		float EndTime
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static FBlendSpaceSampleData MakeBlendSpaceSample(
		UAnimSequence* Animation,
		const FVector& SampleValue
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static EAnimationImportFormat DetectAnimationFormat(const FString& FilePath);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static bool IsAnimationFormatSupported(const FString& FilePath);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static FString GetAnimationFormatName(EAnimationImportFormat Format);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static TArray<EAnimationImportFormat> GetAllSupportedFormats();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Utility")
	static TArray<FString> GetSkeletonBoneNames(USkeleton* Skeleton);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static float GetAnimSequenceDuration(UAnimSequence* AnimSequence);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Utility")
	static int32 GetAnimSequenceFrameCount(UAnimSequence* AnimSequence);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curve")
	static FATK_AnimationCurveData CreateCurveData(
		const FName& CurveName,
		const TArray<FAnimationCurveKeyframe>& Keyframes
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curve")
	static FAnimationCurveKeyframe MakeCurveKeyframe(
		float Time,
		float Value
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curve", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CreateAnimationWithCurves(
		UObject* WorldContextObject,
		const FString& AnimationName,
		USkeleton* Skeleton,
		const TArray<FAnimationTrackData>& Tracks,
		const TArray<FATK_AnimationCurveData>& Curves,
		float Duration,
		float FrameRate
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curve")
	static bool AddCurveToAnimSequence(
		UAnimSequence* AnimSequence,
		const FATK_AnimationCurveData& CurveData
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Batch", meta = (WorldContext = "WorldContextObject"))
	static FAnimationBatchImportResult BatchImportAnimations(
		UObject* WorldContextObject,
		const TArray<FAnimationBatchImportItem>& Items,
		USkeleton* Skeleton
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Batch", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchImportAsSequences(
		UObject* WorldContextObject,
		const TArray<FAnimationBatchImportItem>& Items,
		USkeleton* Skeleton
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Retarget")
	static FAnimationRetargetMapping MakeRetargetMapping(
		const FName& SourceBone,
		const FName& TargetBone
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Retarget")
	static FAnimationRetargetSettings MakeRetargetSettings(
		EAnimationRetargetType RetargetType,
		const TArray<FAnimationRetargetMapping>& CustomMappings
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Retarget", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* RetargetAnimation(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		USkeleton* TargetSkeleton,
		const FAnimationRetargetSettings& Settings
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Retarget", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchRetargetAnimations(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& SourceAnimations,
		USkeleton* TargetSkeleton,
		const FAnimationRetargetSettings& Settings
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Compression")
	static FAnimationCompressionSettings MakeCompressionSettings(
		bool bCompressTranslation,
		bool bCompressRotation,
		bool bCompressScale,
		float TranslationThreshold,
		float RotationThreshold,
		float ScaleThreshold
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Compression")
	static bool CompressAnimation(
		UAnimSequence* AnimSequence,
		const FAnimationCompressionSettings& Settings
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Compression")
	static int32 BatchCompressAnimations(
		const TArray<UAnimSequence*>& AnimSequences,
		const FAnimationCompressionSettings& Settings
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Compression")
	static float GetAnimationCompressionRatio(UAnimSequence* AnimSequence);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Compression")
	static int32 GetAnimationKeyCount(UAnimSequence* AnimSequence);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Async", meta = (WorldContext = "WorldContextObject"))
	static UAsyncImportHelper* StartAsyncImport(
		UObject* WorldContextObject,
		const FString& FilePath,
		const FString& AnimationName,
		USkeleton* Skeleton,
		EAnimationImportFormat Format
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Blending", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* BlendAnimations(
		UObject* WorldContextObject,
		const FString& AnimationName,
		USkeleton* Skeleton,
		const TArray<UAnimSequence*>& Animations,
		const TArray<float>& Weights
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Blending", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CrossfadeAnimations(
		UObject* WorldContextObject,
		const FString& AnimationName,
		USkeleton* Skeleton,
		UAnimSequence* From,
		UAnimSequence* To,
		float CrossfadeDuration,
		float FromEndTime = -1.0f,
		float ToStartTime = 0.0f
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Mirror", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* MirrorAnimation(
		UObject* WorldContextObject,
		const FString& AnimationName,
		USkeleton* Skeleton,
		UAnimSequence* SourceAnimation,
		EMirrorFlags MirrorFlags
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Mirror", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* MirrorAnimationXY(
		UObject* WorldContextObject,
		const FString& AnimationName,
		USkeleton* Skeleton,
		UAnimSequence* SourceAnimation,
		bool bMirrorX,
		bool bMirrorY
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Export")
	static bool ExportAnimationFBX(
		UAnimSequence* AnimSequence,
		const FString& OutputPath
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Export")
	static bool ExportAnimationBVH(
		UAnimSequence* AnimSequence,
		const FString& OutputPath,
		const FString& CharacterName = TEXT("Character")
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Export")
	static bool ExportAnimationglTF(
		UAnimSequence* AnimSequence,
		const FString& OutputPath
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Export")
	static bool ExportAnimationCollada(
		UAnimSequence* AnimSequence,
		const FString& OutputPath
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Export")
	static bool ExportAnimation(
		UAnimSequence* AnimSequence,
		const FString& OutputPath,
		EAnimationImportFormat Format
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Tags")
	static void SetAnimationTag(
		UAnimSequence* AnimSequence,
		const FName& Tag
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Tags")
	static void SetAnimationTags(
		UAnimSequence* AnimSequence,
		const TArray<FName>& Tags
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Tags")
	static void RemoveAnimationTag(
		UAnimSequence* AnimSequence,
		const FName& Tag
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Tags")
	static bool HasAnimationTag(
		UAnimSequence* AnimSequence,
		const FName& Tag
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Tags")
	static TArray<FName> GetAnimationTags(
		UAnimSequence* AnimSequence
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Tags")
	static TArray<UAnimSequence*> FindAnimationsByTag(
		const TArray<UAnimSequence*>& Animations,
		const FName& Tag
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Tags")
	static void ClearAnimationTags(
		UAnimSequence* AnimSequence
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Progress")
	static void ClearGlobalProgressCallback();

	// ─── GUIDE API ─────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CreateEmptyAnimation(
		UObject* WorldContextObject,
		USkeleton* Skeleton,
		float Duration,
		int32 FrameRate = 30
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide")
	static bool AddBoneKeyframe(
		UAnimSequence* Animation,
		FName BoneName,
		float Time,
		FTransform Transform,
		bool bUpdateExistingKey = true
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide")
	static int32 AddKeyframesBatch(
		UAnimSequence* Animation,
		const TArray<FAnimationKeyframe>& Keyframes
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide")
	static bool RemoveBoneKeyframe(
		UAnimSequence* Animation,
		FName BoneName,
		float Time
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide")
	static FTransform GetBoneTransformAtTime(
		UAnimSequence* Animation,
		FName BoneName,
		float Time,
		bool bExtractRootMotion = false
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* RetargetSpeed(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		float SpeedMultiplier
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CropAnimation(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		float StartTime,
		float EndTime
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide", meta = (WorldContext = "WorldContextObject"))
	static bool ApplyRootOffset(
		UAnimSequence* Animation,
		FVector LocationOffset,
		FRotator RotationOffset
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide")
	static bool SaveAnimationAsAsset(
		UAnimSequence* Animation,
		const FString& PackagePath,
		const FString& AssetName
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Guide")
	static UAnimSequence* DuplicateAnimation(
		UAnimSequence* SourceAnimation
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Guide")
	static bool IsAnimationCompatibleWithSkeleton(
		UAnimSequence* Animation,
		USkeleton* Skeleton
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Guide")
	static FTransform GetRuntimeBoneTransform(
		USkeletalMeshComponent* SkelMesh,
		FName BoneName,
		EBoneSpaces::Type Space = EBoneSpaces::WorldSpace
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Guide")
	static TArray<FName> GetAnimationBoneNames(
		UAnimSequence* Animation
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Guide")
	static float GetAnimationDuration(
		UAnimSequence* Animation
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Guide")
	static int32 GetAnimationFrameCount(
		UAnimSequence* Animation
	);
};

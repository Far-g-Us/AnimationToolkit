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
};

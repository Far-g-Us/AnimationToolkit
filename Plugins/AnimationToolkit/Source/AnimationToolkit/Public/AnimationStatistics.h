#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationStatistics.generated.h"

USTRUCT(BlueprintType)
struct FAnimationStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 BoneCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 FrameCount = 0;

	UPROPERTY(BlueprintReadOnly)
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalKeyframes = 0;

	UPROPERTY(BlueprintReadOnly)
	float MemoryUsageKB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 TranslationKeys = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 RotationKeys = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ScaleKeys = 0;

	UPROPERTY(BlueprintReadOnly)
	FVector AveragePosition = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector PositionRange = FVector::ZeroVector;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationStatistics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Statistics")
	static FAnimationStats GetAnimationStatistics(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Statistics")
	static float EstimateMemoryUsage(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Statistics")
	static int32 GetTotalKeyframes(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Statistics")
	static FVector GetPositionRange(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Statistics")
	static FRotator GetRotationRange(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Statistics")
	static FString GenerateReport(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Statistics")
	static TArray<FAnimationStats> BatchGetStatistics(
		const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Statistics")
	static float CompareStatistics(
		const FAnimationStats& StatsA,
		const FAnimationStats& StatsB);
};

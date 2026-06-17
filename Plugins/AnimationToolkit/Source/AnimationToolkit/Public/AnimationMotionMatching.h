#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationMotionMatching.generated.h"

USTRUCT(BlueprintType)
struct FMotionMatchingPose
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PoseIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationMotionMatching : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|MotionMatching")
	static int32 FindBestMatchingPose(
		const TArray<UAnimSequence*>& Animations,
		const FVector& CurrentPosition,
		const FRotator& CurrentRotation,
		const FVector& DesiredVelocity);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|MotionMatching")
	static float CalculatePoseCost(
		const FMotionMatchingPose& PoseA,
		const FMotionMatchingPose& PoseB);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|MotionMatching")
	static TArray<FMotionMatchingPose> ExtractPoses(
		UAnimSequence* Animation,
		float SampleRate = 10.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|MotionMatching")
	static void BuildPoseDatabase(
		const TArray<UAnimSequence*>& Animations,
		TArray<FMotionMatchingPose>& OutPoseDatabase);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|MotionMatching")
	static int32 FindBestMatchingPoseInDatabase(
		const TArray<FMotionMatchingPose>& PoseDatabase,
		const FVector& CurrentPosition,
		const FRotator& CurrentRotation,
		const FVector& DesiredVelocity);
};

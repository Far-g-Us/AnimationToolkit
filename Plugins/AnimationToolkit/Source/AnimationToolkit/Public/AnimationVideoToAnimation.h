#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformProcess.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "AnimationVideoToAnimation.generated.h"

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationVideoToAnimation : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|Video")
	static UAnimSequence* ExtractMotionFromVideo(
		const FString& VideoPath,
		const TArray<FString>& BoneNames,
		float SampleRate = 30.0f
	);

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|Video")
	static float DetectVideoFPS(const FString& VideoPath);

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|Video")
	static int32 GetVideoFrameCount(const FString& VideoPath);

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|Video")
	static UAnimSequence* ExtractMotionFromImageSequence(
		const FString& ImageDir,
		const FString& Pattern,
		float FPS
	);

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|Video")
	static FVector DetectMotion(const FString& Image1Path, const FString& Image2Path);

private:

	static bool RunFFmpegCommand(const FString& Arguments, FString& OutResult);

	static bool RunFFprobeCommand(const FString& Arguments, FString& OutResult);

	static bool ExtractFramesFromVideo(const FString& VideoPath, const FString& OutputDir, float SampleRate);

	static TArray<uint8> ReadImageAsGrayscale(const FString& ImagePath, int32& OutWidth, int32& OutHeight);

	static void ComputeBlockMotion(
		const TArray<uint8>& PrevFrame,
		const TArray<uint8>& CurrFrame,
		int32 Width,
		int32 Height,
		int32 BlockSize,
		FVector& OutMotion
	);

	static void PopulateAnimSequence(
		UAnimSequence* AnimSequence,
		const TArray<FString>& BoneNames,
		const TArray<FVector>& MotionData,
		float SampleRate,
		int32 FrameCount
	);
};

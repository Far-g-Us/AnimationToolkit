#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/Skeleton.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformProcess.h"
#include "AnimationUAFPreparation.generated.h"

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationUAFPreparation : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|UAF")
	bool PrepareForUAF(UAnimSequence* Animation, const FString& OutputPath);

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|UAF")
	bool ValidateForUAF(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|UAF")
	int32 BatchPrepareForUAF(const TArray<UAnimSequence*>& Animations, const FString& OutputDir);

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|UAF")
	TArray<FName> GetUAFBoneHierarchy();

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|UAF")
	bool AddMotionMatchingTags(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|UAF")
	bool SetSamplingRate(UAnimSequence* Animation, float TargetRate);

	UFUNCTION(BlueprintCallable, Category = "AnimationToolkit|UAF")
	bool ValidateBoneHierarchy(UAnimSequence* Animation);
};

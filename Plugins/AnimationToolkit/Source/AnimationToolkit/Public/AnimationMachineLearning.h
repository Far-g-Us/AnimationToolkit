#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationMachineLearning.generated.h"

USTRUCT(BlueprintType)
struct FMLTrainingData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> InputFeatures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> OutputTargets;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationMachineLearning : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|ML")
	static void TrainOptimizationModel(const TArray<UAnimSequence*>& TrainingAnimations);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|ML")
	static TArray<float> ExtractAnimationFeatures(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|ML")
	static float PredictOptimalCompression(const TArray<float>& Features);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|ML")
	static void SaveModel(const FString& ModelPath);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|ML")
	static void LoadModel(const FString& ModelPath);
};

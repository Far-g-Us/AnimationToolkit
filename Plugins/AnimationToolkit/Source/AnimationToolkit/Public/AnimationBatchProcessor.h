#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationBatchProcessor.generated.h"

UENUM(BlueprintType)
enum class EBatchOperation : uint8
{
	Compress,
	Optimize,
	Retarget,
	Mirror,
	Reverse,
	ChangePlayRate
};

USTRUCT(BlueprintType)
struct FBatchProcessSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBatchOperation Operation = EBatchOperation::Compress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Parameter = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeleton* TargetSkeleton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMirrorFlags MirrorFlags = EMirrorFlags::None;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationBatchProcessor : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Batch", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchProcessAnimations(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations,
		const FBatchProcessSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Batch", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchCompress(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Batch", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchOptimize(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Batch", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchRetarget(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations,
		USkeleton* TargetSkeleton);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Batch", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchMirror(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations,
		EMirrorFlags MirrorFlags);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Batch", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchReverse(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Batch", meta = (WorldContext = "WorldContextObject"))
	static TArray<UAnimSequence*> BatchChangePlayRate(
		UObject* WorldContextObject,
		const TArray<UAnimSequence*>& Animations,
		float PlayRate);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Batch")
	static int32 GetBatchProgress();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Batch")
	static FString GetBatchStatus();
};

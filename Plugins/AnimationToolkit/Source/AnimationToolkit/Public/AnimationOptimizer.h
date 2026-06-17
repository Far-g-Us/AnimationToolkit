#pragma once

#include "CoreMinimal.h"
#include "AnimationData.h"
#include "UObject/NoExportTypes.h"
#include "AnimationOptimizer.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimationOptimizer : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Optimizer")
	static FAnimationClipData SimplifyCurves(
		const FAnimationClipData& ClipData,
		float Tolerance = 0.01f
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Optimizer")
	static FAnimationClipData ReduceKeys(
		const FAnimationClipData& ClipData,
		int32 MaxKeysPerTrack = 100,
		float Threshold = 0.01f
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Optimizer")
	static FAnimationClipData ApplyLOD(
		const FAnimationClipData& ClipData,
		int32 LODLevel = 0
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Optimizer")
	static FAnimationClipData OptimizeAll(
		const FAnimationClipData& ClipData,
		float CurveTolerance = 0.01f,
		int32 MaxKeysPerTrack = 100,
		int32 LODLevel = 0
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Optimizer")
	static float CalculateOptimalTolerance(const FAnimationClipData& ClipData);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Optimizer")
	static int32 EstimateOptimalKeyCount(const FAnimationClipData& ClipData, int32 LODLevel = 0);

private:
	static FVector SimplifyVectorCurve(
		const TArray<FVector>& Keys,
		float Tolerance
	);

	static FQuat SimplifyRotationCurve(
		const TArray<FQuat>& Keys,
		float Tolerance
	);

	static bool ShouldRemoveKey(
		const FTransform& Current,
		const FTransform& Next,
		float Tolerance
	);
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationObjectPool : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Pool")
	static UAnimationObjectPool* GetPool();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Pool")
	UObject* Acquire(UClass* ObjectClass);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Pool")
	void Release(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Pool")
	void Clear();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Pool")
	int32 GetPoolSize() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Pool")
	int32 GetActiveCount() const;

private:
	UPROPERTY()
	TArray<UObject*> AvailableObjects;

	UPROPERTY()
	TArray<UObject*> ActiveObjects;

	static TWeakObjectPtr<UAnimationObjectPool> Instance;
};

USTRUCT(BlueprintType)
struct FAnimationLODDistanceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|LOD")
	float Distance0 = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|LOD")
	float Distance1 = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|LOD")
	float Distance2 = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|LOD")
	int32 MaxKeysLOD0 = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|LOD")
	int32 MaxKeysLOD1 = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|LOD")
	int32 MaxKeysLOD2 = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|LOD")
	int32 MaxKeysLOD3 = 10;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimationLODManager : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LOD")
	static UAnimationLODManager* GetLODManager();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LOD")
	void UpdateLOD(AActor* Actor, const FAnimationLODDistanceSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LOD")
	int32 GetCurrentLOD(AActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LOD")
	FAnimationLODDistanceSettings GetSettingsForLOD(int32 LODLevel) const;

private:
	UPROPERTY()
	TMap<AActor*, int32> ActorLODs;

	UPROPERTY()
	FAnimationLODDistanceSettings DefaultSettings;

	static TWeakObjectPtr<UAnimationLODManager> Instance;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimationLazyLoader : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LazyLoader")
	static UAnimationLazyLoader* GetLazyLoader();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LazyLoader")
	void RegisterAnimation(const FString& FilePath, const FName& AnimationName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LazyLoader")
	void UnregisterAnimation(const FName& AnimationName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LazyLoader")
	UAnimSequence* LoadAnimation(const FName& AnimationName, USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LazyLoader")
	void UnloadAnimation(const FName& AnimationName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|LazyLoader")
	void UnloadAll();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LazyLoader")
	bool IsAnimationLoaded(const FName& AnimationName) const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LazyLoader")
	int32 GetRegisteredCount() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LazyLoader")
	int32 GetLoadedCount() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LazyLoader")
	TArray<FName> GetLoadedAnimations() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|LazyLoader")
	TArray<FName> GetRegisteredAnimations() const;

private:
	UPROPERTY()
	TMap<FName, FString> RegisteredAnimations;

	UPROPERTY()
	TMap<FName, UAnimSequence*> LoadedAnimations;

	static TWeakObjectPtr<UAnimationLazyLoader> Instance;
};

USTRUCT(BlueprintType)
struct FAnimationKeyframeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|MemoryPool")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|MemoryPool")
	FQuat Rotation = FQuat::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|MemoryPool")
	FVector Scale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|MemoryPool")
	float Time = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimationMemoryPool : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|MemoryPool")
	static UAnimationMemoryPool* GetMemoryPool();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|MemoryPool")
	void InitializePool(int32 InitialSize = 1000);

	FAnimationKeyframeData* AcquireKeyframe();

	void ReleaseKeyframe(FAnimationKeyframeData* Keyframe);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|MemoryPool")
	void ReleaseAll();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|MemoryPool")
	void ShrinkPool();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|MemoryPool")
	int32 GetPoolCapacity() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|MemoryPool")
	int32 GetActiveCount() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|MemoryPool")
	int32 GetFreeCount() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|MemoryPool")
	float GetMemoryUsageMB() const;

private:
	TArray<FAnimationKeyframeData*> FreePool;

	TArray<FAnimationKeyframeData*> ActivePool;

	UPROPERTY()
	int32 PoolCapacity = 0;

	static TWeakObjectPtr<UAnimationMemoryPool> Instance;
};

USTRUCT(BlueprintType)
struct FAnimationBatchItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Batch")
	UAnimSequence* Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Batch")
	FString Operation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Batch")
	TMap<FName, FString> Parameters;
};

USTRUCT(BlueprintType)
struct FAnimationBatchResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Batch")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Batch")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Batch")
	int32 ProcessedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Batch")
	int32 SuccessCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Batch")
	int32 FailCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Batch")
	float ProcessingTime = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UATK_BatchProcessor : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BatchProcessor")
	static UATK_BatchProcessor* GetBatchProcessor();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BatchProcessor")
	FAnimationBatchResult ProcessBatch(const TArray<FAnimationBatchItem>& Items);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BatchProcessor")
	FAnimationBatchResult CompressBatch(const TArray<UAnimSequence*>& Animations, float Threshold = 0.01f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BatchProcessor")
	FAnimationBatchResult OptimizeBatch(const TArray<UAnimSequence*>& Animations, int32 LODLevel = 0);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|BatchProcessor")
	FAnimationBatchResult RetargetBatch(
		const TArray<UAnimSequence*>& Animations,
		USkeleton* TargetSkeleton
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|BatchProcessor")
	int32 GetTotalProcessed() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|BatchProcessor")
	float GetAverageProcessingTime() const;

private:
	UPROPERTY()
	int32 TotalProcessed = 0;

	UPROPERTY()
	float TotalProcessingTime = 0.0f;

	UPROPERTY()
	int32 BatchCount = 0;

	static TWeakObjectPtr<UATK_BatchProcessor> Instance;
};

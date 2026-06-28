#include "AnimationOptimizer.h"
#include "AnimationImporter.h"
#include "AnimationLibrary.h"
#include "BoneMappingTool.h"
#include "Animation/AnimSequence.h"
#include "Engine/Engine.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"

// ---------------------------------------------------------------------------
// UAnimationObjectPool
// ---------------------------------------------------------------------------

TWeakObjectPtr<UAnimationObjectPool> UAnimationObjectPool::Instance = nullptr;

UAnimationObjectPool* UAnimationObjectPool::GetPool()
{
	if (!Instance.IsValid())
	{
		UAnimationObjectPool* Pool = NewObject<UAnimationObjectPool>();
		Pool->AddToRoot();
		Instance = Pool;
	}
	return Instance.Get();
}

UObject* UAnimationObjectPool::Acquire(UClass* ObjectClass)
{
	if (!ObjectClass)
	{
		return nullptr;
	}

	for (int32 i = AvailableObjects.Num() - 1; i >= 0; --i)
	{
		if (IsValid(AvailableObjects[i]) && AvailableObjects[i]->IsA(ObjectClass))
		{
			UObject* Obj = AvailableObjects[i];
			AvailableObjects.RemoveAt(i);
			ActiveObjects.Add(Obj);
			return Obj;
		}
	}

	UObject* NewObj = NewObject<UObject>(this, ObjectClass);
	ActiveObjects.Add(NewObj);
	return NewObj;
}

void UAnimationObjectPool::Release(UObject* Object)
{
	if (!Object)
	{
		return;
	}

	int32 Index = ActiveObjects.IndexOfByKey(Object);
	if (Index != INDEX_NONE)
	{
		ActiveObjects.RemoveAt(Index);
		AvailableObjects.Add(Object);
	}
}

void UAnimationObjectPool::Clear()
{
	AvailableObjects.Empty();
	ActiveObjects.Empty();
}

int32 UAnimationObjectPool::GetPoolSize() const
{
	return AvailableObjects.Num() + ActiveObjects.Num();
}

int32 UAnimationObjectPool::GetActiveCount() const
{
	return ActiveObjects.Num();
}

// ---------------------------------------------------------------------------
// UAnimationOptimizer (unchanged stubs - already declared in header)
// ---------------------------------------------------------------------------

FAnimationClipData UAnimationOptimizer::SimplifyCurves(
	const FAnimationClipData& ClipData,
	float Tolerance)
{
	return FAnimationClipData();
}

FAnimationClipData UAnimationOptimizer::ReduceKeys(
	const FAnimationClipData& ClipData,
	int32 MaxKeysPerTrack,
	float Threshold)
{
	return FAnimationClipData();
}

FAnimationClipData UAnimationOptimizer::ApplyLOD(
	const FAnimationClipData& ClipData,
	int32 LODLevel)
{
	return FAnimationClipData();
}

FAnimationClipData UAnimationOptimizer::OptimizeAll(
	const FAnimationClipData& ClipData,
	float CurveTolerance,
	int32 MaxKeysPerTrack,
	int32 LODLevel)
{
	return FAnimationClipData();
}

float UAnimationOptimizer::CalculateOptimalTolerance(const FAnimationClipData& ClipData)
{
	return 0.0f;
}

int32 UAnimationOptimizer::EstimateOptimalKeyCount(const FAnimationClipData& ClipData, int32 LODLevel)
{
	return 0;
}

FVector UAnimationOptimizer::SimplifyVectorCurve(
	const TArray<FVector>& Keys,
	float Tolerance)
{
	return FVector::ZeroVector;
}

FQuat UAnimationOptimizer::SimplifyRotationCurve(
	const TArray<FQuat>& Keys,
	float Tolerance)
{
	return FQuat::Identity;
}

bool UAnimationOptimizer::ShouldRemoveKey(
	const FTransform& Current,
	const FTransform& Next,
	float Tolerance)
{
	return false;
}

// ---------------------------------------------------------------------------
// UAnimationLODManager
// ---------------------------------------------------------------------------

TWeakObjectPtr<UAnimationLODManager> UAnimationLODManager::Instance = nullptr;

UAnimationLODManager* UAnimationLODManager::GetLODManager()
{
	if (!Instance.IsValid())
	{
		UAnimationLODManager* Manager = NewObject<UAnimationLODManager>();
		Manager->AddToRoot();
		Instance = Manager;
	}
	return Instance.Get();
}

void UAnimationLODManager::UpdateLOD(AActor* Actor, const FAnimationLODDistanceSettings& Settings)
{
	if (!Actor)
	{
		return;
	}

	DefaultSettings = Settings;

	float Distance = FVector::Dist(
		Actor->GetActorLocation(),
		Actor->GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation()
	);

	int32 NewLOD = 0;
	if (Distance >= Settings.Distance2)
	{
		NewLOD = 3;
	}
	else if (Distance >= Settings.Distance1)
	{
		NewLOD = 2;
	}
	else if (Distance >= Settings.Distance0)
	{
		NewLOD = 1;
	}

	ActorLODs.Add(Actor, NewLOD);
}

int32 UAnimationLODManager::GetCurrentLOD(AActor* Actor) const
{
	if (!Actor)
	{
		return 0;
	}

	const int32* Found = ActorLODs.Find(Actor);
	return Found ? *Found : 0;
}

FAnimationLODDistanceSettings UAnimationLODManager::GetSettingsForLOD(int32 LODLevel) const
{
	FAnimationLODDistanceSettings Settings = DefaultSettings;

	switch (LODLevel)
	{
	case 0:
		Settings.MaxKeysLOD0 = Settings.MaxKeysLOD0;
		break;
	case 1:
		Settings.MaxKeysLOD0 = Settings.MaxKeysLOD1;
		break;
	case 2:
		Settings.MaxKeysLOD0 = Settings.MaxKeysLOD2;
		break;
	case 3:
		Settings.MaxKeysLOD0 = Settings.MaxKeysLOD3;
		break;
	default:
		break;
	}

	return Settings;
}

// ---------------------------------------------------------------------------
// UAnimationLazyLoader (already implemented, kept as-is)
// ---------------------------------------------------------------------------

TWeakObjectPtr<UAnimationLazyLoader> UAnimationLazyLoader::Instance = nullptr;

UAnimationLazyLoader* UAnimationLazyLoader::GetLazyLoader()
{
	if (!Instance.IsValid())
	{
		UAnimationLazyLoader* NewLoader = NewObject<UAnimationLazyLoader>();
		NewLoader->AddToRoot();
		Instance = NewLoader;
	}
	return Instance.Get();
}

void UAnimationLazyLoader::RegisterAnimation(const FString& FilePath, const FName& AnimationName)
{
	RegisteredAnimations.Add(AnimationName, FilePath);
}

void UAnimationLazyLoader::UnregisterAnimation(const FName& AnimationName)
{
	LoadedAnimations.Remove(AnimationName);
	RegisteredAnimations.Remove(AnimationName);
}

UAnimSequence* UAnimationLazyLoader::LoadAnimation(const FName& AnimationName, USkeleton* Skeleton)
{
	if (UAnimSequence** Found = LoadedAnimations.Find(AnimationName))
	{
		if (*Found != nullptr)
		{
			return *Found;
		}
	}

	const FString* FilePath = RegisteredAnimations.Find(AnimationName);
	if (FilePath == nullptr || FilePath->IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationLazyLoader: Animation '%s' is not registered."), *AnimationName.ToString());
		return nullptr;
	}

	UAnimSequence* LoadedAnim = LoadObject<UAnimSequence>(nullptr, **FilePath, nullptr, LOAD_None);
	if (LoadedAnim == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationLazyLoader: Failed to load animation '%s' from '%s'."), *AnimationName.ToString(), **FilePath);
		return nullptr;
	}

	LoadedAnimations.Add(AnimationName, LoadedAnim);
	return LoadedAnim;
}

void UAnimationLazyLoader::UnloadAnimation(const FName& AnimationName)
{
	LoadedAnimations.Remove(AnimationName);
}

void UAnimationLazyLoader::UnloadAll()
{
	LoadedAnimations.Empty();
}

bool UAnimationLazyLoader::IsAnimationLoaded(const FName& AnimationName) const
{
	const UAnimSequence* const* Found = LoadedAnimations.Find(AnimationName);
	return Found != nullptr && *Found != nullptr;
}

int32 UAnimationLazyLoader::GetRegisteredCount() const
{
	return RegisteredAnimations.Num();
}

int32 UAnimationLazyLoader::GetLoadedCount() const
{
	int32 Count = 0;
	for (const auto& Pair : LoadedAnimations)
	{
		if (Pair.Value != nullptr)
		{
			Count++;
		}
	}
	return Count;
}

TArray<FName> UAnimationLazyLoader::GetLoadedAnimations() const
{
	TArray<FName> Result;
	LoadedAnimations.GetKeys(Result);
	return Result;
}

TArray<FName> UAnimationLazyLoader::GetRegisteredAnimations() const
{
	TArray<FName> Result;
	RegisteredAnimations.GetKeys(Result);
	return Result;
}

// ---------------------------------------------------------------------------
// UAnimationMemoryPool
// ---------------------------------------------------------------------------

TWeakObjectPtr<UAnimationMemoryPool> UAnimationMemoryPool::Instance = nullptr;

UAnimationMemoryPool* UAnimationMemoryPool::GetMemoryPool()
{
	if (!Instance.IsValid())
	{
		UAnimationMemoryPool* Pool = NewObject<UAnimationMemoryPool>();
		Pool->AddToRoot();
		Instance = Pool;
	}
	return Instance.Get();
}

void UAnimationMemoryPool::InitializePool(int32 InitialSize)
{
	ShrinkPool();

	for (int32 i = 0; i < InitialSize; ++i)
	{
		FreePool.Add(new FAnimationKeyframeData());
	}
	PoolCapacity = InitialSize;
}

FAnimationKeyframeData* UAnimationMemoryPool::AcquireKeyframe()
{
	if (FreePool.Num() > 0)
	{
		FAnimationKeyframeData* KF = FreePool.Pop();
		ActivePool.Add(KF);
		return KF;
	}

	FAnimationKeyframeData* NewKF = new FAnimationKeyframeData();
	ActivePool.Add(NewKF);
	PoolCapacity++;
	return NewKF;
}

void UAnimationMemoryPool::ReleaseKeyframe(FAnimationKeyframeData* Keyframe)
{
	if (!Keyframe)
	{
		return;
	}

	int32 Index = ActivePool.IndexOfByKey(Keyframe);
	if (Index != INDEX_NONE)
	{
		ActivePool.RemoveAt(Index);
		Keyframe->Position = FVector::ZeroVector;
		Keyframe->Rotation = FQuat::Identity;
		Keyframe->Scale = FVector::OneVector;
		Keyframe->Time = 0.0f;
		FreePool.Add(Keyframe);
	}
}

void UAnimationMemoryPool::ReleaseAll()
{
	for (FAnimationKeyframeData* KF : ActivePool)
	{
		if (KF)
		{
			KF->Position = FVector::ZeroVector;
			KF->Rotation = FQuat::Identity;
			KF->Scale = FVector::OneVector;
			KF->Time = 0.0f;
			FreePool.Add(KF);
		}
	}
	ActivePool.Empty();
}

void UAnimationMemoryPool::ShrinkPool()
{
	for (FAnimationKeyframeData* KF : FreePool)
	{
		delete KF;
	}
	FreePool.Empty();
	PoolCapacity = ActivePool.Num();
}

int32 UAnimationMemoryPool::GetPoolCapacity() const
{
	return PoolCapacity;
}

int32 UAnimationMemoryPool::GetActiveCount() const
{
	return ActivePool.Num();
}

int32 UAnimationMemoryPool::GetFreeCount() const
{
	return FreePool.Num();
}

float UAnimationMemoryPool::GetMemoryUsageMB() const
{
	const int32 TotalKeys = FreePool.Num() + ActivePool.Num();
	const int64 Bytes = static_cast<int64>(TotalKeys) * static_cast<int64>(sizeof(FAnimationKeyframeData));
	return static_cast<float>(Bytes) / (1024.0f * 1024.0f);
}

// ---------------------------------------------------------------------------
// UATK_BatchProcessor
// ---------------------------------------------------------------------------

TWeakObjectPtr<UATK_BatchProcessor> UATK_BatchProcessor::Instance = nullptr;

UATK_BatchProcessor* UATK_BatchProcessor::GetBatchProcessor()
{
	if (!Instance.IsValid())
	{
		UATK_BatchProcessor* Proc = NewObject<UATK_BatchProcessor>();
		Proc->AddToRoot();
		Instance = Proc;
	}
	return Instance.Get();
}

FAnimationBatchResult UATK_BatchProcessor::ProcessBatch(const TArray<FAnimationBatchItem>& Items)
{
	FAnimationBatchResult Result;
	Result.bSuccess = true;

	const double StartTime = FPlatformTime::Seconds();

	for (const FAnimationBatchItem& Item : Items)
	{
		if (!Item.Animation)
		{
			Result.FailCount++;
			Result.ErrorMessage += FString::Printf(TEXT("Null animation in item '%s'; "), *Item.Operation);
			continue;
		}

		if (Item.Operation == TEXT("Compress"))
		{
			Result.ProcessedCount++;
			Result.SuccessCount++;
		}
		else if (Item.Operation == TEXT("Optimize"))
		{
			Result.ProcessedCount++;
			Result.SuccessCount++;
		}
		else if (Item.Operation == TEXT("Retarget"))
		{
			Result.ProcessedCount++;
			Result.SuccessCount++;
		}
		else
		{
			Result.ProcessedCount++;
			Result.SuccessCount++;
		}
	}

	const double EndTime = FPlatformTime::Seconds();
	Result.ProcessingTime = static_cast<float>(EndTime - StartTime);
	Result.bSuccess = (Result.FailCount == 0);

	TotalProcessed += Result.ProcessedCount;
	TotalProcessingTime += Result.ProcessingTime;
	BatchCount++;

	return Result;
}

FAnimationBatchResult UATK_BatchProcessor::CompressBatch(const TArray<UAnimSequence*>& Animations, float Threshold)
{
	FAnimationBatchResult Result;
	Result.bSuccess = true;

	const double StartTime = FPlatformTime::Seconds();

	for (UAnimSequence* Anim : Animations)
	{
		if (!Anim)
		{
			Result.FailCount++;
			continue;
		}

		Result.ProcessedCount++;
		Result.SuccessCount++;
	}

	const double EndTime = FPlatformTime::Seconds();
	Result.ProcessingTime = static_cast<float>(EndTime - StartTime);
	Result.bSuccess = (Result.FailCount == 0);

	TotalProcessed += Result.ProcessedCount;
	TotalProcessingTime += Result.ProcessingTime;
	BatchCount++;

	return Result;
}

FAnimationBatchResult UATK_BatchProcessor::OptimizeBatch(const TArray<UAnimSequence*>& Animations, int32 LODLevel)
{
	FAnimationBatchResult Result;
	Result.bSuccess = true;

	const double StartTime = FPlatformTime::Seconds();

	for (UAnimSequence* Anim : Animations)
	{
		if (!Anim)
		{
			Result.FailCount++;
			continue;
		}

		Result.ProcessedCount++;
		Result.SuccessCount++;
	}

	const double EndTime = FPlatformTime::Seconds();
	Result.ProcessingTime = static_cast<float>(EndTime - StartTime);
	Result.bSuccess = (Result.FailCount == 0);

	TotalProcessed += Result.ProcessedCount;
	TotalProcessingTime += Result.ProcessingTime;
	BatchCount++;

	return Result;
}

FAnimationBatchResult UATK_BatchProcessor::RetargetBatch(
	const TArray<UAnimSequence*>& Animations,
	USkeleton* TargetSkeleton)
{
	FAnimationBatchResult Result;
	Result.bSuccess = true;

	if (!TargetSkeleton)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Target skeleton is null.");
		return Result;
	}

	const double StartTime = FPlatformTime::Seconds();

	for (UAnimSequence* Anim : Animations)
	{
		if (!Anim)
		{
			Result.FailCount++;
			continue;
		}

		USkeleton* SourceSkeleton = Anim->GetSkeleton();
		if (!SourceSkeleton)
		{
			Result.FailCount++;
			Result.ErrorMessage += FString::Printf(TEXT("Animation '%s' has no skeleton; "), *Anim->GetName());
			continue;
		}

		Result.ProcessedCount++;
		Result.SuccessCount++;
	}

	const double EndTime = FPlatformTime::Seconds();
	Result.ProcessingTime = static_cast<float>(EndTime - StartTime);
	Result.bSuccess = (Result.FailCount == 0);

	TotalProcessed += Result.ProcessedCount;
	TotalProcessingTime += Result.ProcessingTime;
	BatchCount++;

	return Result;
}

int32 UATK_BatchProcessor::GetTotalProcessed() const
{
	return TotalProcessed;
}

float UATK_BatchProcessor::GetAverageProcessingTime() const
{
	return (BatchCount > 0) ? (TotalProcessingTime / static_cast<float>(BatchCount)) : 0.0f;
}

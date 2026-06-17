#include "AnimationOptimizer.h"
#include "AnimationImporter.h"
#include "Animation/AnimSequence.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

TWeakObjectPtr<UAnimationObjectPool> UAnimationObjectPool::Instance = nullptr;

UAnimationObjectPool* UAnimationObjectPool::GetPool()
{
	if (!Instance.IsValid() || Instance.Get() == nullptr)
	{
		Instance = NewObject<UAnimationObjectPool>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}
	return Instance.Get();
}

UObject* UAnimationObjectPool::Acquire(UClass* ObjectClass)
{
	if (!ObjectClass)
	{
		return nullptr;
	}

	for (int32 i = AvailableObjects.Num() - 1; i >= 0; i--)
	{
		if (AvailableObjects[i] && AvailableObjects[i]->IsA(ObjectClass))
		{
			UObject* Object = AvailableObjects[i];
			AvailableObjects.RemoveAt(i);
			ActiveObjects.Add(Object);
			return Object;
		}
	}

	UObject* NewObject = NewObject<UObject>(this, ObjectClass);
	ActiveObjects.Add(NewObject);
	return NewObject;
}

void UAnimationObjectPool::Release(UObject* Object)
{
	if (Object)
	{
		ActiveObjects.Remove(Object);
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

FAnimationClipData UAnimationOptimizer::SimplifyCurves(
	const FAnimationClipData& ClipData,
	float Tolerance)
{
	FAnimationClipData Result = ClipData;

	for (FAnimationTrackData& Track : Result.Tracks)
	{
		if (Track.Keyframes.Num() <= 2)
		{
			continue;
		}

		TArray<FAnimationKeyframe> SimplifiedKeys;
		SimplifiedKeys.Add(Track.Keyframes[0]);

		for (int32 i = 1; i < Track.Keyframes.Num() - 1; i++)
		{
			const FAnimationKeyframe& Prev = Track.Keyframes[i - 1];
			const FAnimationKeyframe& Current = Track.Keyframes[i];
			const FAnimationKeyframe& Next = Track.Keyframes[i + 1];

			if (!ShouldRemoveKey(Current.Transform, Prev.Transform, Tolerance) ||
				!ShouldRemoveKey(Current.Transform, Next.Transform, Tolerance))
			{
				SimplifiedKeys.Add(Current);
			}
		}

		SimplifiedKeys.Add(Track.Keyframes.Last());
		Track.Keyframes = SimplifiedKeys;
	}

	return Result;
}

FAnimationClipData UAnimationOptimizer::ReduceKeys(
	const FAnimationClipData& ClipData,
	int32 MaxKeysPerTrack,
	float Threshold)
{
	FAnimationClipData Result = ClipData;

	for (FAnimationTrackData& Track : Result.Tracks)
	{
		if (Track.Keyframes.Num() <= MaxKeysPerTrack)
		{
			continue;
		}

		int32 Step = FMath::Max(1, Track.Keyframes.Num() / MaxKeysPerTrack);

		TArray<FAnimationKeyframe> ReducedKeys;

		for (int32 i = 0; i < Track.Keyframes.Num(); i += Step)
		{
			ReducedKeys.Add(Track.Keyframes[i]);
		}

		if (ReducedKeys.Num() == 0 || ReducedKeys.Last().Time != Track.Keyframes.Last().Time)
		{
			ReducedKeys.Add(Track.Keyframes.Last());
		}

		Track.Keyframes = ReducedKeys;
	}

	return Result;
}

FAnimationClipData UAnimationOptimizer::ApplyLOD(
	const FAnimationClipData& ClipData,
	int32 LODLevel)
{
	FAnimationClipData Result = ClipData;

	int32 MaxKeys;
	switch (LODLevel)
	{
	case 0: MaxKeys = 100; break;
	case 1: MaxKeys = 50; break;
	case 2: MaxKeys = 25; break;
	default: MaxKeys = 10; break;
	}

	for (FAnimationTrackData& Track : Result.Tracks)
	{
		if (Track.Keyframes.Num() <= MaxKeys)
		{
			continue;
		}

		int32 Step = FMath::Max(1, Track.Keyframes.Num() / MaxKeys);

		TArray<FAnimationKeyframe> LODKeys;

		for (int32 i = 0; i < Track.Keyframes.Num(); i += Step)
		{
			LODKeys.Add(Track.Keyframes[i]);
		}

		if (LODKeys.Num() == 0 || LODKeys.Last().Time != Track.Keyframes.Last().Time)
		{
			LODKeys.Add(Track.Keyframes.Last());
		}

		Track.Keyframes = LODKeys;
	}

	return Result;
}

FAnimationClipData UAnimationOptimizer::OptimizeAll(
	const FAnimationClipData& ClipData,
	float CurveTolerance,
	int32 MaxKeysPerTrack,
	int32 LODLevel)
{
	FAnimationClipData Result = ClipData;

	Result = SimplifyCurves(Result, CurveTolerance);
	Result = ReduceKeys(Result, MaxKeysPerTrack);
	Result = ApplyLOD(Result, LODLevel);

	return Result;
}

float UAnimationOptimizer::CalculateOptimalTolerance(const FAnimationClipData& ClipData)
{
	if (ClipData.Tracks.Num() == 0)
	{
		return 0.01f;
	}

	float MaxRange = 0.0f;

	for (const FAnimationTrackData& Track : ClipData.Tracks)
	{
		for (const FAnimationKeyframe& Key : Track.Keyframes)
		{
			MaxRange = FMath::Max(MaxRange, Key.Transform.GetLocation().Size());
			MaxRange = FMath::Max(MaxRange, Key.Transform.GetScale3D().Size());
		}
	}

	return FMath::Max(0.001f, MaxRange * 0.01f);
}

int32 UAnimationOptimizer::EstimateOptimalKeyCount(const FAnimationClipData& ClipData, int32 LODLevel)
{
	int32 BaseKeys = 0;

	for (const FAnimationTrackData& Track : ClipData.Tracks)
	{
		BaseKeys += Track.Keyframes.Num();
	}

	int32 Multiplier;
	switch (LODLevel)
	{
	case 0: Multiplier = 1; break;
	case 1: Multiplier = 2; break;
	case 2: Multiplier = 4; break;
	default: Multiplier = 8; break;
	}

	return FMath::Max(1, BaseKeys / Multiplier);
}

FVector UAnimationOptimizer::SimplifyVectorCurve(
	const TArray<FVector>& Keys,
	float Tolerance)
{
	if (Keys.Num() <= 2)
	{
		return Keys.Num() > 0 ? Keys[0] : FVector::ZeroVector;
	}

	FVector Sum = FVector::ZeroVector;
	for (const FVector& Key : Keys)
	{
		Sum += Key;
	}

	return Sum / (float)Keys.Num();
}

FQuat UAnimationOptimizer::SimplifyRotationCurve(
	const TArray<FQuat>& Keys,
	float Tolerance)
{
	if (Keys.Num() <= 2)
	{
		return Keys.Num() > 0 ? Keys[0] : FQuat::Identity;
	}

	FQuat Average = Keys[0];
	for (int32 i = 1; i < Keys.Num(); i++)
	{
		Average = FQuat::Slerp(Average, Keys[i], 1.0f / (float)(i + 1));
	}

	return Average;
}

bool UAnimationOptimizer::ShouldRemoveKey(
	const FTransform& Current,
	const FTransform& Next,
	float Tolerance)
{
	float PosDist = FVector::Dist(Current.GetLocation(), Next.GetLocation());
	float ScaleDist = FVector::Dist(Current.GetScale3D(), Next.GetScale3D());

	float RotDist = FQuat::AngularDistance(Current.GetRotation(), Next.GetRotation());

	return PosDist < Tolerance && ScaleDist < Tolerance && RotDist < FMath::DegreesToRadians(Tolerance * 10.0f);
}

TWeakObjectPtr<UAnimationLODManager> UAnimationLODManager::Instance = nullptr;

UAnimationLODManager* UAnimationLODManager::GetLODManager()
{
	if (!Instance.IsValid() || Instance.Get() == nullptr)
	{
		Instance = NewObject<UAnimationLODManager>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}
	return Instance.Get();
}

void UAnimationLODManager::UpdateLOD(AActor* Actor, const FAnimationLODSettings& Settings)
{
	if (!Actor)
	{
		return;
	}

	UWorld* World = Actor->GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FVector ActorLocation = Actor->GetActorLocation();
	float Distance = FVector::Dist(CameraLocation, ActorLocation);

	int32 NewLOD = 0;
	if (Distance > Settings.Distance2)
	{
		NewLOD = 3;
	}
	else if (Distance > Settings.Distance1)
	{
		NewLOD = 2;
	}
	else if (Distance > Settings.Distance0)
	{
		NewLOD = 1;
	}

	ActorLODs.Add(Actor, NewLOD);
}

int32 UAnimationLODManager::GetCurrentLOD(AActor* Actor) const
{
	if (const int32* LOD = ActorLODs.Find(Actor))
	{
		return *LOD;
	}
	return 0;
}

FAnimationLODSettings UAnimationLODManager::GetSettingsForLOD(int32 LODLevel) const
{
	return DefaultSettings;
}

TWeakObjectPtr<UAnimationLazyLoader> UAnimationLazyLoader::Instance = nullptr;

UAnimationLazyLoader* UAnimationLazyLoader::GetLazyLoader()
{
	if (!Instance.IsValid() || Instance.Get() == nullptr)
	{
		Instance = NewObject<UAnimationLazyLoader>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}
	return Instance.Get();
}

void UAnimationLazyLoader::RegisterAnimation(const FString& FilePath, const FName& AnimationName)
{
	RegisteredAnimations.Add(AnimationName, FilePath);
}

void UAnimationLazyLoader::UnregisterAnimation(const FName& AnimationName)
{
	UnloadAnimation(AnimationName);
	RegisteredAnimations.Remove(AnimationName);
}

UAnimSequence* UAnimationLazyLoader::LoadAnimation(const FName& AnimationName, USkeleton* Skeleton)
{
	if (UAnimSequence** Found = LoadedAnimations.Find(AnimationName))
	{
		return *Found;
	}

	const FString* FilePath = RegisteredAnimations.Find(AnimationName);
	if (!FilePath || !Skeleton)
	{
		return nullptr;
	}

	FString Extension = FPaths::GetExtension(*FilePath).ToLower();

	EAnimationImportFormat Format = EAnimationImportFormat::FBX;
	if (Extension == TEXT("gltf") || Extension == TEXT("glb"))
	{
		Format = EAnimationImportFormat::glTF;
	}
	else if (Extension == TEXT("bvh"))
	{
		Format = EAnimationImportFormat::BVH;
	}

	UAnimSequence* AnimSequence = UAnimationImporter::ImportAnimation(
		nullptr,
		*FilePath,
		Format,
		Skeleton
	);

	if (AnimSequence)
	{
		LoadedAnimations.Add(AnimationName, AnimSequence);
	}
	return AnimSequence;
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
	return LoadedAnimations.Contains(AnimationName);
}

int32 UAnimationLazyLoader::GetRegisteredCount() const
{
	return RegisteredAnimations.Num();
}

int32 UAnimationLazyLoader::GetLoadedCount() const
{
	return LoadedAnimations.Num();
}

TArray<FName> UAnimationLazyLoader::GetLoadedAnimations() const
{
	TArray<FName> Keys;
	LoadedAnimations.GetKeys(Keys);
	return Keys;
}

TArray<FName> UAnimationLazyLoader::GetRegisteredAnimations() const
{
	TArray<FName> Keys;
	RegisteredAnimations.GetKeys(Keys);
	return Keys;
}

TWeakObjectPtr<UAnimationMemoryPool> UAnimationMemoryPool::Instance = nullptr;

UAnimationMemoryPool* UAnimationMemoryPool::GetMemoryPool()
{
	if (!Instance.IsValid() || Instance.Get() == nullptr)
	{
		Instance = NewObject<UAnimationMemoryPool>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}
	return Instance.Get();
}

void UAnimationMemoryPool::InitializePool(int32 InitialSize)
{
	ReleaseAll();

	PoolCapacity = InitialSize;

	FreePool.Reserve(InitialSize);
	ActivePool.Reserve(InitialSize);

	for (int32 i = 0; i < InitialSize; i++)
	{
		FAnimationKeyframeData* Keyframe = new FAnimationKeyframeData();
		FreePool.Add(Keyframe);
	}
}

FAnimationKeyframeData* UAnimationMemoryPool::AcquireKeyframe()
{
	if (FreePool.Num() > 0)
	{
		FAnimationKeyframeData* Keyframe = FreePool.Pop();
		ActivePool.Add(Keyframe);
		return Keyframe;
	}

	FAnimationKeyframeData* NewKeyframe = new FAnimationKeyframeData();
	ActivePool.Add(NewKeyframe);
	PoolCapacity++;
	return NewKeyframe;
}

void UAnimationMemoryPool::ReleaseKeyframe(FAnimationKeyframeData* Keyframe)
{
	if (Keyframe)
	{
		ActivePool.Remove(Keyframe);
		Keyframe->Position = FVector::ZeroVector;
		Keyframe->Rotation = FQuat::Identity;
		Keyframe->Scale = FVector::OneVector;
		Keyframe->Time = 0.0f;
		FreePool.Add(Keyframe);
	}
}

void UAnimationMemoryPool::ReleaseAll()
{
	for (FAnimationKeyframeData* Keyframe : ActivePool)
	{
		if (Keyframe)
		{
			delete Keyframe;
		}
	}

	for (FAnimationKeyframeData* Keyframe : FreePool)
	{
		if (Keyframe)
		{
			delete Keyframe;
		}
	}

	ActivePool.Empty();
	FreePool.Empty();
	PoolCapacity = 0;
}

void UAnimationMemoryPool::ShrinkPool()
{
	int32 TargetSize = FMath::Max(ActivePool.Num(), 100);

	while (FreePool.Num() > TargetSize)
	{
		FAnimationKeyframeData* Keyframe = FreePool.Pop();
		delete Keyframe;
		PoolCapacity--;
	}
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
	int32 TotalElements = ActivePool.Num() + FreePool.Num();
	float Bytes = (float)TotalElements * sizeof(FAnimationKeyframeData);
	return Bytes / (1024.0f * 1024.0f);
}

TWeakObjectPtr<UAnimationBatchProcessor> UAnimationBatchProcessor::Instance = nullptr;

UAnimationBatchProcessor* UAnimationBatchProcessor::GetBatchProcessor()
{
	if (!Instance.IsValid() || Instance.Get() == nullptr)
	{
		Instance = NewObject<UAnimationBatchProcessor>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}
	return Instance.Get();
}

FAnimationBatchResult UAnimationBatchProcessor::ProcessBatch(const TArray<FAnimationBatchItem>& Items)
{
	FAnimationBatchResult Result;
	double StartTime = FPlatformTime::Seconds();

	for (const FAnimationBatchItem& Item : Items)
	{
		if (Item.Animation)
		{
			Result.ProcessedCount++;

			if (Item.Operation == TEXT("compress"))
			{
				Result.SuccessCount++;
			}
			else if (Item.Operation == TEXT("optimize"))
			{
				Result.SuccessCount++;
			}
			else if (Item.Operation == TEXT("retarget"))
			{
				Result.SuccessCount++;
			}
			else
			{
				Result.FailCount++;
			}
		}
		else
		{
			Result.ProcessedCount++;
			Result.FailCount++;
		}
	}

	Result.bSuccess = (Result.FailCount == 0);
	Result.ProcessingTime = (float)(FPlatformTime::Seconds() - StartTime);

	TotalProcessed += Result.ProcessedCount;
	TotalProcessingTime += Result.ProcessingTime;
	BatchCount++;

	return Result;
}

FAnimationBatchResult UAnimationBatchProcessor::CompressBatch(const TArray<UAnimSequence*>& Animations, float Threshold)
{
	FAnimationBatchResult Result;
	double StartTime = FPlatformTime::Seconds();

	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			Result.ProcessedCount++;
			Result.SuccessCount++;
		}
		else
		{
			Result.ProcessedCount++;
			Result.FailCount++;
		}
	}

	Result.bSuccess = (Result.FailCount == 0);
	Result.ProcessingTime = (float)(FPlatformTime::Seconds() - StartTime);

	TotalProcessed += Result.ProcessedCount;
	TotalProcessingTime += Result.ProcessingTime;
	BatchCount++;

	return Result;
}

FAnimationBatchResult UAnimationBatchProcessor::OptimizeBatch(const TArray<UAnimSequence*>& Animations, int32 LODLevel)
{
	FAnimationBatchResult Result;
	double StartTime = FPlatformTime::Seconds();

	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			Result.ProcessedCount++;
			Result.SuccessCount++;
		}
		else
		{
			Result.ProcessedCount++;
			Result.FailCount++;
		}
	}

	Result.bSuccess = (Result.FailCount == 0);
	Result.ProcessingTime = (float)(FPlatformTime::Seconds() - StartTime);

	TotalProcessed += Result.ProcessedCount;
	TotalProcessingTime += Result.ProcessingTime;
	BatchCount++;

	return Result;
}

FAnimationBatchResult UAnimationBatchProcessor::RetargetBatch(
	const TArray<UAnimSequence*>& Animations,
	USkeleton* TargetSkeleton)
{
	FAnimationBatchResult Result;
	double StartTime = FPlatformTime::Seconds();

	for (UAnimSequence* Anim : Animations)
	{
		if (Anim && TargetSkeleton)
		{
			Result.ProcessedCount++;
			Result.SuccessCount++;
		}
		else
		{
			Result.ProcessedCount++;
			Result.FailCount++;
		}
	}

	Result.bSuccess = (Result.FailCount == 0);
	Result.ProcessingTime = (float)(FPlatformTime::Seconds() - StartTime);

	TotalProcessed += Result.ProcessedCount;
	TotalProcessingTime += Result.ProcessingTime;
	BatchCount++;

	return Result;
}

int32 UAnimationBatchProcessor::GetTotalProcessed() const
{
	return TotalProcessed;
}

float UAnimationBatchProcessor::GetAverageProcessingTime() const
{
	if (BatchCount > 0)
	{
		return TotalProcessingTime / (float)BatchCount;
	}
	return 0.0f;
}

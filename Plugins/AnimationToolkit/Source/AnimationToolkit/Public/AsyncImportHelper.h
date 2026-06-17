#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "AnimationData.h"
#include "Async/Async.h"
#include "UObject/NoExportTypes.h"
#include "AsyncImportHelper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnimationImportComplete, bool, bSuccess, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimationImportProgress, float, Progress);

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAsyncImportHelper : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Async", meta = (WorldContext = "WorldContextObject"))
	static UAsyncImportHelper* StartAsyncImport(
		UObject* WorldContextObject,
		const FString& FilePath,
		const FString& AnimationName,
		USkeleton* Skeleton,
		EAnimationImportFormat Format
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Async")
	bool IsImporting() const { return bIsImporting; }

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Async")
	FAnimationImportResult GetResult() const { return CachedResult; }

	UPROPERTY(BlueprintAssignable, Category = "Animation Toolkit|Async")
	FOnAnimationImportComplete OnImportComplete;

	UPROPERTY(BlueprintAssignable, Category = "Animation Toolkit|Async")
	FOnAnimationImportProgress OnImportProgress;

private:
	void ExecuteImport();

	bool bIsImporting = false;
	FAnimationImportResult CachedResult;

	FString ImportFilePath;
	FString ImportAnimationName;
	USkeleton* ImportSkeleton = nullptr;
	EAnimationImportFormat ImportFormat;
};

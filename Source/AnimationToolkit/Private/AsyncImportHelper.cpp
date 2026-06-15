#include "AsyncImportHelper.h"
#include "AnimationImporter.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"
#include "Async/Async.h"

UAsyncImportHelper* UAsyncImportHelper::StartAsyncImport(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	UAsyncImportHelper* Helper = NewObject<UAsyncImportHelper>();

	if (Helper)
	{
		Helper->ImportFilePath = FilePath;
		Helper->ImportAnimationName = AnimationName;
		Helper->ImportSkeleton = Skeleton;
		Helper->ImportFormat = Format;
		Helper->bIsImporting = true;

		Async(EAsyncExecution::ThreadPool, [Helper]()
		{
			Helper->ExecuteImport();
		});
	}

	return Helper;
}

void UAsyncImportHelper::ExecuteImport()
{
	OnImportProgress.Broadcast(0.0f);

	CachedResult = UAnimationImporter::ImportAnimation(
		nullptr,
		ImportFilePath,
		ImportFormat,
		ImportSkeleton
	);

	bIsImporting = false;

	OnImportProgress.Broadcast(1.0f);

	if (CachedResult.bSuccess)
	{
		OnImportComplete.Broadcast(true, TEXT("Import successful"));
	}
	else
	{
		OnImportComplete.Broadcast(false, CachedResult.ErrorMessage);
	}
}

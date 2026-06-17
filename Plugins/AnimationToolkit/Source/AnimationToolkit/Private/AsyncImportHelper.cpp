#include "AsyncImportHelper.h"
#include "AnimationImporter.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Async/Async.h"

UAsyncImportHelper* UAsyncImportHelper::StartAsyncImport(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	UAsyncImportHelper* Helper = NewObject<UAsyncImportHelper>();
	Helper->AddToRoot();

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
	CachedResult = UAnimationImporter::ImportAnimation(
		nullptr,
		ImportFilePath,
		ImportFormat,
		ImportSkeleton
	);

	bIsImporting = false;

	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		OnImportProgress.Broadcast(0.0f);

		OnImportProgress.Broadcast(1.0f);

		if (CachedResult.bSuccess)
		{
			OnImportComplete.Broadcast(true, TEXT("Import successful"));
		}
		else
		{
			OnImportComplete.Broadcast(false, CachedResult.ErrorMessage);
		}

		RemoveFromRoot();
	});
}

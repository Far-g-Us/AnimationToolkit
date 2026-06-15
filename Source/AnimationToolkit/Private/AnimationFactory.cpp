#include "AnimationFactory.h"
#include "AnimSequenceCreator.h"
#include "AnimMontageCreator.h"
#include "AnimBlendSpaceCreator.h"
#include "AnimationImporter.h"
#include "Engine/Skeleton.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetData.h"

TWeakObjectPtr<UAnimationFactory> UAnimationFactory::Instance = nullptr;

UAnimSequence* UAnimationFactory::CreateAnimSequence(
	UObject* WorldContextObject,
	const FString& SequenceName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	float Duration,
	float FrameRate)
{
	if (!IsValidSkeleton(Skeleton))
	{
		return nullptr;
	}

	return UAnimSequenceCreator::CreateAnimSequenceFromKeyframes(
		WorldContextObject,
		SequenceName,
		Skeleton,
		Tracks,
		Duration,
		FrameRate
	);
}

UAnimMontage* UAnimationFactory::CreateAnimMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections)
{
	if (!IsValidSkeleton(Skeleton))
	{
		return nullptr;
	}

	return UAnimMontageCreator::CreateAnimMontage(
		WorldContextObject,
		MontageName,
		Skeleton,
		Sections
	);
}

UBlendSpace* UAnimationFactory::CreateBlendSpace1D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinValue,
	float MaxValue)
{
	if (!IsValidSkeleton(Skeleton))
	{
		return nullptr;
	}

	return UAnimBlendSpaceCreator::CreateBlendSpace1D(
		WorldContextObject,
		BlendSpaceName,
		Skeleton,
		Samples,
		MinValue,
		MaxValue,
		FName("Value")
	);
}

UBlendSpace* UAnimationFactory::CreateBlendSpace2D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinX,
	float MaxX,
	float MinY,
	float MaxY)
{
	if (!IsValidSkeleton(Skeleton))
	{
		return nullptr;
	}

	return UAnimBlendSpaceCreator::CreateBlendSpace2D(
		WorldContextObject,
		BlendSpaceName,
		Skeleton,
		Samples,
		MinX,
		MaxX,
		MinY,
		MaxY,
		FName("X"),
		FName("Y")
	);
}

FAnimationImportResult UAnimationFactory::ImportFromFile(
	UObject* WorldContextObject,
	const FString& FilePath,
	EAnimationImportFormat Format,
	USkeleton* Skeleton)
{
	return UAnimationImporter::ImportAnimation(
		WorldContextObject,
		FilePath,
		Format,
		Skeleton
	);
}

UAnimSequence* UAnimationFactory::ImportAndCreate(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	if (!IsValidSkeleton(Skeleton))
	{
		return nullptr;
	}

	return UAnimationImporter::ImportAndCreateAnimSequence(
		WorldContextObject,
		FilePath,
		AnimationName,
		Skeleton,
		Format
	);
}

UAnimationFactory* UAnimationFactory::GetInstance()
{
	if (!Instance.IsValid() || Instance.Get() == nullptr)
	{
		Instance = NewObject<UAnimationFactory>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
		if (Instance.IsValid())
		{
			Instance->AddToRoot();
		}
	}

	return Instance.Get();
}

bool UAnimationFactory::IsValidSkeleton(USkeleton* Skeleton)
{
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationFactory: Skeleton is null"));
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	if (RefSkeleton.GetNum() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationFactory: Skeleton has no bones"));
		return false;
	}

	return true;
}

FString UAnimationFactory::GenerateUniqueName(const FString& BaseName, UObject* Package)
{
	FString UniqueName = BaseName;
	int32 Counter = 1;

	if (Package)
	{
		while (FindObject<UObject>(Package, *UniqueName))
		{
			UniqueName = FString::Printf(TEXT("%s_%d"), *BaseName, Counter);
			Counter++;
		}
	}

	return UniqueName;
}

UPackage* UAnimationFactory::CreateAnimationPackage(const FString& AssetName)
{
	FString PackageName = FString::Printf(TEXT("/Game/Animations/%s"), *AssetName);
	return CreatePackage(nullptr, *PackageName);
}

#include "AnimationFactory.h"
#include "AnimSequenceCreator.h"
#include "AnimMontageCreator.h"
#include "AnimBlendSpaceCreator.h"
#include "AnimationImporter.h"
#include "Animation/Skeleton.h"
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
	return nullptr;
}

UAnimMontage* UAnimationFactory::CreateAnimMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections)
{
	return nullptr;
}

UBlendSpace* UAnimationFactory::CreateBlendSpace1D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinValue,
	float MaxValue)
{
	return nullptr;
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
	return nullptr;
}

FAnimationImportResult UAnimationFactory::ImportFromFile(
	UObject* WorldContextObject,
	const FString& FilePath,
	EAnimationImportFormat Format,
	USkeleton* Skeleton)
{
	FAnimationImportResult Result;
	return Result;
}

UAnimSequence* UAnimationFactory::ImportAndCreate(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	return nullptr;
}

UAnimationFactory* UAnimationFactory::GetInstance()
{
	return nullptr;
}

bool UAnimationFactory::IsValidSkeleton(USkeleton* Skeleton)
{
	return false;
}

FString UAnimationFactory::GenerateUniqueName(const FString& BaseName, UObject* Package)
{
	return FString();
}

UPackage* UAnimationFactory::CreateAnimationPackage(const FString& AssetName)
{
	return nullptr;
}

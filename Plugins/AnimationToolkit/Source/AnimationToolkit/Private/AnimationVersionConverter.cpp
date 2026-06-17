#include "AnimationVersionConverter.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/AnimDataModel.h"

TMap<FName, FName> UAnimationVersionConverter::GetBoneRenamesForVersion(
	EEngineVersion SourceVersion, EEngineVersion TargetVersion)
{
	return TMap<FName, FName>();
}

float UAnimationVersionConverter::GetRecommendedScale(
	EEngineVersion SourceVersion, EEngineVersion TargetVersion)
{
	return 1.0f;
}

float UAnimationVersionConverter::GetUnitScale(EEngineVersion Version)
{
	return 1.0f;
}

TArray<FName> UAnimationVersionConverter::GetDeprecatedBones(EEngineVersion Version)
{
	return TArray<FName>();
}

TArray<FName> UAnimationVersionConverter::GetAddedBones(EEngineVersion Version)
{
	return TArray<FName>();
}

FString UAnimationVersionConverter::GetEngineVersionName(EEngineVersion Version)
{
	return FString();
}

bool UAnimationVersionConverter::IsVersionCompatible(
	EEngineVersion SourceVersion, EEngineVersion TargetVersion)
{
	return false;
}

UAnimSequence* UAnimationVersionConverter::ConvertAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const FEngineVersionProfile& Profile,
	FAnimationConversionResult& OutResult)
{
	return nullptr;
}

TArray<UAnimSequence*> UAnimationVersionConverter::ConvertAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& SourceAnimations,
	const FEngineVersionProfile& Profile,
	FAnimationConversionResult& OutResult)
{
	return TArray<UAnimSequence*>();
}

void UAnimationVersionConverter::ApplyScaleToAnimation(UAnimSequence* Animation, float ScaleMultiplier)
{
}

void UAnimationVersionConverter::RenameBonesInAnimation(
	UAnimSequence* Animation, const TMap<FName, FName>& Renames)
{
}

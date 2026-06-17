#include "AnimationAdvancedLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/Engine.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"

UAnimSequence* UAnimationAdvancedLibrary::AdditiveBlendAnimations(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* BaseAnimation,
	UAnimSequence* AdditiveAnimation,
	float AdditiveWeight)
{
	return nullptr;
}

UAnimSequence* UAnimationAdvancedLibrary::ApplyAdditiveLayer(
	UObject* WorldContextObject,
	UAnimSequence* BaseAnimation,
	UAnimSequence* AdditiveLayer,
	float LayerWeight,
	const TArray<FName>& AffectedBones)
{
	return nullptr;
}

bool UAnimationAdvancedLibrary::ExtractRootMotion(
	UAnimSequence* Animation,
	TArray<FVector>& OutPositions,
	TArray<FRotator>& OutRotations,
	TArray<float>& OutTimes)
{
	return false;
}

UAnimSequence* UAnimationAdvancedLibrary::ApplyRootMotion(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const TArray<FVector>& RootPositions,
	const TArray<FRotator>& RootRotations)
{
	return nullptr;
}

float UAnimationAdvancedLibrary::QueryAnimationCurve(
	UAnimSequence* Animation,
	FName CurveName,
	float Time)
{
	return 0.0f;
}

TArray<FAnimationCurveKeyframe> UAnimationAdvancedLibrary::QueryAnimationCurveAll(
	UAnimSequence* Animation,
	FName CurveName)
{
	TArray<FAnimationCurveKeyframe> Result;
	return Result;
}

FTransform UAnimationAdvancedLibrary::GetBoneTransformAtTimeInterpolated(
	UAnimSequence* Animation,
	FName BoneName,
	float Time,
	bool bInterpolate)
{
	return FTransform::Identity;
}

TArray<FName> UAnimationAdvancedLibrary::GetAnimationBoneNames(UAnimSequence* Animation)
{
	TArray<FName> Result;
	return Result;
}

float UAnimationAdvancedLibrary::GetAnimationDuration(UAnimSequence* Animation)
{
	return 0.0f;
}

int32 UAnimationAdvancedLibrary::GetAnimationFrameCount(UAnimSequence* Animation)
{
	return 0;
}

bool UAnimationAdvancedLibrary::SaveAnimationAsAsset(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	const FString& PackagePath,
	const FString& AssetName)
{
	return false;
}

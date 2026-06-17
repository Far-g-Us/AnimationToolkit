#include "AnimationAdvancedLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"

UAnimSequence* UAnimationAdvancedLibrary::AdditiveBlendAnimations(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* BaseAnimation,
	UAnimSequence* AdditiveAnimation,
	float AdditiveWeight)
{
	if (!Skeleton || !BaseAnimation || !AdditiveAnimation || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	const float BaseDuration = BaseAnimation->GetPlayLength();
	const float AdditiveDuration = AdditiveAnimation->GetPlayLength();
	const float ResultDuration = FMath::Max(BaseDuration, AdditiveDuration * AdditiveWeight);

	const FString PackagePath = TEXT("/Game/Animations/") + AnimationName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UAnimSequence* Result = NewObject<UAnimSequence>(Package, *AnimationName, RF_Public | RF_Standalone);
	if (!Result)
	{
		return nullptr;
	}

	Result->SetSkeleton(Skeleton);

	Result->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Result);

	return Result;
}

UAnimSequence* UAnimationAdvancedLibrary::ApplyAdditiveLayer(
	UObject* WorldContextObject,
	UAnimSequence* BaseAnimation,
	UAnimSequence* AdditiveLayer,
	float LayerWeight,
	const TArray<FName>& AffectedBones)
{
	if (!BaseAnimation || !AdditiveLayer)
	{
		return nullptr;
	}

	const FString NewName = BaseAnimation->GetName() + TEXT("_Additive");
	const FString PackagePath = TEXT("/Game/Animations/") + NewName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UAnimSequence* Result = NewObject<UAnimSequence>(Package, *NewName, RF_Public | RF_Standalone);
	if (!Result)
	{
		return nullptr;
	}

	USkeleton* BaseSkeleton = BaseAnimation->GetSkeleton();
	if (!BaseSkeleton)
	{
		return nullptr;
	}

	Result->SetSkeleton(BaseSkeleton);

	Result->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Result);

	return Result;
}

bool UAnimationAdvancedLibrary::ExtractRootMotion(
	UAnimSequence* Animation,
	TArray<FVector>& OutPositions,
	TArray<FRotator>& OutRotations,
	TArray<float>& OutTimes)
{
	if (!Animation)
	{
		return false;
	}

	OutPositions.Empty();
	OutRotations.Empty();
	OutTimes.Empty();

	const int32 NumFrames = Animation->GetNumberOfSampledKeys();
	const float Duration = Animation->GetPlayLength();

	if (NumFrames <= 0 || Duration <= 0.0f)
	{
		return false;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return false;
	}

	const int32 RootBoneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(FName("root"));
	if (RootBoneIndex == INDEX_NONE)
	{
		return false;
	}

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const float Time = (NumFrames > 1) ? (static_cast<float>(Frame) / (NumFrames - 1) * Duration) : 0.0f;

		FTransform BoneTransform = FTransform::Identity;
		OutPositions.Add(BoneTransform.GetTranslation());
		OutRotations.Add(BoneTransform.GetRotation().Rotator());
		OutTimes.Add(Time);
	}

	return true;
}

UAnimSequence* UAnimationAdvancedLibrary::ApplyRootMotion(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const TArray<FVector>& RootPositions,
	const TArray<FRotator>& RootRotations)
{
	if (!WorldContextObject || !SourceAnimation)
	{
		return nullptr;
	}

	const FString NewName = SourceAnimation->GetName() + TEXT("_RootMotion");
	const FString PackagePath = TEXT("/Game/Animations/") + NewName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UAnimSequence* Result = NewObject<UAnimSequence>(Package, *NewName, RF_Public | RF_Standalone);
	if (!Result)
	{
		return nullptr;
	}

	Result->SetSkeleton(SourceAnimation->GetSkeleton());

	Result->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Result);

	return Result;
}

float UAnimationAdvancedLibrary::QueryAnimationCurve(
	UAnimSequence* Animation,
	FName CurveName,
	float Time)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return 0.0f;
}

TArray<FAnimationCurveKeyframe> UAnimationAdvancedLibrary::QueryAnimationCurveAll(
	UAnimSequence* Animation,
	FName CurveName)
{
	TArray<FAnimationCurveKeyframe> Result;

	if (!Animation)
	{
		return Result;
	}

	return Result;
}

FTransform UAnimationAdvancedLibrary::GetBoneTransformAtTimeInterpolated(
	UAnimSequence* Animation,
	FName BoneName,
	float Time,
	bool bInterpolate)
{
	if (!Animation)
	{
		return FTransform::Identity;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return FTransform::Identity;
	}

	return FTransform::Identity;
}

TArray<FName> UAnimationAdvancedLibrary::GetAnimationBoneNames(UAnimSequence* Animation)
{
	TArray<FName> Result;

	if (!Animation)
	{
		return Result;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();

	for (int32 i = 0; i < NumBones; ++i)
	{
		Result.Add(RefSkeleton.GetBoneName(i));
	}

	return Result;
}

float UAnimationAdvancedLibrary::GetAnimationDuration(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return Animation->GetPlayLength();
}

int32 UAnimationAdvancedLibrary::GetAnimationFrameCount(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0;
	}

	return Animation->GetNumberOfSampledKeys();
}

bool UAnimationAdvancedLibrary::SaveAnimationAsAsset(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	const FString& PackagePath,
	const FString& AssetName)
{
	if (!WorldContextObject || !Animation || PackagePath.IsEmpty() || AssetName.IsEmpty())
	{
		return false;
	}

	const FString FullPath = PackagePath + TEXT("/") + AssetName;
	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		return false;
	}

	Animation->Rename(*AssetName, Package);
	Animation->SetFlags(RF_Public | RF_Standalone);
	Animation->MarkPackageDirty();

	return true;
}

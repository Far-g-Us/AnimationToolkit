#include "AnimationAdvancedLibrary.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"
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
	if (!WorldContextObject || !Skeleton || !BaseAnimation || !AdditiveAnimation)
	{
		return nullptr;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 FrameCount = BaseAnimation->GetNumberOfSampledKeys();
	float Duration = BaseAnimation->GetSequenceLength();

	FString NewName = AnimationName.IsEmpty() ?
		FString::Printf(TEXT("%s_Additive"), *BaseAnimation->GetName()) : AnimationName;

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);

		int32 BaseBoneIdx = BaseAnimation->GetSkeleton() ?
			BaseAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName) : INDEX_NONE;
		int32 AddBoneIdx = AdditiveAnimation->GetSkeleton() ?
			AdditiveAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName) : INDEX_NONE;

		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform BaseTransform = (BaseBoneIdx != INDEX_NONE) ?
				BaseAnimation->GetBoneTransform(BaseBoneIdx, Frame) : FTransform::Identity;

			FTransform AddTransform = (AddBoneIdx != INDEX_NONE) ?
				AdditiveAnimation->GetBoneTransform(AddBoneIdx, Frame) : FTransform::Identity;

			FVector BasePos = BaseTransform.GetLocation();
			FQuat BaseRot = BaseTransform.GetRotation();
			FVector BaseScale = BaseTransform.GetScale3D();

			FVector AddPos = AddTransform.GetLocation();
			FQuat AddRot = AddTransform.GetRotation();
			FVector AddScale = AddTransform.GetScale3D();

			FVector BlendedPos = BasePos + AddPos * AdditiveWeight;
			FQuat BlendedRot = FQuat::Slerp(BaseRot, BaseRot * AddRot, AdditiveWeight);
			FVector BlendedScale = FMath::Lerp(BaseScale, BaseScale * AddScale, AdditiveWeight);

			DestTrack.PosKeys[Frame] = BlendedPos;
			DestTrack.RotKeys[Frame] = BlendedRot;
			DestTrack.ScaleKeys[Frame] = BlendedScale;
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
}

UAnimSequence* UAnimationAdvancedLibrary::ApplyAdditiveLayer(
	UObject* WorldContextObject,
	UAnimSequence* BaseAnimation,
	UAnimSequence* AdditiveLayer,
	float LayerWeight,
	const TArray<FName>& AffectedBones)
{
	if (!WorldContextObject || !BaseAnimation || !AdditiveLayer)
	{
		return nullptr;
	}

	USkeleton* Skeleton = BaseAnimation->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 FrameCount = BaseAnimation->GetNumberOfSampledKeys();

	FString NewName = FString::Printf(TEXT("%s_Layered"), *BaseAnimation->GetName());

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);

		bool bAffected = AffectedBones.Num() == 0 || AffectedBones.Contains(BoneName);

		int32 BaseBoneIdx = BaseAnimation->GetSkeleton() ?
			BaseAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName) : INDEX_NONE;
		int32 AddBoneIdx = AdditiveLayer->GetSkeleton() ?
			AdditiveLayer->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName) : INDEX_NONE;

		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform BaseTransform = (BaseBoneIdx != INDEX_NONE) ?
				BaseAnimation->GetBoneTransform(BaseBoneIdx, Frame) : FTransform::Identity;

			if (bAffected && AddBoneIdx != INDEX_NONE)
			{
				FTransform AddTransform = AdditiveLayer->GetBoneTransform(AddBoneIdx, Frame);

				FVector BasePos = BaseTransform.GetLocation();
				FQuat BaseRot = BaseTransform.GetRotation();
				FVector BaseScale = BaseTransform.GetScale3D();

				FVector AddPos = AddTransform.GetLocation();
				FQuat AddRot = AddTransform.GetRotation();
				FVector AddScale = AddTransform.GetScale3D();

				DestTrack.PosKeys[Frame] = BasePos + AddPos * LayerWeight;
				DestTrack.RotKeys[Frame] = FQuat::Slerp(BaseRot, BaseRot * AddRot, LayerWeight);
				DestTrack.ScaleKeys[Frame] = FMath::Lerp(BaseScale, BaseScale * AddScale, LayerWeight);
			}
			else
			{
				DestTrack.PosKeys[Frame] = BaseTransform.GetLocation();
				DestTrack.RotKeys[Frame] = BaseTransform.GetRotation();
				DestTrack.ScaleKeys[Frame] = BaseTransform.GetScale3D();
			}
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
}

bool UAnimationAdvancedLibrary::ExtractRootMotion(
	UAnimSequence* Animation,
	TArray<FVector>& OutPositions,
	TArray<FRotator>& OutRotations,
	TArray<float>& OutTimes)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Animation->GetSkeleton()->GetReferenceSkeleton();
	int32 RootIdx = RefSkeleton.FindBoneIndex(FName("root"));
	if (RootIdx == INDEX_NONE) RootIdx = 0;

	if (RootIdx >= Animation->GetEditableRawAnimationData().Tracks.Num())
	{
		return false;
	}

	const FRawAnimSequenceTrack& RootTrack = Animation->GetRawAnimationData().Tracks[RootIdx];
	int32 NumKeys = RootTrack.PosKeys.Num();

	OutPositions.SetNum(NumKeys);
	OutRotations.SetNum(NumKeys);
	OutTimes.SetNum(NumKeys);

	float Duration = Animation->GetSequenceLength();

	for (int32 i = 0; i < NumKeys; i++)
	{
		OutPositions[i] = RootTrack.PosKeys[i];
		OutRotations[i] = RootTrack.RotKeys[i].Rotator();
		OutTimes[i] = (NumKeys > 1) ? (float)i / (NumKeys - 1) * Duration : 0.0f;
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

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 RootIdx = RefSkeleton.FindBoneIndex(FName("root"));
	if (RootIdx == INDEX_NONE) RootIdx = 0;

	FString NewName = FString::Printf(TEXT("%s_RootApplied"), *SourceAnimation->GetName());

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);

		int32 SourceBoneIdx = SourceAnimation->GetSkeleton() ?
			SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName) : INDEX_NONE;

		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform SourceTransform = (SourceBoneIdx != INDEX_NONE) ?
				SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame) : FTransform::Identity;

			if (BoneIdx == RootIdx && Frame < RootPositions.Num())
			{
				DestTrack.PosKeys[Frame] = RootPositions[Frame];
				DestTrack.RotKeys[Frame] = (Frame < RootRotations.Num()) ?
					RootRotations[Frame].Quaternion() : SourceTransform.GetRotation();
			}
			else
			{
				DestTrack.PosKeys[Frame] = SourceTransform.GetLocation();
				DestTrack.RotKeys[Frame] = SourceTransform.GetRotation();
			}

			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
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

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return FTransform::Identity;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 BoneIdx = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIdx == INDEX_NONE)
	{
		return FTransform::Identity;
	}

	int32 NumFrames = Animation->GetNumberOfSampledKeys();
	float Duration = Animation->GetSequenceLength();

	if (NumFrames <= 1 || Duration <= 0.0f)
	{
		return Animation->GetBoneTransform(BoneIdx, 0);
	}

	float NormalizedTime = FMath::Clamp(Time / Duration, 0.0f, 1.0f);
	float FloatFrame = NormalizedTime * (NumFrames - 1);
	int32 Frame0 = FMath::Clamp((int32)FloatFrame, 0, NumFrames - 2);
	int32 Frame1 = Frame0 + 1;
	float Alpha = FloatFrame - Frame0;

	FTransform Transform0 = Animation->GetBoneTransform(BoneIdx, Frame0);
	FTransform Transform1 = Animation->GetBoneTransform(BoneIdx, Frame1);

	if (bInterpolate)
	{
		return FTransform::Blend(Transform0, Transform1, Alpha);
	}

	return Transform0;
}

TArray<FName> UAnimationAdvancedLibrary::GetAnimationBoneNames(UAnimSequence* Animation)
{
	TArray<FName> Result;

	if (!Animation || !Animation->GetSkeleton())
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkeleton = Animation->GetSkeleton()->GetReferenceSkeleton();
	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
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

	return Animation->GetSequenceLength();
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
	if (!Animation)
	{
		return false;
	}

	FString FullPath = PackagePath / AssetName;
	UPackage* Package = CreatePackage(nullptr, *FullPath);
	if (!Package)
	{
		return false;
	}

	UAnimSequence* SavedAnim = DuplicateObject<UAnimSequence>(Animation, Package, *AssetName);
	if (!SavedAnim)
	{
		return false;
	}

	SavedAnim->SetFlags(RF_Public | RF_Standalone);
	Package->MarkPackageDirty();

	FString PackageFileName = FPackageName::LongPackageNameToFilename(FullPath, FPackageName::GetAssetExtension());

	if (!UPackage::SavePackage(Package, nullptr, *PackageFileName, FOutputDeviceNull::Get()))
	{
		return false;
	}

	return true;
}

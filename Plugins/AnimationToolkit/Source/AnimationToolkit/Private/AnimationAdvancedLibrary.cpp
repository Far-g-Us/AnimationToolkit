#include "AnimationAdvancedLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimData/IAnimationDataController.h"

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

	const IAnimationDataModel* BaseModel = BaseAnimation->GetDataModel();
	const IAnimationDataModel* AdditiveModel = AdditiveAnimation->GetDataModel();
	if (!BaseModel || !AdditiveModel)
	{
		return nullptr;
	}

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

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();
	const float Duration = FMath::Max(BaseAnimation->GetPlayLength(), AdditiveAnimation->GetPlayLength());
	const int32 NumFrames = FMath::Max(BaseModel->GetNumberOfFrames(), AdditiveModel->GetNumberOfFrames());

	IAnimationDataController& ResultController = Result->GetController();

	const TArray<FBoneAnimationTrack>& BaseTracks = BaseModel->GetBoneAnimationTracks();
	const TArray<FBoneAnimationTrack>& AddTracks = AdditiveModel->GetBoneAnimationTracks();

	for (int32 BoneIdx = 0; BoneIdx < NumBones; ++BoneIdx)
	{
		const FName BoneName = RefSkeleton.GetBoneName(BoneIdx);

		FRawAnimSequenceTrack BaseTrack;
		bool bHasBase = false;
		for (const FBoneAnimationTrack& T : BaseTracks)
		{
			if (T.Name == BoneName)
			{
				BaseTrack = T.InternalTrackData;
				bHasBase = true;
				break;
			}
		}

		FRawAnimSequenceTrack AdditiveTrack;
		bool bHasAdditive = false;
		for (const FBoneAnimationTrack& T : AddTracks)
		{
			if (T.Name == BoneName)
			{
				AdditiveTrack = T.InternalTrackData;
				bHasAdditive = true;
				break;
			}
		}

		if (!bHasBase)
		{
			continue;
		}

		FRawAnimSequenceTrack ResultTrack;
		const int32 TrackNumKeys = BaseTrack.PosKeys.Num();

		for (int32 KeyIdx = 0; KeyIdx < TrackNumKeys; ++KeyIdx)
		{
			FVector3f BasePos = BaseTrack.PosKeys[KeyIdx];
			FQuat4f BaseRot = BaseTrack.RotKeys[KeyIdx];
			FVector3f BaseScale = (KeyIdx < BaseTrack.ScaleKeys.Num()) ? BaseTrack.ScaleKeys[KeyIdx] : FVector3f(1.0f);

			FVector3f AddPos = FVector3f::ZeroVector;
			FQuat4f AddRot = FQuat4f::Identity;
			FVector3f AddScale = FVector3f::ZeroVector;

			if (bHasAdditive && AdditiveTrack.PosKeys.Num() > 0)
			{
				const int32 AddIdx = FMath::Min(KeyIdx, AdditiveTrack.PosKeys.Num() - 1);
				AddPos = AdditiveTrack.PosKeys[AddIdx];
				AddRot = (AddIdx < AdditiveTrack.RotKeys.Num()) ? AdditiveTrack.RotKeys[AddIdx] : FQuat4f::Identity;
				AddScale = (AddIdx < AdditiveTrack.ScaleKeys.Num()) ? AdditiveTrack.ScaleKeys[AddIdx] : FVector3f::ZeroVector;
			}

			ResultTrack.PosKeys.Add(BasePos + FVector3f(FVector(AddPos) * AdditiveWeight));
			ResultTrack.RotKeys.Add(BaseRot);
			ResultTrack.ScaleKeys.Add(BaseScale);
		}

		ResultController.AddBoneCurve(BoneName);
		ResultController.SetBoneTrackKeys(BoneName, ResultTrack.PosKeys, ResultTrack.RotKeys, ResultTrack.ScaleKeys);
	}

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

	const IAnimationDataModel* BaseModel = BaseAnimation->GetDataModel();
	const IAnimationDataModel* LayerModel = AdditiveLayer->GetDataModel();
	if (!BaseModel || !LayerModel)
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

	const FReferenceSkeleton& RefSkeleton = BaseSkeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();

	TSet<FName> AffectedSet(AffectedBones);

	IAnimationDataController& ResultController = Result->GetController();

	const TArray<FBoneAnimationTrack>& BaseTracks = BaseModel->GetBoneAnimationTracks();
	const TArray<FBoneAnimationTrack>& LayerTracks = LayerModel->GetBoneAnimationTracks();

	for (int32 BoneIdx = 0; BoneIdx < NumBones; ++BoneIdx)
	{
		const FName BoneName = RefSkeleton.GetBoneName(BoneIdx);

		FRawAnimSequenceTrack BaseTrack;
		bool bHasBase = false;
		for (const FBoneAnimationTrack& T : BaseTracks)
		{
			if (T.Name == BoneName)
			{
				BaseTrack = T.InternalTrackData;
				bHasBase = true;
				break;
			}
		}

		FRawAnimSequenceTrack LayerTrack;
		bool bHasLayer = false;
		for (const FBoneAnimationTrack& T : LayerTracks)
		{
			if (T.Name == BoneName)
			{
				LayerTrack = T.InternalTrackData;
				bHasLayer = true;
				break;
			}
		}

		if (!bHasBase)
		{
			continue;
		}

		const bool bIsAffected = AffectedSet.Num() == 0 || AffectedSet.Contains(BoneName);

		FRawAnimSequenceTrack ResultTrack;
		const int32 TrackNumKeys = BaseTrack.PosKeys.Num();

		for (int32 KeyIdx = 0; KeyIdx < TrackNumKeys; ++KeyIdx)
		{
			FVector3f Pos = BaseTrack.PosKeys[KeyIdx];
			FQuat4f Rot = BaseTrack.RotKeys[KeyIdx];
			FVector3f Scale = (KeyIdx < BaseTrack.ScaleKeys.Num()) ? BaseTrack.ScaleKeys[KeyIdx] : FVector3f(1.0f);

			if (bIsAffected && bHasLayer && LayerTrack.PosKeys.Num() > 0)
			{
				const int32 LayerIdx = FMath::Min(KeyIdx, LayerTrack.PosKeys.Num() - 1);
				const FVector3f LayerPos = LayerTrack.PosKeys[LayerIdx];
				const FQuat4f LayerRot = (LayerIdx < LayerTrack.RotKeys.Num()) ? LayerTrack.RotKeys[LayerIdx] : FQuat4f::Identity;

				Pos = Pos + FVector3f(FVector(LayerPos) * LayerWeight);
				Rot = FQuat4f(FQuat::Slerp(FQuat(Rot), FQuat(LayerRot), LayerWeight));
			}

			ResultTrack.PosKeys.Add(Pos);
			ResultTrack.RotKeys.Add(Rot);
			ResultTrack.ScaleKeys.Add(Scale);
		}

		ResultController.AddBoneCurve(BoneName);
		ResultController.SetBoneTrackKeys(BoneName, ResultTrack.PosKeys, ResultTrack.RotKeys, ResultTrack.ScaleKeys);
	}

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

	const IAnimationDataModel* DataModel = Animation->GetDataModel();
	if (!DataModel)
	{
		return false;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return false;
	}

	const FName RootBoneName = FName("root");
	const int32 RootBoneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(RootBoneName);
	if (RootBoneIndex == INDEX_NONE)
	{
		return false;
	}

	FRawAnimSequenceTrack RootTrack;
	bool bFound = false;
	for (const FBoneAnimationTrack& T : DataModel->GetBoneAnimationTracks())
	{
		if (T.Name == RootBoneName)
		{
			RootTrack = T.InternalTrackData;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		return false;
	}

	const int32 NumPosKeys = RootTrack.PosKeys.Num();
	const int32 NumRotKeys = RootTrack.RotKeys.Num();
	const int32 NumKeys = FMath::Max(NumPosKeys, NumRotKeys);

	if (NumKeys <= 0)
	{
		return false;
	}

	const float Duration = Animation->GetPlayLength();
	const int32 NumFrames = DataModel->GetNumberOfFrames();

	for (int32 KeyIdx = 0; KeyIdx < NumKeys; ++KeyIdx)
	{
		const float Time = (NumKeys > 1) ? (static_cast<float>(KeyIdx) / (NumKeys - 1) * Duration) : 0.0f;

		FVector Pos = FVector::ZeroVector;
		if (KeyIdx < NumPosKeys)
		{
			Pos = FVector(RootTrack.PosKeys[KeyIdx]);
		}

		FRotator Rot = FRotator::ZeroRotator;
		if (KeyIdx < NumRotKeys)
		{
			Rot = FQuat(RootTrack.RotKeys[KeyIdx]).Rotator();
		}

		OutPositions.Add(Pos);
		OutRotations.Add(Rot);
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

	const IAnimationDataModel* DataModel = Animation->GetDataModel();
	if (!DataModel)
	{
		return 0.0f;
	}

	const TArray<FFloatCurve>& FloatCurves = DataModel->GetFloatCurves();

	for (const FFloatCurve& Curve : FloatCurves)
	{
		if (Curve.GetName() == CurveName)
		{
			const float Duration = Animation->GetPlayLength();
			const float ClampedTime = FMath::Clamp(Time, 0.0f, Duration);
			return Curve.FloatCurve.Eval(ClampedTime);
		}
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

	const IAnimationDataModel* DataModel = Animation->GetDataModel();
	if (!DataModel)
	{
		return Result;
	}

	const TArray<FFloatCurve>& FloatCurves = DataModel->GetFloatCurves();

	for (const FFloatCurve& Curve : FloatCurves)
	{
		if (Curve.GetName() == CurveName)
		{
			const float Duration = Animation->GetPlayLength();
			const int32 NumFrames = DataModel->GetNumberOfFrames();

			if (NumFrames <= 1)
			{
				FAnimationCurveKeyframe KF;
				KF.Time = 0.0f;
				KF.Value = Curve.FloatCurve.Eval(0.0f);
				Result.Add(KF);
				return Result;
			}

			for (int32 Frame = 0; Frame < NumFrames; ++Frame)
			{
				const float Time = (static_cast<float>(Frame) / (NumFrames - 1)) * Duration;
				FAnimationCurveKeyframe KF;
				KF.Time = Time;
				KF.Value = Curve.FloatCurve.Eval(Time);
				Result.Add(KF);
			}

			return Result;
		}
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

	const IAnimationDataModel* DataModel = Animation->GetDataModel();
	if (!DataModel)
	{
		return FTransform::Identity;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return FTransform::Identity;
	}

	FRawAnimSequenceTrack Track;
	bool bFound = false;
	for (const FBoneAnimationTrack& T : DataModel->GetBoneAnimationTracks())
	{
		if (T.Name == BoneName)
		{
			Track = T.InternalTrackData;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		return FTransform::Identity;
	}

	const int32 NumPosKeys = Track.PosKeys.Num();
	const int32 NumRotKeys = Track.RotKeys.Num();
	const int32 NumScaleKeys = Track.ScaleKeys.Num();

	if (NumPosKeys == 0 && NumRotKeys == 0)
	{
		return FTransform::Identity;
	}

	const float Duration = Animation->GetPlayLength();
	const float ClampedTime = FMath::Clamp(Time, 0.0f, Duration);
	const float Alpha = (Duration > 0.0f) ? (ClampedTime / Duration) : 0.0f;

	const bool bInterp = bInterpolate;

	auto InterpolateVector = [&bInterp](const TArray<FVector3f>& Keys, float InAlpha) -> FVector3f
	{
		if (Keys.Num() == 0) return FVector3f::ZeroVector;
		if (Keys.Num() == 1 || !bInterp) return Keys[0];

		const float ScaledAlpha = InAlpha * (Keys.Num() - 1);
		const int32 Index = FMath::Clamp(FMath::FloorToInt(ScaledAlpha), 0, Keys.Num() - 2);
		const float Frac = ScaledAlpha - Index;

		return FMath::Lerp(Keys[Index], Keys[Index + 1], Frac);
	};

	auto InterpolateQuat = [&bInterp](const TArray<FQuat4f>& Keys, float InAlpha) -> FQuat4f
	{
		if (Keys.Num() == 0) return FQuat4f::Identity;
		if (Keys.Num() == 1 || !bInterp) return Keys[0];

		const float ScaledAlpha = InAlpha * (Keys.Num() - 1);
		const int32 Index = FMath::Clamp(FMath::FloorToInt(ScaledAlpha), 0, Keys.Num() - 2);
		const float Frac = ScaledAlpha - Index;

		FQuat A(Keys[Index]);
		FQuat B(Keys[Index + 1]);
		return FQuat4f(FQuat::Slerp(A, B, Frac));
	};

	const FVector3f Pos = InterpolateVector(Track.PosKeys, Alpha);
	const FQuat4f Rot = InterpolateQuat(Track.RotKeys, Alpha);

	FVector3f Scale = FVector3f(1.0f);
	if (NumScaleKeys > 0)
	{
		Scale = InterpolateVector(Track.ScaleKeys, Alpha);
	}

	return FTransform(FQuat(Rot), FVector(Pos), FVector(Scale));
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

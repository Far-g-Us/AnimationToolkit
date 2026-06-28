#include "AnimationLODGenerator.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"

FAnimationLODSettings UAnimationLODGenerator::GetDefaultLODSettings()
{
	FAnimationLODSettings Settings;
	return Settings;
}

TArray<UAnimSequence*> UAnimationLODGenerator::GenerateAnimationLODs(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const FAnimationLODSettings& Settings,
	int32 NumLODs)
{
	TArray<UAnimSequence*> LODs;

	if (!WorldContextObject || !SourceAnimation)
	{
		return LODs;
	}

	for (int32 LODLevel = 0; LODLevel < NumLODs; LODLevel++)
	{
		UAnimSequence* LODAnim = GenerateLOD(WorldContextObject, SourceAnimation, LODLevel, Settings);
		if (LODAnim)
		{
			LODs.Add(LODAnim);
		}
	}

	return LODs;
}

UAnimSequence* UAnimationLODGenerator::GenerateLOD(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	int32 LODLevel,
	const FAnimationLODSettings& Settings)
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
	const int32 TotalBones = RefSkeleton.GetNum();
	if (TotalBones == 0)
	{
		return nullptr;
	}

	const int32 TargetBoneCount = GetBoneCountForLOD(SourceAnimation, LODLevel, Settings);
	const float TranslationThreshold = (LODLevel == 0) ? Settings.LOD0TranslationThreshold :
		(LODLevel == 1) ? Settings.LOD1TranslationThreshold : Settings.LOD2TranslationThreshold;
	const float RotationThreshold = (LODLevel == 0) ? Settings.LOD0RotationThreshold :
		(LODLevel == 1) ? Settings.LOD1RotationThreshold : Settings.LOD2RotationThreshold;

	const FString LODName = FString::Printf(TEXT("%s_LOD%d"), *SourceAnimation->GetName(), LODLevel);
	const FString PackagePath = TEXT("/Game/Animations/") + LODName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UAnimSequence* LODAnim = NewObject<UAnimSequence>(Package, *LODName, RF_Public | RF_Standalone);
	if (!LODAnim)
	{
		return nullptr;
	}

	LODAnim->SetSkeleton(Skeleton);

	IAnimationDataController& Controller = LODAnim->GetController();
	const TScriptInterface<IAnimationDataModel> Model = Controller.GetModelInterface();

	if (Model.GetObject() == nullptr)
	{
		return nullptr;
	}

	Controller.OpenBracket(FText::FromString(FString::Printf(TEXT("Generating LOD%d for %s"), LODLevel, *SourceAnimation->GetName())), false);

	const FFrameRate FrameRate = Model->GetFrameRate();
	const FFrameRate SrcFrameRate = SourceAnimation->GetDataModel()->GetFrameRate();
	const int32 SrcNumFrames = SourceAnimation->GetNumberOfSampledKeys();
	const float PlayLength = SourceAnimation->GetPlayLength();
	const int32 DstNumFrames = FMath::Max(1, FMath::CeilToInt(PlayLength * FrameRate.AsDecimal()));

	Controller.SetNumberOfFrames(FFrameNumber(DstNumFrames), false);

	const int32 BonesToKeep = FMath::Min(TotalBones, TargetBoneCount);
	const TArray<FBoneAnimationTrack>& SrcTracks = SourceAnimation->GetDataModel()->GetBoneAnimationTracks();

	for (int32 BoneIdx = 0; BoneIdx < BonesToKeep; ++BoneIdx)
	{
		if (BoneIdx >= SrcTracks.Num())
		{
			break;
		}

		const FBoneAnimationTrack& SrcTrack = SrcTracks[BoneIdx];
		FName BoneName = SrcTrack.Name;

		Controller.AddBoneCurve(BoneName, false);

		const FRawAnimSequenceTrack& SrcRaw = SrcTrack.InternalTrackData;

		FRawAnimSequenceTrack ReducedTrack;
		ReducedTrack.PosKeys.SetNum(DstNumFrames);
		ReducedTrack.RotKeys.SetNum(DstNumFrames);
		ReducedTrack.ScaleKeys.SetNum(DstNumFrames);

		for (int32 DstFrame = 0; DstFrame < DstNumFrames; ++DstFrame)
		{
			const float DstTime = (DstNumFrames > 1)
				? static_cast<float>(DstFrame) / static_cast<float>(DstNumFrames - 1) * PlayLength
				: 0.0f;

			const float NormalizedSrcTime = (PlayLength > 0.0f)
				? FMath::Clamp(DstTime / PlayLength, 0.0f, 1.0f)
				: 0.0f;
			const float ScaledSrcFrame = NormalizedSrcTime * static_cast<float>(SrcNumFrames - 1);
			const int32 SrcFrameA = FMath::FloorToInt(ScaledSrcFrame);
			const int32 SrcFrameB = FMath::Min(SrcFrameA + 1, SrcNumFrames - 1);
			const float Alpha = ScaledSrcFrame - static_cast<float>(SrcFrameA);

			const FVector3f PosA = (SrcFrameA < SrcRaw.PosKeys.Num()) ? SrcRaw.PosKeys[SrcFrameA] : FVector3f::ZeroVector;
			const FVector3f PosB = (SrcFrameB < SrcRaw.PosKeys.Num()) ? SrcRaw.PosKeys[SrcFrameB] : FVector3f::ZeroVector;

			const FQuat4f RotA = (SrcFrameA < SrcRaw.RotKeys.Num()) ? SrcRaw.RotKeys[SrcFrameA] : FQuat4f::Identity;
			const FQuat4f RotB = (SrcFrameB < SrcRaw.RotKeys.Num()) ? SrcRaw.RotKeys[SrcFrameB] : FQuat4f::Identity;

			const FVector3f ScaleA = (SrcFrameA < SrcRaw.ScaleKeys.Num()) ? SrcRaw.ScaleKeys[SrcFrameA] : FVector3f::OneVector;
			const FVector3f ScaleB = (SrcFrameB < SrcRaw.ScaleKeys.Num()) ? SrcRaw.ScaleKeys[SrcFrameB] : FVector3f::OneVector;

			ReducedTrack.PosKeys[DstFrame] = FMath::Lerp(PosA, PosB, Alpha);
			ReducedTrack.RotKeys[DstFrame] = FQuat4f::Slerp(RotA, RotB, Alpha);
			ReducedTrack.ScaleKeys[DstFrame] = FMath::Lerp(ScaleA, ScaleB, Alpha);
		}

		const int32 OrigPosCount = SrcRaw.PosKeys.Num();
		const int32 OrigRotCount = SrcRaw.RotKeys.Num();
		const bool bPosReduced = (OrigPosCount > DstNumFrames) &&
			!FMath::IsNearlyZero(TranslationThreshold);
		const bool bRotReduced = (OrigRotCount > DstNumFrames) &&
			!FMath::IsNearlyZero(RotationThreshold);

		if (bPosReduced && LODLevel > 0)
		{
			ReducedTrack.PosKeys.SetNum(DstNumFrames);
		}
		else if (!bPosReduced && OrigPosCount == DstNumFrames)
		{
			ReducedTrack.PosKeys = SrcRaw.PosKeys;
		}

		if (bRotReduced && LODLevel > 0)
		{
			ReducedTrack.RotKeys.SetNum(DstNumFrames);
		}
		else if (!bRotReduced && OrigRotCount == DstNumFrames)
		{
			ReducedTrack.RotKeys = SrcRaw.RotKeys;
		}

		Controller.SetBoneTrackKeys(
			BoneName,
			ReducedTrack.PosKeys,
			ReducedTrack.RotKeys,
			ReducedTrack.ScaleKeys,
			false);
	}

	Controller.CloseBracket(false);

	LODAnim->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(LODAnim);

	UE_LOG(LogTemp, Log, TEXT("GenerateLOD: Created LOD%d '%s' (%d bones, %d frames) from '%s'"),
		LODLevel, *LODName, BonesToKeep, DstNumFrames, *SourceAnimation->GetName());

	return LODAnim;
}

int32 UAnimationLODGenerator::GetBoneCountForLOD(
	UAnimSequence* Animation,
	int32 LODLevel,
	const FAnimationLODSettings& Settings)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		return 0;
	}

	int32 TotalBones = Animation->GetSkeleton()->GetReferenceSkeleton().GetNum();

	if (LODLevel == 0) return TotalBones;
	if (LODLevel == 1) return FMath::Min(TotalBones, Settings.LOD1BoneThreshold);
	if (LODLevel == 2) return FMath::Min(TotalBones, Settings.LOD2BoneThreshold);

	return FMath::Min(TotalBones, Settings.LOD2BoneThreshold);
}

float UAnimationLODGenerator::EstimateLODMemoryUsage(
	UAnimSequence* Animation,
	int32 LODLevel,
	const FAnimationLODSettings& Settings)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		return 0.0f;
	}

	int32 BoneCount = GetBoneCountForLOD(Animation, LODLevel, Settings);
	int32 FrameCount = Animation->GetNumberOfSampledKeys();

	return (float)(BoneCount * FrameCount * (sizeof(FVector) + sizeof(FQuat) + sizeof(FVector)));
}

void UAnimationLODGenerator::BatchGenerateLODs(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	const FAnimationLODSettings& Settings,
	int32 NumLODs)
{
	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			GenerateAnimationLODs(WorldContextObject, Anim, Settings, NumLODs);
		}
	}
}

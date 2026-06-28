#include "AnimationFactory.h"
#include "AnimSequenceCreator.h"
#include "AnimMontageCreator.h"
#include "AnimBlendSpaceCreator.h"
#include "AnimationImporter.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"

TWeakObjectPtr<UAnimationFactory> UAnimationFactory::Instance = nullptr;

UAnimSequence* UAnimationFactory::CreateAnimSequence(
	UObject* WorldContextObject,
	const FString& SequenceName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	float Duration,
	float FrameRate)
{
	if (!Skeleton || SequenceName.IsEmpty())
	{
		return nullptr;
	}

	const FString PackagePath = TEXT("/Game/Animations/") + SequenceName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UAnimSequence* AnimSequence = NewObject<UAnimSequence>(Package, *SequenceName, RF_Public | RF_Standalone);
	if (!AnimSequence)
	{
		return nullptr;
	}

	AnimSequence->SetSkeleton(Skeleton);
	AnimSequence->SetPreviewMesh(nullptr);

	IAnimationDataController& Controller = AnimSequence->GetController();

	const float SafeFrameRate = FMath::Max(0.01f, FrameRate);

	for (const FAnimationTrackData& Track : Tracks)
	{
		Controller.AddBoneCurve(Track.TrackName, false);
		FRawAnimSequenceTrack RawTrack;
		const int32 NumFrames = FMath::Max(1, FMath::RoundToInt(Duration * SafeFrameRate));

		RawTrack.PosKeys.SetNum(NumFrames);
		RawTrack.RotKeys.SetNum(NumFrames);
		RawTrack.ScaleKeys.SetNum(NumFrames);

		for (int32 i = 0; i < NumFrames; ++i)
		{
			const float Time = (SafeFrameRate > 0.0f) ? (static_cast<float>(i) / SafeFrameRate) : 0.0f;

			FTransform InterpolatedTransform = FTransform::Identity;
			if (Track.Keyframes.Num() > 0)
			{
				if (Track.Keyframes.Num() == 1)
				{
					InterpolatedTransform = Track.Keyframes[0].Transform;
				}
				else
				{
					const float SafeDuration = FMath::Max(0.001f, Duration);
					const float NormalizedTime = FMath::Clamp(Time / SafeDuration, 0.0f, 1.0f);
					const float ScaledTime = NormalizedTime * (Track.Keyframes.Num() - 1);
					const int32 IndexA = FMath::FloorToInt(ScaledTime);
					const int32 IndexB = FMath::Min(IndexA + 1, Track.Keyframes.Num() - 1);
					const float Alpha = ScaledTime - static_cast<float>(IndexA);

					InterpolatedTransform = Track.Keyframes[IndexA].Transform;
					InterpolatedTransform.BlendWith(Track.Keyframes[IndexB].Transform, Alpha);
				}
			}

			RawTrack.PosKeys[i] = FVector3f(InterpolatedTransform.GetTranslation());
			RawTrack.RotKeys[i] = FQuat4f(InterpolatedTransform.GetRotation());
			RawTrack.ScaleKeys[i] = FVector3f(InterpolatedTransform.GetScale3D());
		}

		Controller.SetBoneTrackKeys(Track.TrackName, RawTrack.PosKeys, RawTrack.RotKeys, RawTrack.ScaleKeys, false);
	}

	AnimSequence->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(AnimSequence);

	return AnimSequence;
}

UAnimMontage* UAnimationFactory::CreateAnimMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections)
{
	if (!Skeleton || MontageName.IsEmpty())
	{
		return nullptr;
	}

	const FString PackagePath = TEXT("/Game/Animations/") + MontageName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UAnimMontage* Montage = NewObject<UAnimMontage>(Package, *MontageName, RF_Public | RF_Standalone);
	if (!Montage)
	{
		return nullptr;
	}

	Montage->SetSkeleton(Skeleton);

	for (const FAnimMontageSectionData& SectionData : Sections)
	{
		FSlotAnimationTrack SlotTrack;
		SlotTrack.SlotName = SectionData.SectionName;

		FAnimSegment Segment;
		Segment.SetAnimReference(SectionData.Animation);
		Segment.StartPos = SectionData.StartTime;
		Segment.AnimEndTime = SectionData.EndTime;
		Segment.AnimPlayRate = 1.0f;

		SlotTrack.AnimTrack.AnimSegments.Add(Segment);
		Montage->SlotAnimTracks.Add(SlotTrack);
	}

	Montage->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Montage);

	return Montage;
}

UBlendSpace* UAnimationFactory::CreateBlendSpace1D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinValue,
	float MaxValue)
{
	if (!Skeleton || BlendSpaceName.IsEmpty())
	{
		return nullptr;
	}

	const FString PackagePath = TEXT("/Game/Animations/") + BlendSpaceName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UBlendSpace* BlendSpace = NewObject<UBlendSpace>(Package, *BlendSpaceName, RF_Public | RF_Standalone);
	if (!BlendSpace)
	{
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

	for (const FBlendSpaceSampleData& Sample : Samples)
	{
		BlendSpace->AddSample(Sample.Animation, FVector(Sample.SampleValue.X, 0.0f, 0.0f));
	}

	BlendSpace->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(BlendSpace);

	return BlendSpace;
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
	if (!Skeleton || BlendSpaceName.IsEmpty())
	{
		return nullptr;
	}

	const FString PackagePath = TEXT("/Game/Animations/") + BlendSpaceName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		return nullptr;
	}

	UBlendSpace* BlendSpace = NewObject<UBlendSpace>(Package, *BlendSpaceName, RF_Public | RF_Standalone);
	if (!BlendSpace)
	{
		return nullptr;
	}

	BlendSpace->SetSkeleton(Skeleton);

	for (const FBlendSpaceSampleData& Sample : Samples)
	{
		BlendSpace->AddSample(Sample.Animation, Sample.SampleValue);
	}

	BlendSpace->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(BlendSpace);

	return BlendSpace;
}

FAnimationImportResult UAnimationFactory::ImportFromFile(
	UObject* WorldContextObject,
	const FString& FilePath,
	EAnimationImportFormat Format,
	USkeleton* Skeleton)
{
	FAnimationImportResult Result;

	if (!FPaths::FileExists(FilePath))
	{
		Result.ErrorMessage = TEXT("File not found: ") + FilePath;
		return Result;
	}

	if (!Skeleton)
	{
		Result.ErrorMessage = TEXT("Invalid skeleton");
		return Result;
	}

	Result = UAnimationImporter::ImportAnimation(WorldContextObject, FilePath, Format, Skeleton);
	return Result;
}

UAnimSequence* UAnimationFactory::ImportAndCreate(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	if (!FPaths::FileExists(FilePath) || !Skeleton || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimationImporter::ImportAndCreateAnimSequence(
		WorldContextObject, FilePath, AnimationName, Skeleton, Format);
}

UAnimationFactory* UAnimationFactory::GetInstance()
{
	if (!Instance.IsValid())
	{
		Instance = NewObject<UAnimationFactory>();
		Instance->AddToRoot();
	}
	return Instance.Get();
}

bool UAnimationFactory::IsValidSkeleton(USkeleton* Skeleton)
{
	return Skeleton != nullptr && Skeleton->GetReferenceSkeleton().GetNum() > 0;
}

FString UAnimationFactory::GenerateUniqueName(const FString& BaseName, UObject* Package)
{
	if (BaseName.IsEmpty())
	{
		return TEXT("Unnamed_Anim");
	}

	FString UniqueName = BaseName;
	int32 Counter = 1;

	while (FindObject<UObject>(Package ? Package : (UObject*)GetTransientPackage(), *UniqueName))
	{
		UniqueName = FString::Printf(TEXT("%s_%d"), *BaseName, Counter++);
	}

	return UniqueName;
}

UPackage* UAnimationFactory::CreateAnimationPackage(const FString& AssetName)
{
	if (AssetName.IsEmpty())
	{
		return nullptr;
	}

	const FString PackagePath = TEXT("/Game/Animations/") + AssetName;
	return CreatePackage(*PackagePath);
}

#include "AnimationLibrary.h"
#include "AnimSequenceCreator.h"
#include "AnimMontageCreator.h"
#include "AnimBlendSpaceCreator.h"
#include "AnimationImporter.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Skeleton.h"
#include "Engine/Engine.h"

UAnimSequence* UAnimationLibrary::CreateAnimation(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	float Duration,
	float FrameRate)
{
	return UAnimSequenceCreator::CreateAnimSequenceFromKeyframes(
		WorldContextObject,
		AnimationName,
		Skeleton,
		Tracks,
		Duration,
		FrameRate
	);
}

UAnimMontage* UAnimationLibrary::CreateMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections)
{
	return UAnimMontageCreator::CreateAnimMontage(
		WorldContextObject,
		MontageName,
		Skeleton,
		Sections
	);
}

UBlendSpace* UAnimationLibrary::CreateBlendSpace1D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinValue,
	float MaxValue)
{
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

UBlendSpace* UAnimationLibrary::CreateBlendSpace2D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinX,
	float MaxX,
	float MinY,
	float MaxY)
{
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

UAnimSequence* UAnimationLibrary::ImportAnimationAsSequence(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	return UAnimationImporter::ImportAndCreateAnimSequence(
		WorldContextObject,
		FilePath,
		AnimationName,
		Skeleton,
		Format
	);
}

FAnimationImportResult UAnimationLibrary::ImportAnimationFile(
	UObject* WorldContextObject,
	const FString& FilePath,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	return UAnimationImporter::ImportAnimation(
		WorldContextObject,
		FilePath,
		Format,
		Skeleton
	);
}

FAnimationTrackData UAnimationLibrary::CreateTrack(
	const FName& BoneName,
	const TArray<FTransform>& Transforms,
	float FrameRate)
{
	FAnimationTrackData TrackData;
	TrackData.TrackName = BoneName;

	for (int32 i = 0; i < Transforms.Num(); i++)
	{
		FAnimationKeyframe Keyframe;
		Keyframe.Time = (float)i / FrameRate;
		Keyframe.Transform = Transforms[i];
		TrackData.Keyframes.Add(Keyframe);
	}

	return TrackData;
}

FAnimationKeyframe UAnimationLibrary::MakeKeyframe(
	float Time,
	const FTransform& Transform)
{
	return UAnimSequenceCreator::CreateKeyframe(Time, Transform);
}

FAnimMontageSectionData UAnimationLibrary::MakeMontageSection(
	const FName& SectionName,
	UAnimSequence* Animation,
	float StartTime,
	float EndTime)
{
	return UAnimMontageCreator::CreateMontageSectionData(
		SectionName,
		Animation,
		StartTime,
		EndTime
	);
}

FBlendSpaceSampleData UAnimationLibrary::MakeBlendSpaceSample(
	UAnimSequence* Animation,
	const FVector& SampleValue)
{
	return UAnimBlendSpaceCreator::CreateBlendSpaceSample(
		Animation,
		SampleValue
	);
}

EAnimationImportFormat UAnimationLibrary::DetectAnimationFormat(const FString& FilePath)
{
	return UAnimationImporter::DetectFormat(FilePath);
}

bool UAnimationLibrary::IsAnimationFormatSupported(const FString& FilePath)
{
	EAnimationImportFormat Format;
	return UAnimationImporter::IsSupportedFormat(FilePath, Format);
}

FString UAnimationLibrary::GetAnimationFormatName(EAnimationImportFormat Format)
{
	return UAnimationImporter::GetFormatExtension(Format);
}

TArray<EAnimationImportFormat> UAnimationLibrary::GetAllSupportedFormats()
{
	return UAnimationImporter::GetSupportedFormats();
}

TArray<FString> UAnimationLibrary::GetSkeletonBoneNames(USkeleton* Skeleton)
{
	TArray<FString> BoneNames;

	if (Skeleton)
	{
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
		const int32 NumBones = RefSkeleton.GetNum();

		for (int32 i = 0; i < NumBones; i++)
		{
			BoneNames.Add(RefSkeleton.GetBoneName(i).ToString());
		}
	}

	return BoneNames;
}

float UAnimationLibrary::GetAnimSequenceDuration(UAnimSequence* AnimSequence)
{
	if (AnimSequence)
	{
		return AnimSequence->GetSequenceLength();
	}
	return 0.0f;
}

int32 UAnimationLibrary::GetAnimSequenceFrameCount(UAnimSequence* AnimSequence)
{
	if (AnimSequence)
	{
		return AnimSequence->GetNumberOfSampledKeys();
	}
	return 0;
}

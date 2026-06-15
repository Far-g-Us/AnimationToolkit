#include "AnimMontageCreator.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UAnimMontage* UAnimMontageCreator::CreateAnimMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections,
	UPackage* Package)
{
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimMontageCreator: Skeleton is null"));
		return nullptr;
	}

	if (Sections.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimMontageCreator: No sections provided"));
		return nullptr;
	}

	FString AssetPath = FString::Printf(TEXT("/Game/Animations/%s"), *MontageName);
	if (!Package)
	{
		Package = CreatePackage(nullptr, *AssetPath);
	}

	UAnimMontage* Montage = NewObject<UAnimMontage>(
		Package,
		*MontageName,
		RF_Public | RF_Standalone | RF_MarkAsRootSet
	);

	if (!Montage)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimMontageCreator: Failed to create AnimMontage"));
		return nullptr;
	}

	Montage->SetSkeleton(Skeleton);

	float MaxEndTime = 0.0f;
	for (const FAnimMontageSectionData& Section : Sections)
	{
		if (Section.Animation)
		{
			MaxEndTime = FMath::Max(MaxEndTime, Section.EndTime);
		}
	}

	Montage->SetRawNumberOfFrames(FMath::Max(1, FMath::CeilToInt(MaxEndTime * 30.0f)));
	Montage->SetSamplingFrameRate(FFrameRate(30.0f, 1));

	ConfigureMontageSections(Montage, Sections);

	Montage->MarkPackageDirty();
	Montage->PostEditChange();

	return Montage;
}

UAnimMontage* UAnimMontageCreator::CreateAnimMontageFromSequence(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	UAnimSequence* AnimSequence,
	float StartTime,
	float EndTime,
	UPackage* Package)
{
	if (!AnimSequence)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimMontageCreator: Animation is null"));
		return nullptr;
	}

	if (EndTime < 0.0f)
	{
		EndTime = AnimSequence->GetSequenceLength();
	}

	FAnimMontageSectionData SectionData;
	SectionData.SectionName = FName("Default");
	SectionData.Animation = AnimSequence;
	SectionData.StartTime = StartTime;
	SectionData.EndTime = EndTime;

	TArray<FAnimMontageSectionData> Sections;
	Sections.Add(SectionData);

	return CreateAnimMontage(WorldContextObject, MontageName, Skeleton, Sections, Package);
}

FAnimMontageSectionData UAnimMontageCreator::CreateMontageSectionData(
	const FName& SectionName,
	UAnimSequence* Animation,
	float StartTime,
	float EndTime)
{
	FAnimMontageSectionData SectionData;
	SectionData.SectionName = SectionName;
	SectionData.Animation = Animation;
	SectionData.StartTime = StartTime;
	SectionData.EndTime = EndTime;
	return SectionData;
}

bool UAnimMontageCreator::AddSectionToMontage(
	UAnimMontage* Montage,
	const FAnimMontageSectionData& SectionData,
	float StartTime,
	float EndTime)
{
	if (!Montage || !SectionData.Animation)
	{
		return false;
	}

	FAnimMontageSection NewSection;
	NewSection.SetAnimSequence(SectionData.Animation);
	NewSection.SetRangeStart(StartTime);
	NewSection.SetRangeEnd(EndTime);

	Montage->GetSlotAnimTrackForMontage().GetSegmentContainer().Add(FAnimSegment());
	FAnimSegment& Segment = Montage->GetSlotAnimTrackForMontage().GetSegmentContainer().Last();
	Segment.SetAnimReference(SectionData.Animation);
	Segment.SetStartPos(StartTime);
	Segment.SetEndPos(EndTime);
	Segment.SetAnimPlayRate(1.0f);

	Montage->MarkPackageDirty();
	return true;
}

bool UAnimMontageCreator::SetMontageBlendIn(
	UAnimMontage* Montage,
	float BlendInTime,
	EAlphaBlendOption BlendInOption)
{
	if (!Montage)
	{
		return false;
	}

	Montage->SetBlendInArg(FAlphaBlend(BlendInTime, BlendInOption));
	Montage->MarkPackageDirty();
	return true;
}

bool UAnimMontageCreator::SetMontageBlendOut(
	UAnimMontage* Montage,
	float BlendOutTime,
	EAlphaBlendOption BlendOutOption)
{
	if (!Montage)
	{
		return false;
	}

	Montage->SetBlendOutArg(FAlphaBlend(BlendOutTime, BlendOutOption));
	Montage->MarkPackageDirty();
	return true;
}

void UAnimMontageCreator::ConfigureMontageSections(
	UAnimMontage* Montage,
	const TArray<FAnimMontageSectionData>& Sections)
{
	for (const FAnimMontageSectionData& SectionData : Sections)
	{
		if (!SectionData.Animation)
		{
			continue;
		}

		FAnimSegment Segment;
		Segment.SetAnimReference(SectionData.Animation);
		Segment.SetStartPos(SectionData.StartTime);
		Segment.SetEndPos(SectionData.EndTime);
		Segment.SetAnimPlayRate(1.0f);

		FAnimTrack AnimTrack;
		AnimTrack.GetSegmentContainer().Add(Segment);

		Montage->GetSlotAnimTracks().Add(FSlotAnimTrack());
		FSlotAnimTrack& SlotTrack = Montage->GetSlotAnimTracks().Last();
		SlotTrack.GetSegmentContainer().Add(Segment);
	}
}

#include "AnimMontageCreator.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"

UAnimMontage* UAnimMontageCreator::CreateAnimMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections,
	UPackage* Package)
{
	if (!Skeleton || MontageName.IsEmpty())
	{
		return nullptr;
	}

	if (!Package)
	{
		const FString PackagePath = TEXT("/Game/Animations/") + MontageName;
		Package = CreatePackage(*PackagePath);
	}

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

UAnimMontage* UAnimMontageCreator::CreateAnimMontageFromSequence(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	UAnimSequence* AnimSequence,
	float StartTime,
	float EndTime,
	UPackage* Package)
{
	if (!Skeleton || !AnimSequence || MontageName.IsEmpty())
	{
		return nullptr;
	}

	const float Duration = AnimSequence->GetPlayLength();
	if (EndTime < 0.0f)
	{
		EndTime = Duration;
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

	FSlotAnimationTrack SlotTrack;
	SlotTrack.SlotName = SectionData.SectionName;

	FAnimSegment Segment;
	Segment.SetAnimReference(SectionData.Animation);
	Segment.StartPos = StartTime;
	Segment.AnimEndTime = EndTime;
	Segment.AnimPlayRate = 1.0f;

	SlotTrack.AnimTrack.AnimSegments.Add(Segment);
	Montage->SlotAnimTracks.Add(SlotTrack);

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

	Montage->BlendIn.SetBlendOption(BlendInOption);
	Montage->BlendIn.SetBlendTime(BlendInTime);
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

	Montage->BlendOut.SetBlendOption(BlendOutOption);
	Montage->BlendOut.SetBlendTime(BlendOutTime);
	Montage->MarkPackageDirty();

	return true;
}

void UAnimMontageCreator::ConfigureMontageSections(
	UAnimMontage* Montage,
	const TArray<FAnimMontageSectionData>& Sections)
{
	if (!Montage)
	{
		return;
	}

	Montage->SlotAnimTracks.Empty();

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
}

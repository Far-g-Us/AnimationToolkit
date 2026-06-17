#include "AnimMontageCreator.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UAnimMontage* UAnimMontageCreator::CreateAnimMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections,
	UPackage* Package)
{
	return nullptr;
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
	return nullptr;
}

FAnimMontageSectionData UAnimMontageCreator::CreateMontageSectionData(
	const FName& SectionName,
	UAnimSequence* Animation,
	float StartTime,
	float EndTime)
{
	FAnimMontageSectionData SectionData;
	return SectionData;
}

bool UAnimMontageCreator::AddSectionToMontage(
	UAnimMontage* Montage,
	const FAnimMontageSectionData& SectionData,
	float StartTime,
	float EndTime)
{
	return false;
}

bool UAnimMontageCreator::SetMontageBlendIn(
	UAnimMontage* Montage,
	float BlendInTime,
	EAlphaBlendOption BlendInOption)
{
	return false;
}

bool UAnimMontageCreator::SetMontageBlendOut(
	UAnimMontage* Montage,
	float BlendOutTime,
	EAlphaBlendOption BlendOutOption)
{
	return false;
}

void UAnimMontageCreator::ConfigureMontageSections(
	UAnimMontage* Montage,
	const TArray<FAnimMontageSectionData>& Sections)
{
}

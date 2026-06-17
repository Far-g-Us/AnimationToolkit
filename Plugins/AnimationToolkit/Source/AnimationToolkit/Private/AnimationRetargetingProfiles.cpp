#include "AnimationRetargetingProfiles.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

FRetargetingProfile UAnimationRetargetingProfiles::CreateProfile(
	const FString& ProfileName,
	USkeleton* SourceSkeleton,
	USkeleton* TargetSkeleton)
{
	return FRetargetingProfile();
}

void UAnimationRetargetingProfiles::AddBoneMapping(
	FRetargetingProfile& Profile,
	const FName& SourceBone,
	const FName& TargetBone)
{
}

void UAnimationRetargetingProfiles::RemoveBoneMapping(
	FRetargetingProfile& Profile,
	const FName& SourceBone)
{
}

FName UAnimationRetargetingProfiles::GetMappedBone(
	const FRetargetingProfile& Profile,
	const FName& SourceBone)
{
	return NAME_None;
}

bool UAnimationRetargetingProfiles::HasMapping(
	const FRetargetingProfile& Profile,
	const FName& SourceBone)
{
	return false;
}

UAnimSequence* UAnimationRetargetingProfiles::RetargetWithProfile(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const FRetargetingProfile& Profile)
{
	return nullptr;
}

UAnimSequence* UAnimationRetargetingProfiles::AutoRetarget(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	USkeleton* TargetSkeleton)
{
	return nullptr;
}

TArray<FName> UAnimationRetargetingProfiles::GetUnmappedBones(const FRetargetingProfile& Profile)
{
	return TArray<FName>();
}

float UAnimationRetargetingProfiles::CalculateRetargetCoverage(const FRetargetingProfile& Profile)
{
	return 0.0f;
}

FRetargetingProfile UAnimationRetargetingProfiles::MergeProfiles(
	const FRetargetingProfile& ProfileA,
	const FRetargetingProfile& ProfileB)
{
	return FRetargetingProfile();
}

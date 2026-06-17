#include "AnimationRetargetingProfiles.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

FRetargetingProfile UAnimationRetargetingProfiles::CreateProfile(
	const FString& ProfileName,
	USkeleton* SourceSkeleton,
	USkeleton* TargetSkeleton)
{
	FRetargetingProfile Profile;
	Profile.ProfileName = ProfileName;
	Profile.SourceSkeleton = SourceSkeleton;
	Profile.TargetSkeleton = TargetSkeleton;
	return Profile;
}

void UAnimationRetargetingProfiles::AddBoneMapping(
	FRetargetingProfile& Profile,
	const FName& SourceBone,
	const FName& TargetBone)
{
	Profile.BoneMapping.Add(SourceBone, TargetBone);
}

void UAnimationRetargetingProfiles::RemoveBoneMapping(
	FRetargetingProfile& Profile,
	const FName& SourceBone)
{
	Profile.BoneMapping.Remove(SourceBone);
}

FName UAnimationRetargetingProfiles::GetMappedBone(
	const FRetargetingProfile& Profile,
	const FName& SourceBone)
{
	if (Profile.BoneMapping.Contains(SourceBone))
	{
		return Profile.BoneMapping[SourceBone];
	}

	return NAME_None;
}

bool UAnimationRetargetingProfiles::HasMapping(
	const FRetargetingProfile& Profile,
	const FName& SourceBone)
{
	return Profile.BoneMapping.Contains(SourceBone);
}

UAnimSequence* UAnimationRetargetingProfiles::RetargetWithProfile(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const FRetargetingProfile& Profile)
{
	if (!WorldContextObject || !SourceAnimation || !Profile.TargetSkeleton)
	{
		return nullptr;
	}

	USkeleton* TargetSkeleton = Profile.TargetSkeleton;
	const FReferenceSkeleton& TargetRefSkeleton = TargetSkeleton->GetReferenceSkeleton();

	FString NewName = FString::Printf(TEXT("%s_Retargeted"), *SourceAnimation->GetName());

	UAnimSequence* RetargetedAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!RetargetedAnim)
	{
		return nullptr;
	}

	RetargetedAnim->SetSkeleton(TargetSkeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	RetargetedAnim->GetEditableRawAnimationData().Tracks.SetNum(TargetRefSkeleton.GetNum());

	for (int32 TargetBoneIdx = 0; TargetBoneIdx < TargetRefSkeleton.GetNum(); TargetBoneIdx++)
	{
		FName TargetBoneName = TargetRefSkeleton.GetBoneName(TargetBoneIdx);

		FName SourceBoneName = NAME_None;
		for (const auto& Pair : Profile.BoneMapping)
		{
			if (Pair.Value == TargetBoneName)
			{
				SourceBoneName = Pair.Key;
				break;
			}
		}

		int32 SourceBoneIdx = INDEX_NONE;
		if (SourceBoneName != NAME_None && SourceAnimation->GetSkeleton())
		{
			SourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(SourceBoneName);
		}

		FRawAnimSequenceTrack& DestTrack = RetargetedAnim->GetEditableRawAnimationData().Tracks[TargetBoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform SourceTransform = (SourceBoneIdx != INDEX_NONE) ?
				SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame) : FTransform::Identity;

			FVector Pos = SourceTransform.GetLocation() * Profile.TranslationMultiplier;
			FQuat Rot = SourceTransform.GetRotation();

			if (Profile.RotationMultiplier != 1.0f)
			{
				FRotator Rotator = Rot.Rotator();
				Rotator.Pitch *= Profile.RotationMultiplier;
				Rotator.Yaw *= Profile.RotationMultiplier;
				Rotator.Roll *= Profile.RotationMultiplier;
				Rot = Rotator.Quaternion();
			}

			DestTrack.PosKeys[Frame] = Pos;
			DestTrack.RotKeys[Frame] = Rot;
			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();
		}
	}

	RetargetedAnim->MarkPackageDirty();
	return RetargetedAnim;
}

UAnimSequence* UAnimationRetargetingProfiles::AutoRetarget(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	USkeleton* TargetSkeleton)
{
	if (!WorldContextObject || !SourceAnimation || !TargetSkeleton)
	{
		return nullptr;
	}

	USkeleton* SourceSkeleton = SourceAnimation->GetSkeleton();
	if (!SourceSkeleton)
	{
		return nullptr;
	}

	FRetargetingProfile Profile;
	Profile.SourceSkeleton = SourceSkeleton;
	Profile.TargetSkeleton = TargetSkeleton;

	const FReferenceSkeleton& SourceRefSkeleton = SourceSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& TargetRefSkeleton = TargetSkeleton->GetReferenceSkeleton();

	for (int32 i = 0; i < SourceRefSkeleton.GetNum(); i++)
	{
		FName SourceBoneName = SourceRefSkeleton.GetBoneName(i);
		int32 TargetBoneIdx = TargetRefSkeleton.FindBoneIndex(SourceBoneName);

		if (TargetBoneIdx != INDEX_NONE)
		{
			Profile.BoneMapping.Add(SourceBoneName, SourceBoneName);
		}
		else
		{
			FString SourceBoneStr = SourceBoneName.ToString();

			TArray<FString> Prefixes = {TEXT("mixamorig:"), TEXT("Armature_"), TEXT("Bip001_"),
				TEXT("DEF-"), TEXT("ValveBiped."), TEXT("Character_"), TEXT("SK_"), TEXT("SKEL_")};

			for (const FString& Prefix : Prefixes)
			{
				if (SourceBoneStr.StartsWith(Prefix))
				{
					FString StrippedName = SourceBoneStr.Mid(Prefix.Len());
					int32 StrippedIdx = TargetRefSkeleton.FindBoneIndex(FName(*StrippedName));

					if (StrippedIdx != INDEX_NONE)
					{
						Profile.BoneMapping.Add(SourceBoneName, FName(*StrippedName));
						break;
					}
				}
			}
		}
	}

	return RetargetWithProfile(WorldContextObject, SourceAnimation, Profile);
}

TArray<FName> UAnimationRetargetingProfiles::GetUnmappedBones(const FRetargetingProfile& Profile)
{
	TArray<FName> UnmappedBones;

	if (!Profile.SourceSkeleton || !Profile.TargetSkeleton)
	{
		return UnmappedBones;
	}

	const FReferenceSkeleton& SourceRefSkeleton = Profile.SourceSkeleton->GetReferenceSkeleton();

	for (int32 i = 0; i < SourceRefSkeleton.GetNum(); i++)
	{
		FName SourceBoneName = SourceRefSkeleton.GetBoneName(i);

		if (!Profile.BoneMapping.Contains(SourceBoneName))
		{
			UnmappedBones.Add(SourceBoneName);
		}
	}

	return UnmappedBones;
}

float UAnimationRetargetingProfiles::CalculateRetargetCoverage(const FRetargetingProfile& Profile)
{
	if (!Profile.SourceSkeleton)
	{
		return 0.0f;
	}

	const FReferenceSkeleton& SourceRefSkeleton = Profile.SourceSkeleton->GetReferenceSkeleton();
	int32 TotalBones = SourceRefSkeleton.GetNum();

	if (TotalBones == 0)
	{
		return 0.0f;
	}

	int32 MappedBones = Profile.BoneMapping.Num();
	return (float)MappedBones / (float)TotalBones;
}

FRetargetingProfile UAnimationRetargetingProfiles::MergeProfiles(
	const FRetargetingProfile& ProfileA,
	const FRetargetingProfile& ProfileB)
{
	FRetargetingProfile MergedProfile;
	MergedProfile.ProfileName = ProfileA.ProfileName + TEXT("_") + ProfileB.ProfileName;
	MergedProfile.SourceSkeleton = ProfileA.SourceSkeleton;
	MergedProfile.TargetSkeleton = ProfileB.TargetSkeleton;

	MergedProfile.BoneMapping.Append(ProfileA.BoneMapping);
	MergedProfile.BoneMapping.Append(ProfileB.BoneMapping);

	MergedProfile.TranslationMultiplier = ProfileA.TranslationMultiplier;
	MergedProfile.RotationMultiplier = ProfileA.RotationMultiplier;

	return MergedProfile;
}

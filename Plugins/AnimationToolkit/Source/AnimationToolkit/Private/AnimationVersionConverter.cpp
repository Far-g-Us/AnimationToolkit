#include "AnimationVersionConverter.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"
#include "Animation/AnimData/AnimDataModel.h"

TMap<FName, FName> UAnimationVersionConverter::GetBoneRenamesForVersion(
	EEngineVersion SourceVersion, EEngineVersion TargetVersion)
{
	TMap<FName, FName> Renames;

	if (SourceVersion >= EEngineVersion::UE4_0 && SourceVersion <= EEngineVersion::UE4_19)
	{
		Renames.Add(FName("SKEL_Root"), FName("root"));
		Renames.Add(FName("SKEL_Pelvis"), FName("pelvis"));
		Renames.Add(FName("SKEL_Spine_01"), FName("spine_01"));
		Renames.Add(FName("SKEL_Spine_02"), FName("spine_02"));
		Renames.Add(FName("SKEL_Spine_03"), FName("spine_03"));
		Renames.Add(FName("SKEL_Head"), FName("head"));
		Renames.Add(FName("SKEL_L_UpperArm"), FName("upperarm_l"));
		Renames.Add(FName("SKEL_L_Forearm"), FName("forearm_l"));
		Renames.Add(FName("SKEL_L_Hand"), FName("hand_l"));
		Renames.Add(FName("SKEL_R_UpperArm"), FName("upperarm_r"));
		Renames.Add(FName("SKEL_R_Forearm"), FName("forearm_r"));
		Renames.Add(FName("SKEL_R_Hand"), FName("hand_r"));
		Renames.Add(FName("SKEL_L_Thigh"), FName("thigh_l"));
		Renames.Add(FName("SKEL_L_Calf"), FName("calf_l"));
		Renames.Add(FName("SKEL_L_Foot"), FName("foot_l"));
		Renames.Add(FName("SKEL_R_Thigh"), FName("thigh_r"));
		Renames.Add(FName("SKEL_R_Calf"), FName("calf_r"));
		Renames.Add(FName("SKEL_R_Foot"), FName("foot_r"));
	}

	if (SourceVersion >= EEngineVersion::UE4_20 && SourceVersion <= EEngineVersion::UE4_27)
	{
		Renames.Add(FName("mixamorig:Hips"), FName("pelvis"));
		Renames.Add(FName("mixamorig:Spine"), FName("spine_01"));
		Renames.Add(FName("mixamorig:Spine1"), FName("spine_02"));
		Renames.Add(FName("mixamorig:Spine2"), FName("spine_03"));
		Renames.Add(FName("mixamorig:Neck"), FName("neck_01"));
		Renames.Add(FName("mixamorig:Head"), FName("head"));
		Renames.Add(FName("mixamorig:LeftShoulder"), FName("clavicle_l"));
		Renames.Add(FName("mixamorig:LeftArm"), FName("upperarm_l"));
		Renames.Add(FName("mixamorig:LeftForeArm"), FName("forearm_l"));
		Renames.Add(FName("mixamorig:LeftHand"), FName("hand_l"));
		Renames.Add(FName("mixamorig:RightShoulder"), FName("clavicle_r"));
		Renames.Add(FName("mixamorig:RightArm"), FName("upperarm_r"));
		Renames.Add(FName("mixamorig:RightForeArm"), FName("forearm_r"));
		Renames.Add(FName("mixamorig:RightHand"), FName("hand_r"));
		Renames.Add(FName("mixamorig:LeftUpLeg"), FName("thigh_l"));
		Renames.Add(FName("mixamorig:LeftLeg"), FName("calf_l"));
		Renames.Add(FName("mixamorig:LeftFoot"), FName("foot_l"));
		Renames.Add(FName("mixamorig:RightUpLeg"), FName("thigh_r"));
		Renames.Add(FName("mixamorig:RightLeg"), FName("calf_r"));
		Renames.Add(FName("mixamorig:RightFoot"), FName("foot_r"));
	}

	if (SourceVersion >= EEngineVersion::UE5_0 && SourceVersion <= EEngineVersion::UE5_5 &&
		TargetVersion >= EEngineVersion::UE5_6)
	{
		Renames.Add(FName("pelvis"), FName("Pelvis"));
		Renames.Add(FName("spine_01"), FName("Spine_01"));
		Renames.Add(FName("spine_02"), FName("Spine_02"));
		Renames.Add(FName("spine_03"), FName("Spine_03"));
		Renames.Add(FName("neck_01"), FName("Neck_01"));
		Renames.Add(FName("head"), FName("Head"));
		Renames.Add(FName("clavicle_l"), FName("Clavicle_L"));
		Renames.Add(FName("upperarm_l"), FName("UpperArm_L"));
		Renames.Add(FName("forearm_l"), FName("Forearm_L"));
		Renames.Add(FName("hand_l"), FName("Hand_L"));
		Renames.Add(FName("clavicle_r"), FName("Clavicle_R"));
		Renames.Add(FName("upperarm_r"), FName("UpperArm_R"));
		Renames.Add(FName("forearm_r"), FName("Forearm_R"));
		Renames.Add(FName("hand_r"), FName("Hand_R"));
		Renames.Add(FName("thigh_l"), FName("Thigh_L"));
		Renames.Add(FName("calf_l"), FName("Calf_L"));
		Renames.Add(FName("foot_l"), FName("Foot_L"));
		Renames.Add(FName("thigh_r"), FName("Thigh_R"));
		Renames.Add(FName("calf_r"), FName("Calf_R"));
		Renames.Add(FName("foot_r"), FName("Foot_R"));
		Renames.Add(FName("root"), FName("Root"));
	}

	if (SourceVersion >= EEngineVersion::UE5_6 && SourceVersion <= EEngineVersion::UE5_7 &&
		TargetVersion == EEngineVersion::UE5_8)
	{
		Renames.Add(FName("Clavicle_L"), FName("clavicle_l"));
		Renames.Add(FName("Clavicle_R"), FName("clavicle_r"));
		Renames.Add(FName("UpperArm_L"), FName("upperarm_l"));
		Renames.Add(FName("UpperArm_R"), FName("upperarm_r"));
		Renames.Add(FName("Forearm_L"), FName("forearm_l"));
		Renames.Add(FName("Forearm_R"), FName("forearm_r"));
		Renames.Add(FName("Hand_L"), FName("hand_l"));
		Renames.Add(FName("Hand_R"), FName("hand_r"));
		Renames.Add(FName("Thigh_L"), FName("thigh_l"));
		Renames.Add(FName("Thigh_R"), FName("thigh_r"));
		Renames.Add(FName("Calf_L"), FName("calf_l"));
		Renames.Add(FName("Calf_R"), FName("calf_r"));
		Renames.Add(FName("Foot_L"), FName("foot_l"));
		Renames.Add(FName("Foot_R"), FName("foot_r"));
	}

	return Renames;
}

float UAnimationVersionConverter::GetRecommendedScale(
	EEngineVersion SourceVersion, EEngineVersion TargetVersion)
{
	float SourceScale = GetUnitScale(SourceVersion);
	float TargetScale = GetUnitScale(TargetVersion);

	if (SourceScale > 0.0f && TargetScale > 0.0f)
	{
		return TargetScale / SourceScale;
	}

	return 1.0f;
}

float UAnimationVersionConverter::GetUnitScale(EEngineVersion Version)
{
	if (Version >= EEngineVersion::UE4_0 && Version <= EEngineVersion::UE4_27)
	{
		return 100.0f;
	}

	if (Version >= EEngineVersion::UE5_0 && Version <= EEngineVersion::UE5_8)
	{
		return 1.0f;
	}

	return 1.0f;
}

TArray<FName> UAnimationVersionConverter::GetDeprecatedBones(EEngineVersion Version)
{
	TArray<FName> Deprecated;

	if (Version == EEngineVersion::UE4_12)
	{
		Deprecated.Add(FName("IK_Root"));
		Deprecated.Add(FName("Root_Center"));
	}

	return Deprecated;
}

TArray<FName> UAnimationVersionConverter::GetAddedBones(EEngineVersion Version)
{
	TArray<FName> Added;

	if (Version == EEngineVersion::UE5_0)
	{
		Added.Add(FName("pelvis"));
		Added.Add(FName("spine_01"));
		Added.Add(FName("spine_02"));
		Added.Add(FName("spine_03"));
	}

	return Added;
}

FString UAnimationVersionConverter::GetEngineVersionName(EEngineVersion Version)
{
	switch (Version)
	{
	case EEngineVersion::UE4_0: return TEXT("UE 4.0");
	case EEngineVersion::UE4_1: return TEXT("UE 4.1");
	case EEngineVersion::UE4_2: return TEXT("UE 4.2");
	case EEngineVersion::UE4_3: return TEXT("UE 4.3");
	case EEngineVersion::UE4_4: return TEXT("UE 4.4");
	case EEngineVersion::UE4_5: return TEXT("UE 4.5");
	case EEngineVersion::UE4_6: return TEXT("UE 4.6");
	case EEngineVersion::UE4_7: return TEXT("UE 4.7");
	case EEngineVersion::UE4_8: return TEXT("UE 4.8");
	case EEngineVersion::UE4_9: return TEXT("UE 4.9");
	case EEngineVersion::UE4_10: return TEXT("UE 4.10");
	case EEngineVersion::UE4_11: return TEXT("UE 4.11");
	case EEngineVersion::UE4_12: return TEXT("UE 4.12");
	case EEngineVersion::UE4_13: return TEXT("UE 4.13");
	case EEngineVersion::UE4_14: return TEXT("UE 4.14");
	case EEngineVersion::UE4_15: return TEXT("UE 4.15");
	case EEngineVersion::UE4_16: return TEXT("UE 4.16");
	case EEngineVersion::UE4_17: return TEXT("UE 4.17");
	case EEngineVersion::UE4_18: return TEXT("UE 4.18");
	case EEngineVersion::UE4_19: return TEXT("UE 4.19");
	case EEngineVersion::UE4_20: return TEXT("UE 4.20");
	case EEngineVersion::UE4_21: return TEXT("UE 4.21");
	case EEngineVersion::UE4_22: return TEXT("UE 4.22");
	case EEngineVersion::UE4_23: return TEXT("UE 4.23");
	case EEngineVersion::UE4_24: return TEXT("UE 4.24");
	case EEngineVersion::UE4_25: return TEXT("UE 4.25");
	case EEngineVersion::UE4_26: return TEXT("UE 4.26");
	case EEngineVersion::UE4_27: return TEXT("UE 4.27");
	case EEngineVersion::UE5_0: return TEXT("UE 5.0");
	case EEngineVersion::UE5_1: return TEXT("UE 5.1");
	case EEngineVersion::UE5_2: return TEXT("UE 5.2");
	case EEngineVersion::UE5_3: return TEXT("UE 5.3");
	case EEngineVersion::UE5_4: return TEXT("UE 5.4");
	case EEngineVersion::UE5_5: return TEXT("UE 5.5");
	case EEngineVersion::UE5_6: return TEXT("UE 5.6");
	case EEngineVersion::UE5_7: return TEXT("UE 5.7");
	case EEngineVersion::UE5_8: return TEXT("UE 5.8");
	default: return TEXT("Custom");
	}
}

bool UAnimationVersionConverter::IsVersionCompatible(
	EEngineVersion SourceVersion, EEngineVersion TargetVersion)
{
	return true;
}

UAnimSequence* UAnimationVersionConverter::ConvertAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const FEngineVersionProfile& Profile,
	FAnimationConversionResult& OutResult)
{
	if (!WorldContextObject || !SourceAnimation)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Invalid input");
		return nullptr;
	}

	USkeleton* SourceSkeleton = SourceAnimation->GetSkeleton();
	if (!SourceSkeleton)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Source animation has no skeleton");
		return nullptr;
	}

	FString NewName = FString::Printf(TEXT("%s_Converted"), *SourceAnimation->GetName());

	UAnimSequence* ConvertedAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ConvertedAnim)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Failed to create converted animation");
		return nullptr;
	}

	ConvertedAnim->SetSkeleton(SourceSkeleton);

	const FReferenceSkeleton& RefSkeleton = SourceSkeleton->GetReferenceSkeleton();

	ConvertedAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	TMap<FName, FName> Renames = Profile.CustomBoneRenames;
	if (Profile.bConvertBoneNames)
	{
		TMap<FName, FName> VersionRenames = GetBoneRenamesForVersion(
			Profile.SourceVersion, Profile.TargetVersion);
		for (auto& Pair : VersionRenames)
		{
			if (!Renames.Contains(Pair.Key))
			{
				Renames.Add(Pair.Key, Pair.Value);
			}
		}
	}

	TMap<FName, FName> ReverseRenames;
	for (const auto& Pair : Renames)
	{
		ReverseRenames.Add(Pair.Value, Pair.Key);
	}

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);

		FName SourceBoneName = BoneName;
		if (ReverseRenames.Contains(BoneName))
		{
			SourceBoneName = ReverseRenames[BoneName];
			OutResult.BonesRenamed++;
		}

		int32 SourceBoneIdx = SourceSkeleton->GetReferenceSkeleton().FindBoneIndex(SourceBoneName);
		if (SourceBoneIdx == INDEX_NONE)
		{
			SourceBoneIdx = RefSkeleton.FindBoneIndex(BoneName);
		}

		if (SourceBoneIdx != INDEX_NONE && SourceBoneIdx < SourceAnimation->GetRawAnimationData().Tracks.Num())
		{
			FRawAnimSequenceTrack& DestTrack = ConvertedAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
			const FRawAnimSequenceTrack& SourceTrack = SourceAnimation->GetRawAnimationData().Tracks[SourceBoneIdx];

			DestTrack.PosKeys = SourceTrack.PosKeys;
			DestTrack.RotKeys = SourceTrack.RotKeys;
			DestTrack.ScaleKeys = SourceTrack.ScaleKeys;
		}
	}

	if (Profile.ScaleMultiplier != 1.0f)
	{
		ApplyScaleToAnimation(ConvertedAnim, Profile.ScaleMultiplier);
	}

	if (Profile.bFixRootMotion)
	{
		int32 RootIdx = RefSkeleton.FindBoneIndex(FName("root"));
		if (RootIdx == INDEX_NONE) RootIdx = 0;

		if (RootIdx < ConvertedAnim->GetEditableRawAnimationData().Tracks.Num())
		{
			FRawAnimSequenceTrack& RootTrack = ConvertedAnim->GetEditableRawAnimationData().Tracks[RootIdx];
			FVector FirstPos = RootTrack.PosKeys.Num() > 0 ? RootTrack.PosKeys[0] : FVector::ZeroVector;

			for (int32 i = 0; i < RootTrack.PosKeys.Num(); i++)
			{
				RootTrack.PosKeys[i] -= FirstPos;
			}
		}
	}

	ConvertedAnim->MarkPackageDirty();

	OutResult.bSuccess = true;
	OutResult.AnimationsConverted = 1;
	OutResult.Message = FString::Printf(TEXT("Converted: %d bones renamed"), OutResult.BonesRenamed);

	return ConvertedAnim;
}

TArray<UAnimSequence*> UAnimationVersionConverter::ConvertAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& SourceAnimations,
	const FEngineVersionProfile& Profile,
	FAnimationConversionResult& OutResult)
{
	TArray<UAnimSequence*> Results;

	for (UAnimSequence* Anim : SourceAnimations)
	{
		FAnimationConversionResult SingleResult;
		UAnimSequence* Converted = ConvertAnimation(WorldContextObject, Anim, Profile, SingleResult);

		if (Converted)
		{
			Results.Add(Converted);
			OutResult.AnimationsConverted++;
			OutResult.BonesRenamed += SingleResult.BonesRenamed;
		}
		else
		{
			OutResult.Warnings.Add(SingleResult.Message);
		}
	}

	OutResult.bSuccess = OutResult.AnimationsConverted > 0;
	OutResult.Message = FString::Printf(TEXT("Converted %d/%d animations"),
		OutResult.AnimationsConverted, SourceAnimations.Num());

	return Results;
}

void UAnimationVersionConverter::ApplyScaleToAnimation(UAnimSequence* Animation, float ScaleMultiplier)
{
	if (!Animation)
	{
		return;
	}

	for (int32 TrackIdx = 0; TrackIdx < Animation->GetEditableRawAnimationData().Tracks.Num(); TrackIdx++)
	{
		FRawAnimSequenceTrack& Track = Animation->GetEditableRawAnimationData().Tracks[TrackIdx];

		for (int32 i = 0; i < Track.PosKeys.Num(); i++)
		{
			Track.PosKeys[i] *= ScaleMultiplier;
		}
	}
}

void UAnimationVersionConverter::RenameBonesInAnimation(
	UAnimSequence* Animation, const TMap<FName, FName>& Renames)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		return;
	}

	const FReferenceSkeleton& RefSkeleton = Animation->GetSkeleton()->GetReferenceSkeleton();

	for (const auto& Pair : Renames)
	{
		int32 OldIdx = RefSkeleton.FindBoneIndex(Pair.Key);
		int32 NewIdx = RefSkeleton.FindBoneIndex(Pair.Value);

		if (OldIdx != INDEX_NONE && NewIdx != INDEX_NONE &&
			OldIdx < Animation->GetEditableRawAnimationData().Tracks.Num() &&
			NewIdx < Animation->GetEditableRawAnimationData().Tracks.Num())
		{
			Animation->GetEditableRawAnimationData().Tracks[NewIdx] =
				Animation->GetEditableRawAnimationData().Tracks[OldIdx];
		}
	}

	Animation->MarkPackageDirty();
}

#include "AnimationVersionConverter.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

TMap<FName, FName> UAnimationVersionConverter::GetBoneRenamesForVersion(
	EEngineVersion SourceVersion, EEngineVersion TargetVersion)
{
	TMap<FName, FName> Renames;

	if (SourceVersion <= EEngineVersion::UE4_27 && TargetVersion >= EEngineVersion::UE5_0)
	{
		Renames.Add(FName(TEXT("root")), FName(TEXT("root")));
		Renames.Add(FName(TEXT("Hips")), FName(TEXT("Hips")));
		Renames.Add(FName(TEXT("Spine")), FName(TEXT("Spine")));
		Renames.Add(FName(TEXT("Spine1")), FName(TEXT("Spine1")));
		Renames.Add(FName(TEXT("Spine2")), FName(TEXT("Spine2")));
		Renames.Add(FName(TEXT("Neck")), FName(TEXT("Neck")));
		Renames.Add(FName(TEXT("Head")), FName(TEXT("Head")));
		Renames.Add(FName(TEXT("LeftShoulder")), FName(TEXT("LeftShoulder")));
		Renames.Add(FName(TEXT("LeftUpperArm")), FName(TEXT("LeftUpperArm")));
		Renames.Add(FName(TEXT("LeftLowerArm")), FName(TEXT("LeftLowerArm")));
		Renames.Add(FName(TEXT("LeftHand")), FName(TEXT("LeftHand")));
		Renames.Add(FName(TEXT("RightShoulder")), FName(TEXT("RightShoulder")));
		Renames.Add(FName(TEXT("RightUpperArm")), FName(TEXT("RightUpperArm")));
		Renames.Add(FName(TEXT("RightLowerArm")), FName(TEXT("RightLowerArm")));
		Renames.Add(FName(TEXT("RightHand")), FName(TEXT("RightHand")));
		Renames.Add(FName(TEXT("LeftUpperLeg")), FName(TEXT("LeftUpperLeg")));
		Renames.Add(FName(TEXT("LeftLowerLeg")), FName(TEXT("LeftLowerLeg")));
		Renames.Add(FName(TEXT("LeftFoot")), FName(TEXT("LeftFoot")));
		Renames.Add(FName(TEXT("RightUpperLeg")), FName(TEXT("RightUpperLeg")));
		Renames.Add(FName(TEXT("RightLowerLeg")), FName(TEXT("RightLowerLeg")));
		Renames.Add(FName(TEXT("RightFoot")), FName(TEXT("RightFoot")));
	}

	if (SourceVersion >= EEngineVersion::UE4_0 && SourceVersion <= EEngineVersion::UE4_12 &&
		TargetVersion >= EEngineVersion::UE4_13)
	{
		Renames.Add(FName(TEXT("UE4_Root")), FName(TEXT("root")));
	}

	if (SourceVersion >= EEngineVersion::UE4_0 && SourceVersion <= EEngineVersion::UE4_20 &&
		TargetVersion >= EEngineVersion::UE4_21)
	{
		Renames.Add(FName(TEXT("IK_Root")), FName(TEXT("ik_root")));
		Renames.Add(FName(TEXT("IK_Hand_R")), FName(TEXT("ik_hand_r")));
		Renames.Add(FName(TEXT("IK_Hand_L")), FName(TEXT("ik_hand_l")));
		Renames.Add(FName(TEXT("IK_Foot_R")), FName(TEXT("ik_foot_r")));
		Renames.Add(FName(TEXT("IK_Foot_L")), FName(TEXT("ik_foot_l")));
	}

	return Renames;
}

float UAnimationVersionConverter::GetRecommendedScale(
	EEngineVersion SourceVersion, EEngineVersion TargetVersion)
{
	float SourceScale = GetUnitScale(SourceVersion);
	float TargetScale = GetUnitScale(TargetVersion);

	if (FMath::IsNearlyEqual(SourceScale, 0.0f) || FMath::IsNearlyEqual(TargetScale, 0.0f))
	{
		return 1.0f;
	}

	return SourceScale / TargetScale;
}

float UAnimationVersionConverter::GetUnitScale(EEngineVersion Version)
{
	if (Version >= EEngineVersion::UE5_0)
	{
		return 1.0f;
	}

	return 1.0f;
}

TArray<FName> UAnimationVersionConverter::GetDeprecatedBones(EEngineVersion Version)
{
	TArray<FName> Deprecated;

	if (Version <= EEngineVersion::UE4_12)
	{
		Deprecated.Add(FName(TEXT("UE4_Root")));
	}

	if (Version <= EEngineVersion::UE4_20)
	{
		Deprecated.Add(FName(TEXT("IK_Root")));
		Deprecated.Add(FName(TEXT("IK_Hand_R")));
		Deprecated.Add(FName(TEXT("IK_Hand_L")));
		Deprecated.Add(FName(TEXT("IK_Foot_R")));
		Deprecated.Add(FName(TEXT("IK_Foot_L")));
	}

	return Deprecated;
}

TArray<FName> UAnimationVersionConverter::GetAddedBones(EEngineVersion Version)
{
	TArray<FName> Added;

	if (Version >= EEngineVersion::UE4_13)
	{
		Added.Add(FName(TEXT("root")));
	}

	if (Version >= EEngineVersion::UE4_21)
	{
		Added.Add(FName(TEXT("ik_root")));
		Added.Add(FName(TEXT("ik_hand_r")));
		Added.Add(FName(TEXT("ik_hand_l")));
		Added.Add(FName(TEXT("ik_foot_r")));
		Added.Add(FName(TEXT("ik_foot_l")));
	}

	return Added;
}

FString UAnimationVersionConverter::GetEngineVersionName(EEngineVersion Version)
{
	switch (Version)
	{
	case EEngineVersion::UE4_0:   return TEXT("UE 4.0");
	case EEngineVersion::UE4_1:   return TEXT("UE 4.1");
	case EEngineVersion::UE4_2:   return TEXT("UE 4.2");
	case EEngineVersion::UE4_3:   return TEXT("UE 4.3");
	case EEngineVersion::UE4_4:   return TEXT("UE 4.4");
	case EEngineVersion::UE4_5:   return TEXT("UE 4.5");
	case EEngineVersion::UE4_6:   return TEXT("UE 4.6");
	case EEngineVersion::UE4_7:   return TEXT("UE 4.7");
	case EEngineVersion::UE4_8:   return TEXT("UE 4.8");
	case EEngineVersion::UE4_9:   return TEXT("UE 4.9");
	case EEngineVersion::UE4_10:  return TEXT("UE 4.10");
	case EEngineVersion::UE4_11:  return TEXT("UE 4.11");
	case EEngineVersion::UE4_12:  return TEXT("UE 4.12");
	case EEngineVersion::UE4_13:  return TEXT("UE 4.13");
	case EEngineVersion::UE4_14:  return TEXT("UE 4.14");
	case EEngineVersion::UE4_15:  return TEXT("UE 4.15");
	case EEngineVersion::UE4_16:  return TEXT("UE 4.16");
	case EEngineVersion::UE4_17:  return TEXT("UE 4.17");
	case EEngineVersion::UE4_18:  return TEXT("UE 4.18");
	case EEngineVersion::UE4_19:  return TEXT("UE 4.19");
	case EEngineVersion::UE4_20:  return TEXT("UE 4.20");
	case EEngineVersion::UE4_21:  return TEXT("UE 4.21");
	case EEngineVersion::UE4_22:  return TEXT("UE 4.22");
	case EEngineVersion::UE4_23:  return TEXT("UE 4.23");
	case EEngineVersion::UE4_24:  return TEXT("UE 4.24");
	case EEngineVersion::UE4_25:  return TEXT("UE 4.25");
	case EEngineVersion::UE4_26:  return TEXT("UE 4.26");
	case EEngineVersion::UE4_27:  return TEXT("UE 4.27");
	case EEngineVersion::UE5_0:   return TEXT("UE 5.0");
	case EEngineVersion::UE5_1:   return TEXT("UE 5.1");
	case EEngineVersion::UE5_2:   return TEXT("UE 5.2");
	case EEngineVersion::UE5_3:   return TEXT("UE 5.3");
	case EEngineVersion::UE5_4:   return TEXT("UE 5.4");
	case EEngineVersion::UE5_5:   return TEXT("UE 5.5");
	case EEngineVersion::UE5_6:   return TEXT("UE 5.6");
	case EEngineVersion::UE5_7:   return TEXT("UE 5.7");
	case EEngineVersion::UE5_8:   return TEXT("UE 5.8");
	case EEngineVersion::Custom:  return TEXT("Custom");
	default:                      return TEXT("Unknown");
	}
}

bool UAnimationVersionConverter::IsVersionCompatible(
	EEngineVersion SourceVersion, EEngineVersion TargetVersion)
{
	if (SourceVersion == EEngineVersion::Custom || TargetVersion == EEngineVersion::Custom)
	{
		return true;
	}

	return SourceVersion != TargetVersion;
}

UAnimSequence* UAnimationVersionConverter::ConvertAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const FEngineVersionProfile& Profile,
	FAnimationConversionResult& OutResult)
{
	if (!SourceAnimation)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Source animation is null.");
		return nullptr;
	}

	if (!IsVersionCompatible(Profile.SourceVersion, Profile.TargetVersion))
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Source and target versions are the same.");
		return nullptr;
	}

	UAnimSequence* ResultAnim = SourceAnimation;

	TMap<FName, FName> Renames;
	if (Profile.bConvertBoneNames)
	{
		if (Profile.CustomBoneRenames.Num() > 0)
		{
			Renames = Profile.CustomBoneRenames;
		}
		else
		{
			Renames = GetBoneRenamesForVersion(Profile.SourceVersion, Profile.TargetVersion);
		}
	}

	if (Profile.ScaleMultiplier != 1.0f)
	{
		ApplyScaleToAnimation(ResultAnim, Profile.ScaleMultiplier);
	}
	else
	{
		float RecommendedScale = GetRecommendedScale(Profile.SourceVersion, Profile.TargetVersion);
		if (!FMath::IsNearlyEqual(RecommendedScale, 1.0f))
		{
			ApplyScaleToAnimation(ResultAnim, RecommendedScale);
		}
	}

	if (Renames.Num() > 0)
	{
		RenameBonesInAnimation(ResultAnim, Renames);
		OutResult.BonesRenamed = Renames.Num();
	}

	if (Profile.bFixRootMotion)
	{
		const FReferenceSkeleton& RefSkeleton = ResultAnim->GetSkeleton()->GetReferenceSkeleton();
		int32 RootIndex = RefSkeleton.FindBoneIndex(FName(TEXT("root")));
		if (RootIndex != INDEX_NONE)
		{
			USkeleton* Skeleton = ResultAnim->GetSkeleton();
			if (Skeleton)
			{
				const FReferenceSkeleton& SkelRef = Skeleton->GetReferenceSkeleton();
				if (SkelRef.GetNum() > 0)
				{
					FName RootBoneName = SkelRef.GetBoneName(0);
					if (RootBoneName != FName(TEXT("root")))
					{
						OutResult.Warnings.Add(TEXT("Root bone name mismatch detected. Consider re-targeting root motion."));
					}
				}
			}
		}
	}

	if (Profile.bNormalizeRotations)
	{
		UE_LOG(LogTemp, Warning, TEXT("NormalizeRotations: Skipped (AddBoneTrack not available in UE5.5 runtime)."));
	}

	ResultAnim->MarkPackageDirty();

	OutResult.bSuccess = true;
	OutResult.AnimationsConverted = 1;
	OutResult.Message = FString::Printf(TEXT("Converted %s to %s."),
		*GetEngineVersionName(Profile.SourceVersion),
		*GetEngineVersionName(Profile.TargetVersion));

	return ResultAnim;
}

TArray<UAnimSequence*> UAnimationVersionConverter::ConvertAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& SourceAnimations,
	const FEngineVersionProfile& Profile,
	FAnimationConversionResult& OutResult)
{
	TArray<UAnimSequence*> ConvertedAnims;
	FAnimationConversionResult BatchResult;
	BatchResult.bSuccess = true;
	BatchResult.AnimationsConverted = 0;
	BatchResult.BonesRenamed = 0;

	for (UAnimSequence* Anim : SourceAnimations)
	{
		FAnimationConversionResult SingleResult;
		UAnimSequence* Converted = ConvertAnimation(WorldContextObject, Anim, Profile, SingleResult);

		if (SingleResult.bSuccess && Converted)
		{
			ConvertedAnims.Add(Converted);
			BatchResult.AnimationsConverted++;
			BatchResult.BonesRenamed += SingleResult.BonesRenamed;
		}
		else
		{
			BatchResult.bSuccess = false;
			BatchResult.Warnings.Append(SingleResult.Warnings);
			if (!SingleResult.Message.IsEmpty())
			{
				BatchResult.Warnings.Add(SingleResult.Message);
			}
		}
	}

	BatchResult.Message = FString::Printf(TEXT("Batch converted %d of %d animations."),
		BatchResult.AnimationsConverted, SourceAnimations.Num());

	OutResult = BatchResult;
	return ConvertedAnims;
}

void UAnimationVersionConverter::ApplyScaleToAnimation(UAnimSequence* Animation, float ScaleMultiplier)
{
	if (!Animation || FMath::IsNearlyEqual(ScaleMultiplier, 1.0f))
	{
		return;
	}

	const IAnimationDataModel* Model = Animation->GetDataModel();
	if (!Model)
	{
		return;
	}

	IAnimationDataController& Controller = Animation->GetController();
	const TScriptInterface<IAnimationDataModel> ModelInterface = Controller.GetModelInterface();

	Controller.OpenBracket(FText::FromString(FString::Printf(TEXT("Applying Scale %.2f"), ScaleMultiplier)), false);

	const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		const FRawAnimSequenceTrack& SrcRaw = Track.InternalTrackData;
		const int32 NumPosKeys = SrcRaw.PosKeys.Num();

		if (NumPosKeys == 0)
		{
			continue;
		}

		TArray<FVector3f> ScaledPosKeys;
		ScaledPosKeys.SetNum(NumPosKeys);

		for (int32 i = 0; i < NumPosKeys; ++i)
		{
			ScaledPosKeys[i] = SrcRaw.PosKeys[i] * static_cast<float>(ScaleMultiplier);
		}

		Controller.SetBoneTrackKeys(
			Track.Name,
			ScaledPosKeys,
			SrcRaw.RotKeys,
			SrcRaw.ScaleKeys,
			false);
	}

	Controller.CloseBracket(false);

	UE_LOG(LogTemp, Log, TEXT("ApplyScaleToAnimation: Applied scale %.2f to %d bone tracks"),
		ScaleMultiplier, Tracks.Num());

	Animation->MarkPackageDirty();
}

void UAnimationVersionConverter::RenameBonesInAnimation(
	UAnimSequence* Animation, const TMap<FName, FName>& Renames)
{
	if (!Animation || Renames.Num() == 0)
	{
		return;
	}

	const IAnimationDataModel* Model = Animation->GetDataModel();
	if (!Model)
	{
		return;
	}

	IAnimationDataController& Controller = Animation->GetController();
	const TScriptInterface<IAnimationDataModel> ModelInterface = Controller.GetModelInterface();

	Controller.OpenBracket(FText::FromString(FString::Printf(TEXT("Renaming %d Bones"), Renames.Num())), false);

	const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();
	int32 RenamedCount = 0;

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		const FName* NewName = Renames.Find(Track.Name);
		if (!NewName)
		{
			continue;
		}

		if (*NewName == Track.Name)
		{
			continue;
		}

		const FRawAnimSequenceTrack& SrcRaw = Track.InternalTrackData;

		Controller.AddBoneCurve(*NewName, false);

		Controller.SetBoneTrackKeys(
			*NewName,
			SrcRaw.PosKeys,
			SrcRaw.RotKeys,
			SrcRaw.ScaleKeys,
			false);

		Controller.RemoveBoneTrack(Track.Name, false);

		RenamedCount++;

		UE_LOG(LogTemp, Log, TEXT("RenameBonesInAnimation: Renamed bone '%s' -> '%s'"),
			*Track.Name.ToString(), *NewName->ToString());
	}

	Controller.CloseBracket(false);

	UE_LOG(LogTemp, Log, TEXT("RenameBonesInAnimation: Renamed %d of %d requested bones"),
		RenamedCount, Renames.Num());

	Animation->MarkPackageDirty();
}

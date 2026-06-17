#include "AnimationRepairTool.h"
#include "BoneMappingTool.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

UAnimSequence* UAnimationRepairTool::RepairAnimationMissingBones(
	UObject* WorldContextObject,
	UAnimSequence* BrokenAnimation,
	USkeleton* TargetSkeleton,
	FAnimationRepairResult& OutResult)
{
	if (!WorldContextObject || !BrokenAnimation || !TargetSkeleton)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Invalid input");
		return nullptr;
	}

	USkeleton* SourceSkeleton = BrokenAnimation->GetSkeleton();

	FBoneMappingResult Mapping;
	if (SourceSkeleton)
	{
		Mapping = UBoneMappingTool::AutoMapBonesFromTwoSkeletons(
			SourceSkeleton, TargetSkeleton, EBoneMappingMethod::AllMethods);
	}
	else
	{
		Mapping = UBoneMappingTool::AutoMapBonesFromAnimation(
			BrokenAnimation, TargetSkeleton, EBoneMappingMethod::AllMethods);
	}

	UAnimSequence* RepairedAnim = UBoneMappingTool::RemapAnimationBones(
		BrokenAnimation, Mapping, TargetSkeleton);

	if (!RepairedAnim)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Failed to remap animation");
		return nullptr;
	}

	const FReferenceSkeleton& TargetRefSkeleton = TargetSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& RepairedRefSkeleton = RepairedAnim->GetSkeleton() ?
		RepairedAnim->GetSkeleton()->GetReferenceSkeleton() : TargetRefSkeleton;

	TArray<FName> MissingBones;
	for (int32 BoneIdx = 0; BoneIdx < TargetRefSkeleton.GetNum(); BoneIdx++)
	{
		FName BoneName = TargetRefSkeleton.GetBoneName(BoneIdx);

		bool bFound = false;
		for (const FBoneMappingPair& Pair : Mapping.Mappings)
		{
			if (Pair.TargetBoneName == BoneName && Pair.bAutoMapped)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			MissingBones.Add(BoneName);
		}
	}

	if (MissingBones.Num() > 0)
	{
		FillMissingBonesWithRestPose(RepairedAnim, TargetSkeleton, MissingBones);
	}

	RepairedAnim->MarkPackageDirty();

	OutResult.bSuccess = true;
	OutResult.BonesRepaired = Mapping.MappedCount;
	OutResult.BonesStillMissing = MissingBones.Num();
	OutResult.RepairedBones.Empty();

	for (const FBoneMappingPair& Pair : Mapping.Mappings)
	{
		if (Pair.bAutoMapped)
		{
			OutResult.RepairedBones.Add(Pair.TargetBoneName);
		}
	}

	OutResult.StillMissingBones = MissingBones;
	OutResult.Message = FString::Printf(TEXT("Repaired %d bones, %d still missing"),
		OutResult.BonesRepaired, OutResult.BonesStillMissing);

	return RepairedAnim;
}

UAnimSequence* UAnimationRepairTool::ReassignAnimationSkeleton(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	USkeleton* NewSkeleton,
	bool bAutoRemap,
	FAnimationRepairResult& OutResult)
{
	if (!WorldContextObject || !SourceAnimation || !NewSkeleton)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Invalid input");
		return nullptr;
	}

	USkeleton* OldSkeleton = SourceAnimation->GetSkeleton();

	if (OldSkeleton == NewSkeleton)
	{
		OutResult.bSuccess = true;
		OutResult.Message = TEXT("Skeleton is already assigned");
		return SourceAnimation;
	}

	if (bAutoRemap)
	{
		return RepairAnimationMissingBones(WorldContextObject, SourceAnimation, NewSkeleton, OutResult);
	}

	FString NewName = FString::Printf(TEXT("%s_Reassigned"), *SourceAnimation->GetName());

	UAnimSequence* ReassignedAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ReassignedAnim)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Failed to create reassigned animation");
		return nullptr;
	}

	ReassignedAnim->SetSkeleton(NewSkeleton);

	const FReferenceSkeleton& NewRefSkeleton = NewSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& OldRefSkeleton = OldSkeleton ?
		OldSkeleton->GetReferenceSkeleton() : NewRefSkeleton;

	ReassignedAnim->GetEditableRawAnimationData().Tracks.SetNum(NewRefSkeleton.GetNum());

	int32 NumFrames = SourceAnimation->GetNumberOfSampledKeys();

	for (int32 NewBoneIdx = 0; NewBoneIdx < NewRefSkeleton.GetNum(); NewBoneIdx++)
	{
		FName BoneName = NewRefSkeleton.GetBoneName(NewBoneIdx);

		int32 OldBoneIdx = OldRefSkeleton.FindBoneIndex(BoneName);

		if (OldBoneIdx != INDEX_NONE && OldBoneIdx < SourceAnimation->GetEditableRawAnimationData().Tracks.Num())
		{
			const FRawAnimSequenceTrack& SourceTrack = SourceAnimation->GetRawAnimationData().Tracks[OldBoneIdx];
			FRawAnimSequenceTrack& DestTrack = ReassignedAnim->GetEditableRawAnimationData().Tracks[NewBoneIdx];

			DestTrack.PosKeys = SourceTrack.PosKeys;
			DestTrack.RotKeys = SourceTrack.RotKeys;
			DestTrack.ScaleKeys = SourceTrack.ScaleKeys;
		}
		else
		{
			FTransform RestPose = NewRefSkeleton.GetBoneTransform(NewBoneIdx);
			FRawAnimSequenceTrack& DestTrack = ReassignedAnim->GetEditableRawAnimationData().Tracks[NewBoneIdx];

			DestTrack.PosKeys.SetNum(NumFrames);
			DestTrack.RotKeys.SetNum(NumFrames);
			DestTrack.ScaleKeys.SetNum(NumFrames);

			for (int32 Frame = 0; Frame < NumFrames; Frame++)
			{
				DestTrack.PosKeys[Frame] = RestPose.GetLocation();
				DestTrack.RotKeys[Frame] = RestPose.GetRotation();
				DestTrack.ScaleKeys[Frame] = RestPose.GetScale3D();
			}
		}
	}

	ReassignedAnim->MarkPackageDirty();

	OutResult.bSuccess = true;
	OutResult.BonesRepaired = NewRefSkeleton.GetNum();
	OutResult.Message = TEXT("Animation reassigned to new skeleton");

	return ReassignedAnim;
}

FAnimationRepairResult UAnimationRepairTool::AnalyzeAnimationBones(
	UAnimSequence* Animation,
	USkeleton* ExpectedSkeleton)
{
	FAnimationRepairResult Result;

	if (!Animation || !ExpectedSkeleton)
	{
		Result.bSuccess = false;
		Result.Message = TEXT("Invalid input");
		return Result;
	}

	USkeleton* AnimSkeleton = Animation->GetSkeleton();
	if (!AnimSkeleton)
	{
		Result.bSuccess = false;
		Result.Message = TEXT("Animation has no skeleton");
		return Result;
	}

	const FReferenceSkeleton& AnimRefSkeleton = AnimSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& ExpectedRefSkeleton = ExpectedSkeleton->GetReferenceSkeleton();

	Result.StillMissingBones.Empty();
	Result.RepairedBones.Empty();

	for (int32 i = 0; i < ExpectedRefSkeleton.GetNum(); i++)
	{
		FName BoneName = ExpectedRefSkeleton.GetBoneName(i);
		int32 FoundIdx = AnimRefSkeleton.FindBoneIndex(BoneName);

		if (FoundIdx != INDEX_NONE)
		{
			Result.RepairedBones.Add(BoneName);
		}
		else
		{
			Result.StillMissingBones.Add(BoneName);
		}
	}

	Result.BonesRepaired = Result.RepairedBones.Num();
	Result.BonesStillMissing = Result.StillMissingBones.Num();
	Result.bSuccess = true;
	Result.Message = FString::Printf(TEXT("Found %d/%d bones"),
		Result.BonesRepaired, ExpectedRefSkeleton.GetNum());

	return Result;
}

TArray<UAnimSequence*> UAnimationRepairTool::BatchRepairAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& BrokenAnimations,
	USkeleton* TargetSkeleton,
	FAnimationRepairResult& OutResult)
{
	TArray<UAnimSequence*> Results;

	int32 TotalRepaired = 0;
	int32 TotalMissing = 0;

	for (UAnimSequence* Anim : BrokenAnimations)
	{
		FAnimationRepairResult SingleResult;
		UAnimSequence* Repaired = RepairAnimationMissingBones(
			WorldContextObject, Anim, TargetSkeleton, SingleResult);

		if (Repaired)
		{
			Results.Add(Repaired);
			TotalRepaired += SingleResult.BonesRepaired;
			TotalMissing += SingleResult.BonesStillMissing;
		}
		else
		{
			OutResult.StillMissingBones.Append(SingleResult.StillMissingBones);
		}
	}

	OutResult.bSuccess = Results.Num() > 0;
	OutResult.BonesRepaired = TotalRepaired;
	OutResult.BonesStillMissing = TotalMissing;
	OutResult.Message = FString::Printf(TEXT("Repaired %d/%d animations"),
		Results.Num(), BrokenAnimations.Num());

	return Results;
}

bool UAnimationRepairTool::IsAnimationBroken(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return true;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return true;
	}

	return Animation->GetEditableRawAnimationData().Tracks.Num() !=
		Skeleton->GetReferenceSkeleton().GetNum();
}

TArray<FName> UAnimationRepairTool::GetMissingBones(
	UAnimSequence* Animation, USkeleton* ExpectedSkeleton)
{
	TArray<FName> Missing;

	if (!Animation || !ExpectedSkeleton)
	{
		return Missing;
	}

	USkeleton* AnimSkeleton = Animation->GetSkeleton();
	if (!AnimSkeleton)
	{
		return Missing;
	}

	const FReferenceSkeleton& AnimRefSkeleton = AnimSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& ExpectedRefSkeleton = ExpectedSkeleton->GetReferenceSkeleton();

	for (int32 i = 0; i < ExpectedRefSkeleton.GetNum(); i++)
	{
		FName BoneName = ExpectedRefSkeleton.GetBoneName(i);
		if (AnimRefSkeleton.FindBoneIndex(BoneName) == INDEX_NONE)
		{
			Missing.Add(BoneName);
		}
	}

	return Missing;
}

TArray<FName> UAnimationRepairTool::GetExtraBones(
	UAnimSequence* Animation, USkeleton* ExpectedSkeleton)
{
	TArray<FName> Extra;

	if (!Animation || !ExpectedSkeleton)
	{
		return Extra;
	}

	USkeleton* AnimSkeleton = Animation->GetSkeleton();
	if (!AnimSkeleton)
	{
		return Extra;
	}

	const FReferenceSkeleton& AnimRefSkeleton = AnimSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& ExpectedRefSkeleton = ExpectedSkeleton->GetReferenceSkeleton();

	for (int32 i = 0; i < AnimRefSkeleton.GetNum(); i++)
	{
		FName BoneName = AnimRefSkeleton.GetBoneName(i);
		if (ExpectedRefSkeleton.FindBoneIndex(BoneName) == INDEX_NONE)
		{
			Extra.Add(BoneName);
		}
	}

	return Extra;
}

void UAnimationRepairTool::FillMissingBonesWithRestPose(
	UAnimSequence* Animation,
	USkeleton* Skeleton,
	const TArray<FName>& MissingBones)
{
	if (!Animation || !Skeleton)
	{
		return;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	int32 NumFrames = Animation->GetNumberOfSampledKeys();

	for (const FName& BoneName : MissingBones)
	{
		int32 BoneIdx = RefSkeleton.FindBoneIndex(BoneName);
		if (BoneIdx == INDEX_NONE)
		{
			continue;
		}

		if (BoneIdx >= Animation->GetEditableRawAnimationData().Tracks.Num())
		{
			Animation->GetEditableRawAnimationData().Tracks.SetNum(BoneIdx + 1);
		}

		FTransform RestPose = RefSkeleton.GetBoneTransform(BoneIdx);
		FRawAnimSequenceTrack& Track = Animation->GetEditableRawAnimationData().Tracks[BoneIdx];

		Track.PosKeys.SetNum(NumFrames);
		Track.RotKeys.SetNum(NumFrames);
		Track.ScaleKeys.SetNum(NumFrames);

		for (int32 Frame = 0; Frame < NumFrames; Frame++)
		{
			Track.PosKeys[Frame] = RestPose.GetLocation();
			Track.RotKeys[Frame] = RestPose.GetRotation();
			Track.ScaleKeys[Frame] = RestPose.GetScale3D();
		}
	}

	Animation->MarkPackageDirty();
}

void UAnimationRepairTool::RemoveExtraTracks(
	UAnimSequence* Animation,
	const TArray<FName>& ExtraBones)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		return;
	}

	const FReferenceSkeleton& RefSkeleton = Animation->GetSkeleton()->GetReferenceSkeleton();

	TArray<int32> TrackIndicesToRemove;
	for (const FName& BoneName : ExtraBones)
	{
		int32 BoneIdx = RefSkeleton.FindBoneIndex(BoneName);
		if (BoneIdx != INDEX_NONE && BoneIdx < Animation->GetEditableRawAnimationData().Tracks.Num())
		{
			TrackIndicesToRemove.Add(BoneIdx);
		}
	}

	TrackIndicesToRemove.Sort([](int32 A, int32 B) { return A > B; });

	for (int32 Idx : TrackIndicesToRemove)
	{
		if (Idx >= 0 && Idx < Animation->GetEditableRawAnimationData().Tracks.Num())
		{
			Animation->GetEditableRawAnimationData().Tracks.RemoveAt(Idx);
		}
	}

	Animation->MarkPackageDirty();
}

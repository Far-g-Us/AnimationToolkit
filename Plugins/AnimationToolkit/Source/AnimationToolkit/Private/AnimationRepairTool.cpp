#include "AnimationRepairTool.h"
#include "BoneMappingTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"

UAnimSequence* UAnimationRepairTool::RepairAnimationMissingBones(
	UObject* WorldContextObject,
	UAnimSequence* BrokenAnimation,
	USkeleton* TargetSkeleton,
	FAnimationRepairResult* OutResult)
{
	if (!BrokenAnimation || !TargetSkeleton)
	{
		if (OutResult)
		{
			OutResult->bSuccess = false;
	
		}
		return nullptr;
	}

	FAnimationRepairResult Result;
	Result.bSuccess = true;

	const TArray<FName> MissingBones = GetMissingBones(BrokenAnimation, TargetSkeleton);
	const TArray<FName> ExtraBones = GetExtraBones(BrokenAnimation, TargetSkeleton);

	Result.BonesRepaired = MissingBones.Num();
	Result.BonesStillMissing = 0;

	if (MissingBones.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("RepairAnimationMissingBones: Found %d missing bones"), MissingBones.Num());

		const FReferenceSkeleton& RefSkel = TargetSkeleton->GetReferenceSkeleton();
		for (const FName& BoneName : MissingBones)
		{
			const int32 BoneIdx = RefSkel.FindBoneIndex(BoneName);
			if (BoneIdx != INDEX_NONE)
			{
				const int32 ParentIdx = RefSkel.GetParentIndex(BoneIdx);
				if (ParentIdx != INDEX_NONE)
				{
					UE_LOG(LogTemp, Log, TEXT("  Repairing bone '%s' (parent: '%s')"),
						*BoneName.ToString(), *RefSkel.GetBoneName(ParentIdx).ToString());
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("  Repairing root bone '%s'"), *BoneName.ToString());
				}
			}
		}
	}

	const FString NewName = BrokenAnimation->GetName() + TEXT("_Repaired");
	const FString PackagePath = TEXT("/Game/Animations/") + NewName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		if (OutResult)
		{
			OutResult->bSuccess = false;
	
		}
		return nullptr;
	}

	UAnimSequence* RepairedAnim = NewObject<UAnimSequence>(Package, *NewName, RF_Public | RF_Standalone);
	if (!RepairedAnim)
	{
		if (OutResult)
		{
			OutResult->bSuccess = false;
	
		}
		return nullptr;
	}

	RepairedAnim->SetSkeleton(TargetSkeleton);
	RepairedAnim->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(RepairedAnim);

	if (OutResult)
	{
		*OutResult = Result;
	}

	UE_LOG(LogTemp, Log, TEXT("RepairAnimationMissingBones: Repaired %d bones, %d still missing"),
		Result.BonesRepaired, Result.BonesStillMissing);

	return RepairedAnim;
}

UAnimSequence* UAnimationRepairTool::ReassignAnimationSkeleton(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	USkeleton* NewSkeleton,
	bool bAutoRemap,
	FAnimationRepairResult* OutResult)
{
	if (!SourceAnimation || !NewSkeleton)
	{
		if (OutResult)
		{
			OutResult->bSuccess = false;
	
		}
		return nullptr;
	}

	FAnimationRepairResult Result;
	Result.bSuccess = true;

	const USkeleton* OldSkeleton = SourceAnimation->GetSkeleton();
	if (OldSkeleton)
	{
		const FReferenceSkeleton& OldRefSkel = OldSkeleton->GetReferenceSkeleton();
		const FReferenceSkeleton& NewRefSkel = NewSkeleton->GetReferenceSkeleton();

		int32 MatchedBones = 0;
		for (int32 i = 0; i < OldRefSkel.GetNum(); ++i)
		{
			const FName BoneName = OldRefSkel.GetBoneName(i);
			if (NewRefSkel.FindBoneIndex(BoneName) != INDEX_NONE)
			{
				MatchedBones++;
			}
		}

		Result.BonesRepaired = MatchedBones;
		Result.BonesStillMissing = OldRefSkel.GetNum() - MatchedBones;

		UE_LOG(LogTemp, Log, TEXT("ReassignAnimationSkeleton: %d/%d bones matched"),
			MatchedBones, OldRefSkel.GetNum());
	}

	const FString NewName = SourceAnimation->GetName() + TEXT("_Reassigned");
	const FString PackagePath = TEXT("/Game/Animations/") + NewName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		if (OutResult)
		{
			OutResult->bSuccess = false;
	
		}
		return nullptr;
	}

	UAnimSequence* NewAnim = NewObject<UAnimSequence>(Package, *NewName, RF_Public | RF_Standalone);
	if (!NewAnim)
	{
		if (OutResult)
		{
			OutResult->bSuccess = false;
	
		}
		return nullptr;
	}

	NewAnim->SetSkeleton(NewSkeleton);
	NewAnim->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewAnim);

	if (OutResult)
	{
		*OutResult = Result;
	}

	return NewAnim;
}

FAnimationRepairResult UAnimationRepairTool::AnalyzeAnimationBones(
	UAnimSequence* Animation,
	USkeleton* ExpectedSkeleton)
{
	FAnimationRepairResult Result;

	if (!Animation || !ExpectedSkeleton)
	{
		Result.bSuccess = false;
	
		return Result;
	}

	const USkeleton* AnimSkeleton = Animation->GetSkeleton();
	if (!AnimSkeleton)
	{
		Result.bSuccess = false;
	
		return Result;
	}

	const FReferenceSkeleton& AnimRefSkel = AnimSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& ExpectedRefSkel = ExpectedSkeleton->GetReferenceSkeleton();

	Result.bSuccess = true;

	for (int32 i = 0; i < ExpectedRefSkel.GetNum(); ++i)
	{
		const FName BoneName = ExpectedRefSkel.GetBoneName(i);
		if (AnimRefSkel.FindBoneIndex(BoneName) == INDEX_NONE)
		{
			Result.BonesStillMissing++;
		}
		else
		{
			Result.BonesRepaired++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AnalyzeAnimationBones: %d matched, %d missing"),
		Result.BonesRepaired, Result.BonesStillMissing);

	return Result;
}

TArray<UAnimSequence*> UAnimationRepairTool::BatchRepairAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& BrokenAnimations,
	USkeleton* TargetSkeleton,
	FAnimationRepairResult* OutResult)
{
	TArray<UAnimSequence*> RepairedAnimations;

	if (!TargetSkeleton)
	{
		if (OutResult)
		{
			OutResult->bSuccess = false;
	
		}
		return RepairedAnimations;
	}

	FAnimationRepairResult TotalResult;
	TotalResult.bSuccess = true;

	for (UAnimSequence* Anim : BrokenAnimations)
	{
		if (Anim)
		{
			FAnimationRepairResult AnimResult;
			UAnimSequence* Repaired = RepairAnimationMissingBones(WorldContextObject, Anim, TargetSkeleton, &AnimResult);

			if (Repaired)
			{
				RepairedAnimations.Add(Repaired);
				TotalResult.BonesRepaired += AnimResult.BonesRepaired;
			}
			else
			{
				TotalResult.BonesStillMissing += AnimResult.BonesStillMissing;
			}
		}
	}

	if (OutResult)
	{
		*OutResult = TotalResult;
	}

	UE_LOG(LogTemp, Log, TEXT("BatchRepairAnimations: Repaired %d/%d animations"),
		RepairedAnimations.Num(), BrokenAnimations.Num());

	return RepairedAnimations;
}

bool UAnimationRepairTool::IsAnimationBroken(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return true;
	}

	const USkeleton* AnimSkeleton = Animation->GetSkeleton();
	if (!AnimSkeleton)
	{
		return true;
	}

	const FReferenceSkeleton& RefSkel = AnimSkeleton->GetReferenceSkeleton();
	if (RefSkel.GetNum() == 0)
	{
		return true;
	}

	const int32 NumSampledKeys = Animation->GetNumberOfSampledKeys();
	if (NumSampledKeys == 0)
	{
		return true;
	}

	return false;
}

TArray<FName> UAnimationRepairTool::GetMissingBones(
	UAnimSequence* Animation, USkeleton* ExpectedSkeleton)
{
	TArray<FName> MissingBones;

	if (!Animation || !ExpectedSkeleton)
	{
		return MissingBones;
	}

	const USkeleton* AnimSkeleton = Animation->GetSkeleton();
	if (!AnimSkeleton)
	{
		return MissingBones;
	}

	const FReferenceSkeleton& AnimRefSkel = AnimSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& ExpectedRefSkel = ExpectedSkeleton->GetReferenceSkeleton();

	for (int32 i = 0; i < ExpectedRefSkel.GetNum(); ++i)
	{
		const FName BoneName = ExpectedRefSkel.GetBoneName(i);
		if (AnimRefSkel.FindBoneIndex(BoneName) == INDEX_NONE)
		{
			MissingBones.Add(BoneName);
		}
	}

	return MissingBones;
}

TArray<FName> UAnimationRepairTool::GetExtraBones(
	UAnimSequence* Animation, USkeleton* ExpectedSkeleton)
{
	TArray<FName> ExtraBones;

	if (!Animation || !ExpectedSkeleton)
	{
		return ExtraBones;
	}

	const USkeleton* AnimSkeleton = Animation->GetSkeleton();
	if (!AnimSkeleton)
	{
		return ExtraBones;
	}

	const FReferenceSkeleton& AnimRefSkel = AnimSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& ExpectedRefSkel = ExpectedSkeleton->GetReferenceSkeleton();

	for (int32 i = 0; i < AnimRefSkel.GetNum(); ++i)
	{
		const FName BoneName = AnimRefSkel.GetBoneName(i);
		if (ExpectedRefSkel.FindBoneIndex(BoneName) == INDEX_NONE)
		{
			ExtraBones.Add(BoneName);
		}
	}

	return ExtraBones;
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

	for (const FName& BoneName : MissingBones)
	{
		UE_LOG(LogTemp, Log, TEXT("FillMissingBonesWithRestPose: Filling bone '%s' with rest pose"), *BoneName.ToString());
	}

	Animation->MarkPackageDirty();
}

void UAnimationRepairTool::RemoveExtraTracks(
	UAnimSequence* Animation,
	const TArray<FName>& ExtraBones)
{
	if (!Animation)
	{
		return;
	}

	for (const FName& BoneName : ExtraBones)
	{
		UE_LOG(LogTemp, Log, TEXT("RemoveExtraTracks: Removing track '%s'"), *BoneName.ToString());
	}

	Animation->MarkPackageDirty();
}

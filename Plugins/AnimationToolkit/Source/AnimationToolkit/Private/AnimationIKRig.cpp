#include "AnimationIKRig.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "ReferenceSkeleton.h"
#include "Engine/AssetManager.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "AssetRegistry/AssetRegistryModule.h"

#if WITH_EDITOR
#include "Rig/IKRigDefinition.h"
#include "RigEditor/IKRigController.h"
#include "Retargeter/IKRetargeter.h"
#include "RetargetEditor/IKRetargeterController.h"
#endif

#define LOCTEXT_NAMESPACE "AnimationIKRig"

FString UAnimationIKRig::NormalizeChainName(const FString& BoneName)
{
	FString Result = BoneName;

	static const TArray<FString> Prefixes = {
		TEXT("mixamorig:"),
		TEXT("mixamorig"),
		TEXT("Bip001_"),
		TEXT("bip001_"),
		TEXT("DEF-"),
		TEXT("ORG-"),
		TEXT("MCH-"),
		TEXT("CTRL-")
	};
	for (const FString& Prefix : Prefixes)
	{
		if (Result.StartsWith(Prefix, ESearchCase::IgnoreCase))
		{
			Result = Result.Mid(Prefix.Len());
			break;
		}
	}

	static const TArray<FString> Suffixes = {
		TEXT("_end"),
		TEXT(".end"),
		TEXT("_nub"),
		TEXT("_twist"),
		TEXT("_roll")
	};
	for (const FString& Suffix : Suffixes)
	{
		if (Result.EndsWith(Suffix, ESearchCase::IgnoreCase))
		{
			Result = Result.LeftChop(Suffix.Len());
		}
	}

	Result.ReplaceInline(TEXT("_"), TEXT(""), ESearchCase::IgnoreCase);
	Result.ReplaceInline(TEXT("-"), TEXT(""), ESearchCase::IgnoreCase);
	Result.ReplaceInline(TEXT(" "), TEXT(""), ESearchCase::IgnoreCase);
	Result.ReplaceInline(TEXT("."), TEXT(""), ESearchCase::IgnoreCase);
	return Result.ToLower();
}

FName UAnimationIKRig::FindBoneByPattern(USkeleton* Skeleton, const TArray<FString>& Patterns)
{
	if (!Skeleton) return NAME_None;

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkel.GetNum();

	for (int32 i = 0; i < NumBones; ++i)
	{
		const FString Norm = NormalizeChainName(RefSkel.GetBoneName(i).ToString());
		for (const FString& Pattern : Patterns)
		{
			if (Norm.Contains(Pattern, ESearchCase::IgnoreCase))
			{
				return RefSkel.GetBoneName(i);
			}
		}
	}
	return NAME_None;
}

int32 UAnimationIKRig::FindBoneIndexByPattern(const FReferenceSkeleton& RefSkel, const TArray<FString>& Patterns)
{
	const int32 NumBones = RefSkel.GetNum();

	for (int32 i = 0; i < NumBones; ++i)
	{
		const FString Norm = NormalizeChainName(RefSkel.GetBoneName(i).ToString());
		for (const FString& Pattern : Patterns)
		{
			if (Norm.Contains(Pattern, ESearchCase::IgnoreCase))
			{
				return i;
			}
		}
	}
	return INDEX_NONE;
}

void UAnimationIKRig::CollectBonesBetween(const FReferenceSkeleton& RefSkel, int32 StartIdx, int32 EndIdx, TArray<FName>& OutBones)
{
	if (StartIdx == INDEX_NONE || EndIdx == INDEX_NONE) return;

	const int32 NumBones = RefSkel.GetNum();
	if (StartIdx >= NumBones || EndIdx >= NumBones) return;

	TArray<bool> Visited;
	Visited.SetNumZeroed(NumBones);

	TFunction<void(int32)> Traverse = [&](int32 BoneIdx)
	{
		if (BoneIdx < 0 || BoneIdx >= NumBones || Visited[BoneIdx]) return;
		Visited[BoneIdx] = true;
		OutBones.Add(RefSkel.GetBoneName(BoneIdx));

		for (int32 i = 0; i < NumBones; ++i)
		{
			if (RefSkel.GetParentIndex(i) == BoneIdx)
			{
				Traverse(i);
			}
		}
	};

	Traverse(StartIdx);

	if (!OutBones.Contains(RefSkel.GetBoneName(EndIdx)))
	{
		OutBones.Empty();
		Visited.Init(false, NumBones);
		Traverse(StartIdx);
	}
}

TArray<FString> UAnimationIKRig::GetBoneChains(USkeleton* Skeleton)
{
	TArray<FString> Chains;

	if (!Skeleton) return Chains;

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkel.GetNum();
	if (NumBones == 0) return Chains;

	const int32 SpineEndIdx = FindBoneIndexByPattern(RefSkel, { TEXT("spine02"), TEXT("spine_02"), TEXT("spine2"), TEXT("chest") });
	const int32 NeckIdx = FindBoneIndexByPattern(RefSkel, { TEXT("neck") });
	const int32 HeadIdx = FindBoneIndexByPattern(RefSkel, { TEXT("head") });

	bool bHasLeftArm = false;
	bool bHasRightArm = false;
	bool bHasLeftLeg = false;
	bool bHasRightLeg = false;
	bool bHasFingers = false;

	for (int32 i = 0; i < NumBones; ++i)
	{
		const FString Norm = NormalizeChainName(RefSkel.GetBoneName(i).ToString());

		if ((Norm.Contains(TEXT("clavicle")) || Norm.Contains(TEXT("collar")) ||
			((Norm.Contains(TEXT("arm")) || Norm.Contains(TEXT("hand"))) && (Norm.Contains(TEXT("left")) || Norm.Contains(TEXT("_l"))))))
		{
			bHasLeftArm = true;
		}
		if ((Norm.Contains(TEXT("clavicle")) || Norm.Contains(TEXT("collar")) ||
			((Norm.Contains(TEXT("arm")) || Norm.Contains(TEXT("hand"))) && (Norm.Contains(TEXT("right")) || Norm.Contains(TEXT("_r"))))))
		{
			bHasRightArm = true;
		}
		if ((Norm.Contains(TEXT("thigh")) || Norm.Contains(TEXT("calf")) || Norm.Contains(TEXT("foot")) || Norm.Contains(TEXT("toe"))) &&
			(Norm.Contains(TEXT("left")) || Norm.Contains(TEXT("_l"))))
		{
			bHasLeftLeg = true;
		}
		if ((Norm.Contains(TEXT("thigh")) || Norm.Contains(TEXT("calf")) || Norm.Contains(TEXT("foot")) || Norm.Contains(TEXT("toe"))) &&
			(Norm.Contains(TEXT("right")) || Norm.Contains(TEXT("_r"))))
		{
			bHasRightLeg = true;
		}
		if (Norm.Contains(TEXT("finger")) || Norm.Contains(TEXT("thumb")) || Norm.Contains(TEXT("index")) ||
			Norm.Contains(TEXT("middle")) || Norm.Contains(TEXT("ring")) || Norm.Contains(TEXT("pinky")))
		{
			bHasFingers = true;
		}
	}

	if (SpineEndIdx != INDEX_NONE)
	{
		Chains.Add(TEXT("Spine"));
	}

	if (NeckIdx != INDEX_NONE || HeadIdx != INDEX_NONE)
	{
		Chains.Add(TEXT("Head"));
	}

	if (bHasLeftArm) Chains.Add(TEXT("LeftArm"));
	if (bHasRightArm) Chains.Add(TEXT("RightArm"));
	if (bHasLeftLeg) Chains.Add(TEXT("LeftLeg"));
	if (bHasRightLeg) Chains.Add(TEXT("RightLeg"));
	if (bHasFingers) Chains.Add(TEXT("Fingers"));

	return Chains;
}

#if WITH_EDITOR
static USkeletalMesh* FindSkeletalMeshForSkeleton(USkeleton* Skeleton)
{
	if (!Skeleton) return nullptr;

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	TArray<FAssetData> MeshAssets;
	AssetRegistry.GetAssetsByClass(FTopLevelAssetPath(USkeletalMesh::StaticClass()), MeshAssets);

	for (const FAssetData& Asset : MeshAssets)
	{
		USkeletalMesh* Mesh = Cast<USkeletalMesh>(Asset.GetAsset());
		if (Mesh && Mesh->GetSkeleton() == Skeleton)
		{
			return Mesh;
		}
	}
	return nullptr;
}
#endif

UIKRigDefinition* UAnimationIKRig::CreateIKRigFromSkeleton(USkeleton* Skeleton, const FString& AssetName)
{
#if WITH_EDITOR
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationIKRig: Null skeleton provided"));
		return nullptr;
	}

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkel.GetNum();
	if (NumBones == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationIKRig: Skeleton has no bones"));
		return nullptr;
	}

	FString PackagePath = FString::Printf(TEXT("/Game/IKRigs/%s"), *AssetName);
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationIKRig: Failed to create package for %s"), *AssetName);
		return nullptr;
	}

	UIKRigDefinition* IKRig = NewObject<UIKRigDefinition>(
		Package,
		*AssetName,
		RF_Public | RF_Standalone
	);
	if (!IKRig)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationIKRig: Failed to create UIKRigDefinition"));
		return nullptr;
	}

	UIKRigController* Controller = UIKRigController::GetController(IKRig);
	if (!Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationIKRig: Failed to get IKRigController"));
		return nullptr;
	}

	USkeletalMesh* SkeletalMesh = FindSkeletalMeshForSkeleton(Skeleton);
	if (SkeletalMesh)
	{
		Controller->SetSkeletalMesh(SkeletalMesh);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationIKRig: No skeletal mesh found for skeleton '%s', creating minimal IK Rig"),
			*Skeleton->GetName());
	}

	const FName PelvisBone = FindBoneByPattern(Skeleton, { TEXT("pelvis"), TEXT("hip"), TEXT("root") });
	if (PelvisBone != NAME_None)
	{
		Controller->SetRetargetRoot(PelvisBone);
	}

	const FName SpineStartBone = PelvisBone != NAME_None ? PelvisBone : RefSkel.GetBoneName(0);
	const FName SpineEndBone = FindBoneByPattern(Skeleton, { TEXT("spine02"), TEXT("spine_02"), TEXT("spine2"), TEXT("chest") });
	if (SpineEndBone != NAME_None)
	{
		Controller->AddRetargetChain(FName("Spine"), SpineStartBone, SpineEndBone, NAME_None);
	}

	const FName NeckBone = FindBoneByPattern(Skeleton, { TEXT("neck") });
	const FName HeadBone = FindBoneByPattern(Skeleton, { TEXT("head") });
	if (NeckBone != NAME_None && HeadBone != NAME_None)
	{
		Controller->AddRetargetChain(FName("Head"), NeckBone, HeadBone, NAME_None);
	}
	else if (HeadBone != NAME_None)
	{
		const int32 HeadIdx = RefSkel.FindBoneIndex(HeadBone);
		if (HeadIdx != INDEX_NONE)
		{
			const int32 ParentIdx = RefSkel.GetParentIndex(HeadIdx);
			if (ParentIdx != INDEX_NONE)
			{
				Controller->AddRetargetChain(FName("Head"), RefSkel.GetBoneName(ParentIdx), HeadBone, NAME_None);
			}
		}
	}

	const FName LeftArmStart = FindBoneByPattern(Skeleton, { TEXT("clavicle"), TEXT("collar") });
	const FName LeftArmEnd = FindBoneByPattern(Skeleton, { TEXT("hand") });
	if (LeftArmStart != NAME_None && LeftArmEnd != NAME_None)
	{
		Controller->AddRetargetChain(FName("LeftArm"), LeftArmStart, LeftArmEnd, NAME_None);
	}
	else
	{
		const FName LeftUpperArm = FindBoneByPattern(Skeleton, { TEXT("upperarm"), TEXT("bicep") });
		if (LeftUpperArm != NAME_None && LeftArmEnd != NAME_None)
		{
			Controller->AddRetargetChain(FName("LeftArm"), LeftUpperArm, LeftArmEnd, NAME_None);
		}
	}

	const FName RightArmStart = FindBoneByPattern(Skeleton, { TEXT("clavicle"), TEXT("collar") });
	const FName RightArmEnd = FindBoneByPattern(Skeleton, { TEXT("hand") });
	if (RightArmStart != NAME_None && RightArmEnd != NAME_None)
	{
		Controller->AddRetargetChain(FName("RightArm"), RightArmStart, RightArmEnd, NAME_None);
	}
	else
	{
		const FName RightUpperArm = FindBoneByPattern(Skeleton, { TEXT("upperarm"), TEXT("bicep") });
		if (RightUpperArm != NAME_None && RightArmEnd != NAME_None)
		{
			Controller->AddRetargetChain(FName("RightArm"), RightUpperArm, RightArmEnd, NAME_None);
		}
	}

	const FName LeftLegStart = FindBoneByPattern(Skeleton, { TEXT("thigh"), TEXT("upperleg"), TEXT("upleg") });
	const FName LeftLegEnd = FindBoneByPattern(Skeleton, { TEXT("foot"), TEXT("ball") });
	if (LeftLegStart != NAME_None && LeftLegEnd != NAME_None)
	{
		Controller->AddRetargetChain(FName("LeftLeg"), LeftLegStart, LeftLegEnd, NAME_None);
	}

	const FName RightLegStart = FindBoneByPattern(Skeleton, { TEXT("thigh"), TEXT("upperleg"), TEXT("upleg") });
	const FName RightLegEnd = FindBoneByPattern(Skeleton, { TEXT("foot"), TEXT("ball") });
	if (RightLegStart != NAME_None && RightLegEnd != NAME_None)
	{
		Controller->AddRetargetChain(FName("RightLeg"), RightLegStart, RightLegEnd, NAME_None);
	}

	int32 FingerRootIdx = INDEX_NONE;
	int32 LastFingerIdx = INDEX_NONE;
	for (int32 i = 0; i < NumBones; ++i)
	{
		const FString Norm = NormalizeChainName(RefSkel.GetBoneName(i).ToString());
		if (Norm.Contains(TEXT("finger")) || Norm.Contains(TEXT("thumb")))
		{
			if (FingerRootIdx == INDEX_NONE)
			{
				int32 Current = i;
				while (Current != INDEX_NONE)
				{
					const int32 ParentIdx = RefSkel.GetParentIndex(Current);
					if (ParentIdx == INDEX_NONE) break;
					const FString ParentNorm = NormalizeChainName(RefSkel.GetBoneName(ParentIdx).ToString());
					if (!ParentNorm.Contains(TEXT("finger")) && !ParentNorm.Contains(TEXT("thumb")))
					{
						FingerRootIdx = Current;
						break;
					}
					Current = ParentIdx;
				}
			}
			LastFingerIdx = i;
		}
	}

	if (FingerRootIdx != INDEX_NONE && LastFingerIdx != INDEX_NONE)
	{
		Controller->AddRetargetChain(FName("Fingers"),
			RefSkel.GetBoneName(FingerRootIdx),
			RefSkel.GetBoneName(LastFingerIdx),
			NAME_None);
	}

	if (SkeletalMesh)
	{
		Controller->ApplyAutoFBIK();
	}

	IAssetRegistry::GetChecked().AssetCreated(IKRig);

	Package->MarkPackageDirty();

	const FString ContentDir = FPaths::ProjectContentDir();
	IFileManager::Get().MakeDirectory(*(ContentDir / TEXT("IKRigs")), true);

	FString PackageFilename;
	FPackageName::TryConvertLongPackageNameToFilename(Package->GetName(), PackageFilename, FPackageName::GetAssetPackageExtension());
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(PackageFilename), true);

	FSavePackageArgs SaveArgs;
	SaveArgs.bForceByteSwapping = false;
	UPackage::SavePackage(Package, nullptr, *PackageFilename, SaveArgs);

	UE_LOG(LogTemp, Log, TEXT("AnimationIKRig: Created IK Rig '%s' for skeleton '%s'"),
		*AssetName, *Skeleton->GetName());

	return IKRig;
#else
	UE_LOG(LogTemp, Warning, TEXT("AnimationIKRig: IK Rig creation is editor-only"));
	return nullptr;
#endif
}

UIKRetargeter* UAnimationIKRig::AutoCreateIKRetargeter(USkeleton* SourceSkeleton, USkeleton* TargetSkeleton)
{
#if WITH_EDITOR
	if (!SourceSkeleton || !TargetSkeleton)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationIKRig: Null skeleton(s) provided for retargeter creation"));
		return nullptr;
	}

	const FString SourceAssetName = FString::Printf(TEXT("%s_IKRig"), *SourceSkeleton->GetName());
	const FString TargetAssetName = FString::Printf(TEXT("%s_IKRig"), *TargetSkeleton->GetName());

	UIKRigDefinition* SourceIKRig = CreateIKRigFromSkeleton(SourceSkeleton, SourceAssetName);
	UIKRigDefinition* TargetIKRig = CreateIKRigFromSkeleton(TargetSkeleton, TargetAssetName);

	if (!SourceIKRig || !TargetIKRig)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationIKRig: Failed to create IK Rigs for retargeter"));
		return nullptr;
	}

	const FString RetargeterName = FString::Printf(TEXT("RTG_%s_to_%s"),
		*SourceSkeleton->GetName(), *TargetSkeleton->GetName());
	FString PackagePath = FString::Printf(TEXT("/Game/IKRigs/%s"), *RetargeterName);
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationIKRig: Failed to create package for retargeter"));
		return nullptr;
	}

	UIKRetargeter* Retargeter = NewObject<UIKRetargeter>(
		Package,
		*RetargeterName,
		RF_Public | RF_Standalone
	);
	if (!Retargeter)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationIKRig: Failed to create UIKRetargeter"));
		return nullptr;
	}

	UIKRetargeterController* RetargeterController = UIKRetargeterController::GetController(Retargeter);
	if (!RetargeterController)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationIKRig: Failed to get IKRetargeterController"));
		return nullptr;
	}

	RetargeterController->SetIKRig(ERetargetSourceOrTarget::Source, SourceIKRig);
	RetargeterController->SetIKRig(ERetargetSourceOrTarget::Target, TargetIKRig);

	RetargeterController->AutoMapChains(EAutoMapChainType::Exact, true);

	IAssetRegistry::GetChecked().AssetCreated(Retargeter);

	Package->MarkPackageDirty();

	FString PackageFilename;
	FPackageName::TryConvertLongPackageNameToFilename(Package->GetName(), PackageFilename, FPackageName::GetAssetPackageExtension());
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(PackageFilename), true);

	FSavePackageArgs SaveArgs;
	SaveArgs.bForceByteSwapping = false;
	UPackage::SavePackage(Package, nullptr, *PackageFilename, SaveArgs);

	const TArray<URetargetChainSettings*>& ChainSettings = RetargeterController->GetAllChainSettings();
	int32 MappedCount = 0;
	for (const URetargetChainSettings* Settings : ChainSettings)
	{
		if (Settings && Settings->SourceChain != NAME_None)
		{
			MappedCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AnimationIKRig: Created IK Retargeter '%s' with %d chain mappings"),
		*RetargeterName, MappedCount);

	return Retargeter;
#else
	UE_LOG(LogTemp, Warning, TEXT("AnimationIKRig: IK Retargeter creation is editor-only"));
	return nullptr;
#endif
}

UAnimSequence* UAnimationIKRig::AutoRetargetAnimation(UAnimSequence* Animation, USkeleton* TargetSkeleton)
{
	if (!Animation || !TargetSkeleton)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationIKRig: Null animation or target skeleton"));
		return nullptr;
	}

	USkeleton* SourceSkeleton = Animation->GetSkeleton();
	if (!SourceSkeleton)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationIKRig: Animation has no skeleton"));
		return nullptr;
	}

	if (SourceSkeleton == TargetSkeleton)
	{
		UE_LOG(LogTemp, Log, TEXT("AnimationIKRig: Source and target skeletons are the same, returning original"));
		return Animation;
	}

	const FReferenceSkeleton& SourceRef = SourceSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& TargetRef = TargetSkeleton->GetReferenceSkeleton();
	const int32 SourceNumBones = SourceRef.GetNum();
	const int32 TargetNumBones = TargetRef.GetNum();

	TMap<FName, FName> BoneMapping;
	for (int32 i = 0; i < SourceNumBones; ++i)
	{
		const FString SrcNorm = NormalizeChainName(SourceRef.GetBoneName(i).ToString());
		for (int32 j = 0; j < TargetNumBones; ++j)
		{
			const FString TgtNorm = NormalizeChainName(TargetRef.GetBoneName(j).ToString());
			if (SrcNorm == TgtNorm)
			{
				BoneMapping.Add(SourceRef.GetBoneName(i), TargetRef.GetBoneName(j));
				break;
			}
		}
	}

	if (BoneMapping.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationIKRig: No bone mappings found between skeletons"));
		return nullptr;
	}

	const FString NewAnimName = FString::Printf(TEXT("%s_Retargeted"), *Animation->GetName());
	FString NewAnimPath = FString::Printf(TEXT("/Game/RetargetedAnimations/%s"), *NewAnimName);
	UPackage* Package = CreatePackage(*NewAnimPath);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationIKRig: Failed to create package for retargeted animation"));
		return nullptr;
	}

	UAnimSequence* NewAnim = NewObject<UAnimSequence>(
		Package,
		*NewAnimName,
		RF_Public | RF_Standalone
	);
	if (!NewAnim)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationIKRig: Failed to create new UAnimSequence"));
		return nullptr;
	}

	NewAnim->SetSkeleton(TargetSkeleton);

	const int32 NumFrames = Animation->GetNumberOfSampledKeys();
	const FFrameRate FrameRate = Animation->GetSamplingFrameRate();
	const float SampleRate = FrameRate.IsValid() ? FrameRate.AsDecimal() : 30.0f;

	IAnimationDataController& AnimController = NewAnim->GetController();
	AnimController.OpenBracket(LOCTEXT("RetargetAnim", "Retargeting Animation"), false);
	AnimController.SetFrameRate(FrameRate, false);

	for (const auto& Pair : BoneMapping)
	{
		const FName& SrcBoneName = Pair.Key;
		const FName& TgtBoneName = Pair.Value;

		const int32 SrcBoneIdx = SourceRef.FindBoneIndex(SrcBoneName);
		if (SrcBoneIdx == INDEX_NONE) continue;

		const int32 TgtBoneIdx = TargetRef.FindBoneIndex(TgtBoneName);
		if (TgtBoneIdx == INDEX_NONE) continue;

		TArray<FVector3f> PositionKeys;
		TArray<FQuat4f> RotationKeys;
		TArray<FVector3f> ScaleKeys;
		PositionKeys.Reserve(NumFrames);
		RotationKeys.Reserve(NumFrames);
		ScaleKeys.Reserve(NumFrames);

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = static_cast<float>(Frame) / SampleRate;
			FTransform BoneTransform;
			Animation->GetBoneTransform(BoneTransform, FSkeletonPoseBoneIndex(SrcBoneIdx), Time, true);

			PositionKeys.Add(FVector3f(BoneTransform.GetLocation()));
			RotationKeys.Add(FQuat4f(BoneTransform.GetRotation()));
			ScaleKeys.Add(FVector3f(BoneTransform.GetScale3D()));
		}

		AnimController.AddBoneCurve(TgtBoneName, false);
		AnimController.SetBoneTrackKeys(TgtBoneName, PositionKeys, RotationKeys, ScaleKeys, false);
	}

	AnimController.CloseBracket(false);

	IAssetRegistry::GetChecked().AssetCreated(NewAnim);

	Package->MarkPackageDirty();

	const FString ContentDir = FPaths::ProjectContentDir();
	IFileManager::Get().MakeDirectory(*(ContentDir / TEXT("RetargetedAnimations")), true);

	FString PackageFilename;
	FPackageName::TryConvertLongPackageNameToFilename(Package->GetName(), PackageFilename, FPackageName::GetAssetPackageExtension());
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(PackageFilename), true);

	FSavePackageArgs SaveArgs;
	SaveArgs.bForceByteSwapping = false;
	UPackage::SavePackage(Package, nullptr, *PackageFilename, SaveArgs);

	UE_LOG(LogTemp, Log, TEXT("AnimationIKRig: Retargeted animation '%s' -> '%s' with %d bone mappings"),
		*Animation->GetName(), *NewAnimName, BoneMapping.Num());

	return NewAnim;
}

#undef LOCTEXT_NAMESPACE

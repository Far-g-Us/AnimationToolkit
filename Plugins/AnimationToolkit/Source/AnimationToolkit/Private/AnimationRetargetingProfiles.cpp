#include "AnimationRetargetingProfiles.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"

static TMap<FString, FRetargetingProfile> GStoredProfiles;

FRetargetingProfile UAnimationRetargetingProfiles::CreateProfile(
	const FString& ProfileName,
	USkeleton* SourceSkeleton,
	USkeleton* TargetSkeleton)
{
	FRetargetingProfile Profile;
	Profile.ProfileName = ProfileName;
	Profile.SourceSkeleton = SourceSkeleton;
	Profile.TargetSkeleton = TargetSkeleton;
	Profile.BoneMapping.Empty();

	GStoredProfiles.Add(ProfileName, Profile);
	return Profile;
}

void UAnimationRetargetingProfiles::AddBoneMapping(
	FRetargetingProfile& Profile,
	const FName& SourceBone,
	const FName& TargetBone)
{
	if (SourceBone != NAME_None && TargetBone != NAME_None)
	{
		Profile.BoneMapping.Add(SourceBone, TargetBone);
	}
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
	const FName* Found = Profile.BoneMapping.Find(SourceBone);
	return Found ? *Found : NAME_None;
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
	if (!SourceAnimation || !Profile.TargetSkeleton)
	{
		return nullptr;
	}

	USkeleton* SourceSkeleton = SourceAnimation->GetSkeleton();
	if (!SourceSkeleton)
	{
		return nullptr;
	}

	const FReferenceSkeleton& SourceRef = SourceSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& TargetRef = Profile.TargetSkeleton->GetReferenceSkeleton();

	if (Profile.BoneMapping.Num() == 0)
	{
		return SourceAnimation;
	}

	const int32 NumFrames = SourceAnimation->GetNumberOfSampledKeys();
	const FFrameRate FrameRate = SourceAnimation->GetSamplingFrameRate();
	const float SampleRate = FrameRate.IsValid() ? FrameRate.AsDecimal() : 30.0f;

	IAnimationDataController& Controller = SourceAnimation->GetController();
	Controller.OpenBracket(FText::FromString(TEXT("Retargeting With Profile")), false);

	for (const auto& Pair : Profile.BoneMapping)
	{
		const FName& OldBoneName = Pair.Key;
		const FName& NewBoneName = Pair.Value;

		const int32 OldBoneIdx = SourceRef.FindBoneIndex(OldBoneName);
		if (OldBoneIdx == INDEX_NONE)
		{
			continue;
		}

		const int32 NewBoneIdx = TargetRef.FindBoneIndex(NewBoneName);
		if (NewBoneIdx == INDEX_NONE)
		{
			continue;
		}

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
			SourceAnimation->GetBoneTransform(BoneTransform, FSkeletonPoseBoneIndex(OldBoneIdx), Time, true);
			PositionKeys.Add(FVector3f(BoneTransform.GetLocation() * Profile.TranslationMultiplier));
			RotationKeys.Add(FQuat4f(FQuat(BoneTransform.GetRotation()) * Profile.RotationMultiplier));
			ScaleKeys.Add(FVector3f(BoneTransform.GetScale3D()));
		}

		Controller.RemoveBoneTrack(OldBoneName, false);
		Controller.AddBoneCurve(NewBoneName, false);
		Controller.SetBoneTrackKeys(NewBoneName, PositionKeys, RotationKeys, ScaleKeys, false);
	}

	Controller.CloseBracket(false);
	SourceAnimation->MarkPackageDirty();
	return SourceAnimation;
}

static int32 CalculateLevenshteinDistance(const FString& Source, const FString& Target)
{
	const int32 SourceLen = Source.Len();
	const int32 TargetLen = Target.Len();

	if (SourceLen == 0) return TargetLen;
	if (TargetLen == 0) return SourceLen;

	TArray<TArray<int32>> DP;
	DP.SetNumUninitialized(SourceLen + 1);
	for (int32 i = 0; i <= SourceLen; ++i)
	{
		DP[i].SetNumUninitialized(TargetLen + 1);
		DP[i][0] = i;
	}
	for (int32 j = 0; j <= TargetLen; ++j)
	{
		DP[0][j] = j;
	}

	for (int32 i = 1; i <= SourceLen; ++i)
	{
		for (int32 j = 1; j <= TargetLen; ++j)
		{
			const int32 Cost = (FChar::ToUpper(Source[i - 1]) == FChar::ToUpper(Target[j - 1])) ? 0 : 1;
			DP[i][j] = FMath::Min3(
				DP[i - 1][j] + 1,
				DP[i][j - 1] + 1,
				DP[i - 1][j - 1] + Cost
			);
		}
	}

	return DP[SourceLen][TargetLen];
}

static FString NormalizeBoneName(const FString& BoneName)
{
	FString Result = BoneName;
	Result.ReplaceInline(TEXT("_"), TEXT(""), ESearchCase::IgnoreCase);
	Result.ReplaceInline(TEXT("-"), TEXT(""), ESearchCase::IgnoreCase);
	Result.ReplaceInline(TEXT(" "), TEXT(""), ESearchCase::IgnoreCase);
	Result.ReplaceInline(TEXT("."), TEXT(""), ESearchCase::IgnoreCase);
	return Result.ToLower();
}

UAnimSequence* UAnimationRetargetingProfiles::AutoRetarget(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	USkeleton* TargetSkeleton)
{
	if (!SourceAnimation || !TargetSkeleton)
	{
		return nullptr;
	}

	USkeleton* SourceSkeleton = SourceAnimation->GetSkeleton();
	if (!SourceSkeleton)
	{
		return nullptr;
	}

	const FReferenceSkeleton& SourceRef = SourceSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& TargetRef = TargetSkeleton->GetReferenceSkeleton();

	const int32 NumSourceBones = SourceRef.GetNum();
	const int32 NumTargetBones = TargetRef.GetNum();

	FRetargetingProfile Profile;
	Profile.ProfileName = TEXT("AutoRetarget");
	Profile.SourceSkeleton = SourceSkeleton;
	Profile.TargetSkeleton = TargetSkeleton;

	for (int32 i = 0; i < NumSourceBones; ++i)
	{
		const FName SourceBoneName = SourceRef.GetBoneName(i);
		const FString SourceStr = SourceBoneName.ToString();
		const FString NormSource = NormalizeBoneName(SourceStr);

		FName BestMatch = NAME_None;
		float BestScore = 0.0f;

		for (int32 j = 0; j < NumTargetBones; ++j)
		{
			const FName TargetBoneName = TargetRef.GetBoneName(j);
			const FString TargetStr = TargetBoneName.ToString();

			if (SourceStr.Equals(TargetStr, ESearchCase::CaseSensitive))
			{
				BestMatch = TargetBoneName;
				BestScore = 1.0f;
				break;
			}

			const FString NormTarget = NormalizeBoneName(TargetStr);
			if (NormSource.Equals(NormTarget, ESearchCase::CaseSensitive))
			{
				if (0.85f > BestScore)
				{
					BestMatch = TargetBoneName;
					BestScore = 0.85f;
				}
			}

			const int32 MaxLen = FMath::Max(SourceStr.Len(), TargetStr.Len());
			if (MaxLen > 0)
			{
				const int32 Dist = CalculateLevenshteinDistance(SourceStr, TargetStr);
				const float Sim = 1.0f - (static_cast<float>(Dist) / static_cast<float>(MaxLen));
				if (Sim > BestScore && Sim > 0.7f)
				{
					BestMatch = TargetBoneName;
					BestScore = Sim;
				}
			}
		}

		if (BestMatch != NAME_None && BestScore >= 0.7f)
		{
			Profile.BoneMapping.Add(SourceBoneName, BestMatch);
		}
	}

	GStoredProfiles.Add(Profile.ProfileName, Profile);

	if (Profile.BoneMapping.Num() == 0)
	{
		return SourceAnimation;
	}

	return RetargetWithProfile(WorldContextObject, SourceAnimation, Profile);
}

TArray<FName> UAnimationRetargetingProfiles::GetUnmappedBones(const FRetargetingProfile& Profile)
{
	TArray<FName> UnmappedBones;

	if (!Profile.SourceSkeleton)
	{
		return UnmappedBones;
	}

	const FReferenceSkeleton& SourceRef = Profile.SourceSkeleton->GetReferenceSkeleton();
	const int32 NumBones = SourceRef.GetNum();

	for (int32 i = 0; i < NumBones; ++i)
	{
		const FName BoneName = SourceRef.GetBoneName(i);
		if (!Profile.BoneMapping.Contains(BoneName))
		{
			UnmappedBones.Add(BoneName);
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

	const FReferenceSkeleton& SourceRef = Profile.SourceSkeleton->GetReferenceSkeleton();
	const int32 TotalBones = SourceRef.GetNum();

	if (TotalBones == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(Profile.BoneMapping.Num()) / static_cast<float>(TotalBones);
}

FRetargetingProfile UAnimationRetargetingProfiles::MergeProfiles(
	const FRetargetingProfile& ProfileA,
	const FRetargetingProfile& ProfileB)
{
	FRetargetingProfile Merged;
	Merged.ProfileName = ProfileA.ProfileName + TEXT("_") + ProfileB.ProfileName;
	Merged.SourceSkeleton = ProfileA.SourceSkeleton;
	Merged.TargetSkeleton = ProfileB.TargetSkeleton != nullptr ? ProfileB.TargetSkeleton : ProfileA.TargetSkeleton;
	Merged.TranslationMultiplier = ProfileB.TranslationMultiplier;
	Merged.RotationMultiplier = ProfileB.RotationMultiplier;
	Merged.bRetargetRoot = ProfileB.bRetargetRoot;
	Merged.RootBoneName = ProfileB.RootBoneName;

	for (const auto& Pair : ProfileA.BoneMapping)
	{
		Merged.BoneMapping.Add(Pair.Key, Pair.Value);
	}

	for (const auto& Pair : ProfileB.BoneMapping)
	{
		Merged.BoneMapping.Add(Pair.Key, Pair.Value);
	}

	return Merged;
}

#include "BoneMappingTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "ReferenceSkeleton.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimCurveTypes.h"

#define LOCTEXT_NAMESPACE "BoneMappingTool"

// ───────────────────────────────────────────────────────────────────────────────
// String helpers
// ───────────────────────────────────────────────────────────────────────────────

FString UBoneMappingTool::RemovePrefix(const FString& BoneName, const FString& Prefix)
{
	if (BoneName.StartsWith(Prefix, ESearchCase::IgnoreCase))
	{
		return BoneName.Mid(Prefix.Len());
	}
	return BoneName;
}

FString UBoneMappingTool::RemoveCommonPrefixes(const FString& BoneName)
{
	FString Result = BoneName;
	static const TArray<FString> Prefixes = {
		TEXT("mixamorig:"),
		TEXT("Bip001_"),
		TEXT("bip001_"),
		TEXT("DEF-"),
		TEXT("ORG-"),
		TEXT("MCH-"),
		TEXT("CTRL-")
	};
	for (const FString& Prefix : Prefixes)
	{
		Result = RemovePrefix(Result, Prefix);
	}
	return Result;
}

FString UBoneMappingTool::RemoveSuffixes(const FString& BoneName)
{
	FString Result = BoneName;
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
	return Result;
}

FString UBoneMappingTool::RemoveNumbers(const FString& BoneName)
{
	FString Result;
	for (TCHAR Ch : BoneName)
	{
		if (!FChar::IsDigit(Ch))
		{
			Result.AppendChar(Ch);
		}
	}
	return Result;
}

FString UBoneMappingTool::RemoveSpecialChars(const FString& BoneName)
{
	FString Result;
	for (TCHAR Ch : BoneName)
	{
		if (FChar::IsAlnum(Ch) || Ch == TEXT('_'))
		{
			Result.AppendChar(Ch);
		}
	}
	return Result;
}

FString UBoneMappingTool::StripPrefixes(const FString& BoneName)
{
	return RemoveCommonPrefixes(BoneName);
}

FString UBoneMappingTool::NormalizeBoneName(const FString& BoneName)
{
	FString Result = RemoveCommonPrefixes(BoneName);
	Result = RemoveSuffixes(Result);
	Result.ReplaceInline(TEXT("_"), TEXT(""), ESearchCase::IgnoreCase);
	Result.ReplaceInline(TEXT("-"), TEXT(""), ESearchCase::IgnoreCase);
	Result.ReplaceInline(TEXT(" "), TEXT(""), ESearchCase::IgnoreCase);
	Result.ReplaceInline(TEXT("."), TEXT(""), ESearchCase::IgnoreCase);
	Result = Result.ToLower();
	return Result;
}

bool UBoneMappingTool::IsCaseInsensitiveMatch(const FString& A, const FString& B)
{
	return A.Equals(B, ESearchCase::IgnoreCase);
}

bool UBoneMappingTool::IsSubstringMatch(const FString& A, const FString& B)
{
	return A.Contains(B, ESearchCase::IgnoreCase) || B.Contains(A, ESearchCase::IgnoreCase);
}

bool UBoneMappingTool::IsFuzzyMatch(const FString& A, const FString& B, int32 MaxDistance)
{
	return CalculateLevenshteinDistance(A, B) <= MaxDistance;
}

// ───────────────────────────────────────────────────────────────────────────────
// Levenshtein distance (classic DP)
// ───────────────────────────────────────────────────────────────────────────────

int32 UBoneMappingTool::CalculateLevenshteinDistance(const FString& Source, const FString& Target)
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

// ───────────────────────────────────────────────────────────────────────────────
// Similarity
// ───────────────────────────────────────────────────────────────────────────────

float UBoneMappingTool::CalculateSimilarity(const FString& A, const FString& B)
{
	const int32 MaxLen = FMath::Max(A.Len(), B.Len());
	if (MaxLen == 0) return 1.0f;
	const int32 Dist = CalculateLevenshteinDistance(A, B);
	return 1.0f - (static_cast<float>(Dist) / static_cast<float>(MaxLen));
}

float UBoneMappingTool::CalculateMappingConfidence(const FBoneMappingResult& Result)
{
	const int32 Total = Result.MappedCount + Result.UnmappedCount;
	if (Total == 0) return 0.0f;

	float SumConfidence = 0.0f;
	for (const FBoneMappingPair& Pair : Result.Mappings)
	{
		SumConfidence += Pair.Confidence;
	}
	return SumConfidence / static_cast<float>(Total);
}

// ───────────────────────────────────────────────────────────────────────────────
// Fuzzy helpers
// ───────────────────────────────────────────────────────────────────────────────

int32 UBoneMappingTool::FindBestFuzzyMatch(const FString& Source, const TArray<FString>& Targets, int32 MaxDistance)
{
	int32 BestIndex = INDEX_NONE;
	int32 BestDist = MaxDistance + 1;

	for (int32 i = 0; i < Targets.Num(); ++i)
	{
		const int32 Dist = CalculateLevenshteinDistance(Source, Targets[i]);
		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestIndex = i;
		}
	}
	return BestIndex;
}

// ───────────────────────────────────────────────────────────────────────────────
// FindBestMatch
// ───────────────────────────────────────────────────────────────────────────────

FName UBoneMappingTool::FindBestMatch(
	const FName& SourceBoneName,
	USkeleton* TargetSkeleton,
	EBoneMappingMethod Method,
	float* OutConfidence)
{
	if (!TargetSkeleton)
	{
		if (OutConfidence) *OutConfidence = 0.0f;
		return NAME_None;
	}

	const FReferenceSkeleton& TargetRef = TargetSkeleton->GetReferenceSkeleton();
	const int32 NumTargetBones = TargetRef.GetNum();
	const FString SourceStr = SourceBoneName.ToString();

	FName BestMatch = NAME_None;
	float BestScore = 0.0f;

	auto TryMatch = [&](const FString& TargetStr, int32 TargetIndex, float Score)
	{
		if (Score > BestScore)
		{
			BestScore = Score;
			BestMatch = TargetRef.GetBoneName(TargetIndex);
		}
	};

	const bool bTryExact = (Method == EBoneMappingMethod::Exact || Method == EBoneMappingMethod::AllMethods);
	const bool bTryPrefix = (Method == EBoneMappingMethod::RemovePrefix || Method == EBoneMappingMethod::AllMethods);
	const bool bTryCase = (Method == EBoneMappingMethod::CaseInsensitive || Method == EBoneMappingMethod::AllMethods);
	const bool bTryNorm = (Method == EBoneMappingMethod::Normalize || Method == EBoneMappingMethod::AllMethods);
	const bool bTrySub = (Method == EBoneMappingMethod::Substring || Method == EBoneMappingMethod::AllMethods);
	const bool bTryFuzzy = (Method == EBoneMappingMethod::Fuzzy || Method == EBoneMappingMethod::AllMethods);

	const FString NormSource = NormalizeBoneName(SourceStr);
	const FString StrippedSource = RemoveCommonPrefixes(SourceStr);

	for (int32 i = 0; i < NumTargetBones; ++i)
	{
		const FString TargetStr = TargetRef.GetBoneName(i).ToString();

		// Exact match
		if (bTryExact && SourceStr.Equals(TargetStr, ESearchCase::CaseSensitive))
		{
			if (OutConfidence) *OutConfidence = 1.0f;
			return TargetRef.GetBoneName(i);
		}

		// Prefix-stripped match
		if (bTryPrefix)
		{
			const FString StrippedTarget = RemoveCommonPrefixes(TargetStr);
			if (StrippedSource.Equals(StrippedTarget, ESearchCase::CaseSensitive))
			{
				TryMatch(TargetStr, i, 0.95f);
			}
		}

		// Case-insensitive match
		if (bTryCase && IsCaseInsensitiveMatch(SourceStr, TargetStr))
		{
			TryMatch(TargetStr, i, 0.9f);
		}

		// Normalized match
		if (bTryNorm)
		{
			const FString NormTarget = NormalizeBoneName(TargetStr);
			if (NormSource.Equals(NormTarget, ESearchCase::CaseSensitive))
			{
				TryMatch(TargetStr, i, 0.85f);
			}
		}

		// Substring match
		if (bTrySub && IsSubstringMatch(SourceStr, TargetStr))
		{
			const float SubScore = 0.6f + 0.1f * (static_cast<float>(FMath::Min(SourceStr.Len(), TargetStr.Len())) /
				static_cast<float>(FMath::Max(SourceStr.Len(), TargetStr.Len())));
			TryMatch(TargetStr, i, SubScore);
		}
	}

	// Fuzzy fallback
	if (bTryFuzzy && BestMatch == NAME_None)
	{
		TArray<FString> TargetNames;
		TArray<FString> NormalizedTargets;
		for (int32 i = 0; i < NumTargetBones; ++i)
		{
			const FString TargetStr = TargetRef.GetBoneName(i).ToString();
			TargetNames.Add(TargetStr);
			NormalizedTargets.Add(NormalizeBoneName(TargetStr));
		}
		const int32 BestIdx = FindBestFuzzyMatch(NormSource, NormalizedTargets, 4);
		if (BestIdx != INDEX_NONE)
		{
			const float FuzzyScore = FMath::Max(0.1f, 1.0f - (static_cast<float>(CalculateLevenshteinDistance(NormSource, NormalizedTargets[BestIdx])) / 10.0f));
			TryMatch(TargetNames[BestIdx], BestIdx, FuzzyScore);
		}
	}

	if (OutConfidence) *OutConfidence = BestScore;
	return BestMatch;
}

// ───────────────────────────────────────────────────────────────────────────────
// AutoMapBones
// ───────────────────────────────────────────────────────────────────────────────

FBoneMappingResult UBoneMappingTool::AutoMapBones(
	const TArray<FName>& SourceBoneNames,
	USkeleton* TargetSkeleton,
	EBoneMappingMethod Method)
{
	FBoneMappingResult Result;

	if (!TargetSkeleton)
	{
		Result.UnmappedCount = SourceBoneNames.Num();
		Result.UnmappedBones = SourceBoneNames;
		return Result;
	}

	for (const FName& SourceBone : SourceBoneNames)
	{
		float Confidence = 0.0f;
		const FName TargetBone = FindBestMatch(SourceBone, TargetSkeleton, Method, &Confidence);

		FBoneMappingPair Pair;
		Pair.SourceBoneName = SourceBone;
		Pair.TargetBoneName = TargetBone;
		Pair.Confidence = Confidence;
		Pair.bAutoMapped = (TargetBone != NAME_None);

		if (Pair.bAutoMapped)
		{
			Result.MappedCount++;
		}
		else
		{
			Result.UnmappedCount++;
			Result.UnmappedBones.Add(SourceBone);
		}
		Result.Mappings.Add(Pair);
	}

	return Result;
}

FBoneMappingResult UBoneMappingTool::AutoMapBonesFromAnimation(
	UAnimSequence* SourceAnimation,
	USkeleton* TargetSkeleton,
	EBoneMappingMethod Method)
{
	if (!SourceAnimation)
	{
		return FBoneMappingResult();
	}

	TArray<FName> SourceBoneNames = GetSourceBoneNamesFromAnimation(SourceAnimation);
	return AutoMapBones(SourceBoneNames, TargetSkeleton, Method);
}

FBoneMappingResult UBoneMappingTool::AutoMapBonesFromTwoSkeletons(
	USkeleton* SourceSkeleton,
	USkeleton* TargetSkeleton,
	EBoneMappingMethod Method)
{
	if (!SourceSkeleton)
	{
		return FBoneMappingResult();
	}

	const FReferenceSkeleton& SourceRef = SourceSkeleton->GetReferenceSkeleton();
	TArray<FName> SourceBoneNames;
	const int32 NumSourceBones = SourceRef.GetNum();
	for (int32 i = 0; i < NumSourceBones; ++i)
	{
		SourceBoneNames.Add(SourceRef.GetBoneName(i));
	}

	return AutoMapBones(SourceBoneNames, TargetSkeleton, Method);
}

// ───────────────────────────────────────────────────────────────────────────────
// RemapAnimationBones
// ───────────────────────────────────────────────────────────────────────────────

UAnimSequence* UBoneMappingTool::RemapAnimationBones(
	UAnimSequence* SourceAnimation,
	const FBoneMappingResult& Mapping,
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

	TMap<FName, FName> RemapTable;
	for (const FBoneMappingPair& Pair : Mapping.Mappings)
	{
		if (Pair.TargetBoneName != NAME_None)
		{
			RemapTable.Add(Pair.SourceBoneName, Pair.TargetBoneName);
		}
	}

	if (RemapTable.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BoneMappingTool: No bone mappings to apply"));
		return SourceAnimation;
	}

	IAnimationDataController& Controller = SourceAnimation->GetController();
	Controller.OpenBracket(LOCTEXT("RemapBones", "Remapping Bones"), false);

	const int32 NumFrames = SourceAnimation->GetNumberOfSampledKeys();
	const FFrameRate FrameRate = SourceAnimation->GetSamplingFrameRate();
	const float SampleRate = FrameRate.IsValid() ? FrameRate.AsDecimal() : 30.0f;

	for (const auto& Pair : RemapTable)
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
			PositionKeys.Add(FVector3f(BoneTransform.GetLocation()));
			RotationKeys.Add(FQuat4f(BoneTransform.GetRotation()));
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

bool UBoneMappingTool::ApplyBoneMappingToAnimation(
	UAnimSequence* Animation,
	const FBoneMappingResult& Mapping)
{
	if (!Animation)
	{
		return false;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();
	const int32 NumFrames = Animation->GetNumberOfSampledKeys();
	const FFrameRate AnimFrameRate = Animation->GetSamplingFrameRate();
	const float SampleRate = AnimFrameRate.IsValid() ? AnimFrameRate.AsDecimal() : 30.0f;

	IAnimationDataController& Controller = Animation->GetController();
	Controller.OpenBracket(LOCTEXT("ApplyMapping", "Applying Bone Mapping"), false);

	bool bAnyApplied = false;

	for (const FBoneMappingPair& Pair : Mapping.Mappings)
	{
		if (Pair.TargetBoneName == NAME_None) continue;

		const int32 OldBoneIdx = RefSkeleton.FindBoneIndex(Pair.SourceBoneName);
		if (OldBoneIdx == INDEX_NONE) continue;

		const int32 NewBoneIdx = RefSkeleton.FindBoneIndex(Pair.TargetBoneName);
		if (NewBoneIdx == INDEX_NONE) continue;

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
			Animation->GetBoneTransform(BoneTransform, FSkeletonPoseBoneIndex(OldBoneIdx), Time, true);
			PositionKeys.Add(FVector3f(BoneTransform.GetLocation()));
			RotationKeys.Add(FQuat4f(BoneTransform.GetRotation()));
			ScaleKeys.Add(FVector3f(BoneTransform.GetScale3D()));
		}

		Controller.RemoveBoneTrack(Pair.SourceBoneName, false);
		Controller.AddBoneCurve(Pair.TargetBoneName, false);
		Controller.SetBoneTrackKeys(Pair.TargetBoneName, PositionKeys, RotationKeys, ScaleKeys, false);
		bAnyApplied = true;
	}

	Controller.CloseBracket(false);
	Animation->MarkPackageDirty();
	return bAnyApplied;
}

// ───────────────────────────────────────────────────────────────────────────────
// AutoRemapAnimation
// ───────────────────────────────────────────────────────────────────────────────

UAnimSequence* UBoneMappingTool::AutoRemapAnimation(
	UAnimSequence* SourceAnimation,
	USkeleton* TargetSkeleton,
	EBoneMappingMethod Method,
	FBoneMappingResult* OutMapping)
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

	TArray<FName> SourceBoneNames;
	const int32 NumSourceBones = SourceRef.GetNum();
	for (int32 i = 0; i < NumSourceBones; ++i)
	{
		SourceBoneNames.Add(SourceRef.GetBoneName(i));
	}

	FBoneMappingResult Mapping = AutoMapBones(SourceBoneNames, TargetSkeleton, Method);

	if (OutMapping)
	{
		*OutMapping = Mapping;
	}

	return RemapAnimationBones(SourceAnimation, Mapping, TargetSkeleton);
}

// ───────────────────────────────────────────────────────────────────────────────
// Bone name queries
// ───────────────────────────────────────────────────────────────────────────────

TArray<FName> UBoneMappingTool::GetSourceBoneNamesFromAnimation(UAnimSequence* Animation)
{
	TArray<FName> BoneNames;
	if (!Animation)
	{
		return BoneNames;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return BoneNames;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();
	for (int32 i = 0; i < NumBones; ++i)
	{
		BoneNames.Add(RefSkeleton.GetBoneName(i));
	}
	return BoneNames;
}

TArray<FName> UBoneMappingTool::GetTargetBoneNamesFromSkeleton(USkeleton* Skeleton)
{
	TArray<FName> BoneNames;
	if (!Skeleton)
	{
		return BoneNames;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();
	for (int32 i = 0; i < NumBones; ++i)
	{
		BoneNames.Add(RefSkeleton.GetBoneName(i));
	}
	return BoneNames;
}

// ───────────────────────────────────────────────────────────────────────────────
// Mapping summary / lists
// ───────────────────────────────────────────────────────────────────────────────

FString UBoneMappingTool::GetMappingSummary(const FBoneMappingResult& Result)
{
	return FString::Printf(
		TEXT("Mapped: %d / Unmapped: %d / Total: %d / Confidence: %.1f%%"),
		Result.MappedCount,
		Result.UnmappedCount,
		Result.MappedCount + Result.UnmappedCount,
		CalculateMappingConfidence(Result) * 100.0f
	);
}

TArray<FName> UBoneMappingTool::GetMappedSourceBones(const FBoneMappingResult& Result)
{
	TArray<FName> Names;
	for (const FBoneMappingPair& Pair : Result.Mappings)
	{
		if (Pair.TargetBoneName != NAME_None)
		{
			Names.Add(Pair.SourceBoneName);
		}
	}
	return Names;
}

TArray<FName> UBoneMappingTool::GetMappedTargetBones(const FBoneMappingResult& Result)
{
	TArray<FName> Names;
	for (const FBoneMappingPair& Pair : Result.Mappings)
	{
		if (Pair.TargetBoneName != NAME_None)
		{
			Names.Add(Pair.TargetBoneName);
		}
	}
	return Names;
}

// ───────────────────────────────────────────────────────────────────────────────
// Preset system
// ───────────────────────────────────────────────────────────────────────────────

static TArray<UBoneMappingPreset*> GPresets;

UBoneMappingPreset* UBoneMappingPreset::CreatePreset(
	const FString& PresetName,
	const TMap<FName, FName>& Mappings)
{
	UBoneMappingPreset* Preset = NewObject<UBoneMappingPreset>();
	Preset->PresetName = PresetName;
	Preset->BoneMappings = Mappings;
	GPresets.Add(Preset);
	return Preset;
}

void UBoneMappingPreset::SavePreset(UBoneMappingPreset* Preset, const FString& FilePath)
{
	if (!Preset) return;

	FString JsonStr;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonStr);
	JsonWriter->WriteObjectStart();
	JsonWriter->WriteValue(TEXT("PresetName"), Preset->PresetName);
	JsonWriter->WriteValue(TEXT("SourceFormat"), Preset->SourceFormat);
	JsonWriter->WriteValue(TEXT("TargetFormat"), Preset->TargetFormat);
	JsonWriter->WriteArrayStart(TEXT("Mappings"));

	for (const auto& Pair : Preset->BoneMappings)
	{
		JsonWriter->WriteObjectStart();
		JsonWriter->WriteValue(TEXT("Source"), Pair.Key.ToString());
		JsonWriter->WriteValue(TEXT("Target"), Pair.Value.ToString());
		JsonWriter->WriteObjectEnd();
	}

	JsonWriter->WriteArrayEnd();
	JsonWriter->WriteObjectEnd();
	JsonWriter->Close();

	FFileHelper::SaveStringToFile(JsonStr, *FilePath);
}

UBoneMappingPreset* UBoneMappingPreset::LoadPreset(const FString& FilePath)
{
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		return nullptr;
	}

	UBoneMappingPreset* Preset = NewObject<UBoneMappingPreset>();
	JsonObject->TryGetStringField(TEXT("PresetName"), Preset->PresetName);
	JsonObject->TryGetStringField(TEXT("SourceFormat"), Preset->SourceFormat);
	JsonObject->TryGetStringField(TEXT("TargetFormat"), Preset->TargetFormat);

	const TArray<TSharedPtr<FJsonValue>>* MappingArray;
	if (JsonObject->TryGetArrayField(TEXT("Mappings"), MappingArray))
	{
		for (const auto& Val : *MappingArray)
		{
			const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
			if (Obj.IsValid())
			{
				FString Src, Dst;
				Obj->TryGetStringField(TEXT("Source"), Src);
				Obj->TryGetStringField(TEXT("Target"), Dst);
				Preset->BoneMappings.Add(FName(*Src), FName(*Dst));
			}
		}
	}

	GPresets.Add(Preset);
	return Preset;
}

TArray<UBoneMappingPreset*> UBoneMappingPreset::GetAllPresets()
{
	return GPresets;
}

UBoneMappingPreset* UBoneMappingPreset::GetPresetByName(const FString& PresetName)
{
	for (UBoneMappingPreset* P : GPresets)
	{
		if (P && P->PresetName.Equals(PresetName, ESearchCase::IgnoreCase))
		{
			return P;
		}
	}
	return nullptr;
}

// ───────────────────────────────────────────────────────────────────────────────
// Validation
// ───────────────────────────────────────────────────────────────────────────────

bool UBoneMappingValidator::ValidateMapping(
	const FBoneMappingResult& Mapping,
	USkeleton* TargetSkeleton)
{
	if (!TargetSkeleton) return false;

	const FReferenceSkeleton& TargetRef = TargetSkeleton->GetReferenceSkeleton();
	for (const FBoneMappingPair& Pair : Mapping.Mappings)
	{
		if (Pair.TargetBoneName != NAME_None)
		{
			if (TargetRef.FindBoneIndex(Pair.TargetBoneName) == INDEX_NONE)
			{
				return false;
			}
		}
	}
	return true;
}

TArray<FString> UBoneMappingValidator::GetValidationErrors(
	const FBoneMappingResult& Mapping,
	USkeleton* TargetSkeleton)
{
	TArray<FString> Errors;

	if (!TargetSkeleton)
	{
		Errors.Add(TEXT("Target skeleton is null."));
		return Errors;
	}

	const FReferenceSkeleton& TargetRef = TargetSkeleton->GetReferenceSkeleton();
	for (const FBoneMappingPair& Pair : Mapping.Mappings)
	{
		if (Pair.TargetBoneName != NAME_None)
		{
			if (TargetRef.FindBoneIndex(Pair.TargetBoneName) == INDEX_NONE)
			{
				Errors.Add(FString::Printf(
					TEXT("Bone '%s' is mapped to '%s' which does not exist in target skeleton."),
					*Pair.SourceBoneName.ToString(),
					*Pair.TargetBoneName.ToString()
				));
			}
			if (Pair.Confidence < 0.3f)
			{
				Errors.Add(FString::Printf(
					TEXT("Mapping '%s' -> '%s' has low confidence (%.1f%%)."),
					*Pair.SourceBoneName.ToString(),
					*Pair.TargetBoneName.ToString(),
					Pair.Confidence * 100.0f
				));
			}
		}
	}

	return Errors;
}

bool UBoneMappingValidator::IsMappingComplete(
	const FBoneMappingResult& Mapping,
	int32 ExpectedBoneCount)
{
	return Mapping.MappedCount >= ExpectedBoneCount;
}

float UBoneMappingValidator::GetMappingCoverage(
	const FBoneMappingResult& Mapping,
	int32 TotalSourceBones)
{
	if (TotalSourceBones == 0) return 0.0f;
	return static_cast<float>(Mapping.MappedCount) / static_cast<float>(TotalSourceBones);
}

// ───────────────────────────────────────────────────────────────────────────────
// Skeleton Bone Detection
// ───────────────────────────────────────────────────────────────────────────────

TArray<FName> UBoneMappingTool::DetectSkeletonBones(USkeleton* Skeleton)
{
	TArray<FName> Result;
	if (!Skeleton) return Result;

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 BoneCount = RefSkel.GetNum();
	Result.Reserve(BoneCount);

	for (int32 i = 0; i < BoneCount; ++i)
	{
		Result.Add(RefSkel.GetBoneName(i));
	}
	return Result;
}

int32 UBoneMappingTool::GetSkeletonBoneCount(USkeleton* Skeleton)
{
	if (!Skeleton) return 0;
	return Skeleton->GetReferenceSkeleton().GetNum();
}

bool UBoneMappingTool::HasBone(USkeleton* Skeleton, const FName& BoneName)
{
	if (!Skeleton) return false;
	return Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName) != INDEX_NONE;
}

int32 UBoneMappingTool::GetBoneParentIndex(USkeleton* Skeleton, const FName& BoneName)
{
	if (!Skeleton) return INDEX_NONE;
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 BoneIdx = RefSkel.FindBoneIndex(BoneName);
	if (BoneIdx == INDEX_NONE) return INDEX_NONE;
	return RefSkel.GetParentIndex(BoneIdx);
}

TArray<FName> UBoneMappingTool::GetBoneChildren(USkeleton* Skeleton, const FName& BoneName)
{
	TArray<FName> Result;
	if (!Skeleton) return Result;

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 ParentIdx = RefSkel.FindBoneIndex(BoneName);
	if (ParentIdx == INDEX_NONE) return Result;

	const int32 BoneCount = RefSkel.GetNum();
	for (int32 i = 0; i < BoneCount; ++i)
	{
		if (RefSkel.GetParentIndex(i) == ParentIdx)
		{
			Result.Add(RefSkel.GetBoneName(i));
		}
	}
	return Result;
}

FString UBoneMappingTool::DetectBoneNamingConvention(USkeleton* Skeleton)
{
	if (!Skeleton) return TEXT("Unknown");

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 BoneCount = RefSkel.GetNum();
	if (BoneCount == 0) return TEXT("Empty");

	int32 MixamoCount = 0;
	int32 UnrealCount = 0;
	int32 BlenderCount = 0;
	int32 MayaCount = 0;
	int32 GenericCount = 0;

	static const TArray<FString> MixamoPrefixes = { TEXT("mixamorig:"), TEXT("mixamorig") };
	static const TArray<FString> UnrealBones = { TEXT("root"), TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("neck_01"), TEXT("head") };
	static const TArray<FString> BlenderBones = { TEXT("DEF-spine"), TEXT("DEF-upper_arm"), TEXT("ORG-spine"), TEXT("MCH-spine") };
	static const TArray<FString> MayaBones = { TEXT("Hip"), TEXT("Spine"), TEXT("Spine1"), TEXT("Spine2"), TEXT("Neck"), TEXT("Head") };

	for (int32 i = 0; i < BoneCount; ++i)
	{
		const FString BoneName = RefSkel.GetBoneName(i).ToString();

		bool bFound = false;
		for (const FString& Prefix : MixamoPrefixes)
		{
			if (BoneName.StartsWith(Prefix, ESearchCase::IgnoreCase)) { MixamoCount++; bFound = true; break; }
		}
		if (bFound) continue;

		for (const FString& UB : UnrealBones)
		{
			if (BoneName.Equals(UB, ESearchCase::IgnoreCase)) { UnrealCount++; bFound = true; break; }
		}
		if (bFound) continue;

		for (const FString& BB : BlenderBones)
		{
			if (BoneName.StartsWith(BB, ESearchCase::IgnoreCase)) { BlenderCount++; bFound = true; break; }
		}
		if (bFound) continue;

		for (const FString& MB : MayaBones)
		{
			if (BoneName.Equals(MB, ESearchCase::CaseSensitive)) { MayaCount++; bFound = true; break; }
		}
		if (bFound) continue;

		GenericCount++;
	}

	const int32 Total = MixamoCount + UnrealCount + BlenderCount + MayaCount + GenericCount;
	if (Total == 0) return TEXT("Unknown");

	if (MixamoCount * 3 > Total) return TEXT("Mixamo");
	if (UnrealCount * 3 > Total) return TEXT("Unreal");
	if (BlenderCount * 3 > Total) return TEXT("Blender");
	if (MayaCount * 3 > Total) return TEXT("Maya");
	return TEXT("Generic");
}

bool UBoneMappingTool::RenameSkeletonBone(USkeleton* Skeleton, const FName& OldName, const FName& NewName)
{
	if (!Skeleton) return false;

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 BoneIdx = RefSkel.FindBoneIndex(OldName);
	if (BoneIdx == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("BoneMappingTool: Bone '%s' not found in skeleton"), *OldName.ToString());
		return false;
	}

	if (RefSkel.FindBoneIndex(NewName) != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("BoneMappingTool: Bone '%s' already exists in skeleton"), *NewName.ToString());
		return false;
	}

	FReferenceSkeletonModifier Modifier(Skeleton);
	Modifier.Rename(OldName, NewName);
	Skeleton->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("BoneMappingTool: Renamed bone '%s' -> '%s'"), *OldName.ToString(), *NewName.ToString());
	return true;
}

int32 UBoneMappingTool::RenameSkeletonBonesBatch(USkeleton* Skeleton, const TMap<FName, FName>& RenameMap)
{
	if (!Skeleton || RenameMap.Num() == 0) return 0;

	int32 RenamedCount = 0;
	for (const auto& Pair : RenameMap)
	{
		if (RenameSkeletonBone(Skeleton, Pair.Key, Pair.Value))
		{
			RenamedCount++;
		}
	}
	return RenamedCount;
}

bool UBoneMappingTool::RenameSkeletonBonesToConvention(USkeleton* Skeleton, const FString& TargetConvention)
{
	if (!Skeleton) return false;

	const FString CurrentConvention = DetectBoneNamingConvention(Skeleton);
	if (CurrentConvention == TargetConvention)
	{
		UE_LOG(LogTemp, Log, TEXT("BoneMappingTool: Skeleton already uses '%s' convention"), *TargetConvention);
		return true;
	}

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 BoneCount = RefSkel.GetNum();
	TMap<FName, FName> RenameMap;

	for (int32 i = 0; i < BoneCount; ++i)
	{
		const FString OldName = RefSkel.GetBoneName(i).ToString();
		FString NewName = OldName;

		if (CurrentConvention == TEXT("Mixamo") && TargetConvention == TEXT("Unreal"))
		{
			NewName = OldName.Replace(TEXT("mixamorig:"), TEXT(""), ESearchCase::IgnoreCase);
			NewName = NewName.ToLower();
		}
		else if (CurrentConvention == TEXT("Unreal") && TargetConvention == TEXT("Mixamo"))
		{
			NewName = TEXT("mixamorig:") + OldName;
		}
		else if (CurrentConvention == TEXT("Maya") && TargetConvention == TEXT("Unreal"))
		{
			NewName = OldName.ToLower();
			NewName = NewName.Replace(TEXT("_01"), TEXT(""), ESearchCase::IgnoreCase);
			NewName = NewName.Replace(TEXT("_02"), TEXT(""), ESearchCase::IgnoreCase);
		}
		else if (CurrentConvention == TEXT("Unreal") && TargetConvention == TEXT("Maya"))
		{
			NewName = OldName;
			if (NewName.Len() > 0)
			{
				NewName[0] = FChar::ToUpper(NewName[0]);
			}
			NewName = NewName.Replace(TEXT("_01"), TEXT("1"), ESearchCase::IgnoreCase);
			NewName = NewName.Replace(TEXT("_02"), TEXT("2"), ESearchCase::IgnoreCase);
			NewName = NewName.Replace(TEXT("_03"), TEXT("3"), ESearchCase::IgnoreCase);
		}
		else
		{
			continue;
		}

		if (OldName != NewName)
		{
			RenameMap.Add(FName(*OldName), FName(*NewName));
		}
	}

	if (RenameMap.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("BoneMappingTool: No bones need renaming for '%s' -> '%s'"), *CurrentConvention, *TargetConvention);
		return true;
	}

	return RenameSkeletonBonesBatch(Skeleton, RenameMap) > 0;
}

FBoneMappingResult UBoneMappingTool::DetectAndMapSkeletonBones(USkeleton* SourceSkeleton, USkeleton* TargetSkeleton)
{
	TArray<FName> SourceBones = DetectSkeletonBones(SourceSkeleton);
	return AutoMapBones(SourceBones, TargetSkeleton, EBoneMappingMethod::AllMethods);
}

TArray<FString> UBoneMappingTool::GetBoneHierarchy(USkeleton* Skeleton, int32 RootIndex)
{
	TArray<FString> Result;
	if (!Skeleton) return Result;

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 BoneCount = RefSkel.GetNum();
	if (BoneCount == 0) return Result;

	if (RootIndex < 0)
	{
		for (int32 i = 0; i < BoneCount; ++i)
		{
			if (RefSkel.GetParentIndex(i) == INDEX_NONE)
			{
				RootIndex = i;
				break;
			}
		}
	}

	if (RootIndex < 0 || RootIndex >= BoneCount) return Result;

	TFunction<void(int32, int32)> Traverse = [&](int32 BoneIdx, int32 Depth)
	{
		FString Indent;
		for (int32 d = 0; d < Depth; ++d) Indent += TEXT("  ");
		Result.Add(Indent + RefSkel.GetBoneName(BoneIdx).ToString());

		const int32 TotalBones = BoneCount;
		for (int32 i = 0; i < TotalBones; ++i)
		{
			if (RefSkel.GetParentIndex(i) == BoneIdx)
			{
				Traverse(i, Depth + 1);
			}
		}
	};

	Traverse(RootIndex, 0);
	return Result;
}

bool UBoneMappingTool::IsSkeletonCompatible(USkeleton* SkeletonA, USkeleton* SkeletonB)
{
	if (!SkeletonA || !SkeletonB) return false;

	const TArray<FName> BonesA = DetectSkeletonBones(SkeletonA);
	const TArray<FName> BonesB = DetectSkeletonBones(SkeletonB);

	int32 MatchCount = 0;
	for (const FName& BoneA : BonesA)
	{
		for (const FName& BoneB : BonesB)
		{
			if (BoneA == BoneB || IsCaseInsensitiveMatch(BoneA.ToString(), BoneB.ToString()))
			{
				MatchCount++;
				break;
			}
		}
	}

	const float Compatibility = (BonesA.Num() > 0)
		? static_cast<float>(MatchCount) / static_cast<float>(BonesA.Num())
		: 0.0f;

	return Compatibility >= 0.5f;
}

UAnimSequence* UBoneMappingTool::RemapAnimationToSkeleton(UAnimSequence* Animation, USkeleton* TargetSkeleton, bool bAutoDetect)
{
	if (!Animation || !TargetSkeleton) return nullptr;

	if (bAutoDetect)
	{
		USkeleton* SourceSkeleton = Animation->GetSkeleton();
		if (SourceSkeleton && IsSkeletonCompatible(SourceSkeleton, TargetSkeleton))
		{
			FBoneMappingResult Mapping = DetectAndMapSkeletonBones(SourceSkeleton, TargetSkeleton);
			return RemapAnimationBones(Animation, Mapping, TargetSkeleton);
		}
	}

	TArray<FName> AnimBones = GetSourceBoneNamesFromAnimation(Animation);
	FBoneMappingResult Mapping = AutoMapBones(AnimBones, TargetSkeleton, EBoneMappingMethod::AllMethods);
	return RemapAnimationBones(Animation, Mapping, TargetSkeleton);
}

TMap<FName, FName> UBoneMappingTool::FindBoneRenamingPairs(USkeleton* SourceSkeleton, USkeleton* TargetSkeleton)
{
	TMap<FName, FName> Result;
	if (!SourceSkeleton || !TargetSkeleton) return Result;

	const TArray<FName> SourceBones = DetectSkeletonBones(SourceSkeleton);

	for (const FName& SrcBone : SourceBones)
	{
		float Confidence = 0.0f;
		FName BestMatch = FindBestMatch(SrcBone, TargetSkeleton, EBoneMappingMethod::AllMethods, &Confidence);

		if (Confidence > 0.5f && SrcBone != BestMatch)
		{
			Result.Add(SrcBone, BestMatch);
		}
	}
	return Result;
}

#undef LOCTEXT_NAMESPACE

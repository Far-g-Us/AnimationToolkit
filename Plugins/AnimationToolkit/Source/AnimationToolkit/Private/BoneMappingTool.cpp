#include "BoneMappingTool.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
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
		for (int32 i = 0; i < NumTargetBones; ++i)
		{
			TargetNames.Add(TargetRef.GetBoneName(i).ToString());
		}
		const int32 BestIdx = FindBestFuzzyMatch(NormSource, TargetNames, 4);
		if (BestIdx != INDEX_NONE)
		{
			const int32 Dist = CalculateLevenshteinDistance(NormSource, NormalizeBoneName(TargetNames[BestIdx]));
			const float FuzzyScore = FMath::Max(0.1f, 1.0f - (static_cast<float>(Dist) / 10.0f));
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

	return AutoRemapAnimation(SourceAnimation, TargetSkeleton, EBoneMappingMethod::Exact, nullptr);
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

	bool bAnyApplied = false;

	for (int32 BoneIdx = 0; BoneIdx < NumBones; ++BoneIdx)
	{
		FName BoneName = RefSkeleton.GetBoneName(BoneIdx);

		for (const FBoneMappingPair& Pair : Mapping.Mappings)
		{
			if (Pair.SourceBoneName == BoneName && Pair.TargetBoneName != NAME_None)
			{
				bAnyApplied = true;
				break;
			}
		}
	}

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

	return SourceAnimation;
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

#undef LOCTEXT_NAMESPACE

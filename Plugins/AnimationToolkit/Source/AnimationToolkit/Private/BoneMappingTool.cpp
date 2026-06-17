#include "BoneMappingTool.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "BoneMappingTool"

FName UBoneMappingTool::FindBestMatch(
	const FName& SourceBoneName,
	USkeleton* TargetSkeleton,
	EBoneMappingMethod Method,
	float& OutConfidence)
{
	if (!TargetSkeleton)
	{
		OutConfidence = 0.0f;
		return NAME_None;
	}

	const FReferenceSkeleton& RefSkeleton = TargetSkeleton->GetReferenceSkeleton();
	FString SourceStr = SourceBoneName.ToString();

	int32 ExactIdx = RefSkeleton.FindBoneIndex(SourceBoneName);
	if (ExactIdx != INDEX_NONE)
	{
		OutConfidence = 1.0f;
		return SourceBoneName;
	}

	if (Method == EBoneMappingMethod::Exact)
	{
		OutConfidence = 0.0f;
		return NAME_None;
	}

	FString StrippedSource = RemoveCommonPrefixes(SourceStr);
	if (StrippedSource != SourceStr)
	{
		int32 StrippedIdx = RefSkeleton.FindBoneIndex(FName(*StrippedSource));
		if (StrippedIdx != INDEX_NONE)
		{
			OutConfidence = 0.9f;
			return FName(*StrippedSource);
		}
	}

	if (Method == EBoneMappingMethod::RemovePrefix)
	{
		OutConfidence = 0.0f;
		return NAME_None;
	}

	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FName TargetBone = RefSkeleton.GetBoneName(i);
		FString TargetStr = TargetBone.ToString();

		if (SourceStr.Equals(TargetStr, ESearchCase::IgnoreCase))
		{
			OutConfidence = 0.85f;
			return TargetBone;
		}
	}

	if (Method == EBoneMappingMethod::CaseInsensitive)
	{
		OutConfidence = 0.0f;
		return NAME_None;
	}

	FString NormalizedSource = NormalizeBoneName(SourceStr);
	float BestSimilarity = 0.0f;
	FName BestMatch = NAME_None;

	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FName TargetBone = RefSkeleton.GetBoneName(i);
		FString TargetStr = TargetBone.ToString();
		FString NormalizedTarget = NormalizeBoneName(TargetStr);

		if (NormalizedSource.Equals(NormalizedTarget, ESearchCase::IgnoreCase))
		{
			OutConfidence = 0.8f;
			return TargetBone;
		}

		float Similarity = CalculateSimilarity(NormalizedSource, NormalizedTarget);
		if (Similarity > BestSimilarity)
		{
			BestSimilarity = Similarity;
			BestMatch = TargetBone;
		}
	}

	if (Method == EBoneMappingMethod::Normalize)
	{
		OutConfidence = BestSimilarity;
		return BestMatch;
	}

	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FName TargetBone = RefSkeleton.GetBoneName(i);
		FString TargetStr = TargetBone.ToString();

		if (IsSubstringMatch(SourceStr, TargetStr))
		{
			OutConfidence = 0.7f;
			return TargetBone;
		}
	}

	if (Method == EBoneMappingMethod::Substring)
	{
		OutConfidence = 0.0f;
		return NAME_None;
	}

	int32 FuzzyIdx = FindBestFuzzyMatch(SourceStr, 
		[&RefSkeleton]() -> TArray<FString>
		{
			TArray<FString> Names;
			for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
			{
				Names.Add(RefSkeleton.GetBoneName(i).ToString());
			}
			return Names;
		}(), 3);

	if (FuzzyIdx != INDEX_NONE)
	{
		OutConfidence = 0.6f;
		return RefSkeleton.GetBoneName(FuzzyIdx);
	}

	OutConfidence = BestSimilarity;
	return BestMatch;
}

FBoneMappingResult UBoneMappingTool::AutoMapBones(
	const TArray<FName>& SourceBoneNames,
	USkeleton* TargetSkeleton,
	EBoneMappingMethod Method)
{
	FBoneMappingResult Result;

	if (!TargetSkeleton)
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkeleton = TargetSkeleton->GetReferenceSkeleton();

	for (const FName& SourceBone : SourceBoneNames)
	{
		float Confidence = 0.0f;
		FName TargetBone = FindBestMatch(SourceBone, TargetSkeleton, Method, Confidence);

		FBoneMappingPair Pair;
		Pair.SourceBoneName = SourceBone;
		Pair.bAutoMapped = (TargetBone != NAME_None);
		Pair.Confidence = Confidence;

		if (TargetBone != NAME_None)
		{
			Pair.TargetBoneName = TargetBone;
			Result.MappedCount++;
		}
		else
		{
			Pair.TargetBoneName = NAME_None;
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
	if (!SourceSkeleton || !TargetSkeleton)
	{
		return FBoneMappingResult();
	}

	TArray<FName> SourceBoneNames = GetTargetBoneNamesFromSkeleton(SourceSkeleton);
	return AutoMapBones(SourceBoneNames, TargetSkeleton, Method);
}

int32 UBoneMappingTool::CalculateLevenshteinDistance(const FString& Source, const FString& Target)
{
	int32 SourceLen = Source.Len();
	int32 TargetLen = Target.Len();

	TArray<TArray<int32>> DP;
	DP.SetNum(SourceLen + 1);
	for (int32 i = 0; i <= SourceLen; i++)
	{
		DP[i].SetNum(TargetLen + 1);
	}

	for (int32 i = 0; i <= SourceLen; i++)
	{
		DP[i][0] = i;
	}

	for (int32 j = 0; j <= TargetLen; j++)
	{
		DP[0][j] = j;
	}

	for (int32 i = 1; i <= SourceLen; i++)
	{
		for (int32 j = 1; j <= TargetLen; j++)
		{
			int32 Cost = (Source[i - 1] == Target[j - 1]) ? 0 : 1;
			DP[i][j] = FMath::Min3(
				DP[i - 1][j] + 1,
				DP[i][j - 1] + 1,
				DP[i - 1][j - 1] + Cost
			);
		}
	}

	return DP[SourceLen][TargetLen];
}

FString UBoneMappingTool::StripPrefixes(const FString& BoneName)
{
	return RemoveCommonPrefixes(BoneName);
}

FString UBoneMappingTool::NormalizeBoneName(const FString& BoneName)
{
	FString Normalized = BoneName;
	Normalized.RemoveFromStart("mixamorig:");
	Normalized.RemoveFromStart("Armature_");
	Normalized.RemoveFromStart("Bip001_");
	Normalized.RemoveFromStart("ValveBiped.");
	Normalized.RemoveFromStart("DEF-");
	Normalized.RemoveFromStart("DEF_");

	Normalized.ReplaceInline(TEXT("_"), TEXT(""));
	Normalized.ReplaceInline(TEXT("-"), TEXT(""));
	Normalized.ReplaceInline(TEXT("."), TEXT(""));
	Normalized.ReplaceInline(TEXT(" "), TEXT(""));

	return Normalized;
}

bool UBoneMappingTool::IsCaseInsensitiveMatch(const FString& A, const FString& B)
{
	return A.Equals(B, ESearchCase::IgnoreCase);
}

bool UBoneMappingTool::IsSubstringMatch(const FString& A, const FString& B)
{
	return A.Contains(B) || B.Contains(A);
}

bool UBoneMappingTool::IsFuzzyMatch(const FString& A, const FString& B, int32 MaxDistance)
{
	int32 Distance = CalculateLevenshteinDistance(A, B);
	return Distance <= MaxDistance;
}

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

	FString NewName = *FString::Printf(TEXT("%s_Remapped"), *SourceAnimation->GetName());

	UAnimSequence* RemappedAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		NewName,
		RF_Public | RF_Transient
	);

	if (!RemappedAnim)
	{
		return nullptr;
	}

	RemappedAnim->SetSkeleton(TargetSkeleton);

	const FReferenceSkeleton& SourceRefSkeleton = SourceSkeleton->GetReferenceSkeleton();
	const FReferenceSkeleton& TargetRefSkeleton = TargetSkeleton->GetReferenceSkeleton();

	int32 NumFrames = SourceAnimation->GetNumberOfSampledKeys();
	float Duration = SourceAnimation->GetSequenceLength();

	RemappedAnim->GetEditableRawAnimationData().Tracks.SetNum(TargetRefSkeleton.GetNum());

	for (const FBoneMappingPair& Pair : Mapping.Mappings)
	{
		if (!Pair.bAutoMapped)
		{
			continue;
		}

		int32 SourceBoneIdx = SourceRefSkeleton.FindBoneIndex(Pair.SourceBoneName);
		int32 TargetBoneIdx = TargetRefSkeleton.FindBoneIndex(Pair.TargetBoneName);

		if (SourceBoneIdx == INDEX_NONE || TargetBoneIdx == INDEX_NONE)
		{
			continue;
		}

		FRawAnimSequenceTrack& NewTrack = RemappedAnim->GetEditableRawAnimationData().Tracks[TargetBoneIdx];

		NewTrack.PosKeys.Reserve(NumFrames);
		NewTrack.RotKeys.Reserve(NumFrames);
		NewTrack.ScaleKeys.Reserve(NumFrames);

		for (int32 Frame = 0; Frame < NumFrames; Frame++)
		{
			FTransform BoneTransform = SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame);

			NewTrack.PosKeys.Add(BoneTransform.GetLocation());
			NewTrack.RotKeys.Add(BoneTransform.GetRotation());
			NewTrack.ScaleKeys.Add(BoneTransform.GetScale3D());
		}
	}

	RemappedAnim->MarkPackageDirty();

	return RemappedAnim;
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

	for (const FBoneMappingPair& Pair : Mapping.Mappings)
	{
		if (!Pair.bAutoMapped)
		{
			continue;
		}

		int32 BoneIdx = RefSkeleton.FindBoneIndex(Pair.SourceBoneName);
		if (BoneIdx == INDEX_NONE)
		{
			continue;
		}

		if (BoneIdx < Animation->GetEditableRawAnimationData().Tracks.Num())
		{
			FRawAnimSequenceTrack& Track = Animation->GetEditableRawAnimationData().Tracks[BoneIdx];

			int32 TargetBoneIdx = RefSkeleton.FindBoneIndex(Pair.TargetBoneName);
			if (TargetBoneIdx != INDEX_NONE && TargetBoneIdx < Animation->GetEditableRawAnimationData().Tracks.Num())
			{
				Animation->GetEditableRawAnimationData().Tracks[TargetBoneIdx] = Track;
			}
		}
	}

	Animation->MarkPackageDirty();
	return true;
}

TArray<FName> UBoneMappingTool::GetSourceBoneNamesFromAnimation(UAnimSequence* Animation)
{
	TArray<FName> Result;

	if (!Animation)
	{
		return Result;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		Result.Add(RefSkeleton.GetBoneName(i));
	}

	return Result;
}

TArray<FName> UBoneMappingTool::GetTargetBoneNamesFromSkeleton(USkeleton* Skeleton)
{
	TArray<FName> Result;

	if (!Skeleton)
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		Result.Add(RefSkeleton.GetBoneName(i));
	}

	return Result;
}

float UBoneMappingTool::CalculateMappingConfidence(const FBoneMappingResult& Result)
{
	if (Result.Mappings.Num() == 0)
	{
		return 0.0f;
	}

	float TotalConfidence = 0.0f;
	for (const FBoneMappingPair& Pair : Result.Mappings)
	{
		TotalConfidence += Pair.Confidence;
	}

	return TotalConfidence / (float)Result.Mappings.Num();
}

UAnimSequence* UBoneMappingTool::AutoRemapAnimation(
	UAnimSequence* SourceAnimation,
	USkeleton* TargetSkeleton,
	EBoneMappingMethod Method,
	FBoneMappingResult& OutMapping)
{
	if (!SourceAnimation || !TargetSkeleton)
	{
		return nullptr;
	}

	OutMapping = AutoMapBonesFromAnimation(SourceAnimation, TargetSkeleton, Method);

	return RemapAnimationBones(SourceAnimation, OutMapping, TargetSkeleton);
}

FString UBoneMappingTool::GetMappingSummary(const FBoneMappingResult& Result)
{
	return FString::Printf(
		TEXT("Mapped: %d | Unmapped: %d | Confidence: %.1f%%"),
		Result.MappedCount,
		Result.UnmappedCount,
		CalculateMappingConfidence(Result) * 100.0f
	);
}

TArray<FName> UBoneMappingTool::GetMappedSourceBones(const FBoneMappingResult& Result)
{
	TArray<FName> Mapped;

	for (const FBoneMappingPair& Pair : Result.Mappings)
	{
		if (Pair.bAutoMapped)
		{
			Mapped.Add(Pair.SourceBoneName);
		}
	}

	return Mapped;
}

TArray<FName> UBoneMappingTool::GetMappedTargetBones(const FBoneMappingResult& Result)
{
	TArray<FName> Mapped;

	for (const FBoneMappingPair& Pair : Result.Mappings)
	{
		if (Pair.bAutoMapped)
		{
			Mapped.Add(Pair.TargetBoneName);
		}
	}

	return Mapped;
}

FString UBoneMappingTool::RemovePrefix(const FString& BoneName, const FString& Prefix)
{
	if (BoneName.StartsWith(Prefix))
	{
		return BoneName.Mid(Prefix.Len());
	}
	return BoneName;
}

FString UBoneMappingTool::RemoveCommonPrefixes(const FString& BoneName)
{
	FString Result = BoneName;

	Result = RemovePrefix(Result, "mixamorig:");
	Result = RemovePrefix(Result, "Armature_");
	Result = RemovePrefix(Result, "Bip001_");
	Result = RemovePrefix(Result, "ValveBiped.");
	Result = RemovePrefix(Result, "DEF-");
	Result = RemovePrefix(Result, "DEF_");
	Result = RemovePrefix(Result, "Character_");
	Result = RemovePrefix(Result, "SK_");
	Result = RemovePrefix(Result, "SKEL_");

	return Result;
}

FString UBoneMappingTool::RemoveSuffixes(const FString& BoneName)
{
	FString Result = BoneName;

	if (Result.EndsWith("_end"))
	{
		Result = Result.Left(Result.Len() - 4);
	}

	if (Result.EndsWith(".end"))
	{
		Result = Result.Left(Result.Len() - 4);
	}

	return Result;
}

FString UBoneMappingTool::RemoveNumbers(const FString& BoneName)
{
	FString Result = BoneName;
	while (Result.Len() > 0 && FChar::IsDigit(Result.Last()))
	{
		 Result.RemoveAt(Result.Len() - 1);
	}
	return Result;
}

FString UBoneMappingTool::RemoveSpecialChars(const FString& BoneName)
{
	FString Result = BoneName;
	Result.ReplaceInline(TEXT(":"), TEXT(""));
	Result.ReplaceInline(TEXT("."), TEXT(""));
	Result.ReplaceInline(TEXT("-"), TEXT(""));
	Result.ReplaceInline(TEXT("_"), TEXT(""));
	Result.ReplaceInline(TEXT(" "), TEXT(""));
	return Result;
}

float UBoneMappingTool::CalculateSimilarity(const FString& A, const FString& B)
{
	if (A.Equals(B, ESearchCase::IgnoreCase))
	{
		return 1.0f;
	}

	int32 MaxLen = FMath::Max(A.Len(), B.Len());
	if (MaxLen == 0)
	{
		return 0.0f;
	}

	int32 Distance = CalculateLevenshteinDistance(A, B);
	return 1.0f - ((float)Distance / (float)MaxLen);
}

int32 UBoneMappingTool::FindBestFuzzyMatch(const FString& Source, const TArray<FString>& Targets, int32 MaxDistance)
{
	int32 BestIdx = INDEX_NONE;
	int32 BestDistance = MaxDistance + 1;

	for (int32 i = 0; i < Targets.Num(); i++)
	{
		int32 Distance = CalculateLevenshteinDistance(Source, Targets[i]);
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestIdx = i;
		}
	}

	return BestIdx;
}

UBoneMappingPreset* UBoneMappingPreset::CreatePreset(
	const FString& PresetName,
	const TMap<FName, FName>& Mappings)
{
	UBoneMappingPreset* Preset = NewObject<UBoneMappingPreset>();
	Preset->PresetName = PresetName;
	Preset->BoneMappings = Mappings;
	return Preset;
}

void UBoneMappingPreset::SavePreset(UBoneMappingPreset* Preset, const FString& FilePath)
{
	if (!Preset)
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("PresetName", Preset->PresetName);
	JsonObject->SetStringField("SourceFormat", Preset->SourceFormat);
	JsonObject->SetStringField("TargetFormat", Preset->TargetFormat);

	TArray<TSharedPtr<FJsonValue>> MappingsArray;
	for (const auto& Pair : Preset->BoneMappings)
	{
		TSharedPtr<FJsonObject> MappingObj = MakeShareable(new FJsonObject);
		MappingObj->SetStringField("Source", Pair.Key.ToString());
		MappingObj->SetStringField("Target", Pair.Value.ToString());
		MappingsArray.Add(MakeShareable(new FJsonValueObject(MappingObj)));
	}
	JsonObject->SetArrayField("Mappings", MappingsArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	FFileHelper::SaveStringToFile(OutputString, *FilePath);
}

UBoneMappingPreset* UBoneMappingPreset::LoadPreset(const FString& FilePath)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		return nullptr;
	}

	UBoneMappingPreset* Preset = NewObject<UBoneMappingPreset>();
	Preset->PresetName = JsonObject->GetStringField("PresetName");
	Preset->SourceFormat = JsonObject->GetStringField("SourceFormat");
	Preset->TargetFormat = JsonObject->GetStringField("TargetFormat");

	const TArray<TSharedPtr<FJsonValue>>& MappingsArray = JsonObject->GetArrayField("Mappings");
	for (const TSharedPtr<FJsonValue>& Value : MappingsArray)
	{
		const TSharedPtr<FJsonObject>& MappingObj = Value->AsObject();
		if (MappingObj.IsValid())
		{
			FName Source(*MappingObj->GetStringField("Source"));
			FName Target(*MappingObj->GetStringField("Target"));
			Preset->BoneMappings.Add(Source, Target);
		}
	}

	return Preset;
}

TArray<UBoneMappingPreset*> UBoneMappingPreset::GetAllPresets()
{
	return TArray<UBoneMappingPreset*>();
}

UBoneMappingPreset* UBoneMappingPreset::GetPresetByName(const FString& PresetName)
{
	return nullptr;
}

bool UBoneMappingValidator::ValidateMapping(
	const FBoneMappingResult& Mapping,
	USkeleton* TargetSkeleton)
{
	if (!TargetSkeleton)
	{
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = TargetSkeleton->GetReferenceSkeleton();

	for (const FBoneMappingPair& Pair : Mapping.Mappings)
	{
		if (Pair.bAutoMapped)
		{
			int32 Idx = RefSkeleton.FindBoneIndex(Pair.TargetBoneName);
			if (Idx == INDEX_NONE)
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
		Errors.Add("Target skeleton is null");
		return Errors;
	}

	const FReferenceSkeleton& RefSkeleton = TargetSkeleton->GetReferenceSkeleton();

	for (const FBoneMappingPair& Pair : Mapping.Mappings)
	{
		if (Pair.bAutoMapped)
		{
			int32 Idx = RefSkeleton.FindBoneIndex(Pair.TargetBoneName);
			if (Idx == INDEX_NONE)
			{
				Errors.Add(FString::Printf(TEXT("Bone '%s' not found in target skeleton"), *Pair.TargetBoneName.ToString()));
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
	if (TotalSourceBones == 0)
	{
		return 0.0f;
	}

	return (float)Mapping.MappedCount / (float)TotalSourceBones;
}

#undef LOCTEXT_NAMESPACE

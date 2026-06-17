#pragma once

#include "CoreMinimal.h"
#include "AnimationData.h"
#include "UObject/NoExportTypes.h"
#include "BoneMappingTool.generated.h"

USTRUCT(BlueprintType)
struct FBoneMappingPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	FName SourceBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	FName TargetBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	bool bAutoMapped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	float Confidence = 0.0f;
};

USTRUCT(BlueprintType)
struct FBoneMappingResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	TArray<FBoneMappingPair> Mappings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	int32 MappedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	int32 UnmappedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	TArray<FName> UnmappedBones;
};

UENUM(BlueprintType)
enum class EBoneMappingMethod : uint8
{
	Exact,
	RemovePrefix,
	CaseInsensitive,
	Normalize,
	Substring,
	Fuzzy,
	AllMethods
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UBoneMappingTool : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Bone Mapping")
	static FBoneMappingResult AutoMapBones(
		const TArray<FName>& SourceBoneNames,
		USkeleton* TargetSkeleton,
		EBoneMappingMethod Method = EBoneMappingMethod::AllMethods
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Bone Mapping")
	static FBoneMappingResult AutoMapBonesFromAnimation(
		UAnimSequence* SourceAnimation,
		USkeleton* TargetSkeleton,
		EBoneMappingMethod Method = EBoneMappingMethod::AllMethods
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Bone Mapping")
	static FBoneMappingResult AutoMapBonesFromTwoSkeletons(
		USkeleton* SourceSkeleton,
		USkeleton* TargetSkeleton,
		EBoneMappingMethod Method = EBoneMappingMethod::AllMethods
	);

	static FName FindBestMatch(
		const FName& SourceBoneName,
		USkeleton* TargetSkeleton,
		EBoneMappingMethod Method = EBoneMappingMethod::AllMethods,
		float* OutConfidence = nullptr
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static int32 CalculateLevenshteinDistance(
		const FString& Source,
		const FString& Target
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static FString StripPrefixes(const FString& BoneName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static FString NormalizeBoneName(const FString& BoneName);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static bool IsCaseInsensitiveMatch(const FString& A, const FString& B);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static bool IsSubstringMatch(const FString& A, const FString& B);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static bool IsFuzzyMatch(const FString& A, const FString& B, int32 MaxDistance = 3);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Bone Mapping")
	static UAnimSequence* RemapAnimationBones(
		UAnimSequence* SourceAnimation,
		const FBoneMappingResult& Mapping,
		USkeleton* TargetSkeleton
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Bone Mapping")
	static bool ApplyBoneMappingToAnimation(
		UAnimSequence* Animation,
		const FBoneMappingResult& Mapping
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static TArray<FName> GetSourceBoneNamesFromAnimation(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static TArray<FName> GetTargetBoneNamesFromSkeleton(USkeleton* Skeleton);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static float CalculateMappingConfidence(const FBoneMappingResult& Result);

	static UAnimSequence* AutoRemapAnimation(
		UAnimSequence* SourceAnimation,
		USkeleton* TargetSkeleton,
		EBoneMappingMethod Method = EBoneMappingMethod::AllMethods,
		FBoneMappingResult* OutMapping = nullptr
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static FString GetMappingSummary(const FBoneMappingResult& Result);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static TArray<FName> GetMappedSourceBones(const FBoneMappingResult& Result);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static TArray<FName> GetMappedTargetBones(const FBoneMappingResult& Result);

private:
	static FString RemovePrefix(const FString& BoneName, const FString& Prefix);
	static FString RemoveCommonPrefixes(const FString& BoneName);
	static FString RemoveSuffixes(const FString& BoneName);
	static FString RemoveNumbers(const FString& BoneName);
	static FString RemoveSpecialChars(const FString& BoneName);
	static float CalculateSimilarity(const FString& A, const FString& B);
	static int32 FindBestFuzzyMatch(const FString& Source, const TArray<FString>& Targets, int32 MaxDistance);
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UBoneMappingPreset : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Bone Mapping")
	static UBoneMappingPreset* CreatePreset(
		const FString& PresetName,
		const TMap<FName, FName>& Mappings
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Bone Mapping")
	static void SavePreset(
		UBoneMappingPreset* Preset,
		const FString& FilePath
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Bone Mapping")
	static UBoneMappingPreset* LoadPreset(
		const FString& FilePath
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static TArray<UBoneMappingPreset*> GetAllPresets();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static UBoneMappingPreset* GetPresetByName(const FString& PresetName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	FString PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	TMap<FName, FName> BoneMappings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	FString SourceFormat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Mapping")
	FString TargetFormat;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UBoneMappingValidator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static bool ValidateMapping(
		const FBoneMappingResult& Mapping,
		USkeleton* TargetSkeleton
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static TArray<FString> GetValidationErrors(
		const FBoneMappingResult& Mapping,
		USkeleton* TargetSkeleton
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static bool IsMappingComplete(
		const FBoneMappingResult& Mapping,
		int32 ExpectedBoneCount
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Bone Mapping")
	static float GetMappingCoverage(
		const FBoneMappingResult& Mapping,
		int32 TotalSourceBones
	);
};

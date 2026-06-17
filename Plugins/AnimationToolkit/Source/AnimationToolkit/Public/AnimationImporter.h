#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationImporter.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimationImporter : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Import", meta = (WorldContext = "WorldContextObject"))
	static FAnimationImportResult ImportAnimation(
		UObject* WorldContextObject,
		const FString& FilePath,
		EAnimationImportFormat Format,
		USkeleton* TargetSkeleton = nullptr
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Import", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ImportAndCreateAnimSequence(
		UObject* WorldContextObject,
		const FString& FilePath,
		const FString& SequenceName,
		USkeleton* Skeleton,
		EAnimationImportFormat Format,
		UPackage* Package = nullptr
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Import")
	static bool IsSupportedFormat(const FString& FilePath, EAnimationImportFormat& OutFormat);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Import")
	static TArray<EAnimationImportFormat> GetSupportedFormats();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Import")
	static FString GetFormatExtension(EAnimationImportFormat Format);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Import")
	static EAnimationImportFormat DetectFormat(const FString& FilePath);

private:
	static FAnimationImportResult ImportFBX(const FString& FilePath);
	static FAnimationImportResult ImportglTF(const FString& FilePath);
	static FAnimationImportResult ImportBVH(const FString& FilePath);

	static FAnimationClipData ParseFBXData(const FString& FilePath);
	static FAnimationClipData ParseglTFData(const FString& JsonContent);
	static FAnimationClipData ParseBVHData(const FString& FileContent);

	static bool ValidateSkeleton(
		const FAnimationClipData& ClipData,
		USkeleton* Skeleton,
		FString& OutError
	);

	static TArray<FString> ReadFileLines(const FString& FilePath);
	static FString ReadFileContent(const FString& FilePath);
};

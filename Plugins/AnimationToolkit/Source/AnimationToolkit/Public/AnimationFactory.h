#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationFactory.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimationFactory : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Factory", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* CreateAnimSequence(
		UObject* WorldContextObject,
		const FString& SequenceName,
		USkeleton* Skeleton,
		const TArray<FAnimationTrackData>& Tracks,
		float Duration,
		float FrameRate
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Factory", meta = (WorldContext = "WorldContextObject"))
	static UAnimMontage* CreateAnimMontage(
		UObject* WorldContextObject,
		const FString& MontageName,
		USkeleton* Skeleton,
		const TArray<FAnimMontageSectionData>& Sections
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Factory", meta = (WorldContext = "WorldContextObject"))
	static UBlendSpace* CreateBlendSpace1D(
		UObject* WorldContextObject,
		const FString& BlendSpaceName,
		USkeleton* Skeleton,
		const TArray<FBlendSpaceSampleData>& Samples,
		float MinValue,
		float MaxValue
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Factory", meta = (WorldContext = "WorldContextObject"))
	static UBlendSpace* CreateBlendSpace2D(
		UObject* WorldContextObject,
		const FString& BlendSpaceName,
		USkeleton* Skeleton,
		const TArray<FBlendSpaceSampleData>& Samples,
		float MinX,
		float MaxX,
		float MinY,
		float MaxY
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Factory", meta = (WorldContext = "WorldContextObject"))
	static FAnimationImportResult ImportFromFile(
		UObject* WorldContextObject,
		const FString& FilePath,
		EAnimationImportFormat Format,
		USkeleton* Skeleton
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Factory", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ImportAndCreate(
		UObject* WorldContextObject,
		const FString& FilePath,
		const FString& AnimationName,
		USkeleton* Skeleton,
		EAnimationImportFormat Format
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Factory")
	static UAnimationFactory* GetInstance();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Factory")
	static bool IsValidSkeleton(USkeleton* Skeleton);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Factory")
	static FString GenerateUniqueName(const FString& BaseName, UObject* Package);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Factory")
	static UPackage* CreateAnimationPackage(const FString& AssetName);

private:
	static TWeakObjectPtr<UAnimationFactory> Instance;
};

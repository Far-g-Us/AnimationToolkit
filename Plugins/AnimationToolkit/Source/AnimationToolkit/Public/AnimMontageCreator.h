#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimMontage.h"
#include "AnimationData.h"
#include "AnimMontageCreator.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimMontageCreator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|AnimMontage", meta = (WorldContext = "WorldContextObject"))
	static UAnimMontage* CreateAnimMontage(
		UObject* WorldContextObject,
		const FString& MontageName,
		USkeleton* Skeleton,
		const TArray<FAnimMontageSectionData>& Sections,
		UPackage* Package = nullptr
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|AnimMontage", meta = (WorldContext = "WorldContextObject"))
	static UAnimMontage* CreateAnimMontageFromSequence(
		UObject* WorldContextObject,
		const FString& MontageName,
		USkeleton* Skeleton,
		UAnimSequence* AnimSequence,
		float StartTime = 0.0f,
		float EndTime = -1.0f,
		UPackage* Package = nullptr
	);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|AnimMontage")
	static FAnimMontageSectionData CreateMontageSectionData(
		const FName& SectionName,
		UAnimSequence* Animation,
		float StartTime,
		float EndTime
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|AnimMontage")
	static bool AddSectionToMontage(
		UAnimMontage* Montage,
		const FAnimMontageSectionData& SectionData,
		float StartTime,
		float EndTime
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|AnimMontage")
	static bool SetMontageBlendIn(
		UAnimMontage* Montage,
		float BlendInTime,
		EAlphaBlendOption BlendInOption = EAlphaBlendOption::Linear
	);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|AnimMontage")
	static bool SetMontageBlendOut(
		UAnimMontage* Montage,
		float BlendOutTime,
		EAlphaBlendOption BlendOutOption = EAlphaBlendOption::Linear
	);

private:
	static void ConfigureMontageSections(
		UAnimMontage* Montage,
		const TArray<FAnimMontageSectionData>& Sections
	);
};

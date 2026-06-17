#pragma once

#include "CoreMinimal.h"
#include "IAssetTypeActions.h"
#include "AnimationData.h"
#include "AnimationToolkitContentBrowser.generated.h"

class UAnimSequence;

UCLASS()
class ANIMATIONTOOLKIT_API UAnimationToolkitContentBrowserModule : public UObject
{
	GENERATED_BODY()

public:
	void StartupModule();
	void ShutdownModule();

	void RegisterAssetTypeActions();
	void UnregisterAssetTypeActions();

private:
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
};

class ANIMATIONTOOLKIT_API FAnimationToolkitAssetTypeActions : public IAssetTypeActions
{
public:
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;

	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
	virtual bool CanMerge(const UObject* InObject) const override;
	virtual void Merge(const UObject* ObjectA, const UObject* ObjectB, const FString& TargetName) override;
};

UCLASS()
class ANIMATIONTOOLKIT_API UAnimationToolkitContextMenuActions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void ImportAnimationFromContext(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void ExportSelectedAnimation(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void BlendSelectedAnimations(const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void MirrorSelectedAnimation(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void CompressSelectedAnimation(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void OptimizeSelectedAnimation(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void RetargetSelectedAnimation(UAnimSequence* Animation, USkeleton* TargetSkeleton);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void AddTagToSelected(UAnimSequence* Animation, const FName& Tag);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void RemoveTagFromSelected(UAnimSequence* Animation, const FName& Tag);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Context Menu")
	static TArray<FName> GetAnimationTagsFromSelected(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void BatchCompressSelected(const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void BatchOptimizeSelected(const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Context Menu")
	static void ShowAnimationInfo(UAnimSequence* Animation);
};

#pragma once

#include "CoreMinimal.h"
#include "IAssetTypeActions.h"
#include "AnimationData.h"
#include "AnimationToolkitContentBrowser.generated.h"

class UAnimSequence;
class SWidget;

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
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, const EAssetTypeActivationOpenedMethod OpenedMethod, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
	virtual bool AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType) override;
	virtual bool CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	virtual bool CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const override;
	virtual TArray<FAssetData> GetValidAssetsForPreviewOrEdit(TArrayView<const FAssetData> InAssetDatas, bool bIsPreview) override;
	virtual bool CanFilter() override;
	virtual FName GetFilterName() const override;
	virtual bool CanLocalize() const override;
	virtual bool CanMerge() const override;
	virtual void Merge(UObject* InObject) override;
	virtual void Merge(UObject* BaseAsset, UObject* RemoteAsset, UObject* LocalAsset, const FOnMergeResolved& ResolutionCallback) override;
	virtual FString GetObjectDisplayName(UObject* Object) const override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual bool ShouldForceWorldCentric() override;
	virtual void PerformAssetDiff(UObject* OldAsset, UObject* NewAsset, const FRevisionInfo& OldRevision, const FRevisionInfo& NewRevision) const override;
	virtual class UThumbnailInfo* GetThumbnailInfo(UObject* Asset) const override;
	virtual EThumbnailPrimType GetDefaultThumbnailPrimitiveType(UObject* Asset) const override;
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	virtual bool IsImportedAsset() const override;
	virtual void GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;
	virtual void GetSourceFileLabels(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFileLabels) const override;
	virtual void BuildBackendFilter(struct FARFilter& InFilter) override;
	virtual FText GetDisplayNameFromAssetData(const FAssetData& AssetData) const override;
	virtual FTopLevelAssetPath GetClassPathName() const override;
	virtual bool SupportsOpenedMethod(const EAssetTypeActivationOpenedMethod OpenedMethod) const override;
	virtual const FSlateBrush* GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const override;
	virtual const FSlateBrush* GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const override;
};

	UCLASS()
	class ANIMATIONTOOLKIT_API UAnimationToolkitContextMenuActions : public UBlueprintFunctionLibrary
	{
		GENERATED_BODY()

	public:
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

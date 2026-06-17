#include "AnimationToolkitContentBrowser.h"
#include "AnimationLibrary.h"
#include "AnimationOptimizer.h"
#include "Animation/AnimSequence.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Misc/MessageDialog.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "AnimationToolkitContentBrowser"

void UAnimationToolkitContentBrowserModule::StartupModule()
{
}

void UAnimationToolkitContentBrowserModule::ShutdownModule()
{
}

void UAnimationToolkitContentBrowserModule::RegisterAssetTypeActions()
{
}

void UAnimationToolkitContentBrowserModule::UnregisterAssetTypeActions()
{
}

FText FAnimationToolkitAssetTypeActions::GetName() const
{
	return FText::GetEmpty();
}

FColor FAnimationToolkitAssetTypeActions::GetTypeColor() const
{
	return FColor::Black;
}

UClass* FAnimationToolkitAssetTypeActions::GetSupportedClass() const
{
	return nullptr;
}

uint32 FAnimationToolkitAssetTypeActions::GetCategories()
{
	return 0;
}

void FAnimationToolkitAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
}

void FAnimationToolkitAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, const EAssetTypeActivationOpenedMethod OpenedMethod, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
}

bool FAnimationToolkitAssetTypeActions::AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
{
	return false;
}

bool FAnimationToolkitAssetTypeActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return false;
}

bool FAnimationToolkitAssetTypeActions::CanRename(const FAssetData& AssetData, FText* OutErrorMsg) const
{
	return false;
}

bool FAnimationToolkitAssetTypeActions::CanDuplicate(const FAssetData& AssetData, FText* OutErrorMsg) const
{
	return false;
}

TArray<FAssetData> FAnimationToolkitAssetTypeActions::GetValidAssetsForPreviewOrEdit(TArrayView<const FAssetData> InAssetDataList, bool bAllowHiddenAssets)
{
	return TArray<FAssetData>();
}

bool FAnimationToolkitAssetTypeActions::CanFilter()
{
	return false;
}

void UAnimationToolkitContextMenuActions::ImportAnimationFromContext(const FString& FilePath)
{
}

void UAnimationToolkitContextMenuActions::ExportSelectedAnimation(UAnimSequence* Animation)
{
}

void UAnimationToolkitContextMenuActions::BlendSelectedAnimations(const TArray<UAnimSequence*>& Animations)
{
}

void UAnimationToolkitContextMenuActions::MirrorSelectedAnimation(UAnimSequence* Animation)
{
}

void UAnimationToolkitContextMenuActions::CompressSelectedAnimation(UAnimSequence* Animation)
{
}

void UAnimationToolkitContextMenuActions::OptimizeSelectedAnimation(UAnimSequence* Animation)
{
}

void UAnimationToolkitContextMenuActions::RetargetSelectedAnimation(UAnimSequence* Animation, USkeleton* TargetSkeleton)
{
}

void UAnimationToolkitContextMenuActions::AddTagToSelected(UAnimSequence* Animation, const FName& Tag)
{
}

void UAnimationToolkitContextMenuActions::RemoveTagFromSelected(UAnimSequence* Animation, const FName& Tag)
{
}

TArray<FName> UAnimationToolkitContextMenuActions::GetAnimationTagsFromSelected(UAnimSequence* Animation)
{
	return TArray<FName>();
}

void UAnimationToolkitContextMenuActions::BatchCompressSelected(const TArray<UAnimSequence*>& Animations)
{
}

void UAnimationToolkitContextMenuActions::BatchOptimizeSelected(const TArray<UAnimSequence*>& Animations)
{
}

void UAnimationToolkitContextMenuActions::ShowAnimationInfo(UAnimSequence* Animation)
{
}

#undef LOCTEXT_NAMESPACE

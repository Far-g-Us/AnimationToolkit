#include "AnimationToolkitEditor.h"
#include "AnimationToolkitModule.h"
#include "AnimationLibrary.h"
#include "AnimationImporter.h"
#include "AnimationOptimizer.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Styling/AppStyle.h"
#include "Misc/MessageDialog.h"
#include "Framework/Application/SlateApplication.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#define LOCTEXT_NAMESPACE "AnimationToolkitEditor"

void UAnimationToolkitEditorModule::StartupModule()
{
}

void UAnimationToolkitEditorModule::ShutdownModule()
{
}

void UAnimationToolkitEditorModule::RegisterMenus()
{
}

void UAnimationToolkitEditorModule::AddToolbarButton(FToolBarBuilder& Builder)
{
}

void UAnimationToolkitEditorModule::AddMenuBarButton(FMenuBarBuilder& MenuBuilder)
{
}

TSharedRef<SWidget> UAnimationToolkitEditorModule::CreateAnimationToolkitMenu()
{
	return SNew(SVerticalBox);
}

void UAnimationToolkitEditorModule::OnImportClicked()
{
}

void UAnimationToolkitEditorModule::OnExportClicked()
{
}

void UAnimationToolkitEditorModule::OnBlendClicked()
{
}

void UAnimationToolkitEditorModule::OnMirrorClicked()
{
}

void UAnimationToolkitEditorModule::OnCompressClicked()
{
}

void UAnimationToolkitEditorModule::OnOptimizeClicked()
{
}

void UAnimationToolkitEditorModule::OnRetargetClicked()
{
}

void UAnimationToolkitEditorModule::OnBatchProcessClicked()
{
}

void FAnimationToolkitEditorCommands::RegisterCommands()
{
}

UAnimationToolkitEditorSettings* UAnimationToolkitEditorLibrary::GetEditorSettings()
{
	return nullptr;
}

void UAnimationToolkitEditorLibrary::SaveEditorSettings()
{
}

TArray<UAnimSequence*> UAnimationToolkitEditorLibrary::GetSelectedAnimations()
{
	return TArray<UAnimSequence*>();
}

USkeleton* UAnimationToolkitEditorLibrary::GetSelectedSkeleton()
{
	return nullptr;
}

void UAnimationToolkitEditorLibrary::OpenImportDialog(UObject* WorldContextObject)
{
}

void UAnimationToolkitEditorLibrary::OpenExportDialog(UObject* WorldContextObject, UAnimSequence* Animation)
{
}

void UAnimationToolkitEditorLibrary::ShowNotification(const FString& Message, bool bSuccess)
{
}

FName FAnimationToolkitEditor::GetToolkitFName() const
{
	return NAME_None;
}

FText FAnimationToolkitEditor::GetBaseToolkitName() const
{
	return FText::GetEmpty();
}

FString FAnimationToolkitEditor::GetWorldCentricTabPrefix() const
{
	return FString();
}

FLinearColor FAnimationToolkitEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

void FAnimationToolkitEditor::InitEditor(const TArray<UObject*>& InObjects)
{
}

void FAnimationToolkitEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
}

void FAnimationToolkitEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
}

void FAnimationToolkitEditor::OnImport()
{
}

void FAnimationToolkitEditor::OnExport()
{
}

void FAnimationToolkitEditor::OnBlend()
{
}

void FAnimationToolkitEditor::OnMirror()
{
}

void FAnimationToolkitEditor::OnCompress()
{
}

void FAnimationToolkitEditor::OnOptimize()
{
}

void FAnimationToolkitEditor::OnRetarget()
{
}

void FAnimationToolkitEditor::OnBatchProcess()
{
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Import(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab);
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Export(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab);
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Blend(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab);
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Mirror(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab);
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Compress(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab);
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Optimize(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab);
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Retarget(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab);
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Batch(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab);
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Settings(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab);
}

#undef LOCTEXT_NAMESPACE

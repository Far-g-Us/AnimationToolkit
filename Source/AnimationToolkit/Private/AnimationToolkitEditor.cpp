#include "AnimationToolkitEditor.h"
#include "AnimationToolkit.h"
#include "AnimationLibrary.h"
#include "AnimationImporter.h"
#include "AnimationOptimizer.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
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
#include "Widgets/Text/SEditableText.h"
#include "Widgets/Views/SListView.h"
#include "EditorStyleSet.h"
#include "Misc/MessageDialog.h"
#include "Framework/Application/SlateApplication.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#define LOCTEXT_NAMESPACE "AnimationToolkitEditor"

void UAnimationToolkitEditorModule::StartupModule()
{
	FAnimationToolkitEditorCommands::Register();

	FGlobalTabManager::Get()->RegisterNomadTabSpawner("AnimationToolkitTab", FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
	{
		FAnimationToolkitEditor* Editor = new FAnimationToolkitEditor();
		Editor->InitEditor(TArray<UObject*>());
		return SNew(SDockTab);
	}))
	.SetDisplayName(LOCTEXT("AnimationToolkitTabLabel", "Animation Toolkit"))
	.SetMenu(WorkspaceMenu::GetMenuStructure().GetStructureCategory()->AddEntry(FTabManager::NewTabSpawner("AnimationToolkitTab", FOnSpawnTab::CreateLambda([](const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab).Label(LOCTEXT("Tab", "Animation Toolkit"));
	}))));

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &UAnimationToolkitEditorModule::RegisterMenus));
}

void UAnimationToolkitEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FAnimationToolkitEditorCommands::Unregister();
}

void UAnimationToolkitEditorModule::RegisterMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	{
		FToolMenuSection& Section = Menu->AddSection("AnimationToolkit", LOCTEXT("AnimationToolkit", "Animation Toolkit"));
		Section.AddMenuEntry(
			"OpenAnimationToolkit",
			LOCTEXT("OpenAnimationToolkit", "Animation Toolkit"),
			LOCTEXT("OpenAnimationToolkitTooltip", "Open Animation Toolkit Editor"),
			FSlateIcon(FEditorStyle::GetAppStyleSetName(), "LevelEditor.OpenCloseContentBrowser"),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
			}))
		);
	}

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	{
		FToolMenuSection& Section = ToolbarMenu->AddSection("AnimationToolkit", LOCTEXT("AnimationToolkit", "Animation Toolkit"));
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"AnimationToolkitButton",
			FUIAction(
				FExecuteAction::CreateLambda([]()
				{
					FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
				})
			),
			LOCTEXT("AnimationToolkitButton", "Animation Toolkit"),
			LOCTEXT("AnimationToolkitButtonTooltip", "Open Animation Toolkit"),
			FSlateIcon(FEditorStyle::GetAppStyleSetName(), "LevelEditor.OpenCloseContentBrowser")
		));
	}
}

void UAnimationToolkitEditorModule::AddToolbarButton(FToolBarBuilder& Builder)
{
	Builder.AddSeparator();

	Builder.AddToolBarButton(
		FAnimationToolkitEditorCommands::Get().ImportAnimation,
		NAME_None,
		LOCTEXT("Import", "Import"),
		LOCTEXT("ImportTooltip", "Import Animation"),
		FSlateIcon(FEditorStyle::GetAppStyleSetName(), "Icons.Import")
	);

	Builder.AddToolBarButton(
		FAnimationToolkitEditorCommands::Get().ExportAnimation,
		NAME_None,
		LOCTEXT("Export", "Export"),
		LOCTEXT("ExportTooltip", "Export Animation"),
		FSlateIcon(FEditorStyle::GetAppStyleSetName(), "Icons.Export")
	);
}

void UAnimationToolkitEditorModule::AddMenuBarButton(FMenuBarBuilder& MenuBuilder)
{
	MenuBuilder.AddPullDownMenu(
		LOCTEXT("AnimationToolkitMenu", "Animation Toolkit"),
		LOCTEXT("AnimationToolkitMenuTooltip", "Animation Toolkit Commands"),
		FNewMenuDelegate::CreateRaw(this, &UAnimationToolkitEditorModule::CreateAnimationToolkitMenu)
	);
}

TSharedRef<SWidget> UAnimationToolkitEditorModule::CreateAnimationToolkitMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("AnimationToolkitSection", LOCTEXT("AnimationToolkitSection", "Animation Toolkit"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ImportAnimation", "Import Animation"),
			LOCTEXT("ImportAnimationTooltip", "Import animation from file"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &UAnimationToolkitEditorModule::OnImportClicked))
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("ExportAnimation", "Export Animation"),
			LOCTEXT("ExportAnimationTooltip", "Export animation to file"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &UAnimationToolkitEditorModule::OnExportClicked))
		);

		MenuBuilder.AddSeparator();

		MenuBuilder.AddMenuEntry(
			LOCTEXT("BlendAnimations", "Blend Animations"),
			LOCTEXT("BlendAnimationsTooltip", "Blend multiple animations"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &UAnimationToolkitEditorModule::OnBlendClicked))
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("MirrorAnimation", "Mirror Animation"),
			LOCTEXT("MirrorAnimationTooltip", "Mirror animation"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &UAnimationToolkitEditorModule::OnMirrorClicked))
		);

		MenuBuilder.AddSeparator();

		MenuBuilder.AddMenuEntry(
			LOCTEXT("CompressAnimation", "Compress Animation"),
			LOCTEXT("CompressAnimationTooltip", "Compress animation"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &UAnimationToolkitEditorModule::OnCompressClicked))
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("OptimizeAnimation", "Optimize Animation"),
			LOCTEXT("OptimizeAnimationTooltip", "Optimize animation"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &UAnimationToolkitEditorModule::OnOptimizeClicked))
		);

		MenuBuilder.AddSeparator();

		MenuBuilder.AddMenuEntry(
			LOCTEXT("RetargetAnimation", "Retarget Animation"),
			LOCTEXT("RetargetAnimationTooltip", "Retarget animation"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &UAnimationToolkitEditorModule::OnRetargetClicked))
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("BatchProcess", "Batch Process"),
			LOCTEXT("BatchProcessTooltip", "Process multiple animations"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &UAnimationToolkitEditorModule::OnBatchProcessClicked))
		);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void UAnimationToolkitEditorModule::OnImportClicked()
{
	FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
}

void UAnimationToolkitEditorModule::OnExportClicked()
{
	FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
}

void UAnimationToolkitEditorModule::OnBlendClicked()
{
	FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
}

void UAnimationToolkitEditorModule::OnMirrorClicked()
{
	FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
}

void UAnimationToolkitEditorModule::OnCompressClicked()
{
	FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
}

void UAnimationToolkitEditorModule::OnOptimizeClicked()
{
	FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
}

void UAnimationToolkitEditorModule::OnRetargetClicked()
{
	FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
}

void UAnimationToolkitEditorModule::OnBatchProcessClicked()
{
	FGlobalTabManager::Get()->TryInvokeTab(FName("AnimationToolkitTab"));
}

void FAnimationToolkitEditorCommands::RegisterCommands()
{
	UI_COMMAND(ImportAnimation, "Import", "Import animation from file", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ExportAnimation, "Export", "Export animation to file", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(BlendAnimations, "Blend", "Blend multiple animations", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(MirrorAnimation, "Mirror", "Mirror animation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CompressAnimation, "Compress", "Compress animation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OptimizeAnimation, "Optimize", "Optimize animation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RetargetAnimation, "Retarget", "Retarget animation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(BatchProcess, "Batch", "Process multiple animations", EUserInterfaceActionType::Button, FInputChord());
}

UAnimationToolkitEditorSettings* UAnimationToolkitEditorLibrary::GetEditorSettings()
{
	return GetMutableDefault<UAnimationToolkitEditorSettings>();
}

void UAnimationToolkitEditorLibrary::SaveEditorSettings()
{
	GetMutableDefault<UAnimationToolkitEditorSettings>()->SaveConfig();
}

TArray<UAnimSequence*> UAnimationToolkitEditorLibrary::GetSelectedAnimations()
{
	TArray<UAnimSequence*> Animations;

	IAssetRegistry& AssetRegistry = IAssetRegistry::Get();
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UAnimSequence::StaticClass()->GetClassPathName(), AssetDataList);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UAnimSequence* Anim = Cast<UAnimSequence>(AssetData.GetAsset());
		if (Anim)
		{
			Animations.Add(Anim);
		}
	}

	return Animations;
}

USkeleton* UAnimationToolkitEditorLibrary::GetSelectedSkeleton()
{
	return nullptr;
}

void UAnimationToolkitEditorLibrary::ShowNotification(const FString& Message, bool bSuccess)
{
	if (GEngine)
	{
		FColor Color = bSuccess ? FColor::Green : FColor::Red;
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, Color, Message);
	}
}

FName FAnimationToolkitEditor::GetToolkitFName() const
{
	return FName("AnimationToolkitEditor");
}

FText FAnimationToolkitEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Animation Toolkit Editor");
}

FString FAnimationToolkitEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("AnimationToolkit");
}

FLinearColor FAnimationToolkitEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.0f, 0.8f, 0.4f);
}

void FAnimationToolkitEditor::InitEditor(const TArray<UObject*>& InObjects)
{
	FormatOptions.Add(MakeShareable(new FString("FBX")));
	FormatOptions.Add(MakeShareable(new FString("glTF")));
	FormatOptions.Add(MakeShareable(new FString("BVH")));
	FormatOptions.Add(MakeShareable(new FString("COLLADA")));

	const TSharedRef<FTabManager::FLayout> StandaloneLayout = FTabManager::NewLayout("StandaloneAnimationToolkitEditor_v1")
		->AddArea(FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split(FTabManager::NewStack()
				->SetSizeCoefficient(0.25f)
				->AddTab("SettingsTab", ETabState::OpenedTab)
			)
			->Split(FTabManager::NewStack()
				->SetSizeCoefficient(0.75f)
				->AddTab("OperationsTab", ETabState::OpenedTab)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitEditor(InObjects, StandaloneLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar);
}

void FAnimationToolkitEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(TabManager);

	TabManager->RegisterTabSpawner("SettingsTab", FOnSpawnTab::CreateRaw(this, &FAnimationToolkitEditor::SpawnTab_Settings))
		.SetDisplayName(LOCTEXT("SettingsTabLabel", "Settings"))
		.SetMenu(WorkspaceMenu::GetMenuStructure().GetStructureCategory()->AddEntry(FTabManager::NewTabSpawner("SettingsTab", FOnSpawnTab::CreateRaw(this, &FAnimationToolkitEditor::SpawnTab_Settings))));

	TabManager->RegisterTabSpawner("OperationsTab", FOnSpawnTab::CreateRaw(this, &FAnimationToolkitEditor::SpawnTab_Optimize))
		.SetDisplayName(LOCTEXT("OperationsTabLabel", "Operations"))
		.SetMenu(WorkspaceMenu::GetMenuStructure().GetStructureCategory()->AddEntry(FTabManager::NewTabSpawner("OperationsTab", FOnSpawnTab::CreateRaw(this, &FAnimationToolkitEditor::SpawnTab_Optimize))));
}

void FAnimationToolkitEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(TabManager);

	TabManager->UnregisterTabSpawner("SettingsTab");
	TabManager->UnregisterTabSpawner("OperationsTab");
}

void FAnimationToolkitEditor::OnImport()
{
	UAnimationToolkitEditorLibrary::ShowNotification(TEXT("Import Animation"));
}

void FAnimationToolkitEditor::OnExport()
{
	UAnimationToolkitEditorLibrary::ShowNotification(TEXT("Export Animation"));
}

void FAnimationToolkitEditor::OnBlend()
{
	UAnimationToolkitEditorLibrary::ShowNotification(TEXT("Blend Animations"));
}

void FAnimationToolkitEditor::OnMirror()
{
	UAnimationToolkitEditorLibrary::ShowNotification(TEXT("Mirror Animation"));
}

void FAnimationToolkitEditor::OnCompress()
{
	UAnimationToolkitEditorLibrary::ShowNotification(TEXT("Compress Animation"));
}

void FAnimationToolkitEditor::OnOptimize()
{
	UAnimationToolkitEditorLibrary::ShowNotification(TEXT("Optimize Animation"));
}

void FAnimationToolkitEditor::OnRetarget()
{
	UAnimationToolkitEditorLibrary::ShowNotification(TEXT("Retarget Animation"));
}

void FAnimationToolkitEditor::OnBatchProcess()
{
	UAnimationToolkitEditorLibrary::ShowNotification(TEXT("Batch Process"));
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Settings(const FSpawnTabArgs& Args)
{
	UAnimationToolkitEditorSettings* Settings = GetMutableDefault<UAnimationToolkitEditorSettings>();

	return SNew(SDockTab)
		.Label(LOCTEXT("Settings", "Settings"))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(10.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ImportSettings", "Import Settings"))
					.Font(FEditorStyle::GetFontStyle("LargeBoldFont"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("DefaultImportPath", "Default Import Path:"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(ImportPathBox, SEditableTextBox)
						.Text(FText::FromString(Settings->DefaultImportPath))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("DefaultFormat", "Default Format:"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(FormatComboBox, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&FormatOptions)
						.OnSelectionChanged_Lambda([Settings](TSharedPtr<FString> NewValue, ESelectInfo::Type)
						{
							if (NewValue.IsValid())
							{
								if (*NewValue == "FBX") Settings->DefaultImportFormat = EAnimationImportFormat::FBX;
								else if (*NewValue == "glTF") Settings->DefaultImportFormat = EAnimationImportFormat::glTF;
								else if (*NewValue == "BVH") Settings->DefaultImportFormat = EAnimationImportFormat::BVH;
							}
						})
						.Content()
						[
							SNew(STextBlock)
							.Text_Lambda([Settings]()
							{
								switch (Settings->DefaultImportFormat)
								{
								case EAnimationImportFormat::FBX: return LOCTEXT("FBX", "FBX");
								case EAnimationImportFormat::glTF: return LOCTEXT("glTF", "glTF");
								case EAnimationImportFormat::BVH: return LOCTEXT("BVH", "BVH");
								default: return LOCTEXT("Unknown", "Unknown");
								}
							})
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(AutoSelectCheckBox, SCheckBox)
						.IsChecked(Settings->bAutoSelectSkeleton ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
						.OnCheckStateChanged_Lambda([Settings](ECheckBoxState NewState)
						{
							Settings->bAutoSelectSkeleton = (NewState == ECheckBoxState::Checked);
						})
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("AutoSelectSkeleton", "Auto-select Skeleton"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 15.0f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ExportSettings", "Export Settings"))
					.Font(FEditorStyle::GetFontStyle("LargeBoldFont"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("DefaultExportPath", "Default Export Path:"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(ExportPathBox, SEditableTextBox)
						.Text(FText::FromString(Settings->DefaultExportPath))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 15.0f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BlendSettings", "Blend Settings"))
					.Font(FEditorStyle::GetFontStyle("LargeBoldFont"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("BlendTime", "Blend Time:"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(BlendTimeSlider, SSlider)
						.Value(Settings->DefaultBlendTime)
						.OnValueChanged_Lambda([Settings](float NewValue)
						{
							Settings->DefaultBlendTime = NewValue;
						})
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("CompressionThreshold", "Compression Threshold:"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(CompressionThresholdSlider, SSlider)
						.Value(Settings->DefaultCompressionThreshold)
						.OnValueChanged_Lambda([Settings](float NewValue)
						{
							Settings->DefaultCompressionThreshold = NewValue;
						})
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 15.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("SaveSettings", "Save Settings"))
					.OnClicked_Lambda([this]() -> FReply
					{
						if (ImportPathBox.IsValid())
						{
							GetMutableDefault<UAnimationToolkitEditorSettings>()->DefaultImportPath = ImportPathBox->GetText().ToString();
						}
						if (ExportPathBox.IsValid())
						{
							GetMutableDefault<UAnimationToolkitEditorSettings>()->DefaultExportPath = ExportPathBox->GetText().ToString();
						}
						UAnimationToolkitEditorLibrary::SaveEditorSettings();
						UAnimationToolkitEditorLibrary::ShowNotification(TEXT("Settings saved!"));
						return FReply::Handled();
					})
				]
			]
		];
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Optimize(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("Operations", "Operations"))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(10.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("QuickActions", "Quick Actions"))
					.Font(FEditorStyle::GetFontStyle("LargeBoldFont"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(SGridPanel)
					+ SGridPanel::Slot(0, 0)
					.Padding(5.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("Import", "Import"))
						.OnClicked_Lambda([this]() -> FReply
						{
							OnImport();
							return FReply::Handled();
						})
					]
					+ SGridPanel::Slot(1, 0)
					.Padding(5.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("Export", "Export"))
						.OnClicked_Lambda([this]() -> FReply
						{
							OnExport();
							return FReply::Handled();
						})
					]
					+ SGridPanel::Slot(0, 1)
					.Padding(5.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("Blend", "Blend"))
						.OnClicked_Lambda([this]() -> FReply
						{
							OnBlend();
							return FReply::Handled();
						})
					]
					+ SGridPanel::Slot(1, 1)
					.Padding(5.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("Mirror", "Mirror"))
						.OnClicked_Lambda([this]() -> FReply
						{
							OnMirror();
							return FReply::Handled();
						})
					]
					+ SGridPanel::Slot(0, 2)
					.Padding(5.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("Compress", "Compress"))
						.OnClicked_Lambda([this]() -> FReply
						{
							OnCompress();
							return FReply::Handled();
						})
					]
					+ SGridPanel::Slot(1, 2)
					.Padding(5.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("Optimize", "Optimize"))
						.OnClicked_Lambda([this]() -> FReply
						{
							OnOptimize();
							return FReply::Handled();
						})
					]
					+ SGridPanel::Slot(0, 3)
					.Padding(5.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("Retarget", "Retarget"))
						.OnClicked_Lambda([this]() -> FReply
						{
							OnRetarget();
							return FReply::Handled();
						})
					]
					+ SGridPanel::Slot(1, 3)
					.Padding(5.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("BatchProcess", "Batch Process"))
						.OnClicked_Lambda([this]() -> FReply
						{
							OnBatchProcess();
							return FReply::Handled();
						})
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 15.0f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SelectedAnimations", "Selected Animations"))
					.Font(FEditorStyle::GetFontStyle("LargeBoldFont"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([]() -> FText
					{
						TArray<UAnimSequence*> Anims = UAnimationToolkitEditorLibrary::GetSelectedAnimations();
						return FText::Format(LOCTEXT("AnimCount", "Found {0} animations"), FText::AsNumber(Anims.Num()));
					})
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE

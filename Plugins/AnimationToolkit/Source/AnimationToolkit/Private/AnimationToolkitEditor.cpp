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
#include "EditorReimportHandler.h"
#include "ScopedTransaction.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Selection.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Widgets/SWindow.h"

#define LOCTEXT_NAMESPACE "AnimationToolkitEditor"

void UAnimationToolkitEditorModule::StartupModule()
{
	FAnimationToolkitEditorCommands::Register();
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UAnimationToolkitEditorModule::RegisterMenus));
}

void UAnimationToolkitEditorModule::ShutdownModule()
{
	FAnimationToolkitEditorCommands::Unregister();
	UToolMenus::UnRegisterStartupCallback(this);
}

void UAnimationToolkitEditorModule::RegisterMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& Section = Menu->AddSection("AnimationToolkit", LOCTEXT("AnimationToolkitMenu", "Animation Toolkit"));
	Section.AddMenuEntry(
		"OpenAnimationToolkit",
		LOCTEXT("OpenAnimationToolkit", "Animation Toolkit"),
		LOCTEXT("OpenAnimationToolkitTooltip", "Open the Animation Toolkit Editor"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimSequence"),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			if (GEditor)
			{
				USelection* Selection = GEditor->GetSelectedObjects();
				if (Selection)
				{
					TArray<UObject*> SelectedObjects;
					Selection->GetSelectedObjects(SelectedObjects);
					for (UObject* Obj : SelectedObjects)
					{
						if (UAnimSequence* Anim = Cast<UAnimSequence>(Obj))
						{
							TSharedRef<FAnimationToolkitEditor> Editor = MakeShareable(new FAnimationToolkitEditor());
							Editor->InitEditor(SelectedObjects);
							break;
						}
					}
				}
			}
		}))
	);
}

void UAnimationToolkitEditorModule::AddToolbarButton(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateLambda([this]() { OnImportClicked(); })),
		NAME_None,
		LOCTEXT("ImportBtn", "Import"),
		LOCTEXT("ImportTooltip", "Import Animation"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import")
	);
}

void UAnimationToolkitEditorModule::AddMenuBarButton(FMenuBarBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("AnimationToolkitMenuBar", "Animation Toolkit"),
		LOCTEXT("AnimationToolkitMenuBarTooltip", "Open the Animation Toolkit"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimSequence"),
		FUIAction(FExecuteAction::CreateUObject(this, &UAnimationToolkitEditorModule::OnImportClicked))
	);
}

TSharedRef<SWidget> UAnimationToolkitEditorModule::CreateAnimationToolkitMenu()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(4.0f)
		[ SNew(STextBlock).Text(LOCTEXT("ToolkitTitle", "Animation Toolkit")) ];
}

void UAnimationToolkitEditorModule::OnImportClicked()
{
	if (GEditor)
	{
		USelection* Selection = GEditor->GetSelectedObjects();
		TArray<UObject*> SelectedObjects;
		if (Selection)
		{
			Selection->GetSelectedObjects(SelectedObjects);
		}

		TSharedRef<FAnimationToolkitEditor> Editor = MakeShareable(new FAnimationToolkitEditor());
		Editor->InitEditor(SelectedObjects);
	}
}

void UAnimationToolkitEditorModule::OnExportClicked()
{
	TArray<UAnimSequence*> Animations = UAnimationToolkitEditorLibrary::GetSelectedAnimations();
	if (Animations.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSelection", "No animations selected. Select one or more animations in the Content Browser."));
		return;
	}
	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			UAnimationToolkitEditorLibrary::OpenExportDialog(GEditor->GetEditorWorldContext().World(), Anim);
		}
	}
}

void UAnimationToolkitEditorModule::OnBlendClicked()
{
	TArray<UAnimSequence*> Animations = UAnimationToolkitEditorLibrary::GetSelectedAnimations();
	if (Animations.Num() < 2)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BlendNeedTwo", "Select at least two animations to blend."));
		return;
	}

	UAnimationToolkitEditorSettings* Settings = UAnimationToolkitEditorLibrary::GetEditorSettings();
	const FString NewName = Animations[0]->GetName() + TEXT("_Blended");
	USkeleton* Skeleton = UAnimationToolkitEditorLibrary::GetSelectedSkeleton();

	TArray<float> Weights;
	const float Weight = 1.0f / Animations.Num();
	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		Weights.Add(Weight);
	}

	UAnimationLibrary::BlendAnimations(
		GEditor->GetEditorWorldContext().World(),
		NewName,
		Skeleton,
		Animations,
		Weights
	);
}

void UAnimationToolkitEditorModule::OnMirrorClicked()
{
	TArray<UAnimSequence*> Animations = UAnimationToolkitEditorLibrary::GetSelectedAnimations();
	if (Animations.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSelection", "No animations selected. Select one or more animations in the Content Browser."));
		return;
	}

	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			const FString NewName = Anim->GetName() + TEXT("_Mirrored");
			USkeleton* Skeleton = Anim->GetSkeleton();
			UAnimationLibrary::MirrorAnimation(
				GEditor->GetEditorWorldContext().World(),
				NewName,
				Skeleton,
				Anim,
				EMirrorFlags::X
			);
		}
	}
}

void UAnimationToolkitEditorModule::OnCompressClicked()
{
	TArray<UAnimSequence*> Animations = UAnimationToolkitEditorLibrary::GetSelectedAnimations();
	if (Animations.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSelection", "No animations selected. Select one or more animations in the Content Browser."));
		return;
	}

	UAnimationToolkitEditorSettings* Settings = UAnimationToolkitEditorLibrary::GetEditorSettings();
	const float Threshold = Settings ? Settings->DefaultCompressionThreshold : 0.01f;

	FAnimationCompressionSettings CompressSettings = UAnimationLibrary::MakeCompressionSettings(
		true, true, true,
		Threshold, 0.1f, 0.01f
	);

	const int32 Compressed = UAnimationLibrary::BatchCompressAnimations(Animations, CompressSettings);
	UE_LOG(LogTemp, Log, TEXT("OnCompressClicked: Compressed %d/%d animations"), Compressed, Animations.Num());
}

void UAnimationToolkitEditorModule::OnOptimizeClicked()
{
	TArray<UAnimSequence*> Animations = UAnimationToolkitEditorLibrary::GetSelectedAnimations();
	if (Animations.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSelection", "No animations selected. Select one or more animations in the Content Browser."));
		return;
	}

	FAnimationCompressionSettings OptimizeSettings = UAnimationLibrary::MakeCompressionSettings(
		true, true, true,
		0.05f, 0.5f, 0.05f
	);

	const int32 Optimized = UAnimationLibrary::BatchCompressAnimations(Animations, OptimizeSettings);
	UE_LOG(LogTemp, Log, TEXT("OnOptimizeClicked: Optimized %d/%d animations"), Optimized, Animations.Num());
}

void UAnimationToolkitEditorModule::OnRetargetClicked()
{
	TArray<UAnimSequence*> Animations = UAnimationToolkitEditorLibrary::GetSelectedAnimations();
	if (Animations.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSelection", "No animations selected. Select one or more animations in the Content Browser."));
		return;
	}

	USkeleton* TargetSkeleton = UAnimationToolkitEditorLibrary::GetSelectedSkeleton();
	if (!TargetSkeleton)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSkeleton", "No target skeleton found. Select a skeleton or an animation with a valid skeleton."));
		return;
	}

	FAnimationRetargetSettings RetargetSettings = UAnimationLibrary::MakeRetargetSettings(
		EAnimationRetargetType::ByName,
		TArray<FAnimationRetargetMapping>()
	);

	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			UAnimationLibrary::RetargetAnimation(
				GEditor->GetEditorWorldContext().World(),
				Anim,
				TargetSkeleton,
				RetargetSettings
			);
		}
	}
}

void UAnimationToolkitEditorModule::OnBatchProcessClicked()
{
	TArray<UAnimSequence*> Animations = UAnimationToolkitEditorLibrary::GetSelectedAnimations();
	if (Animations.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSelection", "No animations selected. Select one or more animations in the Content Browser."));
		return;
	}

	FAnimationCompressionSettings CompressSettings = UAnimationLibrary::MakeCompressionSettings(
		true, true, true,
		0.01f, 0.1f, 0.01f
	);

	const int32 Processed = UAnimationLibrary::BatchCompressAnimations(Animations, CompressSettings);
	UE_LOG(LogTemp, Log, TEXT("OnBatchProcessClicked: Processed %d/%d animations"), Processed, Animations.Num());
	UAnimationToolkitEditorLibrary::ShowNotification(
		FString::Printf(TEXT("Batch processed %d/%d animations"), Processed, Animations.Num()),
		Processed == Animations.Num()
	);
}

void FAnimationToolkitEditorCommands::RegisterCommands()
{
	UI_COMMAND(ImportAnimation, "Import", "Import an animation file", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ExportAnimation, "Export", "Export animation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(BlendAnimations, "Blend", "Blend animations", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(MirrorAnimation, "Mirror", "Mirror animation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CompressAnimation, "Compress", "Compress animation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OptimizeAnimation, "Optimize", "Optimize animation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RetargetAnimation, "Retarget", "Retarget animation", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(BatchProcess, "Batch", "Batch process animations", EUserInterfaceActionType::Button, FInputChord());
}

UAnimationToolkitEditorSettings* UAnimationToolkitEditorLibrary::GetEditorSettings()
{
	return GetMutableDefault<UAnimationToolkitEditorSettings>();
}

void UAnimationToolkitEditorLibrary::SaveEditorSettings()
{
	UAnimationToolkitEditorSettings* Settings = GetMutableDefault<UAnimationToolkitEditorSettings>();
	if (Settings)
	{
		Settings->SaveConfig();
	}
}

TArray<UAnimSequence*> UAnimationToolkitEditorLibrary::GetSelectedAnimations()
{
	TArray<UAnimSequence*> Result;
	if (GEditor)
	{
		TArray<UObject*> SelectedObjects;
		GEditor->GetSelectedObjects()->GetSelectedObjects(SelectedObjects);
		for (UObject* Obj : SelectedObjects)
		{
			if (UAnimSequence* Anim = Cast<UAnimSequence>(Obj))
			{
				Result.Add(Anim);
			}
		}
	}
	return Result;
}

USkeleton* UAnimationToolkitEditorLibrary::GetSelectedSkeleton()
{
	TArray<UObject*> SelectedObjects;
	if (GEditor)
	{
		GEditor->GetSelectedObjects()->GetSelectedObjects(SelectedObjects);
		for (UObject* Obj : SelectedObjects)
		{
			if (USkeleton* Skel = Cast<USkeleton>(Obj))
			{
				return Skel;
			}
		}
	}

	TArray<UAnimSequence*> Animations = GetSelectedAnimations();
	if (Animations.Num() > 0 && Animations[0])
	{
		return Animations[0]->GetSkeleton();
	}

	return nullptr;
}

void UAnimationToolkitEditorLibrary::OpenImportDialog(UObject* WorldContextObject)
{
	if (!GEditor)
	{
		return;
	}

	UAnimationToolkitEditorSettings* Settings = GetMutableDefault<UAnimationToolkitEditorSettings>();
	FString ImportPath = Settings ? Settings->DefaultImportPath : TEXT("/Game/Animations");

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return;
	}

	void* ParentWindowHandle = nullptr;
	if (FSlateApplication::IsInitialized())
	{
		TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
		if (ActiveWindow.IsValid() && ActiveWindow->GetNativeWindow().IsValid())
		{
			ParentWindowHandle = ActiveWindow->GetNativeWindow()->GetOSWindowHandle();
		}
	}

	TArray<FString> OutFiles;
	if (DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("Select Animation File"),
		ImportPath,
		TEXT(""),
		TEXT("FBX Files (*.fbx)|*.fbx|All Supported Files (*.fbx;*.bvh;*.gltf;*.glTF)|*.fbx;*.bvh;*.gltf;*.glTF"),
		EFileDialogFlags::None,
		OutFiles) && OutFiles.Num() > 0)
	{
		for (const FString& File : OutFiles)
		{
			UAnimationImporter::ImportAnimation(
				GEditor->GetEditorWorldContext().World(),
				File,
				UAnimationImporter::DetectFormat(File)
			);
		}
	}
}

void UAnimationToolkitEditorLibrary::OpenExportDialog(UObject* WorldContextObject, UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}
	UAnimationToolkitEditorSettings* Settings = GetMutableDefault<UAnimationToolkitEditorSettings>();
	if (Settings)
	{
		const FString ExportPath = Settings->DefaultExportPath / Animation->GetName() + TEXT(".fbx");
		UAnimationLibrary::ExportAnimationFBX(Animation, ExportPath);
	}
}

void UAnimationToolkitEditorLibrary::ShowNotification(const FString& Message, bool bSuccess)
{
	if (GEngine)
	{
		const FColor DisplayColor = bSuccess ? FColor::Green : FColor::Red;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, DisplayColor, Message);
	}
}

FName FAnimationToolkitEditor::GetToolkitFName() const { return FName("AnimationToolkitEditor"); }
FText FAnimationToolkitEditor::GetBaseToolkitName() const { return LOCTEXT("AppLabel", "Animation Toolkit Editor"); }
FString FAnimationToolkitEditor::GetWorldCentricTabPrefix() const { return TEXT("AnimationToolkit"); }
FLinearColor FAnimationToolkitEditor::GetWorldCentricTabColorScale() const { return FLinearColor(0.0f, 0.8f, 0.4f); }

void FAnimationToolkitEditor::InitEditor(const TArray<UObject*>& InObjects)
{
	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	const TSharedRef<FTabManager::FLayout> StandaloneLayout = FTabManager::NewLayout("AnimationToolkitEditorLayout")
		->AddArea(FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split(FTabManager::NewStack()
				->SetSizeCoefficient(0.2f)
				->AddTab("SettingsTab", ETabState::OpenedTab))
			->Split(FTabManager::NewStack()
				->SetSizeCoefficient(0.8f)
				->AddTab("ImportTab", ETabState::OpenedTab)
				->AddTab("ExportTab", ETabState::ClosedTab)
				->AddTab("BlendTab", ETabState::ClosedTab)
				->AddTab("MirrorTab", ETabState::ClosedTab)
				->AddTab("CompressTab", ETabState::ClosedTab)
				->AddTab("OptimizeTab", ETabState::ClosedTab)
				->AddTab("RetargetTab", ETabState::ClosedTab)
				->AddTab("BatchTab", ETabState::ClosedTab)));

	InitAssetEditor(EToolkitMode::Standalone, nullptr, TEXT("AnimationToolkitEditorApp"), StandaloneLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, InObjects);
}

void FAnimationToolkitEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	TabManager = InTabManager;
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	const IWorkspaceMenuStructure& WorkspaceMenuStructure = FModuleManager::LoadModuleChecked<FWorkspaceMenuStructureModule>("WorkspaceMenuStructure").GetWorkspaceMenuStructure();
	TSharedRef<FWorkspaceItem> MenuRoot = WorkspaceMenuStructure.GetStructureRoot();

	InTabManager->RegisterTabSpawner("SettingsTab", FOnSpawnTab::CreateSP(this, &FAnimationToolkitEditor::SpawnTab_Settings))
		.SetDisplayName(LOCTEXT("SettingsTabLabel", "Settings"))
		.SetGroup(MenuRoot)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings"));

	InTabManager->RegisterTabSpawner("ImportTab", FOnSpawnTab::CreateSP(this, &FAnimationToolkitEditor::SpawnTab_Import))
		.SetDisplayName(LOCTEXT("ImportTabLabel", "Import"))
		.SetGroup(MenuRoot);

	InTabManager->RegisterTabSpawner("ExportTab", FOnSpawnTab::CreateSP(this, &FAnimationToolkitEditor::SpawnTab_Export))
		.SetDisplayName(LOCTEXT("ExportTabLabel", "Export"))
		.SetGroup(MenuRoot);

	InTabManager->RegisterTabSpawner("BlendTab", FOnSpawnTab::CreateSP(this, &FAnimationToolkitEditor::SpawnTab_Blend))
		.SetDisplayName(LOCTEXT("BlendTabLabel", "Blend"))
		.SetGroup(MenuRoot);

	InTabManager->RegisterTabSpawner("MirrorTab", FOnSpawnTab::CreateSP(this, &FAnimationToolkitEditor::SpawnTab_Mirror))
		.SetDisplayName(LOCTEXT("MirrorTabLabel", "Mirror"))
		.SetGroup(MenuRoot);

	InTabManager->RegisterTabSpawner("CompressTab", FOnSpawnTab::CreateSP(this, &FAnimationToolkitEditor::SpawnTab_Compress))
		.SetDisplayName(LOCTEXT("CompressTabLabel", "Compress"))
		.SetGroup(MenuRoot);

	InTabManager->RegisterTabSpawner("OptimizeTab", FOnSpawnTab::CreateSP(this, &FAnimationToolkitEditor::SpawnTab_Optimize))
		.SetDisplayName(LOCTEXT("OptimizeTabLabel", "Optimize"))
		.SetGroup(MenuRoot);

	InTabManager->RegisterTabSpawner("RetargetTab", FOnSpawnTab::CreateSP(this, &FAnimationToolkitEditor::SpawnTab_Retarget))
		.SetDisplayName(LOCTEXT("RetargetTabLabel", "Retarget"))
		.SetGroup(MenuRoot);

	InTabManager->RegisterTabSpawner("BatchTab", FOnSpawnTab::CreateSP(this, &FAnimationToolkitEditor::SpawnTab_Batch))
		.SetDisplayName(LOCTEXT("BatchTabLabel", "Batch Process"))
		.SetGroup(MenuRoot);
}

void FAnimationToolkitEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner("SettingsTab");
	InTabManager->UnregisterTabSpawner("ImportTab");
	InTabManager->UnregisterTabSpawner("ExportTab");
	InTabManager->UnregisterTabSpawner("BlendTab");
	InTabManager->UnregisterTabSpawner("MirrorTab");
	InTabManager->UnregisterTabSpawner("CompressTab");
	InTabManager->UnregisterTabSpawner("OptimizeTab");
	InTabManager->UnregisterTabSpawner("RetargetTab");
	InTabManager->UnregisterTabSpawner("BatchTab");
}

void FAnimationToolkitEditor::OnImport() { OnImportAction.Broadcast(); }
void FAnimationToolkitEditor::OnExport() { OnExportAction.Broadcast(); }
void FAnimationToolkitEditor::OnBlend() { OnBlendAction.Broadcast(); }
void FAnimationToolkitEditor::OnMirror() { OnMirrorAction.Broadcast(); }
void FAnimationToolkitEditor::OnCompress() { OnCompressAction.Broadcast(); }
void FAnimationToolkitEditor::OnOptimize() { OnOptimizeAction.Broadcast(); }
void FAnimationToolkitEditor::OnRetarget() { OnRetargetAction.Broadcast(); }
void FAnimationToolkitEditor::OnBatchProcess() { OnBatchAction.Broadcast(); }

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Import(const FSpawnTabArgs& Args)
{
	ImportPathBox = SNew(SEditableTextBox).Text(LOCTEXT("DefaultImportPath", "/Game/Animations"));
	return SNew(SDockTab).Label(LOCTEXT("ImportTabLabel", "Import"))
		[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(STextBlock).Text(LOCTEXT("ImportPathLabel", "Import Path:"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[ImportPathBox.ToSharedRef()]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(SButton).Text(LOCTEXT("ImportButton", "Import"))
				.OnClicked_Lambda([this]() -> FReply { OnImport(); return FReply::Handled(); })]];
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Export(const FSpawnTabArgs& Args)
{
	ExportPathBox = SNew(SEditableTextBox).Text(LOCTEXT("DefaultExportPath", "C:/Export/Animations"));
	FormatOptions.Empty();
	FormatOptions.Add(MakeShareable(new FString("FBX")));
	FormatOptions.Add(MakeShareable(new FString("glTF")));
	FormatOptions.Add(MakeShareable(new FString("BVH")));
	FormatOptions.Add(MakeShareable(new FString("COLLADA")));
	FormatComboBox = SNew(SComboBox<TSharedPtr<FString>>).OptionsSource(&FormatOptions)
		.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) { return SNew(STextBlock).Text(FText::FromString(*Item)); })
		.InitiallySelectedItem(FormatOptions[0]);
	return SNew(SDockTab).Label(LOCTEXT("ExportTabLabel", "Export"))
		[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(STextBlock).Text(LOCTEXT("ExportPathLabel", "Export Path:"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[ExportPathBox.ToSharedRef()]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(STextBlock).Text(LOCTEXT("FormatLabel", "Format:"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[FormatComboBox.ToSharedRef()]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(SButton).Text(LOCTEXT("ExportButton", "Export"))
				.OnClicked_Lambda([this]() -> FReply { OnExport(); return FReply::Handled(); })]];
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Blend(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(LOCTEXT("BlendTabLabel", "Blend"))
		[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(SButton).Text(LOCTEXT("BlendButton", "Blend Selected"))
				.OnClicked_Lambda([this]() -> FReply { OnBlend(); return FReply::Handled(); })]];
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Mirror(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(LOCTEXT("MirrorTabLabel", "Mirror"))
		[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(SButton).Text(LOCTEXT("MirrorButton", "Mirror Selected"))
				.OnClicked_Lambda([this]() -> FReply { OnMirror(); return FReply::Handled(); })]];
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Compress(const FSpawnTabArgs& Args)
{
	CompressionThresholdSlider = SNew(SSlider).Value(0.01f);
	return SNew(SDockTab).Label(LOCTEXT("CompressTabLabel", "Compress"))
		[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[CompressionThresholdSlider.ToSharedRef()]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(SButton).Text(LOCTEXT("CompressButton", "Compress Selected"))
				.OnClicked_Lambda([this]() -> FReply { OnCompress(); return FReply::Handled(); })]];
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Optimize(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(LOCTEXT("OptimizeTabLabel", "Optimize"))
		[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(SButton).Text(LOCTEXT("OptimizeButton", "Optimize Selected"))
				.OnClicked_Lambda([this]() -> FReply { OnOptimize(); return FReply::Handled(); })]];
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Retarget(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(LOCTEXT("RetargetTabLabel", "Retarget"))
		[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(SButton).Text(LOCTEXT("RetargetButton", "Retarget Selected"))
				.OnClicked_Lambda([this]() -> FReply { OnRetarget(); return FReply::Handled(); })]];
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Batch(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(LOCTEXT("BatchTabLabel", "Batch Process"))
		[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)[SNew(SButton).Text(LOCTEXT("BatchButton", "Process Selected"))
				.OnClicked_Lambda([this]() -> FReply { OnBatchProcess(); return FReply::Handled(); })]];
}

TSharedRef<SDockTab> FAnimationToolkitEditor::SpawnTab_Settings(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(LOCTEXT("SettingsTabLabel", "Settings"))
		[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)
			[
				SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
				{
					UAnimationToolkitEditorSettings* S = GetMutableDefault<UAnimationToolkitEditorSettings>();
					if (S) S->bAutoSelectSkeleton = (NewState == ECheckBoxState::Checked);
				})
				[SNew(STextBlock).Text(LOCTEXT("AutoSelectSkeleton", "Auto-select Skeleton"))]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.0f)
			[SNew(SButton).Text(LOCTEXT("SaveSettings", "Save Settings"))
				.OnClicked_Lambda([]() -> FReply
				{
					UAnimationToolkitEditorSettings* S = GetMutableDefault<UAnimationToolkitEditorSettings>();
					if (S) S->SaveConfig();
					return FReply::Handled();
				})]];
}

#undef LOCTEXT_NAMESPACE

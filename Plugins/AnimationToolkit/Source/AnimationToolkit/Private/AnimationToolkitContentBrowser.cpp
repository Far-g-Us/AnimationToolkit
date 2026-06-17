#include "AnimationToolkitContentBrowser.h"
#include "AnimationToolkitEditor.h"
#include "AnimationLibrary.h"
#include "AnimationOptimizer.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Editor.h"
#include "ToolMenus.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Misc/MessageDialog.h"
#include "Framework/Application/SlateApplication.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "AnimationToolkitContentBrowser"

void UAnimationToolkitContentBrowserModule::StartupModule()
{
	RegisterAssetTypeActions();
}

void UAnimationToolkitContentBrowserModule::ShutdownModule()
{
	UnregisterAssetTypeActions();
}

void UAnimationToolkitContentBrowserModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedRef<FAnimationToolkitAssetTypeActions> Actions = MakeShared<FAnimationToolkitAssetTypeActions>();
	AssetTools.RegisterAssetTypeActions(Actions);
	RegisteredAssetTypeActions.Add(Actions);
}

void UAnimationToolkitContentBrowserModule::UnregisterAssetTypeActions()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (TSharedRef<IAssetTypeActions>& Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
	RegisteredAssetTypeActions.Empty();
}

FText FAnimationToolkitAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetTypeName", "Animation Toolkit Asset");
}

FColor FAnimationToolkitAssetTypeActions::GetTypeColor() const
{
	return FColor(0, 200, 100);
}

UClass* FAnimationToolkitAssetTypeActions::GetSupportedClass() const
{
	return UAnimSequence::StaticClass();
}

uint32 FAnimationToolkitAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Animation;
}

void FAnimationToolkitAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<UAnimSequence*> AnimSequences;
	for (UObject* Obj : InObjects)
	{
		if (UAnimSequence* Anim = Cast<UAnimSequence>(Obj))
		{
			AnimSequences.Add(Anim);
		}
	}

	if (AnimSequences.Num() > 0)
	{
		Section.AddMenuEntry(
			"CompressAnimation",
			LOCTEXT("CompressAnimation", "Compress Animation"),
			LOCTEXT("CompressAnimationTooltip", "Compress this animation using toolkit compression settings"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([AnimSequences]()
				{
					for (UAnimSequence* Anim : AnimSequences)
					{
						if (Anim)
						{
							UAnimationToolkitContextMenuActions::CompressSelectedAnimation(Anim);
						}
					}
				})
			)
		);

		Section.AddMenuEntry(
			"OptimizeAnimation",
			LOCTEXT("OptimizeAnimation", "Optimize Animation"),
			LOCTEXT("OptimizeAnimationTooltip", "Optimize this animation using toolkit optimization"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([AnimSequences]()
				{
					for (UAnimSequence* Anim : AnimSequences)
					{
						if (Anim)
						{
							UAnimationToolkitContextMenuActions::OptimizeSelectedAnimation(Anim);
						}
					}
				})
			)
		);

		Section.AddMenuEntry(
			"MirrorAnimation",
			LOCTEXT("MirrorAnimation", "Mirror Animation"),
			LOCTEXT("MirrorAnimationTooltip", "Create a mirrored copy of this animation"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([AnimSequences]()
				{
					for (UAnimSequence* Anim : AnimSequences)
					{
						if (Anim)
						{
							UAnimationToolkitContextMenuActions::MirrorSelectedAnimation(Anim);
						}
					}
				})
			)
		);

		Section.AddMenuEntry(
			"ExportAnimation",
			LOCTEXT("ExportAnimation", "Export Animation"),
			LOCTEXT("ExportAnimationTooltip", "Export this animation to FBX"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([AnimSequences]()
				{
					for (UAnimSequence* Anim : AnimSequences)
					{
						if (Anim)
						{
							UAnimationToolkitContextMenuActions::ExportSelectedAnimation(Anim);
						}
					}
				})
			)
		);

		Section.AddMenuEntry(
			"ShowAnimationInfo",
			LOCTEXT("ShowAnimationInfo", "Show Animation Info"),
			LOCTEXT("ShowAnimationInfoTooltip", "Display detailed information about this animation"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([AnimSequences]()
				{
					for (UAnimSequence* Anim : AnimSequences)
					{
						if (Anim)
						{
							UAnimationToolkitContextMenuActions::ShowAnimationInfo(Anim);
						}
					}
				})
			)
		);
	}

	if (AnimSequences.Num() >= 2)
	{
		Section.AddMenuEntry(
			"BlendAnimations",
			LOCTEXT("BlendAnimations", "Blend Animations"),
			LOCTEXT("BlendAnimationsTooltip", "Blend the selected animations together"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([AnimSequences]()
				{
					UAnimationToolkitContextMenuActions::BlendSelectedAnimations(AnimSequences);
				})
			)
		);

		Section.AddMenuEntry(
			"BatchCompress",
			LOCTEXT("BatchCompress", "Batch Compress"),
			LOCTEXT("BatchCompressTooltip", "Compress all selected animations"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([AnimSequences]()
				{
					UAnimationToolkitContextMenuActions::BatchCompressSelected(AnimSequences);
				})
			)
		);

		Section.AddMenuEntry(
			"BatchOptimize",
			LOCTEXT("BatchOptimize", "Batch Optimize"),
			LOCTEXT("BatchOptimizeTooltip", "Optimize all selected animations"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([AnimSequences]()
				{
					UAnimationToolkitContextMenuActions::BatchOptimizeSelected(AnimSequences);
				})
			)
		);
	}
}

void FAnimationToolkitAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	OpenAssetEditor(InObjects, EAssetTypeActivationOpenedMethod::Edit, EditWithinLevelEditor);
}

void FAnimationToolkitAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, const EAssetTypeActivationOpenedMethod OpenedMethod, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor ? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>() : nullptr;
	if (AssetEditorSubsystem)
	{
		AssetEditorSubsystem->OpenEditorForAssets(InObjects, OpenedMethod);
	}
}

bool FAnimationToolkitAssetTypeActions::AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
{
	if (InObjects.Num() > 0)
	{
		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor ? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>() : nullptr;
		if (AssetEditorSubsystem)
		{
			if (ActivationType == EAssetTypeActivationMethod::DoubleClicked || ActivationType == EAssetTypeActivationMethod::Opened)
			{
				AssetEditorSubsystem->OpenEditorForAssets(InObjects);
			}
			return true;
		}
	}
	return false;
}

bool FAnimationToolkitAssetTypeActions::CanRename(const FAssetData& AssetData, FText* OutErrorMsg) const
{
	return AssetData.GetClass() == UAnimSequence::StaticClass();
}

bool FAnimationToolkitAssetTypeActions::CanDuplicate(const FAssetData& AssetData, FText* OutErrorMsg) const
{
	return AssetData.GetClass() == UAnimSequence::StaticClass();
}

TArray<FAssetData> FAnimationToolkitAssetTypeActions::GetValidAssetsForPreviewOrEdit(TArrayView<const FAssetData> InAssetDataList, bool bAllowHiddenAssets)
{
	TArray<FAssetData> ValidAssets;
	for (const FAssetData& AssetData : InAssetDataList)
	{
		if (AssetData.GetClass() == UAnimSequence::StaticClass())
		{
			ValidAssets.Add(AssetData);
		}
	}
	return ValidAssets;
}

bool FAnimationToolkitAssetTypeActions::CanFilter()
{
	return true;
}

FName FAnimationToolkitAssetTypeActions::GetFilterName() const
{
	return FName("Animation");
}

bool FAnimationToolkitAssetTypeActions::CanLocalize() const
{
	return false;
}

bool FAnimationToolkitAssetTypeActions::CanMerge() const
{
	return false;
}

void FAnimationToolkitAssetTypeActions::Merge(UObject* InTargetAsset, UObject* InBaseAsset, UObject* InTheirAsset, const FOnMergeResolved& OnResolved)
{
	if (!InTargetAsset || !InBaseAsset || !InTheirAsset)
	{
		return;
	}

	UAnimSequence* TargetAnim = Cast<UAnimSequence>(InTargetAsset);
	UAnimSequence* BaseAnim = Cast<UAnimSequence>(InBaseAsset);
	UAnimSequence* TheirAnim = Cast<UAnimSequence>(InTheirAsset);

	if (TargetAnim && BaseAnim && TheirAnim)
	{
		TArray<UAnimSequence*> Animations;
		Animations.Add(BaseAnim);
		Animations.Add(TheirAnim);

		TArray<float> Weights;
		Weights.Add(0.5f);
		Weights.Add(0.5f);

		USkeleton* Skeleton = BaseAnim->GetSkeleton();
		if (Skeleton)
		{
			const FString MergeName = TargetAnim->GetName() + TEXT("_Merged");
			UAnimationLibrary::BlendAnimations(nullptr, MergeName, Skeleton, Animations, Weights);
		}
	}
}

void FAnimationToolkitAssetTypeActions::Merge(UObject* InTargetAsset)
{
	if (!InTargetAsset)
	{
		return;
	}

	OpenAssetEditor({InTargetAsset}, EAssetTypeActivationOpenedMethod::Edit, nullptr);
}

FString FAnimationToolkitAssetTypeActions::GetObjectDisplayName(UObject* Object) const
{
	return Object ? Object->GetName() : FString();
}

const TArray<FText>& FAnimationToolkitAssetTypeActions::GetSubMenus() const
{
	static const TArray<FText> Empty;
	return Empty;
}

bool FAnimationToolkitAssetTypeActions::ShouldForceWorldCentric()
{
	return false;
}

void FAnimationToolkitAssetTypeActions::PerformAssetDiff(UObject* Asset1, UObject* Asset2, const FRevisionInfo& OldRevision, const FRevisionInfo& NewRevision) const
{
	if (!Asset1 && !Asset2)
	{
		return;
	}

	UAnimSequence* Anim1 = Cast<UAnimSequence>(Asset1);
	UAnimSequence* Anim2 = Cast<UAnimSequence>(Asset2);

	FString DiffInfo;

	if (Anim1)
	{
		const float Duration1 = UAnimationLibrary::GetAnimationDuration(Anim1);
		const int32 FrameCount1 = UAnimationLibrary::GetAnimationFrameCount(Anim1);
		const int32 KeyCount1 = UAnimationLibrary::GetAnimationKeyCount(Anim1);
		const float CompRatio1 = UAnimationLibrary::GetAnimationCompressionRatio(Anim1);

		DiffInfo += FString::Printf(TEXT("Asset: %s\nDuration: %.2fs | Frames: %d | Keys: %d | Compression: %.2f%%\n\n"),
			*Anim1->GetName(), Duration1, FrameCount1, KeyCount1, CompRatio1 * 100.0f);
	}

	if (Anim2)
	{
		const float Duration2 = UAnimationLibrary::GetAnimationDuration(Anim2);
		const int32 FrameCount2 = UAnimationLibrary::GetAnimationFrameCount(Anim2);
		const int32 KeyCount2 = UAnimationLibrary::GetAnimationKeyCount(Anim2);
		const float CompRatio2 = UAnimationLibrary::GetAnimationCompressionRatio(Anim2);

		DiffInfo += FString::Printf(TEXT("Asset: %s\nDuration: %.2fs | Frames: %d | Keys: %d | Compression: %.2f%%"),
			*Anim2->GetName(), Duration2, FrameCount2, KeyCount2, CompRatio2 * 100.0f);
	}

	if (!DiffInfo.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(DiffInfo));
	}
}

UThumbnailInfo* FAnimationToolkitAssetTypeActions::GetThumbnailInfo(UObject* Asset) const
{
	return nullptr;
}

EThumbnailPrimType FAnimationToolkitAssetTypeActions::GetDefaultThumbnailPrimitiveType(UObject* Asset) const
{
	return EThumbnailPrimType::TPT_None;
}

TSharedPtr<SWidget> FAnimationToolkitAssetTypeActions::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	UAnimSequence* Anim = Cast<UAnimSequence>(AssetData.GetAsset());
	if (!Anim)
	{
		return TSharedPtr<SWidget>();
	}

	const float Duration = UAnimationLibrary::GetAnimationDuration(Anim);
	const int32 FrameCount = UAnimationLibrary::GetAnimationFrameCount(Anim);

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::Format(
				LOCTEXT("ThumbDuration", "{0}s"),
				FText::AsNumber(Duration)))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::Format(
				LOCTEXT("ThumbFrames", "{0} frames"),
				FText::AsNumber(FrameCount)))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
		];
}

FText FAnimationToolkitAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Animation Toolkit Animation Asset");
}

bool FAnimationToolkitAssetTypeActions::IsImportedAsset() const
{
	return true;
}

void FAnimationToolkitAssetTypeActions::GetResolvedSourceFilePaths(const TArray<UObject*>& InAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (UObject* Obj : InAssets)
	{
		if (UAnimSequence* Anim = Cast<UAnimSequence>(Obj))
		{
			OutSourceFilePaths.Add(Anim->GetPathName());
		}
	}
}

void FAnimationToolkitAssetTypeActions::GetSourceFileLabels(const TArray<UObject*>& InAssets, TArray<FString>& OutSourceFileLabels) const
{
	for (UObject* Obj : InAssets)
	{
		if (UAnimSequence* Anim = Cast<UAnimSequence>(Obj))
		{
			OutSourceFileLabels.Add(Anim->GetName());
		}
	}
}

void FAnimationToolkitAssetTypeActions::BuildBackendFilter(FARFilter& Filter)
{
	Filter.ClassPaths.Add(FTopLevelAssetPath(UAnimSequence::StaticClass()));
}

FText FAnimationToolkitAssetTypeActions::GetDisplayNameFromAssetData(const FAssetData& AssetData) const
{
	return FText::FromString(AssetData.AssetName.ToString());
}

FTopLevelAssetPath FAnimationToolkitAssetTypeActions::GetClassPathName() const
{
	return FTopLevelAssetPath(UAnimSequence::StaticClass());
}

bool FAnimationToolkitAssetTypeActions::SupportsOpenedMethod(const EAssetTypeActivationOpenedMethod OpenedMethod) const
{
	return true;
}

const FSlateBrush* FAnimationToolkitAssetTypeActions::GetThumbnailBrush(const FAssetData& AssetData, const FName ClassName) const
{
	return FAppStyle::GetBrush("ClassIcon.AnimSequence");
}

const FSlateBrush* FAnimationToolkitAssetTypeActions::GetIconBrush(const FAssetData& AssetData, const FName ClassName) const
{
	return FAppStyle::GetBrush("ClassIcon.AnimSequence");
}

void UAnimationToolkitContextMenuActions::ImportAnimationFromContext(const FString& FilePath)
{
	if (FilePath.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoFilePath", "No file path specified."));
		return;
	}

	if (!UAnimationLibrary::IsAnimationFormatSupported(FilePath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(
			LOCTEXT("UnsupportedFormat", "Unsupported animation format: {0}"),
			FText::FromString(FPaths::GetExtension(FilePath))));
		return;
	}

	UAnimationToolkitEditorSettings* Settings = GetMutableDefault<UAnimationToolkitEditorSettings>();
	const FString ImportPath = Settings ? Settings->DefaultImportPath : TEXT("/Game/Animations");
	const FString AnimName = FPaths::GetBaseFilename(FilePath);
	const EAnimationImportFormat Format = UAnimationLibrary::DetectAnimationFormat(FilePath);

	UAnimationLibrary::ImportAnimationAsSequence(nullptr, FilePath, AnimName, nullptr, Format);
	UE_LOG(LogTemp, Log, TEXT("Animation Toolkit: Imported animation '%s' from '%s'"), *AnimName, *FilePath);
}

void UAnimationToolkitContextMenuActions::ExportSelectedAnimation(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}
	UAnimationToolkitEditorSettings* Settings = GetMutableDefault<UAnimationToolkitEditorSettings>();
	const FString ExportPath = (Settings ? Settings->DefaultExportPath : TEXT("C:/Export/Animations")) /
		Animation->GetName() + TEXT(".fbx");
	UAnimationLibrary::ExportAnimationFBX(Animation, ExportPath);
}

void UAnimationToolkitContextMenuActions::BlendSelectedAnimations(const TArray<UAnimSequence*>& Animations)
{
	if (Animations.Num() < 2)
	{
		return;
	}

	USkeleton* Skeleton = nullptr;
	for (UAnimSequence* Anim : Animations)
	{
		if (Anim && Anim->GetSkeleton())
		{
			Skeleton = Anim->GetSkeleton();
			break;
		}
	}

	if (!Skeleton)
	{
		return;
	}

	TArray<float> Weights;
	Weights.SetNum(Animations.Num());
	for (float& W : Weights)
	{
		W = 1.0f / Animations.Num();
	}

	const FString BlendName = Animations[0]->GetName() + TEXT("_Blended");
	UAnimationLibrary::BlendAnimations(nullptr, BlendName, Skeleton, Animations, Weights);
}

void UAnimationToolkitContextMenuActions::MirrorSelectedAnimation(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}
	const FString MirrorName = Animation->GetName() + TEXT("_Mirrored");
	UAnimationLibrary::MirrorAnimation(nullptr, MirrorName, Animation->GetSkeleton(), Animation, EMirrorFlags::X);
}

void UAnimationToolkitContextMenuActions::CompressSelectedAnimation(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}
	FAnimationCompressionSettings Settings;
	Settings.bCompressTranslation = true;
	Settings.bCompressRotation = true;
	Settings.bCompressScale = false;
	Settings.TranslationThreshold = 0.01f;
	Settings.RotationThreshold = 0.1f;
	Settings.ScaleThreshold = 0.01f;
	UAnimationLibrary::CompressAnimation(Animation, Settings);
}

void UAnimationToolkitContextMenuActions::OptimizeSelectedAnimation(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}

	FAnimationCompressionSettings CompressionSettings;
	CompressionSettings.bCompressTranslation = true;
	CompressionSettings.bCompressRotation = true;
	CompressionSettings.bCompressScale = false;
	CompressionSettings.TranslationThreshold = 0.01f;
	CompressionSettings.RotationThreshold = 0.1f;
	CompressionSettings.ScaleThreshold = 0.01f;
	UAnimationLibrary::CompressAnimation(Animation, CompressionSettings);

	Animation->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("Animation Toolkit: Optimized animation '%s'"), *Animation->GetName());
}

void UAnimationToolkitContextMenuActions::RetargetSelectedAnimation(UAnimSequence* Animation, USkeleton* TargetSkeleton)
{
	if (!Animation || !TargetSkeleton)
	{
		return;
	}
	FAnimationRetargetSettings RetargetSettings;
	RetargetSettings.RetargetType = EAnimationRetargetType::ByName;
	UAnimationLibrary::RetargetAnimation(nullptr, Animation, TargetSkeleton, RetargetSettings);
}

void UAnimationToolkitContextMenuActions::AddTagToSelected(UAnimSequence* Animation, const FName& Tag)
{
	if (!Animation) return;
	UAnimationLibrary::SetAnimationTag(Animation, Tag);
}

void UAnimationToolkitContextMenuActions::RemoveTagFromSelected(UAnimSequence* Animation, const FName& Tag)
{
	if (!Animation) return;
	UAnimationLibrary::RemoveAnimationTag(Animation, Tag);
}

TArray<FName> UAnimationToolkitContextMenuActions::GetAnimationTagsFromSelected(UAnimSequence* Animation)
{
	if (!Animation) return TArray<FName>();
	return UAnimationLibrary::GetAnimationTags(Animation);
}

void UAnimationToolkitContextMenuActions::BatchCompressSelected(const TArray<UAnimSequence*>& Animations)
{
	FAnimationCompressionSettings Settings;
	Settings.bCompressTranslation = true;
	Settings.bCompressRotation = true;
	Settings.bCompressScale = false;
	Settings.TranslationThreshold = 0.01f;
	Settings.RotationThreshold = 0.1f;
	Settings.ScaleThreshold = 0.01f;
	UAnimationLibrary::BatchCompressAnimations(Animations, Settings);
}

void UAnimationToolkitContextMenuActions::BatchOptimizeSelected(const TArray<UAnimSequence*>& Animations)
{
	FAnimationCompressionSettings Settings;
	Settings.bCompressTranslation = true;
	Settings.bCompressRotation = true;
	Settings.bCompressScale = false;
	Settings.TranslationThreshold = 0.01f;
	Settings.RotationThreshold = 0.1f;
	Settings.ScaleThreshold = 0.01f;

	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			UAnimationLibrary::CompressAnimation(Anim, Settings);
			Anim->MarkPackageDirty();
		}
	}
}

void UAnimationToolkitContextMenuActions::ShowAnimationInfo(UAnimSequence* Animation)
{
	if (!Animation)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoAnimInfo", "No animation selected."));
		return;
	}

	const float Duration = UAnimationLibrary::GetAnimationDuration(Animation);
	const int32 FrameCount = UAnimationLibrary::GetAnimationFrameCount(Animation);

	const FString Info = FString::Printf(
		TEXT("Animation: %s\nDuration: %.2f seconds\nFrame Count: %d"),
		*Animation->GetName(), Duration, FrameCount);

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Info));
}

#undef LOCTEXT_NAMESPACE

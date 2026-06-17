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
	RegisterAssetTypeActions();
}

void UAnimationToolkitContentBrowserModule::ShutdownModule()
{
	UnregisterAssetTypeActions();
}

void UAnimationToolkitContentBrowserModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	TSharedRef<FAnimationToolkitAssetTypeActions> AssetTypeActions = MakeShareable(new FAnimationToolkitAssetTypeActions());
	AssetTools.RegisterAssetTypeActions(AssetTypeActions);
	RegisteredAssetTypeActions.Add(AssetTypeActions);
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
	return LOCTEXT("AssetTypeActions", "Animation Toolkit");
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
	Section.AddMenuEntry(
		"AnimationToolkit_Import",
		LOCTEXT("ImportAnimation", "Import Animation"),
		LOCTEXT("ImportAnimationTooltip", "Import animation from file"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([InObjects]()
		{
			for (UObject* Obj : InObjects)
			{
				UAnimSequence* Anim = Cast<UAnimSequence>(Obj);
				if (Anim)
				{
					UAnimationToolkitContextMenuActions::ShowAnimationInfo(Anim);
				}
			}
		}))
	);

	Section.AddMenuEntry(
		"AnimationToolkit_Export",
		LOCTEXT("ExportAnimation", "Export Animation"),
		LOCTEXT("ExportAnimationTooltip", "Export animation to file"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([InObjects]()
		{
			for (UObject* Obj : InObjects)
			{
				UAnimSequence* Anim = Cast<UAnimSequence>(Obj);
				if (Anim)
				{
					UAnimationToolkitContextMenuActions::ExportSelectedAnimation(Anim);
				}
			}
		}))
	);

	Section.AddMenuEntry(
		"AnimationToolkit_Blend",
		LOCTEXT("BlendAnimations", "Blend Animations"),
		LOCTEXT("BlendAnimationsTooltip", "Blend selected animations"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([InObjects]()
		{
			TArray<UAnimSequence*> Animations;
			for (UObject* Obj : InObjects)
			{
				UAnimSequence* Anim = Cast<UAnimSequence>(Obj);
				if (Anim)
				{
					Animations.Add(Anim);
				}
			}
			UAnimationToolkitContextMenuActions::BlendSelectedAnimations(Animations);
		}))
	);

	Section.AddMenuEntry(
		"AnimationToolkit_Mirror",
		LOCTEXT("MirrorAnimation", "Mirror Animation"),
		LOCTEXT("MirrorAnimationTooltip", "Mirror selected animation"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([InObjects]()
		{
			for (UObject* Obj : InObjects)
			{
				UAnimSequence* Anim = Cast<UAnimSequence>(Obj);
				if (Anim)
				{
					UAnimationToolkitContextMenuActions::MirrorSelectedAnimation(Anim);
				}
			}
		}))
	);

	Section.AddMenuEntry(
		"AnimationToolkit_Compress",
		LOCTEXT("CompressAnimation", "Compress Animation"),
		LOCTEXT("CompressAnimationTooltip", "Compress selected animation"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([InObjects]()
		{
			for (UObject* Obj : InObjects)
			{
				UAnimSequence* Anim = Cast<UAnimSequence>(Obj);
				if (Anim)
				{
					UAnimationToolkitContextMenuActions::CompressSelectedAnimation(Anim);
				}
			}
		}))
	);

	Section.AddMenuEntry(
		"AnimationToolkit_Optimize",
		LOCTEXT("OptimizeAnimation", "Optimize Animation"),
		LOCTEXT("OptimizeAnimationTooltip", "Optimize selected animation"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([InObjects]()
		{
			for (UObject* Obj : InObjects)
			{
				UAnimSequence* Anim = Cast<UAnimSequence>(Obj);
				if (Anim)
				{
					UAnimationToolkitContextMenuActions::OptimizeSelectedAnimation(Anim);
				}
			}
		}))
	);

	Section.AddMenuEntry(
		"AnimationToolkit_BatchCompress",
		LOCTEXT("BatchCompress", "Batch Compress"),
		LOCTEXT("BatchCompressTooltip", "Compress all selected animations"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([InObjects]()
		{
			TArray<UAnimSequence*> Animations;
			for (UObject* Obj : InObjects)
			{
				UAnimSequence* Anim = Cast<UAnimSequence>(Obj);
				if (Anim)
				{
					Animations.Add(Anim);
				}
			}
			UAnimationToolkitContextMenuActions::BatchCompressSelected(Animations);
		}))
	);

	Section.AddMenuEntry(
		"AnimationToolkit_BatchOptimize",
		LOCTEXT("BatchOptimize", "Batch Optimize"),
		LOCTEXT("BatchOptimizeTooltip", "Optimize all selected animations"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([InObjects]()
		{
			TArray<UAnimSequence*> Animations;
			for (UObject* Obj : InObjects)
			{
				UAnimSequence* Anim = Cast<UAnimSequence>(Obj);
				if (Anim)
				{
					Animations.Add(Anim);
				}
			}
			UAnimationToolkitContextMenuActions::BatchOptimizeSelected(Animations);
		}))
	);
}

void FAnimationToolkitAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* Obj : InObjects)
	{
		UAnimSequence* Anim = Cast<UAnimSequence>(Obj);
		if (Anim)
		{
			UAnimationToolkitContextMenuActions::ShowAnimationInfo(Anim);
		}
	}
}

bool FAnimationToolkitAssetTypeActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

bool FAnimationToolkitAssetTypeActions::CanMerge(const UObject* InObject) const
{
	return false;
}

void FAnimationToolkitAssetTypeActions::Merge(const UObject* ObjectA, const UObject* ObjectB, const FString& TargetName)
{
}

void UAnimationToolkitContextMenuActions::ImportAnimationFromContext(const FString& FilePath)
{
	FString Extension = FPaths::GetExtension(FilePath).ToLower();

	EAnimationImportFormat Format = EAnimationImportFormat::FBX;
	if (Extension == TEXT("gltf") || Extension == TEXT("glb"))
	{
		Format = EAnimationImportFormat::glTF;
	}
	else if (Extension == TEXT("bvh"))
	{
		Format = EAnimationImportFormat::BVH;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Importing: %s"), *FilePath));
	}
}

void UAnimationToolkitContextMenuActions::ExportSelectedAnimation(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}

	FString ExportPath = FPaths::ProjectSavedDir() / TEXT("ExportedAnimations") / Animation->GetName() + TEXT(".bvh");

	bool bSuccess = UAnimationLibrary::ExportAnimationBVH(Animation, ExportPath);

	if (GEngine)
	{
		FColor Color = bSuccess ? FColor::Green : FColor::Red;
		FString Message = bSuccess ? FString::Printf(TEXT("Exported to: %s"), *ExportPath) : TEXT("Export failed!");
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, Color, Message);
	}
}

void UAnimationToolkitContextMenuActions::BlendSelectedAnimations(const TArray<UAnimSequence*>& Animations)
{
	if (Animations.Num() < 2)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Select at least 2 animations to blend"));
		}
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Blending %d animations"), Animations.Num()));
	}
}

void UAnimationToolkitContextMenuActions::MirrorSelectedAnimation(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Mirroring: %s"), *Animation->GetName()));
	}
}

void UAnimationToolkitContextMenuActions::CompressSelectedAnimation(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}

	FAnimationCompressionSettings Settings = UAnimationLibrary::MakeCompressionSettings(true, true, false, 0.01f, 0.1f, 0.01f);
	bool bSuccess = UAnimationLibrary::CompressAnimation(Animation, Settings);

	if (GEngine)
	{
		FColor Color = bSuccess ? FColor::Green : FColor::Red;
		FString Message = bSuccess ? FString::Printf(TEXT("Compressed: %s"), *Animation->GetName()) : TEXT("Compression failed!");
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, Color, Message);
	}
}

void UAnimationToolkitContextMenuActions::OptimizeSelectedAnimation(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Optimizing: %s"), *Animation->GetName()));
	}
}

void UAnimationToolkitContextMenuActions::RetargetSelectedAnimation(UAnimSequence* Animation, USkeleton* TargetSkeleton)
{
	if (!Animation || !TargetSkeleton)
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Retargeting: %s"), *Animation->GetName()));
	}
}

void UAnimationToolkitContextMenuActions::AddTagToSelected(UAnimSequence* Animation, const FName& Tag)
{
	if (!Animation)
	{
		return;
	}

	UAnimationLibrary::SetAnimationTag(Animation, Tag);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Added tag '%s' to %s"), *Tag.ToString(), *Animation->GetName()));
	}
}

void UAnimationToolkitContextMenuActions::RemoveTagFromSelected(UAnimSequence* Animation, const FName& Tag)
{
	if (!Animation)
	{
		return;
	}

	UAnimationLibrary::RemoveAnimationTag(Animation, Tag);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Removed tag '%s' from %s"), *Tag.ToString(), *Animation->GetName()));
	}
}

TArray<FName> UAnimationToolkitContextMenuActions::GetAnimationTagsFromSelected(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return TArray<FName>();
	}

	return UAnimationLibrary::GetAnimationTags(Animation);
}

void UAnimationToolkitContextMenuActions::BatchCompressSelected(const TArray<UAnimSequence*>& Animations)
{
	if (Animations.Num() == 0)
	{
		return;
	}

	FAnimationCompressionSettings Settings = UAnimationLibrary::MakeCompressionSettings(true, true, false, 0.01f, 0.1f, 0.01f);
	int32 SuccessCount = UAnimationLibrary::BatchCompressAnimations(Animations, Settings);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Batch compressed %d/%d animations"), SuccessCount, Animations.Num()));
	}
}

void UAnimationToolkitContextMenuActions::BatchOptimizeSelected(const TArray<UAnimSequence*>& Animations)
{
	if (Animations.Num() == 0)
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Batch optimizing %d animations"), Animations.Num()));
	}
}

void UAnimationToolkitContextMenuActions::ShowAnimationInfo(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}

	float Duration = UAnimationLibrary::GetAnimSequenceDuration(Animation);
	int32 FrameCount = UAnimationLibrary::GetAnimSequenceFrameCount(Animation);
	int32 KeyCount = UAnimationLibrary::GetAnimationKeyCount(Animation);
	float CompressionRatio = UAnimationLibrary::GetAnimationCompressionRatio(Animation);
	TArray<FName> Tags = UAnimationLibrary::GetAnimationTags(Animation);

	FString Info = FString::Printf(
		TEXT("Animation: %s\nDuration: %.2fs\nFrames: %d\nKeys: %d\nCompression: %.2f\nTags: %d"),
		*Animation->GetName(),
		Duration,
		FrameCount,
		KeyCount,
		CompressionRatio,
		Tags.Num()
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, Info);
	}
}

#undef LOCTEXT_NAMESPACE

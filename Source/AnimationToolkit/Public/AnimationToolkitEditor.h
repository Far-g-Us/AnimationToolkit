#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSlider.h"
#include "EditorStyleSet.h"
#include "IAssetTypeActions.h"
#include "AnimationData.h"
#include "AnimationToolkitEditor.generated.h"

class UAnimSequence;
class USkeleton;

UCLASS()
class ANIMATIONTOOLKIT_API UAnimationToolkitEditorModule : public IModuleInterface
{
	GENERATED_BODY()

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RegisterMenus();

private:
	void AddToolbarButton(FToolBarBuilder& Builder);
	void AddMenuBarButton(FMenuBarBuilder& MenuBuilder);

	void OnImportClicked();
	void OnExportClicked();
	void OnBlendClicked();
	void OnMirrorClicked();
	void OnCompressClicked();
	void OnOptimizeClicked();
	void OnRetargetClicked();
	void OnBatchProcessClicked();

	TSharedRef<SWidget> CreateAnimationToolkitMenu();
};

UCLASS()
class ANIMATIONTOOLKIT_API FAnimationToolkitEditorCommands : public TCommands<FAnimationToolkitEditorCommands>
{
public:
	FAnimationToolkitEditorCommands()
		: TCommands<FAnimationToolkitEditorCommands>(
			TEXT("AnimationToolkit"),
			NSLOCTEXT("Contexts", "AnimationToolkit", "Animation Toolkit"),
			NAME_None,
			FEditorStyle::GetAppStyleSetName()
		)
	{
	}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> ImportAnimation;
	TSharedPtr<FUICommandInfo> ExportAnimation;
	TSharedPtr<FUICommandInfo> BlendAnimations;
	TSharedPtr<FUICommandInfo> MirrorAnimation;
	TSharedPtr<FUICommandInfo> CompressAnimation;
	TSharedPtr<FUICommandInfo> OptimizeAnimation;
	TSharedPtr<FUICommandInfo> RetargetAnimation;
	TSharedPtr<FUICommandInfo> BatchProcess;
};

DECLARE_DELEGATE(FOnAnimationAction);

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationToolkitEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString DefaultImportPath = TEXT("/Game/Animations");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString DefaultExportPath = TEXT("C:/Export/Animations");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EAnimationImportFormat DefaultImportFormat = EAnimationImportFormat::FBX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoSelectSkeleton = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bShowNotifications = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DefaultBlendTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DefaultCompressionThreshold = 0.01f;
};

UCLASS()
class ANIMATIONTOOLKIT_API UAnimationToolkitEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Editor", meta = (WorldContext = "WorldContextObject"))
	static void OpenImportDialog(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Editor", meta = (WorldContext = "WorldContextObject"))
	static void OpenExportDialog(UObject* WorldContextObject, UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Editor")
	static UAnimationToolkitEditorSettings* GetEditorSettings();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Editor")
	static void SaveEditorSettings();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Editor")
	static TArray<UAnimSequence*> GetSelectedAnimations();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Editor")
	static USkeleton* GetSelectedSkeleton();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Editor")
	static void ShowNotification(const FString& Message, bool bSuccess = true);
};

DECLARE_MULTICAST_DELEGATE(FOnEditorAction);

class ANIMATIONTOOLKIT_API FAnimationToolkitEditor : public FAssetEditorToolkit
{
public:
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	void InitEditor(const TArray<UObject*>& InObjects);

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;

	void OnImport();
	void OnExport();
	void OnBlend();
	void OnMirror();
	void OnCompress();
	void OnOptimize();
	void OnRetarget();
	void OnBatchProcess();

private:
	TSharedRef<SDockTab> SpawnTab_Import(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Export(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Blend(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Mirror(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Compress(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Optimize(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Retarget(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Batch(const FSpawnTabArgs& Args);

	TSharedRef<SDockTab> SpawnTab_Settings(const FSpawnTabArgs& Args);

	TSharedPtr<SEditableTextBox> ImportPathBox;
	TSharedPtr<SEditableTextBox> ExportPathBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> FormatComboBox;
	TSharedPtr<SCheckBox> AutoSelectCheckBox;
	TSharedPtr<SSlider> BlendTimeSlider;
	TSharedPtr<SSlider> CompressionThresholdSlider;

	TArray<TSharedPtr<FString>> FormatOptions;

	FOnEditorAction OnImportAction;
	FOnEditorAction OnExportAction;
	FOnEditorAction OnBlendAction;
	FOnEditorAction OnMirrorAction;
	FOnEditorAction OnCompressAction;
	FOnEditorAction OnOptimizeAction;
	FOnEditorAction OnRetargetAction;
	FOnEditorAction OnBatchAction;
};

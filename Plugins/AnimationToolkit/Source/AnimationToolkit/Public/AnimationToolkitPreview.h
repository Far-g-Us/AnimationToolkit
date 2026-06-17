#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/SListView.h"
#include "AnimationData.h"
#include "AnimationToolkitPreview.generated.h"

class UAnimSequence;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimationProgress, float, Progress);

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationPreviewManager : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	static UAnimationPreviewManager* GetPreviewManager();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void SetPreviewAnimation(UAnimSequence* Animation);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	UAnimSequence* GetPreviewAnimation() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void Play();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void Pause();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void SetPlaybackPosition(float Position);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	float GetPlaybackPosition() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	float GetAnimationDuration() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	bool IsPlaying() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void SetPlayRate(float Rate);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	float GetPlayRate() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void SetLoop(bool bNewLoop);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	bool IsLooping() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void SetShowBoneNames(bool bShow);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	bool GetShowBoneNames() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void SetShowTrail(bool bShow);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	bool GetShowTrail() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Preview")
	void SetTrailLength(float Length);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Preview")
	float GetTrailLength() const;

	UPROPERTY(BlueprintAssignable, Category = "Animation Toolkit|Preview")
	FOnAnimationProgress OnPlaybackUpdated;

private:
	UPROPERTY()
	UAnimSequence* CurrentAnimation = nullptr;

	float PlaybackPosition = 0.0f;
	float PlayRate = 1.0f;
	bool bPlaying = false;
	bool bLoop = true;
	bool bShowBoneNames = false;
	bool bShowTrail = false;
	float TrailLength = 0.5f;

	static TWeakObjectPtr<UAnimationPreviewManager> Instance;
};

class ANIMATIONTOOLKIT_API SAnimationPreviewWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAnimationPreviewWidget) {}
		SLATE_ARGUMENT(UAnimSequence*, Animation)
		SLATE_ARGUMENT(USkeletalMeshComponent*, PreviewMesh)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TSharedRef<SWidget> CreatePreviewControls();
	TSharedRef<SWidget> CreateBoneList();

	void OnPlayClicked();
	void OnPauseClicked();
	void OnStopClicked();
	void OnSliderChanged(float Value);
	void OnPlayRateChanged(float Value);
	void OnLoopChanged(ECheckBoxState NewState);
	void OnShowBonesChanged(ECheckBoxState NewState);
	void OnShowTrailChanged(ECheckBoxState NewState);
	void OnTrailLengthChanged(float Value);

	UAnimSequence* PreviewAnimation = nullptr;
	USkeletalMeshComponent* PreviewMeshComponent = nullptr;

	TSharedPtr<SSlider> PlaybackSlider;
	TSharedPtr<SSlider> PlayRateSlider;
	TSharedPtr<SSlider> TrailLengthSlider;
	TSharedPtr<STextBlock> TimeTextBlock;
	TSharedPtr<STextBlock> DurationTextBlock;
	TSharedPtr<SListView<FName>> BoneListView;
	TArray<FName> BoneNames;

	float CurrentPlaybackPosition = 0.0f;
	float CurrentPlayRate = 1.0f;
	bool bIsPlaying = false;
	bool bLoop = true;
	bool bShowBoneNames = false;
	bool bShowTrail = false;
	float TrailLengthValue = 0.5f;
};

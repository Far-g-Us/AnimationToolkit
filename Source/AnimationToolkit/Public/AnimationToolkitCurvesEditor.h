#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/SListView.h"
#include "AnimationData.h"
#include "AnimationToolkitCurvesEditor.generated.h"

class UAnimSequence;

USTRUCT(BlueprintType)
struct FAnimationCurveEditorKeyframe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Curves Editor")
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Curves Editor")
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Curves Editor")
	FVector2D Tangent = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Curves Editor")
	bool bSelected = false;
};

USTRUCT(BlueprintType)
struct FAnimationCurveEditorCurve
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Curves Editor")
	FName CurveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Curves Editor")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Curves Editor")
	bool bVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit|Curves Editor")
	TArray<FAnimationCurveEditorKeyframe> Keyframes;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationCurveEditorManager : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	static UAnimationCurveEditorManager* GetCurvesEditorManager();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void LoadAnimationCurves(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void SaveAnimationCurves(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void AddCurve(const FName& CurveName, const FLinearColor& Color);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void RemoveCurve(const FName& CurveName);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void AddKeyframe(const FName& CurveName, float Time, float Value);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void RemoveKeyframe(const FName& CurveName, int32 KeyframeIndex);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void UpdateKeyframe(const FName& CurveName, int32 KeyframeIndex, float Time, float Value);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curves Editor")
	TArray<FAnimationCurveEditorCurve> GetCurves() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curves Editor")
	FAnimationCurveEditorCurve GetCurve(const FName& CurveName) const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curves Editor")
	int32 GetKeyframeCount(const FName& CurveName) const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void SelectKeyframe(const FName& CurveName, int32 KeyframeIndex);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void DeselectAll();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curves Editor")
	TArray<FAnimationCurveEditorKeyframe> GetSelectedKeyframes() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void MoveSelectedKeyframes(float DeltaTime, float DeltaValue);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void DeleteSelectedKeyframes();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void SetCurveVisible(const FName& CurveName, bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curves Editor")
	bool IsCurveVisible(const FName& CurveName) const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void SetCurveColor(const FName& CurveName, const FLinearColor& Color);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curves Editor")
	FLinearColor GetCurveColor(const FName& CurveName) const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void ZoomIn();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void ZoomOut();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void ResetZoom();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curves Editor")
	float GetZoomLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Curves Editor")
	void PanView(float DeltaX, float DeltaY);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Curves Editor")
	FVector2D GetViewOffset() const;

private:
	UPROPERTY()
	TArray<FAnimationCurveEditorCurve> Curves;

	float ZoomLevel = 1.0f;
	FVector2D ViewOffset = FVector2D::ZeroVector;

	static TWeakObjectPtr<UAnimationCurveEditorManager> Instance;
};

UCLASS()
class ANIMATIONTOOLKIT_API SCurvesEditorWidget : public SCompoundWidget
{
	GENERATED_BODY()

public:
	SLATE_BEGIN_ARGS(SCurvesEditorWidget) {}
		SLATE_ARGUMENT(UAnimSequence*, Animation)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	TSharedRef<SWidget> CreateCurvesList();
	TSharedRef<SWidget> CreateToolbar();

	void OnAddCurveClicked();
	void OnRemoveCurveClicked();
	void OnLoadClicked();
	void OnSaveClicked();
	void OnZoomInClicked();
	void OnZoomOutClicked();
	void OnResetZoomClicked();
	void OnCurveVisibilityChanged(const FName& CurveName, ECheckBoxState NewState);
	void OnAddKeyframeClicked();

	void DrawCurve(const FAnimationCurveEditorCurve& Curve, const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const;
	void DrawGrid(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const;
	void DrawKeyframes(const FAnimationCurveEditorCurve& Curve, const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const;

	FVector2D TimeToScreen(float Time, const FGeometry& Geometry) const;
	float ScreenToTime(const FVector2D& ScreenPos, const FGeometry& Geometry) const;
	FVector2D ValueToScreen(float Value, const FGeometry& Geometry) const;
	float ScreenToValue(const FVector2D& ScreenPos, const FGeometry& Geometry) const;

	UAnimSequence* EditorAnimation = nullptr;

	TSharedPtr<SListView<FName>> CurvesListView;
	TArray<FName> CurveNames;

	FVector2D MousePosition = FVector2D::ZeroVector;
	bool bIsDragging = false;
	FVector2D DragStart = FVector2D::ZeroVector;

	float ViewMinTime = 0.0f;
	float ViewMaxTime = 5.0f;
	float ViewMinValue = -1.0f;
	float ViewMaxValue = 1.0f;
};

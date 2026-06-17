#include "AnimationToolkitCurvesEditor.h"
#include "AnimationLibrary.h"
#include "Animation/AnimSequence.h"
#include "Styling/AppStyle.h"
#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "AnimationToolkitCurvesEditor"

TWeakObjectPtr<UAnimationCurveEditorManager> UAnimationCurveEditorManager::Instance = nullptr;

UAnimationCurveEditorManager* UAnimationCurveEditorManager::GetCurvesEditorManager()
{
	return nullptr;
}

void UAnimationCurveEditorManager::LoadAnimationCurves(UAnimSequence* Animation)
{
}

void UAnimationCurveEditorManager::SaveAnimationCurves(UAnimSequence* Animation)
{
}

void UAnimationCurveEditorManager::AddCurve(const FName& CurveName, const FLinearColor& Color)
{
}

void UAnimationCurveEditorManager::RemoveCurve(const FName& CurveName)
{
}

void UAnimationCurveEditorManager::AddKeyframe(const FName& CurveName, float Time, float Value)
{
}

void UAnimationCurveEditorManager::RemoveKeyframe(const FName& CurveName, int32 KeyframeIndex)
{
}

void UAnimationCurveEditorManager::UpdateKeyframe(const FName& CurveName, int32 KeyframeIndex, float Time, float Value)
{
}

TArray<FAnimationCurveEditorCurve> UAnimationCurveEditorManager::GetCurves() const
{
	return TArray<FAnimationCurveEditorCurve>();
}

FAnimationCurveEditorCurve UAnimationCurveEditorManager::GetCurve(const FName& CurveName) const
{
	return FAnimationCurveEditorCurve();
}

int32 UAnimationCurveEditorManager::GetKeyframeCount(const FName& CurveName) const
{
	return 0;
}

void UAnimationCurveEditorManager::SelectKeyframe(const FName& CurveName, int32 KeyframeIndex)
{
}

void UAnimationCurveEditorManager::DeselectAll()
{
}

TArray<FAnimationCurveEditorKeyframe> UAnimationCurveEditorManager::GetSelectedKeyframes() const
{
	return TArray<FAnimationCurveEditorKeyframe>();
}

void UAnimationCurveEditorManager::MoveSelectedKeyframes(float DeltaTime, float DeltaValue)
{
}

void UAnimationCurveEditorManager::DeleteSelectedKeyframes()
{
}

void UAnimationCurveEditorManager::SetCurveVisible(const FName& CurveName, bool bVisible)
{
}

bool UAnimationCurveEditorManager::IsCurveVisible(const FName& CurveName) const
{
	return false;
}

void UAnimationCurveEditorManager::SetCurveColor(const FName& CurveName, const FLinearColor& Color)
{
}

FLinearColor UAnimationCurveEditorManager::GetCurveColor(const FName& CurveName) const
{
	return FLinearColor::White;
}

void UAnimationCurveEditorManager::ZoomIn()
{
}

void UAnimationCurveEditorManager::ZoomOut()
{
}

void UAnimationCurveEditorManager::ResetZoom()
{
}

float UAnimationCurveEditorManager::GetZoomLevel() const
{
	return 0.0f;
}

void UAnimationCurveEditorManager::PanView(float DeltaX, float DeltaY)
{
}

FVector2D UAnimationCurveEditorManager::GetViewOffset() const
{
	return FVector2D::ZeroVector;
}

void SCurvesEditorWidget::Construct(const FArguments& InArgs)
{
}

void SCurvesEditorWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

int32 SCurvesEditorWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	return LayerId;
}

FReply SCurvesEditorWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Unhandled();
}

FReply SCurvesEditorWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Unhandled();
}

FReply SCurvesEditorWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Unhandled();
}

FReply SCurvesEditorWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Unhandled();
}

TSharedRef<SWidget> SCurvesEditorWidget::CreateCurvesList()
{
	return SNew(SVerticalBox);
}

TSharedRef<SWidget> SCurvesEditorWidget::CreateToolbar()
{
	return SNew(SHorizontalBox);
}

void SCurvesEditorWidget::OnAddCurveClicked()
{
}

void SCurvesEditorWidget::OnRemoveCurveClicked()
{
}

void SCurvesEditorWidget::OnLoadClicked()
{
}

void SCurvesEditorWidget::OnSaveClicked()
{
}

void SCurvesEditorWidget::OnZoomInClicked()
{
}

void SCurvesEditorWidget::OnZoomOutClicked()
{
}

void SCurvesEditorWidget::OnResetZoomClicked()
{
}

void SCurvesEditorWidget::OnCurveVisibilityChanged(const FName& CurveName, ECheckBoxState NewState)
{
}

void SCurvesEditorWidget::OnAddKeyframeClicked()
{
}

void SCurvesEditorWidget::DrawCurve(const FAnimationCurveEditorCurve& Curve, const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const
{
}

void SCurvesEditorWidget::DrawGrid(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const
{
}

void SCurvesEditorWidget::DrawKeyframes(const FAnimationCurveEditorCurve& Curve, const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const
{
}

FVector2D SCurvesEditorWidget::TimeToScreen(float Time, const FGeometry& Geometry) const
{
	return FVector2D::ZeroVector;
}

float SCurvesEditorWidget::ScreenToTime(const FVector2D& ScreenPos, const FGeometry& Geometry) const
{
	return 0.0f;
}

FVector2D SCurvesEditorWidget::ValueToScreen(float Value, const FGeometry& Geometry) const
{
	return FVector2D::ZeroVector;
}

float SCurvesEditorWidget::ScreenToValue(const FVector2D& ScreenPos, const FGeometry& Geometry) const
{
	return 0.0f;
}

#undef LOCTEXT_NAMESPACE

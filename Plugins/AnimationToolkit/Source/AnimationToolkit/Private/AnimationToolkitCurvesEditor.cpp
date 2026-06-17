#include "AnimationToolkitCurvesEditor.h"
#include "AnimationLibrary.h"
#include "Animation/AnimSequence.h"
#include "Styling/AppStyle.h"
#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"

#define LOCTEXT_NAMESPACE "AnimationToolkitCurvesEditor"

TWeakObjectPtr<UAnimationCurveEditorManager> UAnimationCurveEditorManager::Instance = nullptr;

UAnimationCurveEditorManager* UAnimationCurveEditorManager::GetCurvesEditorManager()
{
	if (!Instance.IsValid())
	{
		Instance = NewObject<UAnimationCurveEditorManager>();
		Instance->AddToRoot();
	}
	return Instance.Get();
}

void UAnimationCurveEditorManager::LoadAnimationCurves(UAnimSequence* Animation)
{
	Curves.Empty();

	if (!Animation)
	{
		return;
	}

	const TArray<FSmartName>& CompressedCurveNames = Animation->GetCompressedCurveNames();
	const float Duration = Animation->GetPlayLength();
	const int32 NumCurves = CompressedCurveNames.Num();
	for (int32 i = 0; i < NumCurves; ++i)
	{
		FAnimationCurveEditorCurve Curve;
		Curve.CurveName = CompressedCurveNames[i].DisplayName;
		Curve.Color = FLinearColor::White;
		Curve.bVisible = true;

		const int32 NumSamples = FMath::Max(2, FMath::CeilToInt(Duration * 30.0f));
		for (int32 s = 0; s < NumSamples; ++s)
		{
			const float Time = (NumSamples > 1)
				? Duration * static_cast<float>(s) / static_cast<float>(NumSamples - 1)
				: 0.0f;
			float Value = 0.0f;
			Value = Animation->EvaluateCurveData(CompressedCurveNames[i].DisplayName, Time);

			FAnimationCurveEditorKeyframe KF;
			KF.Time = Time;
			KF.Value = Value;
			KF.Tangent = FVector2D::ZeroVector;
			KF.bSelected = false;
			Curve.Keyframes.Add(KF);
		}

		Curves.Add(Curve);
	}
}

void UAnimationCurveEditorManager::SaveAnimationCurves(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("SaveAnimationCurves: FindSmartName/SetCurveKeyFrame are not available in UE5.5. Save skipped."));
}

void UAnimationCurveEditorManager::AddCurve(const FName& CurveName, const FLinearColor& Color)
{
	for (const FAnimationCurveEditorCurve& Existing : Curves)
	{
		if (Existing.CurveName == CurveName)
		{
			return;
		}
	}

	FAnimationCurveEditorCurve NewCurve;
	NewCurve.CurveName = CurveName;
	NewCurve.Color = Color;
	NewCurve.bVisible = true;
	Curves.Add(NewCurve);
}

void UAnimationCurveEditorManager::RemoveCurve(const FName& CurveName)
{
	Curves.RemoveAll([&CurveName](const FAnimationCurveEditorCurve& C)
	{
		return C.CurveName == CurveName;
	});
}

void UAnimationCurveEditorManager::AddKeyframe(const FName& CurveName, float Time, float Value)
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			FAnimationCurveEditorKeyframe NewKF;
			NewKF.Time = Time;
			NewKF.Value = Value;
			NewKF.Tangent = FVector2D::ZeroVector;
			NewKF.bSelected = false;

			int32 InsertIndex = Curve.Keyframes.Num();
			for (int32 i = 0; i < Curve.Keyframes.Num(); ++i)
			{
				if (Curve.Keyframes[i].Time > Time)
				{
					InsertIndex = i;
					break;
				}
			}
			Curve.Keyframes.Insert(NewKF, InsertIndex);
			return;
		}
	}
}

void UAnimationCurveEditorManager::RemoveKeyframe(const FName& CurveName, int32 KeyframeIndex)
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			if (Curve.Keyframes.IsValidIndex(KeyframeIndex))
			{
				Curve.Keyframes.RemoveAt(KeyframeIndex);
			}
			return;
		}
	}
}

void UAnimationCurveEditorManager::UpdateKeyframe(const FName& CurveName, int32 KeyframeIndex, float Time, float Value)
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			if (Curve.Keyframes.IsValidIndex(KeyframeIndex))
			{
				Curve.Keyframes[KeyframeIndex].Time = Time;
				Curve.Keyframes[KeyframeIndex].Value = Value;
			}
			return;
		}
	}
}

TArray<FAnimationCurveEditorCurve> UAnimationCurveEditorManager::GetCurves() const
{
	return Curves;
}

FAnimationCurveEditorCurve UAnimationCurveEditorManager::GetCurve(const FName& CurveName) const
{
	for (const FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			return Curve;
		}
	}
	return FAnimationCurveEditorCurve();
}

int32 UAnimationCurveEditorManager::GetKeyframeCount(const FName& CurveName) const
{
	for (const FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			return Curve.Keyframes.Num();
		}
	}
	return 0;
}

void UAnimationCurveEditorManager::SelectKeyframe(const FName& CurveName, int32 KeyframeIndex)
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			if (Curve.Keyframes.IsValidIndex(KeyframeIndex))
			{
				Curve.Keyframes[KeyframeIndex].bSelected = true;
			}
			return;
		}
	}
}

void UAnimationCurveEditorManager::DeselectAll()
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		for (FAnimationCurveEditorKeyframe& KF : Curve.Keyframes)
		{
			KF.bSelected = false;
		}
	}
}

TArray<FAnimationCurveEditorKeyframe> UAnimationCurveEditorManager::GetSelectedKeyframes() const
{
	TArray<FAnimationCurveEditorKeyframe> Selected;
	for (const FAnimationCurveEditorCurve& Curve : Curves)
	{
		for (const FAnimationCurveEditorKeyframe& KF : Curve.Keyframes)
		{
			if (KF.bSelected)
			{
				Selected.Add(KF);
			}
		}
	}
	return Selected;
}

void UAnimationCurveEditorManager::MoveSelectedKeyframes(float DeltaTime, float DeltaValue)
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		for (FAnimationCurveEditorKeyframe& KF : Curve.Keyframes)
		{
			if (KF.bSelected)
			{
				KF.Time += DeltaTime;
				KF.Value += DeltaValue;
			}
		}
	}
}

void UAnimationCurveEditorManager::DeleteSelectedKeyframes()
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		Curve.Keyframes.RemoveAll([](const FAnimationCurveEditorKeyframe& KF)
		{
			return KF.bSelected;
		});
	}
}

void UAnimationCurveEditorManager::SetCurveVisible(const FName& CurveName, bool bVisible)
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			Curve.bVisible = bVisible;
			return;
		}
	}
}

bool UAnimationCurveEditorManager::IsCurveVisible(const FName& CurveName) const
{
	for (const FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			return Curve.bVisible;
		}
	}
	return false;
}

void UAnimationCurveEditorManager::SetCurveColor(const FName& CurveName, const FLinearColor& Color)
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			Curve.Color = Color;
			return;
		}
	}
}

FLinearColor UAnimationCurveEditorManager::GetCurveColor(const FName& CurveName) const
{
	for (const FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			return Curve.Color;
		}
	}
	return FLinearColor::White;
}

void UAnimationCurveEditorManager::ZoomIn()
{
	ZoomLevel = FMath::Min(ZoomLevel * 1.25f, 10.0f);
}

void UAnimationCurveEditorManager::ZoomOut()
{
	ZoomLevel = FMath::Max(ZoomLevel / 1.25f, 0.1f);
}

void UAnimationCurveEditorManager::ResetZoom()
{
	ZoomLevel = 1.0f;
}

float UAnimationCurveEditorManager::GetZoomLevel() const
{
	return ZoomLevel;
}

void UAnimationCurveEditorManager::PanView(float DeltaX, float DeltaY)
{
	ViewOffset.X += DeltaX;
	ViewOffset.Y += DeltaY;
}

FVector2D UAnimationCurveEditorManager::GetViewOffset() const
{
	return ViewOffset;
}

// ─── SCurvesEditorWidget ───────────────────────────────────────

void SCurvesEditorWidget::Construct(const FArguments& InArgs)
{
	EditorAnimation = InArgs._Animation;

	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager && EditorAnimation)
	{
		Manager->LoadAnimationCurves(EditorAnimation);
		TArray<FAnimationCurveEditorCurve> LoadedCurves = Manager->GetCurves();
		CurveNames.Empty();
		for (const FAnimationCurveEditorCurve& C : LoadedCurves)
		{
			CurveNames.Add(C.CurveName);
		}
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			CreateToolbar()
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.25f)
			.Padding(4.0f)
			[
				CreateCurvesList()
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.75f)
			.Padding(4.0f)
			[
				SNew(SBox)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				]
			]
		]
	];
}

void SCurvesEditorWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

int32 SCurvesEditorWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	DrawGrid(AllottedGeometry, OutDrawElements, LayerId);

	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager)
	{
		const TArray<FAnimationCurveEditorCurve> Curves = Manager->GetCurves();
		for (const FAnimationCurveEditorCurve& Curve : Curves)
		{
			if (Curve.bVisible)
			{
				DrawCurve(Curve, AllottedGeometry, OutDrawElements, LayerId);
				DrawKeyframes(Curve, AllottedGeometry, OutDrawElements, LayerId);
			}
		}
	}

	return LayerId;
}

FReply SCurvesEditorWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		DragStart = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SCurvesEditorWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bIsDragging)
		{
			bIsDragging = false;
		}
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SCurvesEditorWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bIsDragging)
	{
		const FVector2D CurrentPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		const float DeltaX = (CurrentPos.X - DragStart.X) * 0.01f;
		const float DeltaY = (CurrentPos.Y - DragStart.Y) * 0.01f;

		UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
		if (Manager)
		{
			Manager->PanView(DeltaX, DeltaY);
		}
		DragStart = CurrentPos;
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SCurvesEditorWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager)
	{
		if (MouseEvent.GetWheelDelta() > 0)
		{
			Manager->ZoomIn();
		}
		else
		{
			Manager->ZoomOut();
		}
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

TSharedRef<SWidget> SCurvesEditorWidget::CreateCurvesList()
{
	CurvesListView = SNew(SListView<FName>)
		.ListItemsSource(&CurveNames)
		.OnGenerateRow_Lambda([](FName Item, const TSharedRef<STableViewBase>& OwnerTable)
		{
			return SNew(STableRow<FName>, OwnerTable)
				[
					SNew(STextBlock)
					.Text(FText::FromName(Item))
				];
		})
		.OnSelectionChanged_Lambda([this](FName NewSelection, ESelectInfo::Type SelectInfo)
		{
			if (NewSelection != NAME_None)
			{
				UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
				if (Manager)
				{
					Manager->DeselectAll();
				}
			}
		});

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CurvesListLabel", "Curves"))
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			CurvesListView.ToSharedRef()
		];
}

TSharedRef<SWidget> SCurvesEditorWidget::CreateToolbar()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("AddCurve", "+"))
			.OnClicked_Lambda([this]() -> FReply { OnAddCurveClicked(); return FReply::Handled(); })
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("RemoveCurve", "-"))
			.OnClicked_Lambda([this]() -> FReply { OnRemoveCurveClicked(); return FReply::Handled(); })
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ZoomIn", "Zoom+"))
			.OnClicked_Lambda([this]() -> FReply { OnZoomInClicked(); return FReply::Handled(); })
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ZoomOut", "Zoom-"))
			.OnClicked_Lambda([this]() -> FReply { OnZoomOutClicked(); return FReply::Handled(); })
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("ResetZoom", "Reset"))
			.OnClicked_Lambda([this]() -> FReply { OnResetZoomClicked(); return FReply::Handled(); })
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("AddKeyframe", "Key"))
			.OnClicked_Lambda([this]() -> FReply { OnAddKeyframeClicked(); return FReply::Handled(); })
		];
}

void SCurvesEditorWidget::OnAddCurveClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager)
	{
		const int32 CurveIndex = Manager->GetCurves().Num();
		const FName NewCurveName = *FString::Printf(TEXT("Curve_%d"), CurveIndex);
		Manager->AddCurve(NewCurveName, FLinearColor(0.2f, 0.8f, 1.0f));
		CurveNames.Add(NewCurveName);
		if (CurvesListView.IsValid())
		{
			CurvesListView->RequestListRefresh();
		}
	}
}

void SCurvesEditorWidget::OnRemoveCurveClicked()
{
	if (CurveNames.Num() > 0)
	{
		UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
		if (Manager)
		{
			Manager->RemoveCurve(CurveNames.Last());
			CurveNames.RemoveAt(CurveNames.Num() - 1);
			if (CurvesListView.IsValid())
			{
				CurvesListView->RequestListRefresh();
			}
		}
	}
}

void SCurvesEditorWidget::OnLoadClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager && EditorAnimation)
	{
		Manager->LoadAnimationCurves(EditorAnimation);
		TArray<FAnimationCurveEditorCurve> LoadedCurves = Manager->GetCurves();
		CurveNames.Empty();
		for (const FAnimationCurveEditorCurve& C : LoadedCurves)
		{
			CurveNames.Add(C.CurveName);
		}
		if (CurvesListView.IsValid())
		{
			CurvesListView->RequestListRefresh();
		}
	}
}

void SCurvesEditorWidget::OnSaveClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager && EditorAnimation)
	{
		Manager->SaveAnimationCurves(EditorAnimation);
	}
}

void SCurvesEditorWidget::OnZoomInClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager)
	{
		Manager->ZoomIn();
	}
}

void SCurvesEditorWidget::OnZoomOutClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager)
	{
		Manager->ZoomOut();
	}
}

void SCurvesEditorWidget::OnResetZoomClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager)
	{
		Manager->ResetZoom();
		Manager->PanView(-Manager->GetViewOffset().X, -Manager->GetViewOffset().Y);
	}
}

void SCurvesEditorWidget::OnCurveVisibilityChanged(const FName& CurveName, ECheckBoxState NewState)
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager)
	{
		Manager->SetCurveVisible(CurveName, NewState == ECheckBoxState::Checked);
	}
}

void SCurvesEditorWidget::OnAddKeyframeClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (Manager && CurveNames.Num() > 0)
	{
		const FName TargetCurve = CurveNames[0];
		const float Time = (ViewMinTime + ViewMaxTime) * 0.5f;
		const float Value = (ViewMinValue + ViewMaxValue) * 0.5f;
		Manager->AddKeyframe(TargetCurve, Time, Value);
	}
}

void SCurvesEditorWidget::DrawCurve(const FAnimationCurveEditorCurve& Curve, const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const
{
	if (Curve.Keyframes.Num() < 2)
	{
		return;
	}

	const FVector2D LocalSize = Geometry.GetLocalSize();
	const float XRange = ViewMaxTime - ViewMinTime;
	const float YRange = ViewMaxValue - ViewMinValue;

	if (XRange <= 0.0f || YRange <= 0.0f)
	{
		return;
	}

	TArray<FVector2D> LinePoints;
	for (const FAnimationCurveEditorKeyframe& KF : Curve.Keyframes)
	{
		const float NormalizedX = (KF.Time - ViewMinTime) / XRange;
		const float NormalizedY = (KF.Value - ViewMinValue) / YRange;
		LinePoints.Add(FVector2D(
			NormalizedX * LocalSize.X,
			(1.0f - NormalizedY) * LocalSize.Y
		));
	}

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId++,
		Geometry.ToPaintGeometry(),
		LinePoints,
		ESlateDrawEffect::None,
		Curve.Color,
		false,
		2.0f
	);
}

void SCurvesEditorWidget::DrawGrid(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const
{
	const FVector2D LocalSize = Geometry.GetLocalSize();

	TArray<FVector2D> HorizontalLines;
	const int32 NumHLines = 5;
	for (int32 i = 0; i <= NumHLines; ++i)
	{
		const float Y = LocalSize.Y * i / NumHLines;
		HorizontalLines.Add(FVector2D(0.0f, Y));
		HorizontalLines.Add(FVector2D(LocalSize.X, Y));
	}

	if (HorizontalLines.Num() > 0)
	{
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId++,
			Geometry.ToPaintGeometry(),
			HorizontalLines,
			ESlateDrawEffect::None,
			FLinearColor(0.3f, 0.3f, 0.3f, 0.5f),
			false,
			1.0f
		);
	}

	TArray<FVector2D> VerticalLines;
	const int32 NumVLines = 10;
	for (int32 i = 0; i <= NumVLines; ++i)
	{
		const float X = LocalSize.X * i / NumVLines;
		VerticalLines.Add(FVector2D(X, 0.0f));
		VerticalLines.Add(FVector2D(X, LocalSize.Y));
	}

	if (VerticalLines.Num() > 0)
	{
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId++,
			Geometry.ToPaintGeometry(),
			VerticalLines,
			ESlateDrawEffect::None,
			FLinearColor(0.3f, 0.3f, 0.3f, 0.5f),
			false,
			1.0f
		);
	}
}

void SCurvesEditorWidget::DrawKeyframes(const FAnimationCurveEditorCurve& Curve, const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const
{
	const FVector2D LocalSize = Geometry.GetLocalSize();
	const float XRange = ViewMaxTime - ViewMinTime;
	const float YRange = ViewMaxValue - ViewMinValue;

	if (XRange <= 0.0f || YRange <= 0.0f)
	{
		return;
	}

	for (const FAnimationCurveEditorKeyframe& KF : Curve.Keyframes)
	{
		const float NormalizedX = (KF.Time - ViewMinTime) / XRange;
		const float NormalizedY = (KF.Value - ViewMinValue) / YRange;

		const FVector2D ScreenPos(
			NormalizedX * LocalSize.X,
			(1.0f - NormalizedY) * LocalSize.Y
		);

		const float Radius = KF.bSelected ? 6.0f : 4.0f;
		const FLinearColor Color = KF.bSelected ? FLinearColor::Yellow : Curve.Color;

		TArray<FVector2D> Diamond;
		Diamond.Add(ScreenPos + FVector2D(0.0f, -Radius));
		Diamond.Add(ScreenPos + FVector2D(Radius, 0.0f));
		Diamond.Add(ScreenPos + FVector2D(0.0f, Radius));
		Diamond.Add(ScreenPos + FVector2D(-Radius, 0.0f));
		Diamond.Add(ScreenPos + FVector2D(0.0f, -Radius));

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId++,
			Geometry.ToPaintGeometry(),
			Diamond,
			ESlateDrawEffect::None,
			Color,
			true,
			2.0f
		);
	}
}

FVector2D SCurvesEditorWidget::TimeToScreen(float Time, const FGeometry& Geometry) const
{
	const FVector2D LocalSize = Geometry.GetLocalSize();
	const float XRange = ViewMaxTime - ViewMinTime;
	const float NormalizedX = (XRange > 0.0f) ? (Time - ViewMinTime) / XRange : 0.0f;
	return FVector2D(NormalizedX * LocalSize.X, LocalSize.Y * 0.5f);
}

float SCurvesEditorWidget::ScreenToTime(const FVector2D& ScreenPos, const FGeometry& Geometry) const
{
	const FVector2D LocalSize = Geometry.GetLocalSize();
	const float XRange = ViewMaxTime - ViewMinTime;
	return ViewMinTime + (ScreenPos.X / LocalSize.X) * XRange;
}

FVector2D SCurvesEditorWidget::ValueToScreen(float Value, const FGeometry& Geometry) const
{
	const FVector2D LocalSize = Geometry.GetLocalSize();
	const float YRange = ViewMaxValue - ViewMinValue;
	const float NormalizedY = (YRange > 0.0f) ? (Value - ViewMinValue) / YRange : 0.0f;
	return FVector2D(LocalSize.X * 0.5f, (1.0f - NormalizedY) * LocalSize.Y);
}

float SCurvesEditorWidget::ScreenToValue(const FVector2D& ScreenPos, const FGeometry& Geometry) const
{
	const FVector2D LocalSize = Geometry.GetLocalSize();
	const float YRange = ViewMaxValue - ViewMinValue;
	const float NormalizedY = 1.0f - (ScreenPos.Y / LocalSize.Y);
	return ViewMinValue + NormalizedY * YRange;
}

#undef LOCTEXT_NAMESPACE

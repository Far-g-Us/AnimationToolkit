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
	if (!Instance.IsValid() || Instance.Get() == nullptr)
	{
		Instance = NewObject<UAnimationCurveEditorManager>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
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

	FAnimationCurveData CurveData;
	TArray<FAnimationCurveKeyframe> Keyframes;

	for (int32 i = 0; i < 10; i++)
	{
		FAnimationCurveEditorCurve Curve;
		Curve.CurveName = *FString::Printf(TEXT("Curve_%d"), i);
		Curve.Color = FLinearColor::MakeRandomColor();
		Curve.bVisible = true;

		for (int32 j = 0; j <= 20; j++)
		{
			FAnimationCurveEditorKeyframe Keyframe;
			Keyframe.Time = (float)j / 4.0f;
			Keyframe.Value = FMath::Sin(Keyframe.Time * 3.14159f + i) * 0.5f;
			Curve.Keyframes.Add(Keyframe);
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

	for (const FAnimationCurveEditorCurve& Curve : Curves)
	{
		FAnimationCurveData CurveData;
		CurveData.CurveName = Curve.CurveName;

		for (const FAnimationCurveEditorKeyframe& Key : Curve.Keyframes)
		{
			FAnimationCurveKeyframe Keyframe;
			Keyframe.Time = Key.Time;
			Keyframe.Value = Key.Value;
			CurveData.Keyframes.Add(Keyframe);
		}

		UAnimationLibrary::AddCurveToAnimSequence(Animation, CurveData);
	}
}

void UAnimationCurveEditorManager::AddCurve(const FName& CurveName, const FLinearColor& Color)
{
	FAnimationCurveEditorCurve NewCurve;
	NewCurve.CurveName = CurveName;
	NewCurve.Color = Color;
	NewCurve.bVisible = true;
	Curves.Add(NewCurve);
}

void UAnimationCurveEditorManager::RemoveCurve(const FName& CurveName)
{
	Curves.RemoveAll([CurveName](const FAnimationCurveEditorCurve& Curve)
	{
		return Curve.CurveName == CurveName;
	});
}

void UAnimationCurveEditorManager::AddKeyframe(const FName& CurveName, float Time, float Value)
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		if (Curve.CurveName == CurveName)
		{
			FAnimationCurveEditorKeyframe NewKey;
			NewKey.Time = Time;
			NewKey.Value = Value;
			Curve.Keyframes.Add(NewKey);

			Curve.Keyframes.Sort([](const FAnimationCurveEditorKeyframe& A, const FAnimationCurveEditorKeyframe& B)
			{
				return A.Time < B.Time;
			});
			break;
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
			break;
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
			break;
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
			break;
		}
	}
}

void UAnimationCurveEditorManager::DeselectAll()
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		for (FAnimationCurveEditorKeyframe& Key : Curve.Keyframes)
		{
			Key.bSelected = false;
		}
	}
}

TArray<FAnimationCurveEditorKeyframe> UAnimationCurveEditorManager::GetSelectedKeyframes() const
{
	TArray<FAnimationCurveEditorKeyframe> Selected;

	for (const FAnimationCurveEditorCurve& Curve : Curves)
	{
		for (const FAnimationCurveEditorKeyframe& Key : Curve.Keyframes)
		{
			if (Key.bSelected)
			{
				Selected.Add(Key);
			}
		}
	}

	return Selected;
}

void UAnimationCurveEditorManager::MoveSelectedKeyframes(float DeltaTime, float DeltaValue)
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		for (FAnimationCurveEditorKeyframe& Key : Curve.Keyframes)
		{
			if (Key.bSelected)
			{
				Key.Time += DeltaTime;
				Key.Value += DeltaValue;
			}
		}
	}
}

void UAnimationCurveEditorManager::DeleteSelectedKeyframes()
{
	for (FAnimationCurveEditorCurve& Curve : Curves)
	{
		Curve.Keyframes.RemoveAll([](const FAnimationCurveEditorKeyframe& Key)
		{
			return Key.bSelected;
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
			break;
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
			break;
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
	ZoomLevel = FMath::Min(ZoomLevel * 1.2f, 10.0f);
}

void UAnimationCurveEditorManager::ZoomOut()
{
	ZoomLevel = FMath::Max(ZoomLevel / 1.2f, 0.1f);
}

void UAnimationCurveEditorManager::ResetZoom()
{
	ZoomLevel = 1.0f;
	ViewOffset = FVector2D::ZeroVector;
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

void SCurvesEditorWidget::Construct(const FArguments& InArgs)
{
	EditorAnimation = InArgs._Animation;

	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	Manager->LoadAnimationCurves(EditorAnimation);

	CurveNames.Empty();
	for (const FAnimationCurveEditorCurve& Curve : Manager->GetCurves())
	{
		CurveNames.Add(Curve.CurveName);
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			CreateToolbar()
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f)
			[
				CreateCurvesList()
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CurvesArea", "Curves Area - Draw curves here"))
					.Justification(ETextJustify::Center)
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
	for (const FAnimationCurveEditorCurve& Curve : Manager->GetCurves())
	{
		if (Curve.bVisible)
		{
			DrawCurve(Curve, AllottedGeometry, OutDrawElements, LayerId);
			DrawKeyframes(Curve, AllottedGeometry, OutDrawElements, LayerId);
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
		bIsDragging = false;
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SCurvesEditorWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	MousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

	if (bIsDragging)
	{
		FVector2D Delta = MousePosition - DragStart;
		UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
		Manager->PanView(Delta.X * 0.01f, Delta.Y * 0.01f);
		DragStart = MousePosition;
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SCurvesEditorWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	float Delta = MouseEvent.GetWheelDelta();
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();

	if (Delta > 0)
	{
		Manager->ZoomIn();
	}
	else
	{
		Manager->ZoomOut();
	}

	return FReply::Handled();
}

TSharedRef<SWidget> SCurvesEditorWidget::CreateCurvesList()
{
	CurvesListView = SNew(SListView<FName>)
	.ListItemsSource(&CurveNames)
	.OnGenerateRow_Lambda([this](FName Item, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
	{
		UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
		FLinearColor Color = Manager->GetCurveColor(Item);
		bool bVisible = Manager->IsCurveVisible(Item);

		return SNew(STableRow<FName>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SCheckBox)
				.IsChecked(bVisible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged_Lambda([this, Item](ECheckBoxState NewState)
				{
					OnCurveVisibilityChanged(Item, NewState);
				})
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Item))
				.ColorAndOpacity(Color)
			]
		];
	});

	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("Curves", "Curves"))
		.Font(FAppStyle::GetFontStyle("LargeBoldFont"))
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
		.Text(LOCTEXT("AddCurve", "Add Curve"))
		.OnClicked_Lambda([this]() -> FReply
		{
			OnAddCurveClicked();
			return FReply::Handled();
		})
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("RemoveCurve", "Remove Curve"))
		.OnClicked_Lambda([this]() -> FReply
		{
			OnRemoveCurveClicked();
			return FReply::Handled();
		})
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("AddKeyframe", "Add Key"))
		.OnClicked_Lambda([this]() -> FReply
		{
			OnAddKeyframeClicked();
			return FReply::Handled();
		})
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(10.0f, 2.0f)
	[
		SNew(SSeparator)
		.Orientation(Orient_Vertical)
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("Load", "Load"))
		.OnClicked_Lambda([this]() -> FReply
		{
			OnLoadClicked();
			return FReply::Handled();
		})
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("Save", "Save"))
		.OnClicked_Lambda([this]() -> FReply
		{
			OnSaveClicked();
			return FReply::Handled();
		})
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(10.0f, 2.0f)
	[
		SNew(SSeparator)
		.Orientation(Orient_Vertical)
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("ZoomIn", "+"))
		.OnClicked_Lambda([this]() -> FReply
		{
			OnZoomInClicked();
			return FReply::Handled();
		})
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("ZoomOut", "-"))
		.OnClicked_Lambda([this]() -> FReply
		{
			OnZoomOutClicked();
			return FReply::Handled();
		})
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("ResetZoom", "Reset"))
		.OnClicked_Lambda([this]() -> FReply
		{
			OnResetZoomClicked();
			return FReply::Handled();
		})
	];
}

void SCurvesEditorWidget::OnAddCurveClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	FName NewName = *FString::Printf(TEXT("Curve_%d"), Manager->GetCurves().Num());
	Manager->AddCurve(NewName, FLinearColor::MakeRandomColor());

	CurveNames.Empty();
	for (const FAnimationCurveEditorCurve& Curve : Manager->GetCurves())
	{
		CurveNames.Add(Curve.CurveName);
	}

	if (CurvesListView.IsValid())
	{
		CurvesListView->RequestListRefresh();
	}
}

void SCurvesEditorWidget::OnRemoveCurveClicked()
{
	if (CurveNames.Num() > 0)
	{
		UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
		Manager->RemoveCurve(CurveNames.Last());

		CurveNames.Pop();
		if (CurvesListView.IsValid())
		{
			CurvesListView->RequestListRefresh();
		}
	}
}

void SCurvesEditorWidget::OnLoadClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	Manager->LoadAnimationCurves(EditorAnimation);

	CurveNames.Empty();
	for (const FAnimationCurveEditorCurve& Curve : Manager->GetCurves())
	{
		CurveNames.Add(Curve.CurveName);
	}

	if (CurvesListView.IsValid())
	{
		CurvesListView->RequestListRefresh();
	}
}

void SCurvesEditorWidget::OnSaveClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	Manager->SaveAnimationCurves(EditorAnimation);
}

void SCurvesEditorWidget::OnZoomInClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	Manager->ZoomIn();
}

void SCurvesEditorWidget::OnZoomOutClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	Manager->ZoomOut();
}

void SCurvesEditorWidget::OnResetZoomClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	Manager->ResetZoom();
}

void SCurvesEditorWidget::OnCurveVisibilityChanged(const FName& CurveName, ECheckBoxState NewState)
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	Manager->SetCurveVisible(CurveName, NewState == ECheckBoxState::Checked);
}

void SCurvesEditorWidget::OnAddKeyframeClicked()
{
	UAnimationCurveEditorManager* Manager = UAnimationCurveEditorManager::GetCurvesEditorManager();
	if (CurveNames.Num() > 0)
	{
		Manager->AddKeyframe(CurveNames[0], 0.0f, 0.0f);
	}
}

void SCurvesEditorWidget::DrawCurve(const FAnimationCurveEditorCurve& Curve, const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const
{
	if (Curve.Keyframes.Num() < 2)
	{
		return;
	}

	TArray<FSlateVertex> Vertices;
	TArray<SlateIndex> Indices;

	for (int32 i = 0; i < Curve.Keyframes.Num() - 1; i++)
	{
		FVector2D Start = TimeToScreen(Curve.Keyframes[i].Time, Geometry);
		FVector2D End = TimeToScreen(Curve.Keyframes[i + 1].Time, Geometry);

		FSlateVertex StartVert;
		StartVert.Position = Start;
		StartVert.TintColor = Curve.Color;

		FSlateVertex EndVert;
		EndVert.Position = End;
		EndVert.TintColor = Curve.Color;

		int32 BaseIndex = Vertices.Num();
		Vertices.Add(StartVert);
		Vertices.Add(EndVert);
		Indices.Add(BaseIndex);
		Indices.Add(BaseIndex + 1);
	}

	if (Vertices.Num() > 0)
	{
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			Geometry.ToPaintGeometry(),
			Vertices,
			Indices,
			ESlateDrawEffect::None,
			Curve.Color,
			true,
			2.0f
		);
	}
}

void SCurvesEditorWidget::DrawGrid(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const
{
	FLinearColor GridColor(0.3f, 0.3f, 0.3f, 0.5f);

	FVector2D Origin = TimeToScreen(0.0f, Geometry);
	FVector2D End = TimeToScreen(ViewMaxTime, Geometry);

	for (float Time = 0.0f; Time <= ViewMaxTime; Time += 1.0f)
	{
		FVector2D Pos = TimeToScreen(Time, Geometry);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			Geometry.ToPaintGeometry(),
			{ Pos, FVector2D(Pos.X, Geometry.GetLocalSize().Y) },
			ESlateDrawEffect::None,
			GridColor,
			true,
			1.0f
		);
	}
}

void SCurvesEditorWidget::DrawKeyframes(const FAnimationCurveEditorCurve& Curve, const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, int32& LayerId) const
{
	for (const FAnimationCurveEditorKeyframe& Key : Curve.Keyframes)
	{
		FVector2D Pos = TimeToScreen(Key.Time, Geometry);
		Pos.Y = ValueToScreen(Key.Value, Geometry).Y;

		float Radius = Key.bSelected ? 6.0f : 4.0f;
		FLinearColor Color = Key.bSelected ? FLinearColor::Yellow : Curve.Color;

		TArray<FSlateVertex> Vertices;
		TArray<SlateIndex> Indices;

		for (int32 i = 0; i < 8; i++)
		{
			float Angle = (float)i / 8.0f * 2.0f * 3.14159f;
			FVector2D Point = Pos + FVector2D(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);

			FSlateVertex Vert;
			Vert.Position = Point;
			Vert.TintColor = Color;

			int32 BaseIndex = Vertices.Num();
			Vertices.Add(Vert);
			Indices.Add(BaseIndex);
			Indices.Add((BaseIndex + 1) % 8);
		}

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			Geometry.ToPaintGeometry(),
			Vertices,
			Indices,
			ESlateDrawEffect::None,
			Color,
			true,
			2.0f
		);
	}
}

FVector2D SCurvesEditorWidget::TimeToScreen(float Time, const FGeometry& Geometry) const
{
	float TimeRange = ViewMaxTime - ViewMinTime;
	float NormalizedTime = (TimeRange > 0.0f) ? (Time - ViewMinTime) / TimeRange : 0.0f;
	float ScreenX = NormalizedTime * Geometry.GetLocalSize().X;

	float ValueRange = ViewMaxValue - ViewMinValue;
	float NormalizedValue = (ValueRange > 0.0f) ? (0.0f - ViewMinValue) / ValueRange : 0.0f;
	float ScreenY = (1.0f - NormalizedValue) * Geometry.GetLocalSize().Y;

	return FVector2D(ScreenX, ScreenY);
}

float SCurvesEditorWidget::ScreenToTime(const FVector2D& ScreenPos, const FGeometry& Geometry) const
{
	float NormalizedTime = ScreenPos.X / Geometry.GetLocalSize().X;
	return ViewMinTime + NormalizedTime * (ViewMaxTime - ViewMinTime);
}

FVector2D SCurvesEditorWidget::ValueToScreen(float Value, const FGeometry& Geometry) const
{
	float ScreenX = 0.0f;
	float ValueRange = ViewMaxValue - ViewMinValue;
	float NormalizedValue = (ValueRange > 0.0f) ? (Value - ViewMinValue) / ValueRange : 0.0f;
	float ScreenY = (1.0f - NormalizedValue) * Geometry.GetLocalSize().Y;

	return FVector2D(ScreenX, ScreenY);
}

float SCurvesEditorWidget::ScreenToValue(const FVector2D& ScreenPos, const FGeometry& Geometry) const
{
	float NormalizedValue = 1.0f - (ScreenPos.Y / Geometry.GetLocalSize().Y);
	return ViewMinValue + NormalizedValue * (ViewMaxValue - ViewMinValue);
}

#undef LOCTEXT_NAMESPACE

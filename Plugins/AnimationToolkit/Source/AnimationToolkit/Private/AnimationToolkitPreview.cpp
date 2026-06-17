#include "AnimationToolkitPreview.h"
#include "AnimationLibrary.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "AnimationToolkitPreview"

TWeakObjectPtr<UAnimationPreviewManager> UAnimationPreviewManager::Instance = nullptr;

UAnimationPreviewManager* UAnimationPreviewManager::GetPreviewManager()
{
	if (!Instance.IsValid() || Instance.Get() == nullptr)
	{
		Instance = NewObject<UAnimationPreviewManager>(GetTransientPackage(), NAME_None, RF_MarkAsRootSet);
	}
	return Instance.Get();
}

void UAnimationPreviewManager::SetPreviewAnimation(UAnimSequence* Animation)
{
	CurrentAnimation = Animation;
	PlaybackPosition = 0.0f;
	bPlaying = false;
}

UAnimSequence* UAnimationPreviewManager::GetPreviewAnimation() const
{
	return CurrentAnimation;
}

void UAnimationPreviewManager::Play()
{
	bPlaying = true;
}

void UAnimationPreviewManager::Pause()
{
	bPlaying = false;
}

void UAnimationPreviewManager::Stop()
{
	bPlaying = false;
	PlaybackPosition = 0.0f;
}

void UAnimationPreviewManager::SetPlaybackPosition(float Position)
{
	PlaybackPosition = FMath::Clamp(Position, 0.0f, GetAnimationDuration());
}

float UAnimationPreviewManager::GetPlaybackPosition() const
{
	return PlaybackPosition;
}

float UAnimationPreviewManager::GetAnimationDuration() const
{
	if (CurrentAnimation)
	{
		return CurrentAnimation->GetSequenceLength();
	}
	return 0.0f;
}

bool UAnimationPreviewManager::IsPlaying() const
{
	return bPlaying;
}

void UAnimationPreviewManager::SetPlayRate(float Rate)
{
	PlayRate = FMath::Max(0.0f, Rate);
}

float UAnimationPreviewManager::GetPlayRate() const
{
	return PlayRate;
}

void UAnimationPreviewManager::SetLoop(bool bNewLoop)
{
	bLoop = bNewLoop;
}

bool UAnimationPreviewManager::IsLooping() const
{
	return bLoop;
}

void UAnimationPreviewManager::SetShowBoneNames(bool bShow)
{
	bShowBoneNames = bShow;
}

bool UAnimationPreviewManager::GetShowBoneNames() const
{
	return bShowBoneNames;
}

void UAnimationPreviewManager::SetShowTrail(bool bShow)
{
	bShowTrail = bShow;
}

bool UAnimationPreviewManager::GetShowTrail() const
{
	return bShowTrail;
}

void UAnimationPreviewManager::SetTrailLength(float Length)
{
	TrailLength = FMath::Max(0.1f, Length);
}

float UAnimationPreviewManager::GetTrailLength() const
{
	return TrailLength;
}

void SAnimationPreviewWidget::Construct(const FArguments& InArgs)
{
	PreviewAnimation = InArgs._Animation;
	PreviewMeshComponent = InArgs._PreviewMesh;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			CreatePreviewControls()
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(5.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				CreateBoneList()
			]
		]
	];

	if (PreviewAnimation)
	{
		UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
		Manager->SetPreviewAnimation(PreviewAnimation);
	}
}

void SAnimationPreviewWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();

	if (Manager->IsPlaying())
	{
		float NewPosition = Manager->GetPlaybackPosition() + (InDeltaTime * Manager->GetPlayRate());
		float Duration = Manager->GetAnimationDuration();

		if (Manager->IsLooping())
		{
			if (Duration > 0.0f)
			{
				while (NewPosition >= Duration)
				{
					NewPosition -= Duration;
				}
			}
			else
			{
				NewPosition = 0.0f;
			}
		}
		else if (NewPosition >= Duration)
		{
			NewPosition = Duration;
			Manager->Pause();
		}

		Manager->SetPlaybackPosition(NewPosition);
		CurrentPlaybackPosition = NewPosition;

		if (PlaybackSlider.IsValid())
		{
			PlaybackSlider->SetValue(CurrentPlaybackPosition / FMath::Max(0.001f, Duration));
		}

		if (TimeTextBlock.IsValid())
		{
			TimeTextBlock->SetText(FText::AsNumber(CurrentPlaybackPosition));
		}
	}
}

TSharedRef<SWidget> SAnimationPreviewWidget::CreatePreviewControls()
{
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();

	float Duration = Manager->GetAnimationDuration();

	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Play", "Play"))
			.OnClicked_Lambda([this]() -> FReply
			{
				OnPlayClicked();
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Pause", "Pause"))
			.OnClicked_Lambda([this]() -> FReply
			{
				OnPauseClicked();
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("Stop", "Stop"))
			.OnClicked_Lambda([this]() -> FReply
			{
				OnStopClicked();
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(5.0f)
		[
			SAssignNew(PlaybackSlider, SSlider)
			.Value(0.0f)
			.OnValueChanged_Raw(this, &SAnimationPreviewWidget::OnSliderChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(TimeTextBlock, STextBlock)
			.Text(FText::AsNumber(0.0f))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("of", " / "))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(DurationTextBlock, STextBlock)
			.Text(FText::AsNumber(Duration))
		]
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PlayRate", "Rate:"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(5.0f)
		[
			SAssignNew(PlayRateSlider, SSlider)
			.Value(CurrentPlayRate)
			.OnValueChanged_Raw(this, &SAnimationPreviewWidget::OnPlayRateChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				return FText::AsNumber(CurrentPlayRate);
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(10.0f, 0.0f)
		[
			SNew(SCheckBox)
			.IsChecked(bLoop ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Raw(this, &SAnimationPreviewWidget::OnLoopChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Loop", "Loop"))
		]
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SSeparator)
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SCheckBox)
			.IsChecked(bShowBoneNames ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Raw(this, &SAnimationPreviewWidget::OnShowBonesChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ShowBones", "Show Bones"))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(10.0f, 0.0f)
		[
			SNew(SCheckBox)
			.IsChecked(bShowTrail ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Raw(this, &SAnimationPreviewWidget::OnShowTrailChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ShowTrail", "Show Trail"))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(5.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TrailLength", "Length:"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SAssignNew(TrailLengthSlider, SSlider)
			.Value(TrailLengthValue)
			.OnValueChanged_Raw(this, &SAnimationPreviewWidget::OnTrailLengthChanged)
		]
	];
}

TSharedRef<SWidget> SAnimationPreviewWidget::CreateBoneList()
{
	if (PreviewAnimation)
	{
		USkeleton* Skeleton = PreviewAnimation->GetSkeleton();
		if (Skeleton)
		{
			const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
			BoneNames.Empty();
			for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
			{
				BoneNames.Add(RefSkeleton.GetBoneName(i));
			}
		}
	}

	BoneListView = SNew(SListView<FName>)
	.ListItemsSource(&BoneNames)
	.OnGenerateRow_Lambda([](FName Item, const TSharedRef<STableViewBase>& OwnerTable) -> TSharedRef<ITableRow>
	{
		return SNew(STableRow<FName>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromName(Item))
		];
	});

	return BoneListView.ToSharedRef();
}

void SAnimationPreviewWidget::OnPlayClicked()
{
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
	Manager->Play();
	bIsPlaying = true;
}

void SAnimationPreviewWidget::OnPauseClicked()
{
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
	Manager->Pause();
	bIsPlaying = false;
}

void SAnimationPreviewWidget::OnStopClicked()
{
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
	Manager->Stop();
	bIsPlaying = false;
	CurrentPlaybackPosition = 0.0f;

	if (PlaybackSlider.IsValid())
	{
		PlaybackSlider->SetValue(0.0f);
	}

	if (TimeTextBlock.IsValid())
	{
		TimeTextBlock->SetText(FText::AsNumber(0.0f));
	}
}

void SAnimationPreviewWidget::OnSliderChanged(float Value)
{
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
	float Duration = Manager->GetAnimationDuration();
	Manager->SetPlaybackPosition(Value * Duration);
	CurrentPlaybackPosition = Value * Duration;

	if (TimeTextBlock.IsValid())
	{
		TimeTextBlock->SetText(FText::AsNumber(CurrentPlaybackPosition));
	}
}

void SAnimationPreviewWidget::OnPlayRateChanged(float Value)
{
	CurrentPlayRate = Value;
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
	Manager->SetPlayRate(Value);
}

void SAnimationPreviewWidget::OnLoopChanged(ECheckBoxState NewState)
{
	bLoop = (NewState == ECheckBoxState::Checked);
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
	Manager->SetLoop(bLoop);
}

void SAnimationPreviewWidget::OnShowBonesChanged(ECheckBoxState NewState)
{
	bShowBoneNames = (NewState == ECheckBoxState::Checked);
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
	Manager->SetShowBoneNames(bShowBoneNames);
}

void SAnimationPreviewWidget::OnShowTrailChanged(ECheckBoxState NewState)
{
	bShowTrail = (NewState == ECheckBoxState::Checked);
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
	Manager->SetShowTrail(bShowTrail);
}

void SAnimationPreviewWidget::OnTrailLengthChanged(float Value)
{
	TrailLengthValue = Value;
	UAnimationPreviewManager* Manager = UAnimationPreviewManager::GetPreviewManager();
	Manager->SetTrailLength(Value);
}

#undef LOCTEXT_NAMESPACE

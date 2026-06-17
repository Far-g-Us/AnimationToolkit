#include "AnimationStateMachine.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

UAnimStateMachine::UAnimStateMachine()
{
	CurrentState = 0;
	PreviousState = 0;
	StateTime = 0.0f;
	BlendAlpha = 0.0f;
	bIsTransitioning = false;
	PlaybackRate = 1.0f;
	bLoopAnimations = true;
	CachedSkeleton = nullptr;
}

void UAnimStateMachine::Initialize(USkeleton* Skeleton)
{
	CachedSkeleton = Skeleton;
	CurrentState = 0;
	PreviousState = 0;
	StateTime = 0.0f;
	BlendAlpha = 0.0f;
	bIsTransitioning = false;

	for (FAnimStateTransition& Transition : Transitions)
	{
		Transition.TransitionTime = 0.0f;
	}
}

void UAnimStateMachine::SetState(int32 NewState)
{
	if (!Animations.IsValidIndex(NewState))
	{
		return;
	}

	if (NewState == CurrentState)
	{
		return;
	}

	PreviousState = CurrentState;
	CurrentState = NewState;
	StateTime = 0.0f;
	BlendAlpha = 0.0f;
	bIsTransitioning = true;

	for (FAnimStateTransition& Transition : Transitions)
	{
		if (Transition.FromState == PreviousState && Transition.ToState == CurrentState)
		{
			Transition.TransitionTime = 0.0f;
		}
	}
}

void UAnimStateMachine::Update(float DeltaTime)
{
	if (Animations.Num() == 0)
	{
		return;
	}

	if (!Animations.IsValidIndex(CurrentState))
	{
		CurrentState = 0;
		return;
	}

	UAnimSequence* CurrentAnim = Animations[CurrentState];
	if (!CurrentAnim)
	{
		return;
	}

	float AdjustedDelta = DeltaTime * PlaybackRate;
	StateTime += AdjustedDelta;

	if (bIsTransitioning)
	{
		FAnimStateTransition* ActiveTransition = nullptr;
		for (FAnimStateTransition& Transition : Transitions)
		{
			if (Transition.FromState == PreviousState && Transition.ToState == CurrentState)
			{
				ActiveTransition = &Transition;
				break;
			}
		}

		if (ActiveTransition)
		{
			ActiveTransition->TransitionTime += AdjustedDelta;

			if (ActiveTransition->TransitionDuration > 0.0f)
			{
				BlendAlpha = FMath::Clamp(
					ActiveTransition->TransitionTime / ActiveTransition->TransitionDuration,
					0.0f, 1.0f);
			}
			else
			{
				BlendAlpha = 1.0f;
			}

			if (ActiveTransition->TransitionTime >= ActiveTransition->TransitionDuration)
			{
				bIsTransitioning = false;
				BlendAlpha = 1.0f;
				ActiveTransition->TransitionTime = 0.0f;
			}
		}
		else
		{
			bIsTransitioning = false;
			BlendAlpha = 1.0f;
		}
	}
	else
	{
		BlendAlpha = 1.0f;

		float AnimLength = CurrentAnim->GetPlayLength();
		if (AnimLength > 0.0f)
		{
			if (bLoopAnimations)
			{
				StateTime = FMath::Fmod(StateTime, AnimLength);
				if (StateTime < 0.0f)
				{
					StateTime += AnimLength;
				}
			}
			else if (StateTime >= AnimLength)
			{
				StateTime = AnimLength;
			}
		}

		for (FAnimStateTransition& Transition : Transitions)
		{
			if (Transition.FromState == CurrentState)
			{
				UAnimSequence* FromAnim = Animations.IsValidIndex(Transition.FromState)
					? Animations[Transition.FromState] : nullptr;

				if (FromAnim)
				{
					float FromLength = FromAnim->GetPlayLength();
					bool bShouldTransition = false;

					if (FromLength > 0.0f && !bLoopAnimations && StateTime >= FromLength)
					{
						bShouldTransition = true;
					}

					if (Transition.TransitionDuration <= 0.0f && bShouldTransition)
					{
						PreviousState = CurrentState;
						CurrentState = Transition.ToState;
						StateTime = 0.0f;
						bIsTransitioning = true;
						BlendAlpha = 0.0f;
						Transition.TransitionTime = 0.0f;
						break;
					}
				}
			}
		}
	}
}

int32 UAnimStateMachine::GetCurrentState() const
{
	return CurrentState;
}

float UAnimStateMachine::GetStateTime() const
{
	return StateTime;
}

FTransform UAnimStateMachine::GetBoneTransform(FName BoneName) const
{
	if (!CachedSkeleton)
	{
		return FTransform::Identity;
	}

	const FReferenceSkeleton& RefSkeleton = CachedSkeleton->GetReferenceSkeleton();
	int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);

	if (BoneIndex == INDEX_NONE)
	{
		return FTransform::Identity;
	}

	FTransform Result = RefSkeleton.GetRefBonePose()[BoneIndex];

	if (!Animations.IsValidIndex(CurrentState) || !Animations[CurrentState])
	{
		return Result;
	}

	UAnimSequence* CurrentAnim = Animations[CurrentState];
	float AnimLength = CurrentAnim->GetPlayLength();

	if (AnimLength <= 0.0f)
	{
		return Result;
	}

	float EvalTime = FMath::Fmod(StateTime, AnimLength);
	if (EvalTime < 0.0f)
	{
		EvalTime += AnimLength;
	}

	if (bIsTransitioning && BlendAlpha < 1.0f
		&& Animations.IsValidIndex(PreviousState) && Animations[PreviousState])
	{
		UAnimSequence* PrevAnim = Animations[PreviousState];
		float PrevLength = PrevAnim->GetPlayLength();

		if (PrevLength > 0.0f)
		{
			float PrevEvalTime = FMath::Fmod(StateTime, PrevLength);
			if (PrevEvalTime < 0.0f)
			{
				PrevEvalTime += PrevLength;
			}

		FTransform PrevResult = RefSkeleton.GetRefBonePose()[BoneIndex];
		Result = BlendTransforms(Result, PrevResult, 1.0f - BlendAlpha);
		}
	}

	return Result;
}

FName UAnimStateMachine::GetCurrentAnimationName() const
{
	if (!Animations.IsValidIndex(CurrentState))
	{
		return NAME_None;
	}

	UAnimSequence* CurrentAnim = Animations[CurrentState];
	if (!CurrentAnim)
	{
		return NAME_None;
	}

	return CurrentAnim->GetFName();
}

void UAnimStateMachine::AddState(UAnimSequence* Animation, EATK_AnimStateType StateType)
{
	if (!Animation)
	{
		return;
	}

	Animations.AddUnique(Animation);

	if (Animations.Num() == 1)
	{
		CurrentState = 0;
		PreviousState = 0;
		StateTime = 0.0f;
	}
}

void UAnimStateMachine::AddTransition(int32 FromState, int32 ToState, float Duration)
{
	if (!Animations.IsValidIndex(FromState) || !Animations.IsValidIndex(ToState))
	{
		return;
	}

	if (FromState == ToState)
	{
		return;
	}

	for (const FAnimStateTransition& Existing : Transitions)
	{
		if (Existing.FromState == FromState && Existing.ToState == ToState)
		{
			return;
		}
	}

	FAnimStateTransition NewTransition;
	NewTransition.FromState = FromState;
	NewTransition.ToState = ToState;
	NewTransition.TransitionDuration = FMath::Max(Duration, 0.0f);
	NewTransition.TransitionTime = 0.0f;
	NewTransition.bIsCrossfade = true;

	Transitions.Add(NewTransition);
}

int32 UAnimStateMachine::GetStateCount() const
{
	return Animations.Num();
}

int32 UAnimStateMachine::GetTransitionCount() const
{
	return Transitions.Num();
}

FTransform UAnimStateMachine::BlendTransforms(
	const FTransform& TransformA,
	const FTransform& TransformB,
	float Alpha) const
{
	float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	FTransform Result = TransformA;
	Result.BlendWith(TransformB, ClampedAlpha);
	return Result;
}

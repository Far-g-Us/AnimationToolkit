#include "AnimationStateMachine.h"

UAnimStateMachine::UAnimStateMachine()
{
}

void UAnimStateMachine::Initialize(USkeleton* Skeleton)
{
	CachedSkeleton = Skeleton;
	CurrentState = 0;
	StateTime = 0.0f;
	BlendAlpha = 0.0f;
	PreviousState = 0;
	bIsTransitioning = false;
}

void UAnimStateMachine::SetState(int32 NewState)
{
	if (NewState < 0 || NewState >= Animations.Num())
	{
		return;
	}

	if (NewState != CurrentState)
	{
		PreviousState = CurrentState;
		CurrentState = NewState;
		StateTime = 0.0f;
		bIsTransitioning = true;
		BlendAlpha = 0.0f;
	}
}

void UAnimStateMachine::Update(float DeltaTime)
{
	if (Animations.Num() == 0 || !Animations.IsValidIndex(CurrentState) || !Animations[CurrentState])
	{
		return;
	}

	StateTime += DeltaTime * PlaybackRate;

	float AnimDuration = Animations[CurrentState]->GetSequenceLength();

	if (bIsTransitioning)
	{
		float TransitionDuration = 0.2f;
		for (const FAnimStateTransition& Trans : Transitions)
		{
			if (Trans.FromState == PreviousState && Trans.ToState == CurrentState)
			{
				TransitionDuration = Trans.TransitionDuration;
				break;
			}
		}

		BlendAlpha = FMath::Clamp(StateTime / TransitionDuration, 0.0f, 1.0f);
		if (BlendAlpha >= 1.0f)
		{
			bIsTransitioning = false;
		}
	}
	else if (AnimDuration > 0.0f && StateTime >= AnimDuration)
	{
		if (bLoopAnimations)
		{
			StateTime = FMath::Fmod(StateTime, AnimDuration);
		}
		else
		{
			StateTime = AnimDuration;
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
	if (!CachedSkeleton || Animations.Num() == 0 || !Animations.IsValidIndex(CurrentState))
	{
		return FTransform::Identity;
	}

	const FReferenceSkeleton& RefSkeleton = CachedSkeleton->GetReferenceSkeleton();
	int32 BoneIdx = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIdx == INDEX_NONE)
	{
		return FTransform::Identity;
	}

	FTransform CurrentTransform = Animations[CurrentState] ?
		Animations[CurrentState]->GetBoneTransform(BoneIdx, StateTime) : FTransform::Identity;

	if (bIsTransitioning && Animations[PreviousState] && BlendAlpha < 1.0f)
	{
		FTransform PreviousTransform = Animations[PreviousState]->GetBoneTransform(BoneIdx, StateTime);
		CurrentTransform = BlendTransforms(PreviousTransform, CurrentTransform, BlendAlpha);
	}

	return CurrentTransform;
}

FName UAnimStateMachine::GetCurrentAnimationName() const
{
	if (Animations.IsValidIndex(CurrentState) && Animations[CurrentState])
	{
		return Animations[CurrentState]->GetFName();
	}

	return NAME_None;
}

void UAnimStateMachine::AddState(UAnimSequence* Animation, EAnimStateType StateType)
{
	if (Animation)
	{
		Animations.Add(Animation);
	}
}

void UAnimStateMachine::AddTransition(int32 FromState, int32 ToState, float Duration)
{
	FAnimStateTransition Trans;
	Trans.FromState = FromState;
	Trans.ToState = ToState;
	Trans.TransitionDuration = Duration;
	Transitions.Add(Trans);
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
	FVector BlendedLocation = FMath::Lerp(TransformA.GetLocation(), TransformB.GetLocation(), Alpha);
	FQuat BlendedRotation = FQuat::Slerp(TransformA.GetRotation(), TransformB.GetRotation(), Alpha);
	FVector BlendedScale = FMath::Lerp(TransformA.GetScale3D(), TransformB.GetScale3D(), Alpha);

	return FTransform(BlendedRotation, BlendedLocation, BlendedScale);
}

#include "AnimationStateMachine.h"

UAnimStateMachine::UAnimStateMachine()
{
}

void UAnimStateMachine::Initialize(USkeleton* Skeleton)
{
}

void UAnimStateMachine::SetState(int32 NewState)
{
}

void UAnimStateMachine::Update(float DeltaTime)
{
}

int32 UAnimStateMachine::GetCurrentState() const
{
	return 0;
}

float UAnimStateMachine::GetStateTime() const
{
	return 0.0f;
}

FTransform UAnimStateMachine::GetBoneTransform(FName BoneName) const
{
	return FTransform::Identity;
}

FName UAnimStateMachine::GetCurrentAnimationName() const
{
	return NAME_None;
}

void UAnimStateMachine::AddState(UAnimSequence* Animation, EATK_AnimStateType StateType)
{
}

void UAnimStateMachine::AddTransition(int32 FromState, int32 ToState, float Duration)
{
}

int32 UAnimStateMachine::GetStateCount() const
{
	return 0;
}

int32 UAnimStateMachine::GetTransitionCount() const
{
	return 0;
}

FTransform UAnimStateMachine::BlendTransforms(
	const FTransform& TransformA,
	const FTransform& TransformB,
	float Alpha) const
{
	return FTransform::Identity;
}

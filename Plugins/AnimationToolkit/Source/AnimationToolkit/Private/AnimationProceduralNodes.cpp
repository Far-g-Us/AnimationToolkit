#include "AnimationProceduralNodes.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

UAnimSequence* UAnimationProceduralNodes::ApplyTwoBoneIK(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	FVector EffectorTarget,
	FVector PoleTarget,
	float IKWeight)
{
	return nullptr;
}

UAnimSequence* UAnimationProceduralNodes::ApplyFBIK(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const TArray<FIKBoneTarget>& Targets,
	int32 Iterations)
{
	return nullptr;
}

UAnimSequence* UAnimationProceduralNodes::ApplyAimOffset(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	FRotator AimOffset,
	float AimOffsetWeight)
{
	return nullptr;
}

UAnimSequence* UAnimationProceduralNodes::ApplySpringPhysics(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	float SpringStiffness,
	float SpringDamping,
	float SpringMass)
{
	return nullptr;
}

FVector UAnimationProceduralNodes::SolveTwoBoneIK(
	const FVector& UpperArmPos,
	const FVector& LowerArmPos,
	const FVector& HandPos,
	const FVector& EffectorTarget,
	const FVector& PoleTarget,
	float UpperArmLength,
	float LowerArmLength,
	FVector& OutUpperArmNew,
	FVector& OutLowerArmNew)
{
	return FVector::ZeroVector;
}

FVector UAnimationProceduralNodes::CalculateSpringForce(
	const FVector& CurrentPos,
	const FVector& TargetPos,
	const FVector& Velocity,
	float Stiffness,
	float Damping,
	float Mass,
	float DeltaTime)
{
	return FVector::ZeroVector;
}

FRotator UAnimationProceduralNodes::CalculateAimOffsetRotation(
	const FVector& CurrentDirection,
	const FVector& AimDirection,
	float MaxPitch,
	float MaxYaw)
{
	return FRotator::ZeroRotator;
}

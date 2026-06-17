#include "AnimationPhysicsSimulation.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

UAnimSequence* UAnimationPhysicsSimulation::SimulatePhysics(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const TArray<FPhysicsBoneSettings>& BoneSettings,
	float SimulationTime)
{
	if (!WorldContextObject || !SourceAnimation)
	{
		return nullptr;
	}

	return nullptr;
}

UAnimSequence* UAnimationPhysicsSimulation::ApplyGravity(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	float GravityStrength)
{
	if (!WorldContextObject || !SourceAnimation)
	{
		return nullptr;
	}

	return nullptr;
}

UAnimSequence* UAnimationPhysicsSimulation::ApplySpring(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	float SpringStiffness,
	float SpringDamping)
{
	if (!WorldContextObject || !SourceAnimation)
	{
		return nullptr;
	}

	return nullptr;
}

FVector UAnimationPhysicsSimulation::CalculatePhysicsForce(
	const FVector& Position,
	const FVector& Velocity,
	float Mass,
	float Gravity)
{
	return FVector(0.0f, 0.0f, -Mass * Gravity);
}

#include "AnimationMotionMatching.h"
#include "Animation/AnimSequence.h"

int32 UAnimationMotionMatching::FindBestMatchingPose(
	const TArray<UAnimSequence*>& Animations,
	const FVector& CurrentPosition,
	const FRotator& CurrentRotation,
	const FVector& DesiredVelocity)
{
	return INDEX_NONE;
}

float UAnimationMotionMatching::CalculatePoseCost(
	const FMotionMatchingPose& PoseA,
	const FMotionMatchingPose& PoseB)
{
	float Cost = 0.0f;

	Cost += FVector::Dist(PoseA.Position, PoseB.Position);
	Cost += PoseA.Rotation.AngularDistance(PoseB.Rotation);

	return Cost;
}

TArray<FMotionMatchingPose> UAnimationMotionMatching::ExtractPoses(
	UAnimSequence* Animation,
	float SampleRate)
{
	TArray<FMotionMatchingPose> Poses;

	if (!Animation)
	{
		return Poses;
	}

	return Poses;
}

void UAnimationMotionMatching::BuildPoseDatabase(
	const TArray<UAnimSequence*>& Animations,
	TArray<FMotionMatchingPose>& OutPoseDatabase)
{
	for (UAnimSequence* Anim : Animations)
	{
		if (Anim)
		{
			TArray<FMotionMatchingPose> Poses = ExtractPoses(Anim);
			OutPoseDatabase.Append(Poses);
		}
	}
}

int32 UAnimationMotionMatching::FindBestMatchingPoseInDatabase(
	const TArray<FMotionMatchingPose>& PoseDatabase,
	const FVector& CurrentPosition,
	const FRotator& CurrentRotation,
	const FVector& DesiredVelocity)
{
	if (PoseDatabase.Num() == 0)
	{
		return INDEX_NONE;
	}

	int32 BestIndex = 0;
	float BestCost = MAX_FLT;

	for (int32 i = 0; i < PoseDatabase.Num(); i++)
	{
		FMotionMatchingPose TestPose;
		TestPose.Position = CurrentPosition;
		TestPose.Rotation = CurrentRotation;

		float Cost = CalculatePoseCost(PoseDatabase[i], TestPose);

		if (Cost < BestCost)
		{
			BestCost = Cost;
			BestIndex = i;
		}
	}

	return BestIndex;
}

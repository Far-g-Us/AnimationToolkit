#include "AnimationMotionMatching.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataModel.h"

int32 UAnimationMotionMatching::FindBestMatchingPose(
	const TArray<UAnimSequence*>& Animations,
	const FVector& CurrentPosition,
	const FRotator& CurrentRotation,
	const FVector& DesiredVelocity)
{
	TArray<FMotionMatchingPose> PoseDatabase;
	BuildPoseDatabase(Animations, PoseDatabase);
	return FindBestMatchingPoseInDatabase(PoseDatabase, CurrentPosition, CurrentRotation, DesiredVelocity);
}

float UAnimationMotionMatching::CalculatePoseCost(
	const FMotionMatchingPose& PoseA,
	const FMotionMatchingPose& PoseB)
{
	float Cost = 0.0f;

	Cost += FVector::Dist(PoseA.Position, PoseB.Position);
	Cost += FQuat(PoseA.Rotation).AngularDistance(FQuat(PoseB.Rotation));

	return Cost;
}

TArray<FMotionMatchingPose> UAnimationMotionMatching::ExtractPoses(
	UAnimSequence* Animation,
	float SampleRate)
{
	TArray<FMotionMatchingPose> Poses;

	if (!Animation || SampleRate <= 0.0f)
	{
		return Poses;
	}

	const IAnimationDataModel* DataModel = Animation->GetDataModel();
	if (!DataModel)
	{
		return Poses;
	}

	const float Duration = DataModel->GetPlayLength();
	if (Duration <= 0.0f)
	{
		return Poses;
	}

	const float SampleInterval = 1.0f / SampleRate;
	const TArray<FBoneAnimationTrack>& BoneTracks = DataModel->GetBoneAnimationTracks();

	int32 RootTrackIndex = INDEX_NONE;
	static const FName RootBoneNames[] = { FName("root"), FName("Root"), FName("pelvis"), FName("Pelvis") };
	for (const FName& Name : RootBoneNames)
	{
		for (int32 i = 0; i < BoneTracks.Num(); i++)
		{
			if (BoneTracks[i].Name == Name)
			{
				RootTrackIndex = i;
				break;
			}
		}
		if (RootTrackIndex != INDEX_NONE)
		{
			break;
		}
	}

	if (RootTrackIndex == INDEX_NONE && BoneTracks.Num() > 0)
	{
		RootTrackIndex = 0;
	}

	if (RootTrackIndex == INDEX_NONE)
	{
		return Poses;
	}

	const FRawAnimSequenceTrack& RawTrack = BoneTracks[RootTrackIndex].InternalTrackData;
	const int32 NumPosKeys = RawTrack.PosKeys.Num();
	const int32 NumRotKeys = RawTrack.RotKeys.Num();
	const int32 NumKeys = FMath::Max(NumPosKeys, NumRotKeys);

	for (float Time = 0.0f; Time < Duration; Time += SampleInterval)
	{
		FMotionMatchingPose Pose;
		Pose.Time = Time;
		Pose.PoseIndex = Poses.Num();

		if (NumKeys > 0)
		{
			const float NormalizedTime = FMath::Clamp(Time / Duration, 0.0f, 1.0f);
			const float FloatIndex = NormalizedTime * (NumKeys - 1);
			const int32 Index0 = FMath::Clamp(FMath::FloorToInt(FloatIndex), 0, NumKeys - 1);
			const int32 Index1 = FMath::Clamp(FMath::CeilToInt(FloatIndex), 0, NumKeys - 1);
			const float Alpha = FloatIndex - FMath::FloorToFloat(FloatIndex);

			FVector Loc = FVector::ZeroVector;
			if (NumPosKeys > 0)
			{
				const int32 P0 = FMath::Clamp(Index0, 0, NumPosKeys - 1);
				const int32 P1 = FMath::Clamp(Index1, 0, NumPosKeys - 1);
				Loc = FMath::Lerp(FVector(RawTrack.PosKeys[P0]), FVector(RawTrack.PosKeys[P1]), Alpha);
			}

			FRotator Rot = FRotator::ZeroRotator;
			if (NumRotKeys > 0)
			{
				const int32 R0 = FMath::Clamp(Index0, 0, NumRotKeys - 1);
				const int32 R1 = FMath::Clamp(Index1, 0, NumRotKeys - 1);
				Rot = FQuat::Slerp(FQuat(RawTrack.RotKeys[R0]), FQuat(RawTrack.RotKeys[R1]), Alpha).Rotator();
			}

			Pose.Position = Loc;
			Pose.Rotation = Rot;
		}

		Poses.Add(MoveTemp(Pose));
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

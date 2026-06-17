#include "AnimationProceduralGeneration.h"
#include "AnimationLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

static TArray<FAnimationTrackData> GenerateBoneTracks(
	USkeleton* Skeleton,
	const FName& BoneName,
	const TArray<FTransform>& Transforms,
	float FrameRate)
{
	TArray<FAnimationTrackData> Tracks;
	if (!Skeleton)
	{
		return Tracks;
	}

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	const int32 BoneIndex = RefSkel.FindBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE)
	{
		return Tracks;
	}

	FAnimationTrackData Track;
	Track.TrackName = BoneName;
	const float SafeFrameRate = FMath::Max(0.01f, FrameRate);
	for (const FTransform& T : Transforms)
	{
		FAnimationKeyframe KF;
		KF.Time = Track.Keyframes.Num() / SafeFrameRate;
		KF.Transform = T;
		Track.Keyframes.Add(KF);
	}
	Tracks.Add(Track);

	return Tracks;
}

UAnimSequence* UAnimationProceduralGeneration::GenerateIdleAnimation(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float Duration)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const float FrameRate = 30.0f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(Duration * FrameRate));
	const float FrameTime = Duration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;
	const FName SpineBone = FName(TEXT("spine_01"));
	const FName HeadBone = FName(TEXT("head"));
	const FName RootBone = FName(TEXT("root"));

	for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
	{
		const FName BoneName = RefSkel.GetBoneName(BoneIdx);
		TArray<FTransform> Transforms;

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = Frame * FrameTime;
			FTransform T = FTransform::Identity;

			if (BoneName == SpineBone)
			{
				const float BreathingOffset = FMath::Sin(Time * 2.0f * PI / Duration) * 0.5f;
				T.SetTranslation(FVector(0.0f, 0.0f, BreathingOffset));
			}
			else if (BoneName == HeadBone)
			{
				const float HeadBob = FMath::Sin(Time * 1.5f * PI / Duration) * 0.2f;
				const float HeadTilt = FMath::Sin(Time * 2.0f * PI / Duration) * 0.02f;
				T.SetTranslation(FVector(0.0f, 0.0f, HeadBob));
				T.SetRotation(FRotator(HeadTilt, 0.0f, 0.0f).Quaternion());
			}
			else if (BoneName == RootBone)
			{
				T.SetTranslation(FVector(0.0f, 0.0f, 0.0f));
			}

			Transforms.Add(T);
		}

		FAnimationTrackData Track = UAnimationLibrary::CreateTrack(BoneName, Transforms, FrameRate);
		AllTracks.Add(Track);
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		TEXT("ProceduralIdle"),
		Skeleton,
		AllTracks,
		Duration,
		FrameRate);
}

UAnimSequence* UAnimationProceduralGeneration::GenerateWalkCycle(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float StepLength,
	float StepHeight)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const float FrameRate = 30.0f;
	const float CycleDuration = 1.0f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(CycleDuration * FrameRate));
	const float FrameTime = CycleDuration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
	{
		const FName BoneName = RefSkel.GetBoneName(BoneIdx);
		const FString BoneNameStr = BoneName.ToString().ToLower();
		TArray<FTransform> Transforms;

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = Frame * FrameTime;
			const float Phase = Time / CycleDuration * 2.0f * PI;
			FTransform T = FTransform::Identity;

			if (BoneNameStr.Contains(TEXT("root")) || BoneNameStr.Contains(TEXT("pelvis")))
			{
				const float BobAmount = FMath::Abs(FMath::Sin(Phase)) * StepHeight * 0.3f;
				T.SetTranslation(FVector(0.0f, 0.0f, BobAmount));
			}
			else if (BoneNameStr.Contains(TEXT("thigh")) || BoneNameStr.Contains(TEXT("calf")) || BoneNameStr.Contains(TEXT("foot")))
			{
				const float SwingPhase = (BoneNameStr.Contains(TEXT("thigh_l")) || BoneNameStr.Contains(TEXT("calf_l")) || BoneNameStr.Contains(TEXT("foot_l")))
					? Phase : Phase + PI;
				const float Swing = FMath::Sin(SwingPhase) * StepLength * 0.5f;
				const float Lift = FMath::Max(0.0f, FMath::Sin(SwingPhase)) * StepHeight;
				T.SetTranslation(FVector(Swing, 0.0f, Lift));
			}
			else if (BoneNameStr.Contains(TEXT("spine")))
			{
				const float Lean = FMath::Sin(Phase) * 0.03f;
				T.SetRotation(FRotator(0.0f, Lean, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("clavicle")) || BoneNameStr.Contains(TEXT("upperarm")))
			{
				const float ArmSwing = -FMath::Sin(Phase) * 0.3f;
				T.SetRotation(FRotator(ArmSwing, 0.0f, 0.0f).Quaternion());
			}

			Transforms.Add(T);
		}

		FAnimationTrackData Track = UAnimationLibrary::CreateTrack(BoneName, Transforms, FrameRate);
		AllTracks.Add(Track);
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		TEXT("ProceduralWalk"),
		Skeleton,
		AllTracks,
		CycleDuration,
		FrameRate);
}

UAnimSequence* UAnimationProceduralGeneration::GenerateRunCycle(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float StepLength,
	float StepHeight)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const float FrameRate = 30.0f;
	const float CycleDuration = 0.6f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(CycleDuration * FrameRate));
	const float FrameTime = CycleDuration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
	{
		const FName BoneName = RefSkel.GetBoneName(BoneIdx);
		const FString BoneNameStr = BoneName.ToString().ToLower();
		TArray<FTransform> Transforms;

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = Frame * FrameTime;
			const float Phase = Time / CycleDuration * 2.0f * PI;
			FTransform T = FTransform::Identity;

			if (BoneNameStr.Contains(TEXT("root")) || BoneNameStr.Contains(TEXT("pelvis")))
			{
				const float BobAmount = FMath::Abs(FMath::Sin(Phase * 2.0f)) * StepHeight * 0.5f;
				T.SetTranslation(FVector(0.0f, 0.0f, BobAmount));
			}
			else if (BoneNameStr.Contains(TEXT("thigh")) || BoneNameStr.Contains(TEXT("calf")) || BoneNameStr.Contains(TEXT("foot")))
			{
				const float SwingPhase = (BoneNameStr.Contains(TEXT("thigh_l")) || BoneNameStr.Contains(TEXT("calf_l")) || BoneNameStr.Contains(TEXT("foot_l")))
					? Phase : Phase + PI;
				const float Swing = FMath::Sin(SwingPhase) * StepLength * 0.7f;
				const float Lift = FMath::Max(0.0f, FMath::Sin(SwingPhase)) * StepHeight;
				T.SetTranslation(FVector(Swing, 0.0f, Lift));
			}
			else if (BoneNameStr.Contains(TEXT("spine")))
			{
				const float Lean = FMath::Sin(Phase) * 0.05f;
				T.SetRotation(FRotator(0.0f, Lean, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("clavicle")) || BoneNameStr.Contains(TEXT("upperarm")))
			{
				const float ArmSwing = -FMath::Sin(Phase) * 0.5f;
				T.SetRotation(FRotator(ArmSwing, 0.0f, 0.0f).Quaternion());
			}

			Transforms.Add(T);
		}

		FAnimationTrackData Track = UAnimationLibrary::CreateTrack(BoneName, Transforms, FrameRate);
		AllTracks.Add(Track);
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		TEXT("ProceduralRun"),
		Skeleton,
		AllTracks,
		CycleDuration,
		FrameRate);
}

UAnimSequence* UAnimationProceduralGeneration::GenerateBreathing(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float BreathRate)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const float FrameRate = 30.0f;
	const float CycleDuration = 60.0f / FMath::Max(1.0f, BreathRate);
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(CycleDuration * FrameRate));
	const float FrameTime = CycleDuration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
	{
		const FName BoneName = RefSkel.GetBoneName(BoneIdx);
		const FString BoneNameStr = BoneName.ToString().ToLower();
		TArray<FTransform> Transforms;

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = Frame * FrameTime;
			const float Phase = Time / CycleDuration * 2.0f * PI;
			FTransform T = FTransform::Identity;

			if (BoneNameStr.Contains(TEXT("spine_02")) || BoneNameStr.Contains(TEXT("spine_03")))
			{
				const float BreathOffset = FMath::Sin(Phase) * 0.3f;
				T.SetTranslation(FVector(0.0f, 0.0f, BreathOffset));
			}
			else if (BoneNameStr.Contains(TEXT("clavicle_l")) || BoneNameStr.Contains(TEXT("clavicle_r")))
			{
				const float ShoulderMove = FMath::Sin(Phase) * 0.1f;
				T.SetTranslation(FVector(0.0f, 0.0f, ShoulderMove));
			}

			Transforms.Add(T);
		}

		FAnimationTrackData Track = UAnimationLibrary::CreateTrack(BoneName, Transforms, FrameRate);
		AllTracks.Add(Track);
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		TEXT("ProceduralBreathing"),
		Skeleton,
		AllTracks,
		CycleDuration,
		FrameRate);
}

UAnimSequence* UAnimationProceduralGeneration::GenerateHeadBob(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float BobFrequency,
	float BobAmplitude)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const float FrameRate = 30.0f;
	const float Duration = 2.0f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(Duration * FrameRate));
	const float FrameTime = Duration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
	{
		const FName BoneName = RefSkel.GetBoneName(BoneIdx);
		const FString BoneNameStr = BoneName.ToString().ToLower();
		TArray<FTransform> Transforms;

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = Frame * FrameTime;
			FTransform T = FTransform::Identity;

			if (BoneNameStr.Contains(TEXT("head")))
			{
				const float BobZ = FMath::Sin(Time * BobFrequency * 2.0f * PI) * BobAmplitude;
				const float BobY = FMath::Cos(Time * BobFrequency * 2.0f * PI) * BobAmplitude * 0.3f;
				T.SetTranslation(FVector(0.0f, BobY, BobZ));
				const float TiltX = FMath::Sin(Time * BobFrequency * PI) * 0.02f;
				T.SetRotation(FRotator(TiltX, 0.0f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("spine_01")))
			{
				const float SpineBob = FMath::Sin(Time * BobFrequency * 2.0f * PI) * BobAmplitude * 0.3f;
				T.SetTranslation(FVector(0.0f, 0.0f, SpineBob));
			}

			Transforms.Add(T);
		}

		FAnimationTrackData Track = UAnimationLibrary::CreateTrack(BoneName, Transforms, FrameRate);
		AllTracks.Add(Track);
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		TEXT("ProceduralHeadBob"),
		Skeleton,
		AllTracks,
		Duration,
		FrameRate);
}

UAnimSequence* UAnimationProceduralGeneration::GenerateJumpAnimation(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float JumpHeight,
	float Duration)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const float FrameRate = 30.0f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(Duration * FrameRate));
	const float FrameTime = Duration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	const float CrouchEnd = Duration * 0.2f;
	const float LaunchEnd = Duration * 0.4f;
	const float ApexEnd = Duration * 0.65f;

	for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
	{
		const FName BoneName = RefSkel.GetBoneName(BoneIdx);
		const FString BoneNameStr = BoneName.ToString().ToLower();
		TArray<FTransform> Transforms;

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = Frame * FrameTime;
			FTransform T = FTransform::Identity;

			if (BoneNameStr.Contains(TEXT("root")) || BoneNameStr.Contains(TEXT("pelvis")))
			{
				float VerticalOffset = 0.0f;
				if (Time < CrouchEnd)
				{
					const float T01 = Time / CrouchEnd;
					VerticalOffset = -FMath::Lerp(0.0f, JumpHeight * 0.15f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
				}
				else if (Time < LaunchEnd)
				{
					const float T01 = (Time - CrouchEnd) / (LaunchEnd - CrouchEnd);
					VerticalOffset = FMath::Lerp(-JumpHeight * 0.15f, JumpHeight, FMath::InterpEaseOut(0.0f, 1.0f, T01, 2.0f));
				}
				else if (Time < ApexEnd)
				{
					const float T01 = (Time - LaunchEnd) / (ApexEnd - LaunchEnd);
					VerticalOffset = FMath::Lerp(JumpHeight, JumpHeight * 1.05f, FMath::Sin(T01 * PI));
				}
				else
				{
					const float T01 = (Time - ApexEnd) / (Duration - ApexEnd);
					VerticalOffset = FMath::Lerp(JumpHeight * 1.05f, 0.0f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
				}
				T.SetTranslation(FVector(0.0f, 0.0f, VerticalOffset));
			}
			else if (BoneNameStr.Contains(TEXT("thigh")) || BoneNameStr.Contains(TEXT("calf")) || BoneNameStr.Contains(TEXT("foot")))
			{
				const bool bLeft = BoneNameStr.Contains(TEXT("_l"));
				const float PhaseOffset = bLeft ? 0.0f : PI * 0.3f;
				float LegAngle = 0.0f;
				if (Time < CrouchEnd)
				{
					const float T01 = Time / CrouchEnd;
					LegAngle = -FMath::Lerp(0.0f, 0.4f, T01) + PhaseOffset * 0.1f;
				}
				else if (Time < LaunchEnd)
				{
					const float T01 = (Time - CrouchEnd) / (LaunchEnd - CrouchEnd);
					LegAngle = FMath::Lerp(0.4f, -0.5f, T01);
				}
				else if (Time < ApexEnd)
				{
					const float T01 = (Time - LaunchEnd) / (ApexEnd - LaunchEnd);
					LegAngle = FMath::Lerp(-0.5f, -0.3f + (bLeft ? 0.2f : -0.2f), T01);
				}
				else
				{
					const float T01 = (Time - ApexEnd) / (Duration - ApexEnd);
					LegAngle = FMath::Lerp(-0.3f, 0.3f, T01);
				}
				T.SetRotation(FRotator(LegAngle * 57.2958f, 0.0f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("spine")))
			{
				float SpineLean = 0.0f;
				if (Time < CrouchEnd)
				{
					SpineLean = 0.1f;
				}
				else if (Time < LaunchEnd)
				{
					const float T01 = (Time - CrouchEnd) / (LaunchEnd - CrouchEnd);
					SpineLean = FMath::Lerp(0.1f, -0.15f, T01);
				}
				else if (Time < ApexEnd)
				{
					SpineLean = -0.15f;
				}
				else
				{
					const float T01 = (Time - ApexEnd) / (Duration - ApexEnd);
					SpineLean = FMath::Lerp(-0.15f, 0.05f, T01);
				}
				T.SetRotation(FRotator(SpineLean * 57.2958f, 0.0f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("upperarm")) || BoneNameStr.Contains(TEXT("lowerarm")))
			{
				const bool bLeft = BoneNameStr.Contains(TEXT("_l"));
				float ArmAngle = 0.0f;
				if (Time < CrouchEnd)
				{
					const float T01 = Time / CrouchEnd;
					ArmAngle = FMath::Lerp(0.0f, 0.4f, T01);
				}
				else if (Time < LaunchEnd)
				{
					const float T01 = (Time - CrouchEnd) / (LaunchEnd - CrouchEnd);
					ArmAngle = FMath::Lerp(0.4f, -0.6f, T01);
				}
				else if (Time < ApexEnd)
				{
					const float T01 = (Time - LaunchEnd) / (ApexEnd - LaunchEnd);
					ArmAngle = FMath::Lerp(-0.6f, bLeft ? 0.2f : -0.3f, T01);
				}
				else
				{
					const float T01 = (Time - ApexEnd) / (Duration - ApexEnd);
					ArmAngle = FMath::Lerp(bLeft ? 0.2f : -0.3f, 0.0f, T01);
				}
				T.SetRotation(FRotator(ArmAngle * 57.2958f, 0.0f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("neck")) || BoneNameStr.Contains(TEXT("head")))
			{
				float HeadTilt = 0.0f;
				if (Time > LaunchEnd && Time < ApexEnd)
				{
					HeadTilt = -0.05f;
				}
				else if (Time >= ApexEnd)
				{
					const float T01 = (Time - ApexEnd) / (Duration - ApexEnd);
					HeadTilt = FMath::Lerp(-0.05f, 0.1f, T01);
				}
				T.SetRotation(FRotator(HeadTilt * 57.2958f, 0.0f, 0.0f).Quaternion());
			}

			Transforms.Add(T);
		}

		FAnimationTrackData Track = UAnimationLibrary::CreateTrack(BoneName, Transforms, FrameRate);
		AllTracks.Add(Track);
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		TEXT("ProceduralJump"),
		Skeleton,
		AllTracks,
		Duration,
		FrameRate);
}

UAnimSequence* UAnimationProceduralGeneration::GenerateAttackAnimation(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float Duration)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const float FrameRate = 30.0f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(Duration * FrameRate));
	const float FrameTime = Duration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	const float WindUpEnd = Duration * 0.35f;
	const float StrikeEnd = Duration * 0.55f;

	for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
	{
		const FName BoneName = RefSkel.GetBoneName(BoneIdx);
		const FString BoneNameStr = BoneName.ToString().ToLower();
		TArray<FTransform> Transforms;

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = Frame * FrameTime;
			FTransform T = FTransform::Identity;

			if (BoneNameStr.Contains(TEXT("spine")))
			{
				float SpineTwist = 0.0f;
				float SpineLean = 0.0f;
				if (Time < WindUpEnd)
				{
					const float T01 = Time / WindUpEnd;
					SpineTwist = FMath::Lerp(0.0f, -0.3f, FMath::InterpEaseOut(0.0f, 1.0f, T01, 2.0f));
					SpineLean = FMath::Lerp(0.0f, -0.1f, T01);
				}
				else if (Time < StrikeEnd)
				{
					const float T01 = (Time - WindUpEnd) / (StrikeEnd - WindUpEnd);
					SpineTwist = FMath::Lerp(-0.3f, 0.4f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 3.0f));
					SpineLean = FMath::Lerp(-0.1f, 0.2f, T01);
				}
				else
				{
					const float T01 = (Time - StrikeEnd) / (Duration - StrikeEnd);
					SpineTwist = FMath::Lerp(0.4f, 0.0f, FMath::InterpEaseOut(0.0f, 1.0f, T01, 2.0f));
					SpineLean = FMath::Lerp(0.2f, 0.0f, T01);
				}
				T.SetRotation(FRotator(SpineLean * 57.2958f, SpineTwist * 57.2958f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("clavicle_r")) || BoneNameStr.Contains(TEXT("upperarm_r")) || BoneNameStr.Contains(TEXT("lowerarm_r")))
			{
				float ArmBack = 0.0f;
				float ArmForward = 0.0f;
				float ElbowBend = 0.0f;
				if (Time < WindUpEnd)
				{
					const float T01 = Time / WindUpEnd;
					ArmBack = FMath::Lerp(0.0f, -1.2f, FMath::InterpEaseOut(0.0f, 1.0f, T01, 2.0f));
					ElbowBend = FMath::Lerp(0.0f, 1.5f, T01);
				}
				else if (Time < StrikeEnd)
				{
					const float T01 = (Time - WindUpEnd) / (StrikeEnd - WindUpEnd);
					ArmBack = FMath::Lerp(-1.2f, 1.0f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 3.0f));
					ArmForward = FMath::Lerp(0.0f, 1.5f, T01);
					ElbowBend = FMath::Lerp(1.5f, 0.1f, T01);
				}
				else
				{
					const float T01 = (Time - StrikeEnd) / (Duration - StrikeEnd);
					ArmForward = FMath::Lerp(1.5f, 0.0f, FMath::InterpEaseOut(0.0f, 1.0f, T01, 2.0f));
					ElbowBend = FMath::Lerp(0.1f, 0.0f, T01);
				}
				if (BoneNameStr.Contains(TEXT("upperarm")))
				{
					T.SetRotation(FRotator((ArmBack + ArmForward) * 57.2958f, 0.0f, 0.0f).Quaternion());
				}
				else if (BoneNameStr.Contains(TEXT("lowerarm")))
				{
					T.SetRotation(FRotator(ElbowBend * 57.2958f, 0.0f, 0.0f).Quaternion());
				}
			}
			else if (BoneNameStr.Contains(TEXT("clavicle_l")) || BoneNameStr.Contains(TEXT("upperarm_l")) || BoneNameStr.Contains(TEXT("lowerarm_l")))
			{
				float ShieldArm = 0.0f;
				if (Time < WindUpEnd)
				{
					const float T01 = Time / WindUpEnd;
					ShieldArm = FMath::Lerp(0.0f, 0.5f, T01);
				}
				else if (Time < StrikeEnd)
				{
					const float T01 = (Time - WindUpEnd) / (StrikeEnd - WindUpEnd);
					ShieldArm = FMath::Lerp(0.5f, 0.3f, T01);
				}
				else
				{
					const float T01 = (Time - StrikeEnd) / (Duration - StrikeEnd);
					ShieldArm = FMath::Lerp(0.3f, 0.0f, T01);
				}
				T.SetRotation(FRotator(ShieldArm * 57.2958f, 0.0f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("thigh")) || BoneNameStr.Contains(TEXT("calf")))
			{
				const bool bRight = BoneNameStr.Contains(TEXT("_r"));
				float Stance = 0.0f;
				if (Time < WindUpEnd)
				{
					const float T01 = Time / WindUpEnd;
					Stance = FMath::Lerp(0.0f, bRight ? 0.3f : -0.2f, T01);
				}
				else if (Time < StrikeEnd)
				{
					const float T01 = (Time - WindUpEnd) / (StrikeEnd - WindUpEnd);
					Stance = FMath::Lerp(bRight ? 0.3f : -0.2f, bRight ? -0.4f : 0.15f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
				}
				else
				{
					const float T01 = (Time - StrikeEnd) / (Duration - StrikeEnd);
					Stance = FMath::Lerp(bRight ? -0.4f : 0.15f, 0.0f, T01);
				}
				T.SetRotation(FRotator(Stance * 57.2958f, 0.0f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("neck")) || BoneNameStr.Contains(TEXT("head")))
			{
				float HeadLook = 0.0f;
				if (Time < WindUpEnd)
				{
					const float T01 = Time / WindUpEnd;
					HeadLook = FMath::Lerp(0.0f, 0.1f, T01);
				}
				else
				{
					const float T01 = (Time - WindUpEnd) / (Duration - WindUpEnd);
					HeadLook = FMath::Lerp(0.1f, 0.0f, T01);
				}
				T.SetRotation(FRotator(0.0f, HeadLook * 57.2958f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("root")) || BoneNameStr.Contains(TEXT("pelvis")))
			{
				float BodyShift = 0.0f;
				if (Time < WindUpEnd)
				{
					const float T01 = Time / WindUpEnd;
					BodyShift = FMath::Lerp(0.0f, -5.0f, T01);
				}
				else if (Time < StrikeEnd)
				{
					const float T01 = (Time - WindUpEnd) / (StrikeEnd - WindUpEnd);
					BodyShift = FMath::Lerp(-5.0f, 15.0f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
				}
				else
				{
					const float T01 = (Time - StrikeEnd) / (Duration - StrikeEnd);
					BodyShift = FMath::Lerp(15.0f, 0.0f, T01);
				}
				T.SetTranslation(FVector(BodyShift, 0.0f, 0.0f));
			}

			Transforms.Add(T);
		}

		FAnimationTrackData Track = UAnimationLibrary::CreateTrack(BoneName, Transforms, FrameRate);
		AllTracks.Add(Track);
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		TEXT("ProceduralAttack"),
		Skeleton,
		AllTracks,
		Duration,
		FrameRate);
}

UAnimSequence* UAnimationProceduralGeneration::GenerateDeathAnimation(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float Duration)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const float FrameRate = 30.0f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(Duration * FrameRate));
	const float FrameTime = Duration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	const float StaggerEnd = Duration * 0.25f;
	const float FallEnd = Duration * 0.6f;
	const float ImpactEnd = Duration * 0.75f;

	for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
	{
		const FName BoneName = RefSkel.GetBoneName(BoneIdx);
		const FString BoneNameStr = BoneName.ToString().ToLower();
		TArray<FTransform> Transforms;

		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Time = Frame * FrameTime;
			FTransform T = FTransform::Identity;

			if (BoneNameStr.Contains(TEXT("root")) || BoneNameStr.Contains(TEXT("pelvis")))
			{
				float VerticalPos = 0.0f;
				float ForwardShift = 0.0f;
				if (Time < StaggerEnd)
				{
					const float T01 = Time / StaggerEnd;
					VerticalPos = FMath::Lerp(0.0f, -20.0f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
					ForwardShift = FMath::Lerp(0.0f, 10.0f, T01);
				}
				else if (Time < FallEnd)
				{
					const float T01 = (Time - StaggerEnd) / (FallEnd - StaggerEnd);
					VerticalPos = FMath::Lerp(-20.0f, -90.0f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
					ForwardShift = FMath::Lerp(10.0f, 30.0f, T01);
				}
				else if (Time < ImpactEnd)
				{
					const float T01 = (Time - FallEnd) / (ImpactEnd - FallEnd);
					VerticalPos = FMath::Lerp(-90.0f, -95.0f, FMath::Sin(T01 * PI * 0.5f));
					ForwardShift = FMath::Lerp(30.0f, 35.0f, T01);
				}
				else
				{
					const float T01 = (Time - ImpactEnd) / (Duration - ImpactEnd);
					VerticalPos = FMath::Lerp(-95.0f, -90.0f, T01);
					ForwardShift = FMath::Lerp(35.0f, 30.0f, T01);
				}
				T.SetTranslation(FVector(ForwardShift, 0.0f, VerticalPos));
			}
			else if (BoneNameStr.Contains(TEXT("spine")))
			{
				float SpineCollapse = 0.0f;
				if (Time < StaggerEnd)
				{
					const float T01 = Time / StaggerEnd;
					SpineCollapse = FMath::Lerp(0.0f, 0.2f, T01);
				}
				else if (Time < FallEnd)
				{
					const float T01 = (Time - StaggerEnd) / (FallEnd - StaggerEnd);
					SpineCollapse = FMath::Lerp(0.2f, 1.2f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
				}
				else if (Time < ImpactEnd)
				{
					const float T01 = (Time - FallEnd) / (ImpactEnd - FallEnd);
					SpineCollapse = FMath::Lerp(1.2f, 1.5f, FMath::Sin(T01 * PI * 0.5f));
				}
				else
				{
					const float T01 = (Time - ImpactEnd) / (Duration - ImpactEnd);
					SpineCollapse = FMath::Lerp(1.5f, 1.4f, T01);
				}
				T.SetRotation(FRotator(SpineCollapse * 57.2958f, 0.0f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("head")))
			{
				float HeadDrop = 0.0f;
				float HeadTilt = 0.0f;
				if (Time < StaggerEnd)
				{
					const float T01 = Time / StaggerEnd;
					HeadDrop = FMath::Lerp(0.0f, 0.1f, T01);
					HeadTilt = FMath::Lerp(0.0f, 0.05f, T01);
				}
				else if (Time < FallEnd)
				{
					const float T01 = (Time - StaggerEnd) / (FallEnd - StaggerEnd);
					HeadDrop = FMath::Lerp(0.1f, 0.8f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
					HeadTilt = FMath::Lerp(0.05f, 0.6f, T01);
				}
				else
				{
					const float T01 = (Time - FallEnd) / (Duration - FallEnd);
					HeadDrop = FMath::Lerp(0.8f, 0.75f, T01);
					HeadTilt = FMath::Lerp(0.6f, 0.55f, T01);
				}
				T.SetRotation(FRotator((HeadDrop + HeadTilt) * 57.2958f, 0.0f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("upperarm")) || BoneNameStr.Contains(TEXT("lowerarm")) || BoneNameStr.Contains(TEXT("hand")))
			{
				const bool bLeft = BoneNameStr.Contains(TEXT("_l"));
				float ArmDrop = 0.0f;
				float ArmFlail = 0.0f;
				if (Time < StaggerEnd)
				{
					const float T01 = Time / StaggerEnd;
					ArmDrop = FMath::Lerp(0.0f, 0.3f, T01);
					ArmFlail = FMath::Lerp(0.0f, bLeft ? 0.2f : -0.3f, T01);
				}
				else if (Time < FallEnd)
				{
					const float T01 = (Time - StaggerEnd) / (FallEnd - StaggerEnd);
					ArmDrop = FMath::Lerp(0.3f, 1.5f, FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
					ArmFlail = FMath::Lerp(bLeft ? 0.2f : -0.3f, bLeft ? -0.5f : 0.6f, T01);
				}
				else if (Time < ImpactEnd)
				{
					const float T01 = (Time - FallEnd) / (ImpactEnd - FallEnd);
					ArmDrop = FMath::Lerp(1.5f, 1.7f, FMath::Sin(T01 * PI * 0.5f));
					ArmFlail = FMath::Lerp(bLeft ? -0.5f : 0.6f, bLeft ? -0.4f : 0.5f, T01);
				}
				else
				{
					const float T01 = (Time - ImpactEnd) / (Duration - ImpactEnd);
					ArmDrop = FMath::Lerp(1.7f, 1.6f, T01);
					ArmFlail = FMath::Lerp(bLeft ? -0.4f : 0.5f, 0.0f, T01);
				}
				T.SetRotation(FRotator(ArmDrop * 57.2958f, ArmFlail * 57.2958f, 0.0f).Quaternion());
			}
			else if (BoneNameStr.Contains(TEXT("thigh")) || BoneNameStr.Contains(TEXT("calf")) || BoneNameStr.Contains(TEXT("foot")))
			{
				const bool bLeft = BoneNameStr.Contains(TEXT("_l"));
				float LegBuckle = 0.0f;
				if (Time < StaggerEnd)
				{
					const float T01 = Time / StaggerEnd;
					LegBuckle = FMath::Lerp(0.0f, 0.2f, T01);
				}
				else if (Time < FallEnd)
				{
					const float T01 = (Time - StaggerEnd) / (FallEnd - StaggerEnd);
					LegBuckle = FMath::Lerp(0.2f, 1.0f + (bLeft ? 0.1f : 0.0f), FMath::InterpEaseIn(0.0f, 1.0f, T01, 2.0f));
				}
				else if (Time < ImpactEnd)
				{
					const float T01 = (Time - FallEnd) / (ImpactEnd - FallEnd);
					LegBuckle = FMath::Lerp(1.0f + (bLeft ? 0.1f : 0.0f), 1.2f, FMath::Sin(T01 * PI * 0.5f));
				}
				else
				{
					const float T01 = (Time - ImpactEnd) / (Duration - ImpactEnd);
					LegBuckle = FMath::Lerp(1.2f, 1.1f, T01);
				}
				T.SetRotation(FRotator(LegBuckle * 57.2958f, 0.0f, 0.0f).Quaternion());
			}

			Transforms.Add(T);
		}

		FAnimationTrackData Track = UAnimationLibrary::CreateTrack(BoneName, Transforms, FrameRate);
		AllTracks.Add(Track);
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		TEXT("ProceduralDeath"),
		Skeleton,
		AllTracks,
		Duration,
		FrameRate);
}

UAnimSequence* UAnimationProceduralGeneration::CombineProceduralAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	const TArray<float>& Weights)
{
	if (!WorldContextObject || Animations.Num() == 0)
	{
		return nullptr;
	}

	USkeleton* Skeleton = nullptr;
	for (UAnimSequence* Anim : Animations)
	{
		if (Anim && Anim->GetSkeleton())
		{
			Skeleton = Anim->GetSkeleton();
			break;
		}
	}

	if (!Skeleton)
	{
		return nullptr;
	}

	TArray<float> SafeWeights = Weights;
	while (SafeWeights.Num() < Animations.Num())
	{
		SafeWeights.Add(1.0f);
	}

	float MaxDuration = 0.0f;
	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		if (Animations[i])
		{
			MaxDuration = FMath::Max(MaxDuration, Animations[i]->GetPlayLength());
		}
	}

	return UAnimationLibrary::BlendAnimations(
		WorldContextObject,
		TEXT("CombinedProcedural"),
		Skeleton,
		Animations,
		SafeWeights);
}

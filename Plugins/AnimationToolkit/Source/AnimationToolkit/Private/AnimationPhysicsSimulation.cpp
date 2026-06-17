#include "AnimationPhysicsSimulation.h"
#include "AnimationLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

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

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	const float FrameRate = 30.0f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(SimulationTime * FrameRate));
	const float FrameTime = SimulationTime / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	TMap<FName, FPhysicsBoneSettings> BoneSettingsMap;
	for (const FPhysicsBoneSettings& Settings : BoneSettings)
	{
		BoneSettingsMap.Add(Settings.BoneName, Settings);
	}

	TMap<FName, FVector> BoneVelocities;
	TMap<FName, FVector> BonePositions;

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const float Time = Frame * FrameTime;

		for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
		{
			const FName BoneName = RefSkel.GetBoneName(BoneIdx);

			FTransform OriginalTransform = UAnimationLibrary::GetBoneTransformAtTime(
				SourceAnimation, BoneName, Time, false);

			const FPhysicsBoneSettings* Settings = BoneSettingsMap.Find(BoneName);

			if (Settings && Settings->bSimulatePhysics)
			{
				const float Mass = FMath::Max(0.001f, Settings->Mass);
				const float Damping = FMath::Clamp(Settings->Damping, 0.0f, 10.0f);
				const float Stiffness = FMath::Max(0.0f, Settings->Stiffness);

				FVector& Velocity = BoneVelocities.FindOrAdd(BoneName, FVector::ZeroVector);
				FVector& SimPosition = BonePositions.FindOrAdd(BoneName, OriginalTransform.GetTranslation());

				const FVector RestPosition = OriginalTransform.GetTranslation();
				const FVector Gravity(0.0f, 0.0f, -980.0f);

				const FVector GravityForce = Gravity * Mass;
				const FVector SpringForce = (RestPosition - SimPosition) * Stiffness;
				const FVector DampingForce = -Velocity * Damping * Mass;
				const FVector TotalForce = GravityForce + SpringForce + DampingForce;

				const FVector Acceleration = TotalForce / Mass;
				Velocity += Acceleration * FrameTime;
				SimPosition += Velocity * FrameTime;

				OriginalTransform.SetTranslation(SimPosition);
			}

			if (Frame == 0)
			{
				FAnimationTrackData& Track = AllTracks.AddDefaulted_GetRef();
				Track.TrackName = BoneName;
			}

			FAnimationKeyframe& KF = AllTracks[BoneIdx].Keyframes.AddDefaulted_GetRef();
			KF.Time = Time;
			KF.Transform = OriginalTransform;
		}
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		SourceAnimation->GetName() + TEXT("_PhysicsSim"),
		Skeleton,
		AllTracks,
		SimulationTime,
		FrameRate);
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

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	const float Duration = SourceAnimation->GetPlayLength();
	const float FrameRate = 30.0f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(Duration * FrameRate));
	const float FrameTime = Duration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	FVector Velocity = FVector::ZeroVector;

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const float Time = Frame * FrameTime;

		for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
		{
			const FName CurrentBone = RefSkel.GetBoneName(BoneIdx);

			FTransform T = UAnimationLibrary::GetBoneTransformAtTime(
				SourceAnimation, CurrentBone, Time, false);

			if (CurrentBone == BoneName)
			{
				const FVector GravityAccel(0.0f, 0.0f, -GravityStrength);
				Velocity += GravityAccel * FrameTime;

				const FVector GravityOffset = Velocity * Time;
				const float MaxFall = 200.0f;
				const FVector ClampedOffset = FVector(
					GravityOffset.X,
					GravityOffset.Y,
					FMath::Clamp(GravityOffset.Z, -MaxFall, MaxFall));

				T.AddToTranslation(ClampedOffset);
			}

			if (Frame == 0)
			{
				FAnimationTrackData& Track = AllTracks.AddDefaulted_GetRef();
				Track.TrackName = CurrentBone;
			}

			FAnimationKeyframe& KF = AllTracks[BoneIdx].Keyframes.AddDefaulted_GetRef();
			KF.Time = Time;
			KF.Transform = T;
		}
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		SourceAnimation->GetName() + TEXT("_Gravity"),
		Skeleton,
		AllTracks,
		Duration,
		FrameRate);
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

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	if (!Skeleton)
	{
		return nullptr;
	}

	const float Duration = SourceAnimation->GetPlayLength();
	const float FrameRate = 30.0f;
	const int32 NumFrames = FMath::Max(2, FMath::CeilToInt(Duration * FrameRate));
	const float FrameTime = Duration / (NumFrames - 1);
	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();

	TArray<FAnimationTrackData> AllTracks;

	FVector SpringVelocity = FVector::ZeroVector;
	FVector SpringDisplacement = FVector(0.0f, 0.0f, 10.0f);

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const float Time = Frame * FrameTime;

		for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
		{
			const FName CurrentBone = RefSkel.GetBoneName(BoneIdx);

			FTransform T = UAnimationLibrary::GetBoneTransformAtTime(
				SourceAnimation, CurrentBone, Time, false);

			if (CurrentBone == BoneName)
			{
				const FVector SpringForce = -SpringStiffness * SpringDisplacement;
				const FVector DampingForce = -SpringDamping * SpringVelocity;
				const FVector TotalForce = SpringForce + DampingForce;

				SpringVelocity += TotalForce * FrameTime;
				SpringDisplacement += SpringVelocity * FrameTime;

				T.AddToTranslation(SpringDisplacement);
			}

			if (Frame == 0)
			{
				FAnimationTrackData& Track = AllTracks.AddDefaulted_GetRef();
				Track.TrackName = CurrentBone;
			}

			FAnimationKeyframe& KF = AllTracks[BoneIdx].Keyframes.AddDefaulted_GetRef();
			KF.Time = Time;
			KF.Transform = T;
		}
	}

	return UAnimationLibrary::CreateAnimation(
		WorldContextObject,
		SourceAnimation->GetName() + TEXT("_Spring"),
		Skeleton,
		AllTracks,
		Duration,
		FrameRate);
}

FVector UAnimationPhysicsSimulation::CalculatePhysicsForce(
	const FVector& Position,
	const FVector& Velocity,
	float Mass,
	float Gravity)
{
	const FVector GravitationalForce(0.0f, 0.0f, -Mass * Gravity);
	const FVector DragCoefficient = FVector(0.1f, 0.1f, 0.1f);
	const FVector DragForce = -Velocity * DragCoefficient * Mass;
	return GravitationalForce + DragForce;
}

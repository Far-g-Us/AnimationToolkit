#include "AnimationProceduralNodes.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimTypes.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "ReferenceSkeleton.h"

static void CopyBoneTracksToAnim(UAnimSequence* DestAnim, UAnimSequence* SrcAnim)
{
	const IAnimationDataModel* SrcModel = SrcAnim->GetDataModel();
	if (!SrcModel) return;

	IAnimationDataController& Controller = DestAnim->GetController();
	const TArray<FBoneAnimationTrack>& Tracks = SrcModel->GetBoneAnimationTracks();

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		Controller.AddBoneCurve(Track.Name, false);
		const FRawAnimSequenceTrack& Raw = Track.InternalTrackData;
		Controller.SetBoneTrackKeys(Track.Name, Raw.PosKeys, Raw.RotKeys, Raw.ScaleKeys, false);
	}
}

UAnimSequence* UAnimationProceduralNodes::ApplyTwoBoneIK(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	FVector EffectorTarget,
	FVector PoleTarget,
	float IKWeight)
{
	if (!SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	const FReferenceSkeleton& RefSkeleton = SourceAnimation->GetSkeleton()->GetReferenceSkeleton();
	const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE) return nullptr;

	const int32 ParentIndex = RefSkeleton.GetParentIndex(BoneIndex);
	if (ParentIndex == INDEX_NONE) return nullptr;

	const int32 GrandParentIndex = RefSkeleton.GetParentIndex(ParentIndex);
	if (GrandParentIndex == INDEX_NONE) return nullptr;

	const FVector UpperArmPos = RefSkeleton.GetRefBonePose()[GrandParentIndex].GetTranslation();
	const FVector LowerArmPos = RefSkeleton.GetRefBonePose()[ParentIndex].GetTranslation();
	const FVector HandPos = RefSkeleton.GetRefBonePose()[BoneIndex].GetTranslation();

	const float UpperArmLength = FVector::Dist(UpperArmPos, LowerArmPos);
	const float LowerArmLength = FVector::Dist(LowerArmPos, HandPos);

	FVector OutUpperArmNew, OutLowerArmNew;
	SolveTwoBoneIK(UpperArmPos, LowerArmPos, HandPos, EffectorTarget, PoleTarget,
		UpperArmLength, LowerArmLength, OutUpperArmNew, OutLowerArmNew);

	UAnimSequence* Result = DuplicateObject<UAnimSequence>(SourceAnimation, SourceAnimation->GetOuter());
	if (!Result) return nullptr;

	IAnimationDataController& Controller = Result->GetController();
	const IAnimationDataModel* Model = SourceAnimation->GetDataModel();
	if (!Model) return nullptr;

	const FName GrandParentName = RefSkeleton.GetBoneName(GrandParentIndex);
	const FName ParentName = RefSkeleton.GetBoneName(ParentIndex);

	const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		Controller.AddBoneCurve(Track.Name, false);
		FRawAnimSequenceTrack NewRaw = Track.InternalTrackData;

		if (Track.Name == GrandParentName && NewRaw.PosKeys.Num() > 0)
		{
			const FVector Delta = (OutUpperArmNew - UpperArmPos) * IKWeight;
			for (FVector3f& Pos : NewRaw.PosKeys)
			{
				Pos = FVector3f(FVector(Pos) + Delta);
			}
		}
		else if (Track.Name == ParentName && NewRaw.PosKeys.Num() > 0)
		{
			const FVector Delta = (OutLowerArmNew - LowerArmPos) * IKWeight;
			for (FVector3f& Pos : NewRaw.PosKeys)
			{
				Pos = FVector3f(FVector(Pos) + Delta);
			}
		}

		Controller.SetBoneTrackKeys(Track.Name, NewRaw.PosKeys, NewRaw.RotKeys, NewRaw.ScaleKeys, false);
	}

	Result->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("ProceduralNodes: Applied TwoBoneIK to '%s' on bone '%s'"), *SourceAnimation->GetName(), *BoneName.ToString());
	return Result;
}

UAnimSequence* UAnimationProceduralNodes::ApplyFBIK(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const TArray<FIKBoneTarget>& Targets,
	int32 Iterations)
{
	if (!SourceAnimation || Targets.Num() == 0 || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	UAnimSequence* Result = DuplicateObject<UAnimSequence>(SourceAnimation, SourceAnimation->GetOuter());
	if (!Result) return nullptr;

	const FReferenceSkeleton& RefSkeleton = SourceAnimation->GetSkeleton()->GetReferenceSkeleton();
	IAnimationDataController& Controller = Result->GetController();
	const IAnimationDataModel* Model = SourceAnimation->GetDataModel();
	if (!Model) return nullptr;

	TMap<FName, FVector3f> BoneDeltas;

	for (int32 Iter = 0; Iter < Iterations; ++Iter)
	{
		for (const FIKBoneTarget& Target : Targets)
		{
			const int32 BoneIndex = RefSkeleton.FindBoneIndex(Target.BoneName);
			if (BoneIndex == INDEX_NONE || Target.Weight <= 0.0f) continue;

			const FVector CurrentPos = RefSkeleton.GetRefBonePose()[BoneIndex].GetTranslation();
			const FVector Delta = (Target.TargetLocation - CurrentPos) * Target.Weight;

			int32 Walk = BoneIndex;
			int32 Dist = 0;
			while (Walk != INDEX_NONE)
			{
				const FName WalkName = RefSkeleton.GetBoneName(Walk);
				const float Influence = FMath::Max(0.0f, 1.0f - Dist * 0.25f) * Target.Weight;
				FVector3f& ExistingDelta = BoneDeltas.FindOrAdd(WalkName);
				ExistingDelta = ExistingDelta + FVector3f(Delta * Influence);
				Walk = RefSkeleton.GetParentIndex(Walk);
				++Dist;
			}
		}
	}

	const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		Controller.AddBoneCurve(Track.Name, false);
		FRawAnimSequenceTrack NewRaw = Track.InternalTrackData;

		const FVector3f* Delta = BoneDeltas.Find(Track.Name);
		if (Delta && !Delta->IsNearlyZero() && NewRaw.PosKeys.Num() > 0)
		{
			for (FVector3f& Pos : NewRaw.PosKeys)
			{
				Pos = Pos + *Delta;
			}
		}

		Controller.SetBoneTrackKeys(Track.Name, NewRaw.PosKeys, NewRaw.RotKeys, NewRaw.ScaleKeys, false);
	}

	Result->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("ProceduralNodes: Applied FBIK to '%s' (%d targets, %d iterations)"),
		*SourceAnimation->GetName(), Targets.Num(), Iterations);
	return Result;
}

UAnimSequence* UAnimationProceduralNodes::ApplyAimOffset(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	FRotator AimOffset,
	float AimOffsetWeight)
{
	if (!SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	const FReferenceSkeleton& RefSkeleton = SourceAnimation->GetSkeleton()->GetReferenceSkeleton();
	const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE) return nullptr;

	const FQuat AimRotation = (AimOffset * AimOffsetWeight).Quaternion();

	UAnimSequence* Result = DuplicateObject<UAnimSequence>(SourceAnimation, SourceAnimation->GetOuter());
	if (!Result) return nullptr;

	IAnimationDataController& Controller = Result->GetController();
	const IAnimationDataModel* Model = SourceAnimation->GetDataModel();
	if (!Model) return nullptr;

	const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		Controller.AddBoneCurve(Track.Name, false);
		FRawAnimSequenceTrack NewRaw = Track.InternalTrackData;

		if (Track.Name == BoneName && NewRaw.RotKeys.Num() > 0)
		{
			for (FQuat4f& Rot : NewRaw.RotKeys)
			{
				Rot = FQuat4f(AimRotation * FQuat(Rot));
			}
		}

		Controller.SetBoneTrackKeys(Track.Name, NewRaw.PosKeys, NewRaw.RotKeys, NewRaw.ScaleKeys, false);
	}

	Result->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("ProceduralNodes: Applied AimOffset to '%s' bone '%s' (pitch=%.1f, yaw=%.1f)"),
		*SourceAnimation->GetName(), *BoneName.ToString(), AimOffset.Pitch, AimOffset.Yaw);
	return Result;
}

UAnimSequence* UAnimationProceduralNodes::ApplySpringPhysics(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	float SpringStiffness,
	float SpringDamping,
	float SpringMass)
{
	if (!SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	const FReferenceSkeleton& RefSkeleton = SourceAnimation->GetSkeleton()->GetReferenceSkeleton();
	const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE) return nullptr;

	const IAnimationDataModel* Model = SourceAnimation->GetDataModel();
	if (!Model) return nullptr;

	UAnimSequence* Result = DuplicateObject<UAnimSequence>(SourceAnimation, SourceAnimation->GetOuter());
	if (!Result) return nullptr;

	IAnimationDataController& Controller = Result->GetController();

	const FVector RestPos = RefSkeleton.GetRefBonePose()[BoneIndex].GetTranslation();
	const float Duration = Model->GetPlayLength();
	const int32 NumFrames = Model->GetNumberOfFrames();
	const float DeltaTime = (NumFrames > 1) ? Duration / (NumFrames - 1) : 0.016f;

	const TArray<FBoneAnimationTrack>& Tracks = Model->GetBoneAnimationTracks();

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		Controller.AddBoneCurve(Track.Name, false);
		FRawAnimSequenceTrack NewRaw = Track.InternalTrackData;

		if (Track.Name == BoneName && NewRaw.PosKeys.Num() > 1)
		{
			FVector Velocity = FVector::ZeroVector;

			for (int32 i = 0; i < NewRaw.PosKeys.Num(); ++i)
			{
				FVector CurrentPos = FVector(NewRaw.PosKeys[i]);
				const FVector SpringForce = CalculateSpringForce(CurrentPos, RestPos, Velocity,
					SpringStiffness, SpringDamping, SpringMass, DeltaTime);
				Velocity = Velocity + (SpringForce / SpringMass) * DeltaTime;
				CurrentPos = CurrentPos + Velocity * DeltaTime;
				NewRaw.PosKeys[i] = FVector3f(CurrentPos);
			}
		}

		Controller.SetBoneTrackKeys(Track.Name, NewRaw.PosKeys, NewRaw.RotKeys, NewRaw.ScaleKeys, false);
	}

	Result->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("ProceduralNodes: Applied SpringPhysics to '%s' bone '%s' (k=%.1f, d=%.1f, m=%.1f)"),
		*SourceAnimation->GetName(), *BoneName.ToString(), SpringStiffness, SpringDamping, SpringMass);
	return Result;
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
	const float TotalLength = UpperArmLength + LowerArmLength;
	const FVector ShoulderToTarget = EffectorTarget - UpperArmPos;
	const float DistanceToTarget = ShoulderToTarget.Size();

	if (DistanceToTarget < KINDA_SMALL_NUMBER)
	{
		OutUpperArmNew = UpperArmPos;
		OutLowerArmNew = UpperArmPos + FVector(UpperArmLength, 0.0f, 0.0f);
		return OutLowerArmNew;
	}

	const float ClampedDist = FMath::Clamp(DistanceToTarget, FMath::Abs(UpperArmLength - LowerArmLength) + KINDA_SMALL_NUMBER, TotalLength - KINDA_SMALL_NUMBER);

	const float CosAngle = (UpperArmLength * UpperArmLength + ClampedDist * ClampedDist - LowerArmLength * LowerArmLength) / (2.0f * UpperArmLength * ClampedDist);
	const float ClampedCos = FMath::Clamp(CosAngle, -1.0f, 1.0f);
	const float AngleAtShoulder = FMath::Acos(ClampedCos);

	const FVector ShoulderToTargetDir = ShoulderToTarget.GetSafeNormal();
	const FVector PoleDir = (PoleTarget - UpperArmPos).GetSafeNormal();
	const FVector PlaneNormal = FVector::CrossProduct(ShoulderToTargetDir, PoleDir).GetSafeNormal();

	FVector RotationAxis = FVector::CrossProduct(ShoulderToTargetDir, PlaneNormal).GetSafeNormal();
	if (RotationAxis.IsNearlyZero())
	{
		RotationAxis = FVector::CrossProduct(ShoulderToTargetDir, FVector::UpVector).GetSafeNormal();
		if (RotationAxis.IsNearlyZero())
			RotationAxis = FVector::CrossProduct(ShoulderToTargetDir, FVector::RightVector).GetSafeNormal();
	}

	const FQuat ShoulderRotation = FQuat(RotationAxis, AngleAtShoulder);
	const FVector UpperArmDir = ShoulderRotation.RotateVector(ShoulderToTargetDir);
	OutUpperArmNew = UpperArmPos + UpperArmDir * UpperArmLength;

	const FVector ElbowToTarget = EffectorTarget - OutUpperArmNew;
	const float ElbowDist = ElbowToTarget.Size();

	float AngleAtElbow = 0.0f;
	if (ElbowDist > KINDA_SMALL_NUMBER)
	{
		const float CosElbow = (UpperArmLength * UpperArmLength + LowerArmLength * LowerArmLength - ClampedDist * ClampedDist) / (2.0f * UpperArmLength * LowerArmLength);
		AngleAtElbow = UE_PI - FMath::Acos(FMath::Clamp(CosElbow, -1.0f, 1.0f));
	}

	const FVector ElbowDir = ElbowToTarget.GetSafeNormal();
	const FVector ElbowNormal = FVector::CrossProduct(UpperArmDir, ElbowDir).GetSafeNormal();

	FVector ElbowAxis = FVector::CrossProduct(UpperArmDir, ElbowNormal).GetSafeNormal();
	if (ElbowAxis.IsNearlyZero())
	{
		ElbowAxis = FVector::CrossProduct(UpperArmDir, FVector::UpVector).GetSafeNormal();
		if (ElbowAxis.IsNearlyZero())
			ElbowAxis = FVector::CrossProduct(UpperArmDir, FVector::RightVector).GetSafeNormal();
	}

	const FQuat ElbowRotation = FQuat(ElbowAxis, AngleAtElbow);
	const FVector LowerArmDir = ElbowRotation.RotateVector(-UpperArmDir);
	OutLowerArmNew = OutUpperArmNew + LowerArmDir * LowerArmLength;

	return OutLowerArmNew;
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
	const FVector Displacement = CurrentPos - TargetPos;
	return -Stiffness * Displacement - Damping * Velocity;
}

FRotator UAnimationProceduralNodes::CalculateAimOffsetRotation(
	const FVector& CurrentDirection,
	const FVector& AimDirection,
	float MaxPitch,
	float MaxYaw)
{
	if (CurrentDirection.IsNearlyZero() || AimDirection.IsNearlyZero())
	{
		return FRotator::ZeroRotator;
	}

	const FQuat CurrentQuat = FQuat::FindBetweenVectors(FVector::ForwardVector, CurrentDirection.GetSafeNormal());
	const FQuat AimQuat = FQuat::FindBetweenVectors(FVector::ForwardVector, AimDirection.GetSafeNormal());
	const FQuat DeltaRotation = AimQuat * CurrentQuat.Inverse();

	FRotator DeltaRot = DeltaRotation.Rotator();
	DeltaRot.Pitch = FMath::Clamp(DeltaRot.Pitch, -MaxPitch, MaxPitch);
	DeltaRot.Yaw = FMath::Clamp(DeltaRot.Yaw, -MaxYaw, MaxYaw);
	DeltaRot.Roll = 0.0f;

	return DeltaRot;
}

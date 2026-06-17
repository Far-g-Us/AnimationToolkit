#include "AnimationProceduralNodes.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"

UAnimSequence* UAnimationProceduralNodes::ApplyTwoBoneIK(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	FVector EffectorTarget,
	FVector PoleTarget,
	float IKWeight)
{
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	int32 HandIdx = RefSkeleton.FindBoneIndex(BoneName);
	if (HandIdx == INDEX_NONE)
	{
		return nullptr;
	}

	int32 LowerArmIdx = RefSkeleton.GetParentIndex(HandIdx);
	int32 UpperArmIdx = RefSkeleton.GetParentIndex(LowerArmIdx);

	if (LowerArmIdx == INDEX_NONE || UpperArmIdx == INDEX_NONE)
	{
		return nullptr;
	}

	FString NewName = FString::Printf(TEXT("%s_IK"), *SourceAnimation->GetName());

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	float UpperArmLength = FVector::Dist(
		RefSkeleton.GetRefPosePoseArray[UpperArmIdx].GetLocation(),
		RefSkeleton.GetRefPosePoseArray[LowerArmIdx].GetLocation()
	);

	float LowerArmLength = FVector::Dist(
		RefSkeleton.GetRefPosePoseArray[LowerArmIdx].GetLocation(),
		RefSkeleton.GetRefPosePoseArray[HandIdx].GetLocation()
	);

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName CurrentBoneName = RefSkeleton.GetBoneName(BoneIdx);
		int32 SourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(CurrentBoneName);

		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform SourceTransform = (SourceBoneIdx != INDEX_NONE) ?
				SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame) : FTransform::Identity;

			FVector FinalLocation = SourceTransform.GetLocation();

			if (BoneIdx == UpperArmIdx || BoneIdx == LowerArmIdx || BoneIdx == HandIdx)
			{
				FTransform UpperArmTransform = SourceAnimation->GetBoneTransform(UpperArmIdx, Frame);
				FVector UpperArmPos = UpperArmTransform.GetLocation();
				FVector HandPos = SourceTransform.GetLocation();

				FVector NewUpperArm, NewLowerArm;
				SolveTwoBoneIK(UpperArmPos, SourceAnimation->GetBoneTransform(LowerArmIdx, Frame).GetLocation(),
					HandPos, EffectorTarget, PoleTarget, UpperArmLength, LowerArmLength,
					NewUpperArm, NewLowerArm);

				if (BoneIdx == UpperArmIdx)
				{
					FinalLocation = FMath::Lerp(HandPos, NewUpperArm, IKWeight);
				}
				else if (BoneIdx == LowerArmIdx)
				{
					FinalLocation = FMath::Lerp(HandPos, NewLowerArm, IKWeight);
				}
				else if (BoneIdx == HandIdx)
				{
					FinalLocation = FMath::Lerp(HandPos, EffectorTarget, IKWeight);
				}
			}

			DestTrack.PosKeys[Frame] = FinalLocation;
			DestTrack.RotKeys[Frame] = SourceTransform.GetRotation();
			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
}

UAnimSequence* UAnimationProceduralNodes::ApplyFBIK(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	const TArray<FIKBoneTarget>& Targets,
	int32 Iterations)
{
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	FString NewName = FString::Printf(TEXT("%s_FBIK"), *SourceAnimation->GetName());

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);
	}

	TArray<int32> SourceBoneIndices;
	SourceBoneIndices.SetNum(RefSkeleton.GetNum());
	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		SourceBoneIndices[BoneIdx] = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(
			RefSkeleton.GetBoneName(BoneIdx));
	}

	for (int32 Frame = 0; Frame < FrameCount; Frame++)
	{
		TArray<FVector> BonePositions;
		BonePositions.SetNum(RefSkeleton.GetNum());

		for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
		{
			int32 SourceBoneIdx = SourceBoneIndices[BoneIdx];

			if (SourceBoneIdx != INDEX_NONE)
			{
				BonePositions[BoneIdx] = SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame).GetLocation();
			}
		}

		for (int32 Iter = 0; Iter < Iterations; Iter++)
		{
			for (const FIKBoneTarget& Target : Targets)
			{
				int32 BoneIdx = RefSkeleton.FindBoneIndex(Target.BoneName);
				if (BoneIdx != INDEX_NONE && BonePositions.IsValidIndex(BoneIdx))
				{
					BonePositions[BoneIdx] = FMath::Lerp(BonePositions[BoneIdx],
						Target.TargetLocation, Target.Weight);
				}
			}
		}

		for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
		{
			int32 SourceBoneIdx = SourceBoneIndices[BoneIdx];

			FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];

			FTransform SourceTransform = (SourceBoneIdx != INDEX_NONE) ?
				SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame) : FTransform::Identity;

			DestTrack.PosKeys[Frame] = BonePositions[BoneIdx];
			DestTrack.RotKeys[Frame] = SourceTransform.GetRotation();
			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
}

UAnimSequence* UAnimationProceduralNodes::ApplyAimOffset(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	FRotator AimOffset,
	float AimOffsetWeight)
{
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	FString NewName = FString::Printf(TEXT("%s_AimOffset"), *SourceAnimation->GetName());

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BoneIdx = 0; BoneIdx < RefSkeleton.GetNum(); BoneIdx++)
	{
		FName CurrentBoneName = RefSkeleton.GetBoneName(BoneIdx);
		int32 SourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(CurrentBoneName);

		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BoneIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);

		for (int32 Frame = 0; Frame < FrameCount; Frame++)
		{
			FTransform SourceTransform = (SourceBoneIdx != INDEX_NONE) ?
				SourceAnimation->GetBoneTransform(SourceBoneIdx, Frame) : FTransform::Identity;

			DestTrack.PosKeys[Frame] = SourceTransform.GetLocation();
			DestTrack.ScaleKeys[Frame] = SourceTransform.GetScale3D();

			if (CurrentBoneName == BoneName)
			{
				FQuat OriginalRotation = SourceTransform.GetRotation();
				FQuat AimRotation = AimOffset.Quaternion();
				FQuat BlendedRotation = FQuat::Slerp(OriginalRotation,
					OriginalRotation * AimRotation, AimOffsetWeight);
				DestTrack.RotKeys[Frame] = BlendedRotation;
			}
			else
			{
				DestTrack.RotKeys[Frame] = SourceTransform.GetRotation();
			}
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
}

UAnimSequence* UAnimationProceduralNodes::ApplySpringPhysics(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	FName BoneName,
	float SpringStiffness,
	float SpringDamping,
	float SpringMass)
{
	if (!WorldContextObject || !SourceAnimation || !SourceAnimation->GetSkeleton())
	{
		return nullptr;
	}

	USkeleton* Skeleton = SourceAnimation->GetSkeleton();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	int32 BoneIdx = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIdx == INDEX_NONE)
	{
		return nullptr;
	}

	FString NewName = FString::Printf(TEXT("%s_Spring"), *SourceAnimation->GetName());

	UAnimSequence* ResultAnim = NewObject<UAnimSequence>(
		GetTransientPackage(),
		*NewName,
		RF_Public | RF_Standalone
	);

	if (!ResultAnim)
	{
		return nullptr;
	}

	ResultAnim->SetSkeleton(Skeleton);

	int32 FrameCount = SourceAnimation->GetNumberOfSampledKeys();
	ResultAnim->GetEditableRawAnimationData().Tracks.SetNum(RefSkeleton.GetNum());

	for (int32 BIdx = 0; BIdx < RefSkeleton.GetNum(); BIdx++)
	{
		FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BIdx];
		DestTrack.PosKeys.SetNum(FrameCount);
		DestTrack.RotKeys.SetNum(FrameCount);
		DestTrack.ScaleKeys.SetNum(FrameCount);
	}

	float Duration = SourceAnimation->GetSequenceLength();
	float DeltaTime = (FrameCount > 1) ? Duration / (FrameCount - 1) : 0.0f;

	FVector SpringVelocity = FVector::ZeroVector;
	int32 SpringSourceBoneIdx = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);

	TArray<int32> SpringSourceBoneIndices;
	SpringSourceBoneIndices.SetNum(RefSkeleton.GetNum());
	for (int32 BIdx = 0; BIdx < RefSkeleton.GetNum(); BIdx++)
	{
		SpringSourceBoneIndices[BIdx] = SourceAnimation->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(
			RefSkeleton.GetBoneName(BIdx));
	}

	for (int32 Frame = 0; Frame < FrameCount; Frame++)
	{
		FTransform SourceTransform = (SpringSourceBoneIdx != INDEX_NONE) ?
			SourceAnimation->GetBoneTransform(SpringSourceBoneIdx, Frame) : FTransform::Identity;

		FVector TargetPos = SourceTransform.GetLocation();
		FVector CurrentPos = TargetPos + SpringVelocity * DeltaTime;

		FVector SpringForce = CalculateSpringForce(CurrentPos, TargetPos, SpringVelocity,
			SpringStiffness, SpringDamping, SpringMass, DeltaTime);

		SpringVelocity += SpringForce * DeltaTime;
		CurrentPos += SpringVelocity * DeltaTime;

		for (int32 BIdx = 0; BIdx < RefSkeleton.GetNum(); BIdx++)
		{
			int32 SrcBoneIdx = SpringSourceBoneIndices[BIdx];

			FRawAnimSequenceTrack& DestTrack = ResultAnim->GetEditableRawAnimationData().Tracks[BIdx];

			FTransform SrcTransform = (SrcBoneIdx != INDEX_NONE) ?
				SourceAnimation->GetBoneTransform(SrcBoneIdx, Frame) : FTransform::Identity;

			DestTrack.PosKeys[Frame] = (BIdx == BoneIdx) ? CurrentPos : SrcTransform.GetLocation();
			DestTrack.RotKeys[Frame] = SrcTransform.GetRotation();
			DestTrack.ScaleKeys[Frame] = SrcTransform.GetScale3D();
		}
	}

	ResultAnim->MarkPackageDirty();
	return ResultAnim;
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
	FVector Direction = EffectorTarget - UpperArmPos;
	float Distance = Direction.Size();

	float TotalLength = UpperArmLength + LowerArmLength;
	FVector ClampedTarget = UpperArmPos + Direction.GetSafeNormal() * FMath::Min(Distance, TotalLength * 0.99f);

	FVector ToTarget = ClampedTarget - UpperArmPos;
	FVector ToPole = PoleTarget - UpperArmPos;

	FVector PlaneNormal = FVector::CrossProduct(ToTarget, ToPole).GetSafeNormal();
	FVector ToPoleProject = FVector::CrossProduct(PlaneNormal, ToTarget).GetSafeNormal();

	float CosAngle = (UpperArmLength * UpperArmLength + ToTarget.SizeSquared() - LowerArmLength * LowerArmLength) /
		(2.0f * UpperArmLength * ToTarget.Size());
	CosAngle = FMath::Clamp(CosAngle, -1.0f, 1.0f);
	float Angle = FMath::Acos(CosAngle);

	FVector UpperArmDir = ToTarget.GetSafeNormal();
	FVector LowerArmDir = UpperArmDir * FMath::Cos(Angle) + ToPoleProject * FMath::Sin(Angle);

	OutUpperArmNew = UpperArmPos + LowerArmDir * UpperArmLength;
	OutLowerArmNew = OutUpperArmNew + (ClampedTarget - OutUpperArmNew).GetSafeNormal() * LowerArmLength;

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
	FVector Displacement = TargetPos - CurrentPos;
	FVector SpringForce = Displacement * Stiffness;
	FVector DampingForce = -Velocity * Damping;

	return (SpringForce + DampingForce) / Mass;
}

FRotator UAnimationProceduralNodes::CalculateAimOffsetRotation(
	const FVector& CurrentDirection,
	const FVector& AimDirection,
	float MaxPitch,
	float MaxYaw)
{
	FVector NormalizedCurrent = CurrentDirection.GetSafeNormal();
	FVector NormalizedAim = AimDirection.GetSafeNormal();

	float PitchAngle = FMath::RadiansToDegrees(FMath::Atan2(NormalizedAim.Z,
		FMath::Sqrt(NormalizedAim.X * NormalizedAim.X + NormalizedAim.Y * NormalizedAim.Y)));

	float YawAngle = FMath::RadiansToDegrees(FMath::Atan2(NormalizedAim.Y, NormalizedAim.X));

	PitchAngle = FMath::Clamp(PitchAngle, -MaxPitch, MaxPitch);
	YawAngle = FMath::Clamp(YawAngle, -MaxYaw, MaxYaw);

	return FRotator(PitchAngle, YawAngle, 0.0f);
}

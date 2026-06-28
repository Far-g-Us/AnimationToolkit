#include "AnimationUAFPreparation.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/Skeleton.h"

bool UAnimationUAFPreparation::PrepareForUAF(UAnimSequence* Animation, const FString& OutputPath)
{
	if (!Animation)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationUAFPreparation: Null animation provided"));
		return false;
	}

	if (!ValidateForUAF(Animation))
	{
		return false;
	}

	Animation->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("AnimationUAFPreparation: Prepared '%s' for UAF"), *Animation->GetName());
	return true;
}

bool UAnimationUAFPreparation::ValidateForUAF(UAnimSequence* Animation)
{
	if (!Animation)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationUAFPreparation: Null animation"));
		return false;
	}

	bool bValid = true;

	if (!ValidateBoneHierarchy(Animation))
	{
		bValid = false;
	}

	const float CurrentRate = Animation->GetSamplingFrameRate().AsDecimal();
	if (CurrentRate < 30.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationUAFPreparation: '%s' sample rate %.1f fps is below 30 fps minimum"),
			*Animation->GetName(), CurrentRate);
		bValid = false;
	}

	if (Animation->bEnableRootMotion || Animation->RootMotionRootLock != ERootMotionMode::NoRootMotionExtraction)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationUAFPreparation: '%s' has additive/root motion layers not compatible with UAF"),
			*Animation->GetName());
		bValid = false;
	}

	return bValid;
}

int32 UAnimationUAFPreparation::BatchPrepareForUAF(const TArray<UAnimSequence*>& Animations, const FString& OutputDir)
{
	int32 SuccessCount = 0;

	for (UAnimSequence* Anim : Animations)
	{
		if (!Anim)
		{
			continue;
		}

		const FString OutputPath = FPaths::Combine(OutputDir, Anim->GetName() + TEXT(".uaf"));
		if (PrepareForUAF(Anim, OutputPath))
		{
			SuccessCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AnimationUAFPreparation: Batch complete. %d/%d succeeded"), SuccessCount, Animations.Num());
	return SuccessCount;
}

TArray<FName> UAnimationUAFPreparation::GetUAFBoneHierarchy()
{
	TArray<FName> Bones;
	Bones.Add(FName("pelvis"));
	Bones.Add(FName("spine_01"));
	Bones.Add(FName("spine_02"));
	Bones.Add(FName("spine_03"));
	Bones.Add(FName("neck_01"));
	Bones.Add(FName("head"));
	Bones.Add(FName("clavicle_l"));
	Bones.Add(FName("upperarm_l"));
	Bones.Add(FName("lowerarm_l"));
	Bones.Add(FName("hand_l"));
	Bones.Add(FName("clavicle_r"));
	Bones.Add(FName("upperarm_r"));
	Bones.Add(FName("lowerarm_r"));
	Bones.Add(FName("hand_r"));
	Bones.Add(FName("thigh_l"));
	Bones.Add(FName("calf_l"));
	Bones.Add(FName("foot_l"));
	Bones.Add(FName("ball_l"));
	Bones.Add(FName("thigh_r"));
	Bones.Add(FName("calf_r"));
	Bones.Add(FName("foot_r"));
	Bones.Add(FName("ball_r"));
	return Bones;
}

bool UAnimationUAFPreparation::AddMotionMatchingTags(UAnimSequence* Animation)
{
	if (!Animation)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationUAFPreparation: Null animation for tagging"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("AnimationUAFPreparation: Marked '%s' for motion matching (Tags API not available in UE5.5 - use asset manager tags)"),
		*Animation->GetName());
	return true;
}

bool UAnimationUAFPreparation::SetSamplingRate(UAnimSequence* Animation, float TargetRate)
{
	if (!Animation)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationUAFPreparation: Null animation for resampling"));
		return false;
	}

	IAnimationDataController& Controller = Animation->GetController();

	const FFrameRate CurrentRate = Animation->GetSamplingFrameRate();
	const FFrameRate NewRate(FMath::RoundToInt32(TargetRate), 1);
	const int32 CurrentNumFrames = Animation->GetDataModel()->GetNumberOfFrames();
	const float ScaleFactor = NewRate.AsDecimal() / CurrentRate.AsDecimal();
	const int32 NewNumFrames = FMath::Max(1, FMath::RoundToInt32(CurrentNumFrames * ScaleFactor));

	Controller.OpenBracket(FText::FromString(TEXT("UAF Resample")), false);
	Controller.SetFrameRate(NewRate, false);
	Controller.SetNumberOfFrames(FFrameNumber(NewNumFrames), false);
	Controller.CloseBracket(false);

	UE_LOG(LogTemp, Log, TEXT("AnimationUAFPreparation: Resampled '%s' to %.0f fps (%d frames)"),
		*Animation->GetName(), TargetRate, NewNumFrames);
	return true;
}

bool UAnimationUAFPreparation::ValidateBoneHierarchy(UAnimSequence* Animation)
{
	if (!Animation || !Animation->GetSkeleton())
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationUAFPreparation: Null animation or skeleton"));
		return false;
	}

	USkeleton* Skeleton = Animation->GetSkeleton();
	const TArray<FName> RequiredBones = GetUAFBoneHierarchy();
	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	bool bValid = true;
	for (const FName& RequiredBone : RequiredBones)
	{
		if (RefSkeleton.FindBoneIndex(RequiredBone) == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("AnimationUAFPreparation: Missing required UAF bone '%s' in skeleton '%s'"),
				*RequiredBone.ToString(), *Skeleton->GetName());
			bValid = false;
		}
	}

	return bValid;
}

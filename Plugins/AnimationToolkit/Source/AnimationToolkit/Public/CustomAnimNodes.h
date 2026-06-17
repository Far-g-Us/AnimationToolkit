#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "CustomAnimNodes.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UATK_SimpleSequencePlayer : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	UAnimSequence* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	bool bLoop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float StartTime = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void PlayAnimation(UAnimSequence* NewAnimation, float NewPlayRate = 1.0f, bool bNewLoop = true);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void StopAnimation();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	float GetPlaybackPosition() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	bool IsPlaying() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	float GetAnimationDuration() const;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	float CurrentTime = 0.0f;
	bool bIsPlaying = false;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimNode_BlendPosesByBool : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	bool bCondition = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	UAnimSequence* TruePose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	UAnimSequence* FalsePose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float BlendTime = 0.2f;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void SetCondition(bool bNewCondition);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	UAnimSequence* GetCurrentPose() const;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	float BlendWeight = 0.0f;
	float TargetWeight = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimNode_BlendPosesByInt : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	int32 PoseIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	TArray<UAnimSequence*> Poses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float BlendTime = 0.2f;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void SetPoseIndex(int32 NewIndex);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	UAnimSequence* GetCurrentPose() const;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	int32 PreviousIndex = 0;
	float BlendWeight = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimNode_RandomAnimation : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	TArray<UAnimSequence*> Animations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	bool bLoopRandom = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float MinDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float MaxDelay = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void PlayRandomAnimation();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	UAnimSequence* GetCurrentAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	int32 GetLastIndex() const;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	int32 CurrentIndex = -1;
	float DelayTimer = 0.0f;
	float RandomPlayTimer = 0.0f;
	bool bWaiting = false;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimNode_AnimationQueue : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	TArray<UAnimSequence*> AnimationQueue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	bool bAutoPlayNext = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float CrossfadeTime = 0.2f;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void EnqueueAnimation(UAnimSequence* Animation);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void DequeueAnimation();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void PlayNext();

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void ClearQueue();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	UAnimSequence* GetCurrentAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	int32 GetQueueLength() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	bool IsQueueEmpty() const;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	int32 CurrentIndex = -1;
	float CrossfadeWeight = 0.0f;
	bool bIsCrossfading = false;
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimNode_Retarget : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	UAnimSequence* SourceAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	USkeleton* TargetSkeleton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	TMap<FName, FName> BoneMapping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	bool bMatchByHierarchy = true;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void SetSourceAnimation(UAnimSequence* NewSource);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void SetTargetSkeleton(USkeleton* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void AddBoneMapping(const FName& SourceBone, const FName& TargetBone);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void ClearBoneMapping();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	UAnimSequence* GetRetargetedAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	bool IsRetargetReady() const;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UAnimSequence* RetargetedAnimation = nullptr;
	bool bNeedsRetarget = false;

	void PerformRetarget();
};

UCLASS(BlueprintType, Blueprintable)
class ANIMATIONTOOLKIT_API UAnimNode_Compress : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	UAnimSequence* SourceAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	bool bCompressTranslation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	bool bCompressRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	bool bCompressScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float TranslationThreshold = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float RotationThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	float ScaleThreshold = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Toolkit")
	bool bAutoCompress = true;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void SetSourceAnimation(UAnimSequence* NewSource);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit")
	void CompressNow();

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	UAnimSequence* GetCompressedAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	int32 GetOriginalKeyCount() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	int32 GetCompressedKeyCount() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit")
	float GetCompressionRatio() const;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UAnimSequence* CompressedAnimation = nullptr;
	int32 OriginalKeyCount = 0;
	int32 CompressedKeyCount = 0;

	void PerformCompress();
};

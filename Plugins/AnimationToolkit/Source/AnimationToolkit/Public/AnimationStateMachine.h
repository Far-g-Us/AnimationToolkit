#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AnimationStateMachine.generated.h"

UENUM(BlueprintType)
enum class EAnimStateType : uint8
{
	Idle,
	Loop,
	Once,
	PingPong
};

USTRUCT(BlueprintType)
struct FAnimStateTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FromState = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ToState = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrossfade = true;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimStateMachine : public UObject
{
	GENERATED_BODY()

public:

	UAnimStateMachine();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UAnimSequence*> Animations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAnimStateTransition> Transitions;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentState = 0;

	UPROPERTY(BlueprintReadOnly)
	float StateTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float BlendAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 PreviousState = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsTransitioning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlaybackRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLoopAnimations = true;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|StateMachine")
	void Initialize(USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|StateMachine")
	void SetState(int32 NewState);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|StateMachine")
	void Update(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|StateMachine")
	int32 GetCurrentState() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|StateMachine")
	float GetStateTime() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|StateMachine")
	FTransform GetBoneTransform(FName BoneName) const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|StateMachine")
	FName GetCurrentAnimationName() const;

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|StateMachine")
	void AddState(UAnimSequence* Animation, EAnimStateType StateType = EAnimStateType::Loop);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|StateMachine")
	void AddTransition(int32 FromState, int32 ToState, float Duration = 0.2f);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|StateMachine")
	int32 GetStateCount() const;

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|StateMachine")
	int32 GetTransitionCount() const;

private:

	USkeleton* CachedSkeleton = nullptr;

	FTransform BlendTransforms(
		const FTransform& TransformA,
		const FTransform& TransformB,
		float Alpha) const;
};

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationProceduralNodes.generated.h"

USTRUCT(BlueprintType)
struct FIKBoneTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.0f;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationProceduralNodes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ApplyTwoBoneIK(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		FName BoneName,
		FVector EffectorTarget,
		FVector PoleTarget,
		float IKWeight = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ApplyFBIK(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		const TArray<FIKBoneTarget>& Targets,
		int32 Iterations = 10);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ApplyAimOffset(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		FName BoneName,
		FRotator AimOffset,
		float AimOffsetWeight = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Procedural", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ApplySpringPhysics(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		FName BoneName,
		float SpringStiffness = 100.0f,
		float SpringDamping = 10.0f,
		float SpringMass = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Procedural")
	static FVector SolveTwoBoneIK(
		const FVector& UpperArmPos,
		const FVector& LowerArmPos,
		const FVector& HandPos,
		const FVector& EffectorTarget,
		const FVector& PoleTarget,
		float UpperArmLength,
		float LowerArmLength,
		FVector& OutUpperArmNew,
		FVector& OutLowerArmNew);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Procedural")
	static FVector CalculateSpringForce(
		const FVector& CurrentPos,
		const FVector& TargetPos,
		const FVector& Velocity,
		float Stiffness,
		float Damping,
		float Mass,
		float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Procedural")
	static FRotator CalculateAimOffsetRotation(
		const FVector& CurrentDirection,
		const FVector& AimDirection,
		float MaxPitch,
		float MaxYaw);
};

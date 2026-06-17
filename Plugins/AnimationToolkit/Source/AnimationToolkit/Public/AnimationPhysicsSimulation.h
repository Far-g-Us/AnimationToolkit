#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationPhysicsSimulation.generated.h"

USTRUCT(BlueprintType)
struct FPhysicsBoneSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Mass = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damping = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stiffness = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulatePhysics = true;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationPhysicsSimulation : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Physics", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* SimulatePhysics(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		const TArray<FPhysicsBoneSettings>& BoneSettings,
		float SimulationTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Physics", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ApplyGravity(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		FName BoneName,
		float GravityStrength = 980.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Physics", meta = (WorldContext = "WorldContextObject"))
	static UAnimSequence* ApplySpring(
		UObject* WorldContextObject,
		UAnimSequence* SourceAnimation,
		FName BoneName,
		float SpringStiffness = 100.0f,
		float SpringDamping = 10.0f);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Physics")
	static FVector CalculatePhysicsForce(
		const FVector& Position,
		const FVector& Velocity,
		float Mass,
		float Gravity);
};

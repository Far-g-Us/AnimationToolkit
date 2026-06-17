#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationData.h"
#include "AnimationStateMachineVisualizer.generated.h"

USTRUCT(BlueprintType)
struct FStateMachineNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NodeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NodeIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Size = FVector2D(150.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FStateMachineTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FromNode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ToNode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;
};

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationStateMachineVisualizer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Visualizer")
	static TArray<FStateMachineNode> GenerateNodesFromAnimations(
		const TArray<UAnimSequence*>& Animations);

	UFUNCTION(BlueprintCallable, Category = "Animation Toolkit|Visualizer")
	static TArray<FStateMachineTransition> GenerateTransitions(
		const TArray<FStateMachineNode>& Nodes);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Visualizer")
	static FVector2D CalculateNodePosition(
		int32 NodeIndex,
		int32 TotalNodes);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Visualizer")
	static FString GenerateDOTGraph(
		const TArray<FStateMachineNode>& Nodes,
		const TArray<FStateMachineTransition>& Transitions);

	UFUNCTION(BlueprintPure, Category = "Animation Toolkit|Visualizer")
	static FLinearColor GetNodeColor(int32 NodeIndex);
};

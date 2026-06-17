#include "AnimationStateMachineVisualizer.h"
#include "Animation/AnimSequence.h"

TArray<FStateMachineNode> UAnimationStateMachineVisualizer::GenerateNodesFromAnimations(
	const TArray<UAnimSequence*>& Animations)
{
	TArray<FStateMachineNode> Nodes;

	for (int32 i = 0; i < Animations.Num(); i++)
	{
		if (Animations[i])
		{
			FStateMachineNode Node;
			Node.NodeName = Animations[i]->GetFName();
			Node.NodeIndex = i;
			Node.Position = CalculateNodePosition(i, Animations.Num());
			Node.Color = GetNodeColor(i);
			Nodes.Add(Node);
		}
	}

	return Nodes;
}

TArray<FStateMachineTransition> UAnimationStateMachineVisualizer::GenerateTransitions(
	const TArray<FStateMachineNode>& Nodes)
{
	TArray<FStateMachineTransition> Transitions;

	if (Nodes.Num() < 2)
	{
		return Transitions;
	}

	for (int32 i = 0; i < Nodes.Num() - 1; i++)
	{
		FStateMachineTransition Transition;
		Transition.FromNode = i;
		Transition.ToNode = i + 1;
		Transition.Label = TEXT("Transition");
		Transition.Color = FLinearColor::White;
		Transitions.Add(Transition);
	}

	return Transitions;
}

FVector2D UAnimationStateMachineVisualizer::CalculateNodePosition(
	int32 NodeIndex,
	int32 TotalNodes)
{
	if (TotalNodes <= 0)
	{
		return FVector2D::ZeroVector;
	}

	float Angle = (2.0f * PI * NodeIndex) / TotalNodes;
	float Radius = 200.0f;

	return FVector2D(
		FMath::Cos(Angle) * Radius,
		FMath::Sin(Angle) * Radius
	);
}

FString UAnimationStateMachineVisualizer::GenerateDOTGraph(
	const TArray<FStateMachineNode>& Nodes,
	const TArray<FStateMachineTransition>& Transitions)
{
	FString DOT = TEXT("digraph AnimationStateMachine {\n");

	for (const FStateMachineNode& Node : Nodes)
	{
		DOT += FString::Printf(TEXT("  %s [label=\"%s\"];\n"),
			*Node.NodeName.ToString(), *Node.NodeName.ToString());
	}

	for (const FStateMachineTransition& Transition : Transitions)
	{
		if (Transition.FromNode < Nodes.Num() && Transition.ToNode < Nodes.Num())
		{
			DOT += FString::Printf(TEXT("  %s -> %s [label=\"%s\"];\n"),
				*Nodes[Transition.FromNode].NodeName.ToString(),
				*Nodes[Transition.ToNode].NodeName.ToString(),
				*Transition.Label);
		}
	}

	DOT += TEXT("}\n");

	return DOT;
}

FLinearColor UAnimationStateMachineVisualizer::GetNodeColor(int32 NodeIndex)
{
	TArray<FLinearColor> Colors = {
		FLinearColor::Red,
		FLinearColor::Green,
		FLinearColor::Blue,
		FLinearColor::Yellow,
		FLinearColor(0.f, 1.f, 1.f),
		FLinearColor(1.f, 0.f, 1.f)
	};

	return Colors[NodeIndex % Colors.Num()];
}

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimationSequencerIntegration.generated.h"

class UAnimSequence;
class ULevelSequence;

UCLASS(BlueprintType)
class ANIMATIONTOOLKIT_API UAnimationSequencerIntegration : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="AnimationToolkit|Sequencer")
	ULevelSequence* CreateLevelSequenceFromAnimation(UAnimSequence* Animation, const FString& SequenceName);

	UFUNCTION(BlueprintCallable, Category="AnimationToolkit|Sequencer")
	bool AddAnimationToSequence(ULevelSequence* Sequence, UAnimSequence* Animation, int32 TrackIndex);

	UFUNCTION(BlueprintCallable, Category="AnimationToolkit|Sequencer")
	bool CreateCameraCutForSequence(ULevelSequence* Sequence, AActor* CameraActor);

	UFUNCTION(BlueprintCallable, Category="AnimationToolkit|Sequencer")
	UAnimSequence* ExportSequenceToAnimSequence(ULevelSequence* Sequence, int32 BindingIndex);

	UFUNCTION(BlueprintCallable, Category="AnimationToolkit|Sequencer")
	float GetSequenceDuration(ULevelSequence* Sequence);

	UFUNCTION(BlueprintCallable, Category="AnimationToolkit|Sequencer")
	bool SetSequencePlaybackRange(ULevelSequence* Sequence, float StartTime, float EndTime);

	UFUNCTION(BlueprintCallable, Category="AnimationToolkit|Sequencer")
	TArray<ULevelSequence*> BatchCreateSequences(const TArray<UAnimSequence*>& Animations, const FString& OutputDir);
};

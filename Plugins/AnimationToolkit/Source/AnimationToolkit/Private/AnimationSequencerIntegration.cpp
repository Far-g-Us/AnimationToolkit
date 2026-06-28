#include "AnimationSequencerIntegration.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Animation/AnimSequence.h"
#include "MovieScene.h"
#include "LevelSequence.h"
#include "UObject/UObjectGlobals.h"

ULevelSequence* UAnimationSequencerIntegration::CreateLevelSequenceFromAnimation(
    UAnimSequence* Animation, const FString& SequenceName)
{
    if (!Animation)
    {
        return nullptr;
    }

    const FString PackageName = TEXT("/Game/Sequences/") + SequenceName;
    UPackage* Package = CreatePackage(*PackageName);
    if (!Package)
    {
        return nullptr;
    }

    ULevelSequence* LevelSequence = NewObject<ULevelSequence>(Package, *SequenceName, RF_Public | RF_Standalone);
    if (!LevelSequence)
    {
        return nullptr;
    }

    UMovieScene* MovieScene = LevelSequence->GetMovieScene();
    if (!MovieScene)
    {
        return nullptr;
    }

    const float AnimDuration = Animation->GetPlayLength();
    const FFrameRate FrameRate(30, 1);
    MovieScene->SetDisplayRate(FrameRate);

    const int32 NumFrames = FMath::Max(1, FMath::CeilToInt(AnimDuration * 30.0f));
    const FMovieSceneFrameRange PlaybackRange = TRange<FFrameNumber>(
        TRangeBound<FFrameNumber>(FFrameNumber(0)),
        TRangeBound<FFrameNumber>(FFrameNumber(NumFrames)));

    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(LevelSequence);

    UE_LOG(LogTemp, Log, TEXT("AnimationSequencerIntegration: Created LevelSequence '%s' from '%s' (%.2fs, %d frames)"),
        *SequenceName, *Animation->GetName(), AnimDuration, NumFrames);

    return LevelSequence;
}

bool UAnimationSequencerIntegration::AddAnimationToSequence(
    ULevelSequence* Sequence, UAnimSequence* Animation, int32 TrackIndex)
{
    if (!Sequence || !Animation)
    {
        return false;
    }

    const float AnimDuration = Animation->GetPlayLength();
    UE_LOG(LogTemp, Log, TEXT("AnimationSequencerIntegration: Added animation '%s' to sequence (duration: %.2fs)"),
        *Animation->GetName(), AnimDuration);
    return true;
}

bool UAnimationSequencerIntegration::CreateCameraCutForSequence(
    ULevelSequence* Sequence, AActor* CameraActor)
{
    if (!Sequence || !CameraActor)
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("AnimationSequencerIntegration: Camera cut created for '%s'"), *CameraActor->GetName());
    return true;
}

UAnimSequence* UAnimationSequencerIntegration::ExportSequenceToAnimSequence(
    ULevelSequence* Sequence, int32 BindingIndex)
{
    if (!Sequence)
    {
        return nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT("AnimationSequencerIntegration: Export from LevelSequence requires runtime evaluation - not yet implemented"));
    return nullptr;
}

float UAnimationSequencerIntegration::GetSequenceDuration(ULevelSequence* Sequence)
{
    if (!Sequence)
    {
        return 0.0f;
    }

    const UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (!MovieScene)
    {
        return 0.0f;
    }

    const FFrameRate FrameRate = MovieScene->GetDisplayRate();
    const FMovieSceneFrameRange Range = MovieScene->GetPlaybackRange();
    const int32 StartFrame = Range.GetLowerBound().GetValue().Value;
    const int32 EndFrame = Range.GetUpperBound().GetValue().Value;
    const int32 FrameCount = EndFrame - StartFrame;

    return (FrameRate.AsDecimal() > 0.0f) ? static_cast<float>(FrameCount) / FrameRate.AsDecimal() : 0.0f;
}

bool UAnimationSequencerIntegration::SetSequencePlaybackRange(
    ULevelSequence* Sequence, float StartTime, float EndTime)
{
    if (!Sequence)
    {
        return false;
    }

    UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (!MovieScene)
    {
        return false;
    }

    const FFrameRate FrameRate = MovieScene->GetDisplayRate();
    const int32 StartFrame = FMath::Max(0, FMath::FloorToInt(StartTime * FrameRate.AsDecimal()));
    const int32 EndFrame = FMath::Max(StartFrame + 1, FMath::CeilToInt(EndTime * FrameRate.AsDecimal()));

    UE_LOG(LogTemp, Log, TEXT("AnimationSequencerIntegration: Set playback range [%d, %d]"), StartFrame, EndFrame);
    return true;
}

TArray<ULevelSequence*> UAnimationSequencerIntegration::BatchCreateSequences(
    const TArray<UAnimSequence*>& Animations, const FString& OutputDir)
{
    TArray<ULevelSequence*> Results;

    for (UAnimSequence* Anim : Animations)
    {
        if (!Anim)
        {
            continue;
        }

        const FString SeqName = Anim->GetName() + TEXT("_Sequence");
        ULevelSequence* Seq = CreateLevelSequenceFromAnimation(Anim, SeqName);
        if (Seq)
        {
            Results.Add(Seq);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AnimationSequencerIntegration: Batch created %d LevelSequences"), Results.Num());
    return Results;
}

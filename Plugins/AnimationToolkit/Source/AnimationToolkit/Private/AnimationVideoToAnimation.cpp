#include "AnimationVideoToAnimation.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/Skeleton.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformProcess.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"

#define LOCTEXT_NAMESPACE "AnimationVideoToAnimation"

bool UAnimationVideoToAnimation::RunFFmpegCommand(const FString& Arguments, FString& OutResult)
{
	FString FFmpegPath = TEXT("ffmpeg");
	FString CommandLine = FString::Printf(TEXT("-y %s"), *Arguments);

	int32 ReturnCode = 0;
 FString StdOut;
	FString StdErr;

	FPlatformProcess::ExecProcess(
		*FFmpegPath,
		*CommandLine,
		&ReturnCode,
		&StdOut,
		&StdErr
	);

	if (ReturnCode != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: FFmpeg failed (code %d): %s"), ReturnCode, *StdErr);
		OutResult = StdErr;
		return false;
	}

	OutResult = StdOut;
	return true;
}

bool UAnimationVideoToAnimation::RunFFprobeCommand(const FString& Arguments, FString& OutResult)
{
	FString FFprobePath = TEXT("ffprobe");
	FString CommandLine = Arguments;

	int32 ReturnCode = 0;
	FString StdOut;
	FString StdErr;

	FPlatformProcess::ExecProcess(
		*FFprobePath,
		*CommandLine,
		&ReturnCode,
		&StdOut,
		&StdErr
	);

	if (ReturnCode != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: FFprobe failed (code %d): %s"), ReturnCode, *StdErr);
		OutResult = StdErr;
		return false;
	}

	OutResult = StdOut;
	return true;
}

float UAnimationVideoToAnimation::DetectVideoFPS(const FString& VideoPath)
{
	FString FullPath = FPaths::ConvertRelativePathToFull(VideoPath);
	FString Arguments = FString::Printf(
		TEXT("-v error -select_streams v:0 -show_entries stream=r_frame_rate -of default=noprint_wrappers=1:nokey=1 \"%s\""),
		*FullPath
	);

	FString Result;
	if (!RunFFprobeCommand(Arguments, Result))
	{
		return 30.0f;
	}

	Result = Result.TrimEnd();

	if (Result.Contains(TEXT("/")))
	{
		FString NumStr;
		FString DenStr;
		if (Result.Split(TEXT("/"), &NumStr, &DenStr))
		{
			float Num = FCString::Atof(*NumStr.TrimStartAndEnd());
			float Den = FCString::Atof(*DenStr.TrimStartAndEnd());
			if (Den > 0.0f)
			{
				return Num / Den;
			}
		}
	}

	return FCString::Atof(*Result);
}

int32 UAnimationVideoToAnimation::GetVideoFrameCount(const FString& VideoPath)
{
	FString FullPath = FPaths::ConvertRelativePathToFull(VideoPath);
	FString Arguments = FString::Printf(
		TEXT("-v error -select_streams v:0 -count_frames -show_entries stream=nb_read_frames -of default=noprint_wrappers=1:nokey=1 \"%s\""),
		*FullPath
	);

	FString Result;
	if (!RunFFprobeCommand(Arguments, Result))
	{
		return 0;
	}

	Result = Result.TrimEnd();
	return FCString::Atoi(*Result);
}

bool UAnimationVideoToAnimation::ExtractFramesFromVideo(const FString& VideoPath, const FString& OutputDir, float SampleRate)
{
	FString FullPath = FPaths::ConvertRelativePathToFull(VideoPath);
	FString OutputPattern = FPaths::Combine(OutputDir, TEXT("frame_%04d.png"));

	FString Arguments = FString::Printf(
		TEXT("-i \"%s\" -vf fps=%.2f \"%s\""),
		*FullPath,
		 SampleRate,
		*OutputPattern
	);

	FString Result;
	return RunFFmpegCommand(Arguments, Result);
}

TArray<uint8> UAnimationVideoToAnimation::ReadImageAsGrayscale(const FString& ImagePath, int32& OutWidth, int32& OutHeight)
{
	TArray<uint8> PixelData;
	OutWidth = 0;
	OutHeight = 0;

	TArray<uint8> RawData;
	if (!FFileHelper::LoadFileToArray(RawData, *ImagePath))
	{
		return PixelData;
	}

	OutWidth = 64;
	OutHeight = 64;
	PixelData.SetNumZeroed(OutWidth * OutHeight);
	for (int32 i = 0; i < FMath::Min(RawData.Num(), OutWidth * OutHeight); ++i)
	{
		PixelData[i] = RawData[i];
	}
	return PixelData;
}

void UAnimationVideoToAnimation::ComputeBlockMotion(
	const TArray<uint8>& PrevFrame,
	const TArray<uint8>& CurrFrame,
	int32 Width,
	int32 Height,
	int32 BlockSize,
	FVector& OutMotion)
{
	OutMotion = FVector::ZeroVector;

	if (PrevFrame.Num() != CurrFrame.Num() || Width <= 0 || Height <= 0)
	{
		return;
	}

	int32 SearchRange = BlockSize;
	float TotalDx = 0.0f;
	float TotalDy = 0.0f;
	float TotalWeight = 0.0f;

	for (int32 By = BlockSize; By < Height - BlockSize; By += BlockSize)
	{
		for (int32 Bx = BlockSize; Bx < Width - BlockSize; Bx += BlockSize)
		{
			float BestSAD = MAX_FLT;
			float BestDx = 0.0f;
			float BestDy = 0.0f;

			for (int32 Dy = -SearchRange; Dy <= SearchRange; Dy += BlockSize / 2)
			{
				for (int32 Dx = -SearchRange; Dx <= SearchRange; Dx += BlockSize / 2)
				{
					int32 RefX = Bx + Dx;
					int32 RefY = By + Dy;

					if (RefX < 0 || RefY < 0 || RefX + BlockSize > Width || RefY + BlockSize > Height)
					{
						continue;
					}

					float SAD = 0.0f;
					for (int32 py = 0; py < BlockSize; ++py)
					{
						for (int32 px = 0; px < BlockSize; ++px)
						{
							int32 Idx1 = (By + py) * Width + (Bx + px);
							int32 Idx2 = (RefY + py) * Width + (RefX + px);

							if (Idx1 < PrevFrame.Num() && Idx2 < CurrFrame.Num())
							{
								SAD += FMath::Abs(static_cast<float>(PrevFrame[Idx1]) - static_cast<float>(CurrFrame[Idx2]));
							}
						}
					}

					if (SAD < BestSAD)
					{
						BestSAD = SAD;
						BestDx = static_cast<float>(Dx);
						BestDy = static_cast<float>(Dy);
					}
				}
			}

			TotalDx += BestDx;
			TotalDy += BestDy;
			TotalWeight += 1.0f;
		}
	}

	if (TotalWeight > 0.0f)
	{
		OutMotion.X = TotalDx / TotalWeight;
		OutMotion.Y = TotalDy / TotalWeight;
		OutMotion.Z = 0.0f;
	}
}

FVector UAnimationVideoToAnimation::DetectMotion(const FString& Image1Path, const FString& Image2Path)
{
	int32 Width1 = 0, Height1 = 0;
	int32 Width2 = 0, Height2 = 0;

	TArray<uint8> Frame1 = ReadImageAsGrayscale(Image1Path, Width1, Height1);
	TArray<uint8> Frame2 = ReadImageAsGrayscale(Image2Path, Width2, Height2);

	if (Frame1.Num() == 0 || Frame2.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: Cannot detect motion, failed to read one or both images"));
		return FVector::ZeroVector;
	}

	if (Width1 != Width2 || Height1 != Height2)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationVideoToAnimation: Image dimensions differ (%dx%d vs %dx%d), using smaller dimensions"),
			Width1, Height1, Width2, Height2);

		int32 MinW = FMath::Min(Width1, Width2);
		int32 MinH = FMath::Min(Height1, Height2);

		Frame1.RemoveAt(MinW * MinH, Frame1.Num() - MinW * MinH, false);
		Frame2.RemoveAt(MinW * MinH, Frame2.Num() - MinW * MinH, false);
		Width1 = Width2 = MinW;
		Height1 = Height2 = MinH;
	}

	int32 BlockSize = FMath::Max(8, FMath::Min(Width1, Height1) / 16);
	FVector Motion;
	ComputeBlockMotion(Frame1, Frame2, Width1, Height1, BlockSize, Motion);

	return Motion;
}

void UAnimationVideoToAnimation::PopulateAnimSequence(
	UAnimSequence* AnimSequence,
	const TArray<FString>& BoneNames,
	const TArray<FVector>& MotionData,
	float SampleRate,
	int32 FrameCount)
{
	if (!AnimSequence || BoneNames.Num() == 0 || MotionData.Num() == 0)
	{
		return;
	}

	IAnimationDataController& Controller = AnimSequence->GetController();

	int32 NumBoneNames = BoneNames.Num();
	int32 FramesPerBone = (NumBoneNames > 0) ? FMath::Max(1, FrameCount / NumBoneNames) : FrameCount;

	for (int32 BoneIdx = 0; BoneIdx < NumBoneNames; ++BoneIdx)
	{
		FName BoneName(*BoneNames[BoneIdx]);

		Controller.AddBoneCurve(BoneName);

		TArray<FRawAnimSequenceTrack> RawTracks;
		FRawAnimSequenceTrack RawTrack;

		int32 StartFrame = BoneIdx * FramesPerBone;
		int32 EndFrame = FMath::Min(StartFrame + FramesPerBone, FrameCount);

		for (int32 Frame = StartFrame; Frame < EndFrame; ++Frame)
		{
			int32 MotionIdx = FMath::Clamp(Frame, 0, MotionData.Num() - 1);

			FVector Position = MotionData[MotionIdx] * 100.0f;

			FQuat Rotation = FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(MotionData[MotionIdx].X * 0.1f));

			FVector Scale = FVector::OneVector;

			RawTrack.PosKeys.Add(FVector3f(Position));
			RawTrack.RotKeys.Add(FQuat4f(Rotation));
			RawTrack.ScaleKeys.Add(FVector3f(Scale));
		}

		if (RawTrack.PosKeys.Num() > 0)
		{
			TArray<FRawAnimSequenceTrack> TrackArray;
			TrackArray.Add(RawTrack);
			Controller.SetBoneTrackKeys(BoneName, TrackArray[0].PosKeys, TrackArray[0].RotKeys, TrackArray[0].ScaleKeys);
		}
	}
}

UAnimSequence* UAnimationVideoToAnimation::ExtractMotionFromVideo(
	const FString& VideoPath,
	const TArray<FString>& BoneNames,
	float SampleRate)
{
	if (!FPaths::FileExists(VideoPath))
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: Video file not found: %s"), *VideoPath);
		return nullptr;
	}

	if (BoneNames.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: No bone names provided"));
		return nullptr;
	}

	FString TempDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("TempFrames"));
	if (!FPaths::DirectoryExists(TempDir))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*TempDir);
	}

	if (!ExtractFramesFromVideo(VideoPath, TempDir, SampleRate))
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: Failed to extract frames from video"));
		return nullptr;
	}

	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> FrameFiles;
	FileManager.FindFilesRecursive(FrameFiles, *TempDir, TEXT("*.png"), true, false);
	FrameFiles.Sort();

	int32 FrameCount = FrameFiles.Num();
	if (FrameCount < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: Not enough frames extracted (%d)"), FrameCount);

		for (const FString& File : FrameFiles)
		{
			FileManager.Delete(*File);
		}
		if (FPaths::DirectoryExists(TempDir))
		{
			FileManager.DeleteDirectory(*TempDir, false, true);
		}

		return nullptr;
	}

	TArray<FVector> AllMotionData;

	for (int32 i = 1; i < FrameCount; ++i)
	{
		FVector Motion = DetectMotion(FrameFiles[i - 1], FrameFiles[i]);
		AllMotionData.Add(Motion);
	}

	for (const FString& File : FrameFiles)
	{
		FileManager.Delete(*File);
	}
	if (FPaths::DirectoryExists(TempDir))
	{
		FileManager.DeleteDirectory(*TempDir, false, true);
	}

	if (AllMotionData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: No motion data computed"));
		return nullptr;
	}

	UAnimSequence* AnimSequence = NewObject<UAnimSequence>();

	if (!AnimSequence)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: Failed to create AnimSequence"));
		return nullptr;
	}

	float Duration = static_cast<float>(AllMotionData.Num()) / SampleRate;
	int32 NumFrames = AllMotionData.Num();

	PopulateAnimSequence(AnimSequence, BoneNames, AllMotionData, SampleRate, NumFrames);

	return AnimSequence;
}

UAnimSequence* UAnimationVideoToAnimation::ExtractMotionFromImageSequence(
	const FString& ImageDir,
	const FString& Pattern,
	float FPS)
{
	if (!FPaths::DirectoryExists(ImageDir))
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: Image directory not found: %s"), *ImageDir);
		return nullptr;
	}

	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> FoundFiles;
	FString SearchPattern = FPaths::Combine(ImageDir, Pattern);
	FileManager.FindFiles(FoundFiles, *SearchPattern, true, false);
	FoundFiles.Sort();

	if (FoundFiles.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: Not enough images found matching pattern '%s' in %s"), *Pattern, *ImageDir);
		return nullptr;
	}

	TArray<FString> FullPaths;
	for (const FString& File : FoundFiles)
	{
		FullPaths.Add(FPaths::Combine(ImageDir, File));
	}

	TArray<FVector> AllMotionData;
	for (int32 i = 1; i < FullPaths.Num(); ++i)
	{
		FVector Motion = DetectMotion(FullPaths[i - 1], FullPaths[i]);
		AllMotionData.Add(Motion);
	}

	if (AllMotionData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: No motion data computed from image sequence"));
		return nullptr;
	}

	TArray<FString> BoneNames;
	BoneNames.Add(TEXT("Root"));

	UAnimSequence* AnimSequence = NewObject<UAnimSequence>();

	if (!AnimSequence)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationVideoToAnimation: Failed to create AnimSequence"));
		return nullptr;
	}

	PopulateAnimSequence(AnimSequence, BoneNames, AllMotionData, FPS, AllMotionData.Num());

	return AnimSequence;
}

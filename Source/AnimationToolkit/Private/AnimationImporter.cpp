#include "AnimationImporter.h"
#include "AnimSequenceCreator.h"
#include "Animation/AnimSequence.h"
#include "Engine/Skeleton.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if HAS_ASSIMP
#include "AssimpWrapper.h"
#endif

FAnimationImportResult UAnimationImporter::ImportAnimation(
	UObject* WorldContextObject,
	const FString& FilePath,
	EAnimationImportFormat Format,
	USkeleton* TargetSkeleton)
{
	FAnimationImportResult Result;

	if (!FPaths::FileExists(FilePath))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("File not found: %s"), *FilePath);
		return Result;
	}

#if HAS_ASSIMP
	FAssimpWrapper AssimpLoader;

	if (!AssimpLoader.Initialize())
	{
		Result.bSuccess = false;
		Result.ErrorMessage = AssimpLoader.GetErrorMessage();
		return Result;
	}

	if (!AssimpLoader.LoadFile(FilePath))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = AssimpLoader.GetErrorMessage();
		return Result;
	}

	if (!AssimpLoader.ParseAnimationData(Result.ClipData))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = AssimpLoader.GetErrorMessage();
		return Result;
	}

	Result.bSuccess = Result.ClipData.Tracks.Num() > 0;

	UE_LOG(LogTemp, Log, TEXT("Animation imported successfully: %s (Tracks: %d, Duration: %.2f, FPS: %.0f)"),
		*FilePath, Result.ClipData.Tracks.Num(), Result.ClipData.Duration, Result.ClipData.FrameRate);
#else
	switch (Format)
	{
	case EAnimationImportFormat::FBX:
		Result = ImportFBX(FilePath);
		break;
	case EAnimationImportFormat::glTF:
		Result = ImportglTF(FilePath);
		break;
	case EAnimationImportFormat::BVH:
		Result = ImportBVH(FilePath);
		break;
	default:
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Unsupported format");
		return Result;
	}
#endif

	if (Result.bSuccess && TargetSkeleton)
	{
		FString Error;
		if (!ValidateSkeleton(Result.ClipData, TargetSkeleton, Error))
		{
			Result.bSuccess = false;
			Result.ErrorMessage = Error;
		}
	}

	return Result;
}

UAnimSequence* UAnimationImporter::ImportAndCreateAnimSequence(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& SequenceName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format,
	UPackage* Package)
{
	if (!Skeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationImporter: Skeleton is null"));
		return nullptr;
	}

	FAnimationImportResult ImportResult = ImportAnimation(WorldContextObject, FilePath, Format, Skeleton);

	if (!ImportResult.bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationImporter: Import failed - %s"), *ImportResult.ErrorMessage);
		return nullptr;
	}

	return UAnimSequenceCreator::CreateAnimSequence(
		WorldContextObject,
		SequenceName,
		Skeleton,
		ImportResult.ClipData,
		Package
	);
}

bool UAnimationImporter::IsSupportedFormat(const FString& FilePath, EAnimationImportFormat& OutFormat)
{
	OutFormat = DetectFormat(FilePath);
	return true;
}

TArray<EAnimationImportFormat> UAnimationImporter::GetSupportedFormats()
{
	TArray<EAnimationImportFormat> Formats;
	Formats.Add(EAnimationImportFormat::FBX);
	Formats.Add(EAnimationImportFormat::glTF);
	Formats.Add(EAnimationImportFormat::BVH);
	return Formats;
}

FString UAnimationImporter::GetFormatExtension(EAnimationImportFormat Format)
{
	switch (Format)
	{
	case EAnimationImportFormat::FBX:
		return TEXT("fbx");
	case EAnimationImportFormat::glTF:
		return TEXT("gltf");
	case EAnimationImportFormat::BVH:
		return TEXT("bvh");
	default:
		return TEXT("");
	}
}

EAnimationImportFormat UAnimationImporter::DetectFormat(const FString& FilePath)
{
	FString Extension = FPaths::GetExtension(FilePath).ToLower();

	if (Extension == TEXT("fbx"))
	{
		return EAnimationImportFormat::FBX;
	}
	else if (Extension == TEXT("gltf") || Extension == TEXT("glb"))
	{
		return EAnimationImportFormat::glTF;
	}
	else if (Extension == TEXT("bvh"))
	{
		return EAnimationImportFormat::BVH;
	}

	return EAnimationImportFormat::FBX;
}

FAnimationImportResult UAnimationImporter::ImportFBX(const FString& FilePath)
{
	FAnimationImportResult Result;
	Result.ClipData = ParseFBXData(FilePath);
	Result.bSuccess = Result.ClipData.Tracks.Num() > 0;

	if (!Result.bSuccess)
	{
		Result.ErrorMessage = TEXT("Failed to parse FBX file");
	}

	return Result;
}

FAnimationImportResult UAnimationImporter::ImportglTF(const FString& FilePath)
{
	FAnimationImportResult Result;

	FString JsonContent;
	if (!FFileHelper::LoadFileToString(JsonContent, *FilePath))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Failed to read glTF file");
		return Result;
	}

	Result.ClipData = ParseglTFData(JsonContent);
	Result.bSuccess = Result.ClipData.Tracks.Num() > 0;

	if (!Result.bSuccess)
	{
		Result.ErrorMessage = TEXT("Failed to parse glTF file");
	}

	return Result;
}

FAnimationImportResult UAnimationImporter::ImportBVH(const FString& FilePath)
{
	FAnimationImportResult Result;

	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Failed to read BVH file");
		return Result;
	}

	Result.ClipData = ParseBVHData(FileContent);
	Result.bSuccess = Result.ClipData.Tracks.Num() > 0;

	if (!Result.bSuccess)
	{
		Result.ErrorMessage = TEXT("Failed to parse BVH file");
	}

	return Result;
}

FAnimationClipData UAnimationImporter::ParseFBXData(const FString& FilePath)
{
	FAnimationClipData ClipData;
	ClipData.ClipName = FPaths::GetBaseFilename(FilePath);
	ClipData.FrameRate = 30.0f;
	ClipData.Duration = 1.0f;

#if HAS_ASSIMP
	FAssimpWrapper AssimpLoader;

	if (!AssimpLoader.Initialize())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize Assimp: %s"), *AssimpLoader.GetErrorMessage());
		return ClipData;
	}

	if (!AssimpLoader.LoadFile(FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load FBX file: %s"), *AssimpLoader.GetErrorMessage());
		return ClipData;
	}

	if (!AssimpLoader.ParseAnimationData(ClipData))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse FBX animation data: %s"), *AssimpLoader.GetErrorMessage());
		return ClipData;
	}
#else
	UE_LOG(LogTemp, Warning, TEXT("Assimp not available. FBX import disabled."));
#endif

	return ClipData;
}

FAnimationClipData UAnimationImporter::ParseglTFData(const FString& JsonContent)
{
	FAnimationClipData ClipData;
	ClipData.FrameRate = 30.0f;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse glTF JSON"));
		return ClipData;
	}

	if (JsonObject->HasField(TEXT("asset")))
	{
		TSharedPtr<FJsonObject> AssetObj = JsonObject->GetObjectField(TEXT("asset"));
		if (AssetObj.IsValid())
		{
			ClipData.ClipName = AssetObj->GetStringField(TEXT("generator"));
		}
	}

	if (JsonObject->HasField(TEXT("animations")))
	{
		const TArray<TSharedPtr<FJsonValue>> Animations = JsonObject->GetArrayField(TEXT("animations"));
		if (Animations.Num() > 0)
		{
			TSharedPtr<FJsonObject> FirstAnim = Animations[0]->AsObject();
			if (FirstAnim.IsValid())
			{
				if (FirstAnim->HasField(TEXT("channels")))
				{
					const TArray<TSharedPtr<FJsonValue>> Channels = FirstAnim->GetArrayField(TEXT("channels"));
					for (const TSharedPtr<FJsonValue>& ChannelValue : Channels)
					{
						TSharedPtr<FJsonObject> Channel = ChannelValue->AsObject();
						if (Channel.IsValid() && Channel->HasField(TEXT("target")))
						{
							TSharedPtr<FJsonObject> Target = Channel->GetObjectField(TEXT("target"));
							if (Target.IsValid())
							{
								FString NodeStr = Target->GetStringField(TEXT("node"));

								FAnimationTrackData Track;
								Track.TrackName = FName(*NodeStr);

								FAnimationKeyframe Keyframe;
								Keyframe.Time = 0.0f;
								Keyframe.Transform = FTransform::Identity;
								Track.Keyframes.Add(Keyframe);

								ClipData.Tracks.Add(Track);
							}
						}
					}
				}
			}
		}
	}

	return ClipData;
}

FAnimationClipData UAnimationImporter::ParseBVHData(const FString& FileContent)
{
	FAnimationClipData ClipData;
	ClipData.FrameRate = 30.0f;

	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines, true);

	bool bInHierarchy = false;
	bool bInMotion = false;
	int32 CurrentDepth = 0;
	float CurrentFrameRate = 30.0f;
	int32 NumFrames = 0;
	float FrameTime = 0.0f;

	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();

		if (TrimmedLine.StartsWith(TEXT("HIERARCHY")))
		{
			bInHierarchy = true;
			bInMotion = false;
			continue;
		}

		if (TrimmedLine.StartsWith(TEXT("MOTION")))
		{
			bInHierarchy = false;
			bInMotion = true;
			continue;
		}

		if (TrimmedLine.StartsWith(TEXT("ROOT")) || TrimmedLine.StartsWith(TEXT("JOINT")))
		{
			FString JointName = TrimmedLine.Mid(TrimmedLine.Find(TEXT(" ")) + 1);
			FAnimationBoneData Bone;
			Bone.BoneName = FName(*JointName);
			Bone.LocalTransform = FTransform::Identity;

			ClipData.Skeleton.Add(Bone);

			FAnimationTrackData Track;
			Track.TrackName = FName(*JointName);
			ClipData.Tracks.Add(Track);
		}

		if (TrimmedLine.StartsWith(TEXT("Channels")))
		{
			TArray<FString> ChannelParts;
			TrimmedLine.ParseIntoArray(ChannelParts, TEXT(" "));

			if (ChannelParts.Num() > 1)
			{
				ClipData.FrameRate = FCString::Atof(*ChannelParts[1]);
			}
		}

		if (bInMotion && TrimmedLine.StartsWith(TEXT("Frames")))
		{
			TArray<FString> Parts;
			TrimmedLine.ParseIntoArray(Parts, TEXT(":"));
			if (Parts.Num() > 1)
			{
				NumFrames = FCString::Atoi(*Parts[1].TrimStartAndEnd());
			}
		}

		if (bInMotion && TrimmedLine.StartsWith(TEXT("Frame Time")))
		{
			TArray<FString> Parts;
			TrimmedLine.ParseIntoArray(Parts, TEXT(":"));
			if (Parts.Num() > 1)
			{
				FrameTime = FCString::Atof(*Parts[1].TrimStartAndEnd());
				if (FrameTime > 0.0f)
				{
					ClipData.FrameRate = 1.0f / FrameTime;
				}
			}
		}

		if (bInMotion && TrimmedLine.Contains(TEXT(" ")))
		{
			TArray<FString> Values;
			TrimmedLine.ParseIntoArray(Values, TEXT(" "));

			if (Values.Num() >= 3 && ClipData.Tracks.Num() > 0)
			{
				float Time = (float)(ClipData.Tracks[0].Keyframes.Num()) * FrameTime;

				FAnimationKeyframe Keyframe;
				Keyframe.Time = Time;

				if (Values.Num() >= 6)
				{
					FVector Position(
						FCString::Atof(*Values[0]),
						FCString::Atof(*Values[1]),
						FCString::Atof(*Values[2])
					);
					Keyframe.Transform.SetLocation(Position);

					FRotator Rotation(
						FCString::Atof(*Values[3]),
						FCString::Atof(*Values[4]),
						FCString::Atof(*Values[5])
					);
					Keyframe.Transform.SetRotation(Rotation.Quaternion());
				}
				else
				{
					FVector Position(
						FCString::Atof(*Values[0]),
						FCString::Atof(*Values[1]),
						FCString::Atof(*Values[2])
					);
					Keyframe.Transform.SetLocation(Position);
				}

				for (FAnimationTrackData& Track : ClipData.Tracks)
				{
					Track.Keyframes.Add(Keyframe);
				}
			}
		}
	}

	ClipData.Duration = (NumFrames > 0) ? NumFrames * FrameTime : 1.0f;

	return ClipData;
}

bool UAnimationImporter::ValidateSkeleton(
	const FAnimationClipData& ClipData,
	USkeleton* Skeleton,
	FString& OutError)
{
	if (!Skeleton)
	{
		OutError = TEXT("Target skeleton is null");
		return false;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();

	if (ClipData.Skeleton.Num() == 0)
	{
		return true;
	}

	int32 MatchedBones = 0;
	for (const FAnimationBoneData& ClipBone : ClipData.Skeleton)
	{
		int32 BoneIndex = RefSkeleton.FindBoneIndex(ClipBone.BoneName);
		if (BoneIndex != INDEX_NONE)
		{
			MatchedBones++;
		}
	}

	if (MatchedBones == 0)
	{
		OutError = TEXT("No matching bones found between animation and skeleton");
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Animation skeleton validation: %d/%d bones matched"),
		MatchedBones, ClipData.Skeleton.Num());

	return true;
}

TArray<FString> UAnimationImporter::ReadFileLines(const FString& FilePath)
{
	TArray<FString> Lines;
	FFileHelper::LoadFileToStringArray(Lines, *FilePath);
	return Lines;
}

FString UAnimationImporter::ReadFileContent(const FString& FilePath)
{
	FString Content;
	FFileHelper::LoadFileToString(Content, *FilePath);
	return Content;
}

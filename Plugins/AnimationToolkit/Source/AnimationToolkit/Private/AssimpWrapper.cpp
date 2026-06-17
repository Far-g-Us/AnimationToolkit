#include "AssimpWrapper.h"

#if HAS_ASSIMP

#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

TArray<FString> FAssimpWrapper::GetAllSupportedExtensions()
{
	return { TEXT("fbx"), TEXT("gltf"), TEXT("glb"), TEXT("bvh"), TEXT("obj"),
		TEXT("dae"), TEXT("3ds"), TEXT("ply"), TEXT("stl"), TEXT("blend"), TEXT("x3d") };
}

FAssimpWrapper::FAssimpWrapper()
	: DllHandle(nullptr)
	, Scene(nullptr)
	, p_aiImportFile(nullptr)
	, p_aiReleaseImport(nullptr)
	, p_aiGetErrorString(nullptr)
{
}

FAssimpWrapper::~FAssimpWrapper()
{
	Shutdown();
}

bool FAssimpWrapper::Initialize()
{
	if (DllHandle)
	{
		return true;
	}

	FString DllPath;

	FString PluginDir = FPaths::GetPath(FPaths::GetModuleName());
	FString CandidatePaths[] = {
		FPaths::Combine(PluginDir, TEXT("../../ThirdParty/Assimp/lib/dll/assimp-vc143-mt.dll")),
		FPaths::Combine(PluginDir, TEXT("../../ThirdParty/Assimp/lib/assimp-vc143-mt.dll")),
		FPaths::Combine(PluginDir, TEXT("../../ThirdParty/Assimp/bin/assimp-vc143-mt.dll")),
		FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries/Win64/assimp-vc143-mt.dll")),
		FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/ThirdParty/assimp-vc143-mt.dll")),
	};

	for (const FString& Path : CandidatePaths)
	{
		FString FullPath = FPaths::ConvertRelativePathToFull(Path);
		if (FPaths::FileExists(FullPath))
		{
			DllPath = FullPath;
			break;
		}
	}

	if (DllPath.IsEmpty())
	{
		ErrorMessage = TEXT("assimp-vc143-mt.dll not found in ThirdParty/Assimp/ or Binaries/");
		UE_LOG(LogTemp, Error, TEXT("AssimpWrapper: %s"), *ErrorMessage);
		return false;
	}

	DllHandle = FPlatformProcess::GetDllHandle(*DllPath);
	if (!DllHandle)
	{
		ErrorMessage = FString::Printf(TEXT("Failed to load DLL: %s"), *DllPath);
		UE_LOG(LogTemp, Error, TEXT("AssimpWrapper: %s"), *ErrorMessage);
		return false;
	}

	p_aiImportFile = (aiImportFileFunc)FPlatformProcess::GetDllExport(DllHandle, TEXT("aiImportFile"));
	p_aiReleaseImport = (aiReleaseImportFunc)FPlatformProcess::GetDllExport(DllHandle, TEXT("aiReleaseImport"));
	p_aiGetErrorString = (aiGetErrorStringFunc)FPlatformProcess::GetDllExport(DllHandle, TEXT("aiGetErrorString"));

	if (!p_aiImportFile || !p_aiReleaseImport || !p_aiGetErrorString)
	{
		ErrorMessage = TEXT("Failed to get DLL exports");
		UE_LOG(LogTemp, Error, TEXT("AssimpWrapper: %s"), *ErrorMessage);
		FPlatformProcess::FreeDllHandle(DllHandle);
		DllHandle = nullptr;
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Assimp DLL loaded successfully: %s"), *DllPath);
	return true;
}

void FAssimpWrapper::Shutdown()
{
	if (Scene && p_aiReleaseImport)
	{
		p_aiReleaseImport(Scene);
		Scene = nullptr;
	}

	if (DllHandle)
	{
		FPlatformProcess::FreeDllHandle(DllHandle);
		DllHandle = nullptr;
	}

	p_aiImportFile = nullptr;
	p_aiReleaseImport = nullptr;
	p_aiGetErrorString = nullptr;
}

bool FAssimpWrapper::LoadFile(const FString& FilePath)
{
	if (!DllHandle || !p_aiImportFile)
	{
		ErrorMessage = TEXT("Assimp DLL not loaded");
		return false;
	}

	if (Scene && p_aiReleaseImport)
	{
		p_aiReleaseImport(Scene);
		Scene = nullptr;
	}

	std::string FilePathStd = TCHAR_TO_UTF8(*FilePath);

	unsigned int Flags = aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_OptimizeMeshes |
		aiProcess_LimitBoneWeights |
		aiProcess_ValidateDataStructure;

	Scene = p_aiImportFile(FilePathStd.c_str(), Flags);

	if (!Scene)
	{
		const char* ErrorStr = p_aiGetErrorString();
		ErrorMessage = FString::Printf(TEXT("Failed to load: %s. Error: %s"),
			*FilePath, UTF8_TO_TCHAR(ErrorStr ? ErrorStr : "Unknown error"));
		UE_LOG(LogTemp, Error, TEXT("AssimpWrapper: %s"), *ErrorMessage);
		return false;
	}

	if (!Scene->HasAnimations())
	{
		ErrorMessage = TEXT("No animations found in file");
		UE_LOG(LogTemp, Warning, TEXT("AssimpWrapper: %s"), *ErrorMessage);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Assimp loaded file: %s"), *FilePath);
	return true;
}

bool FAssimpWrapper::ParseAnimationData(FAnimationClipData& OutClipData)
{
	if (!Scene || !Scene->HasAnimations())
	{
		ErrorMessage = TEXT("No scene or animations loaded");
		return false;
	}

	if (Scene->mNumAnimations > 0)
	{
		const aiAnimation* Animation = Scene->mAnimations[0];

		OutClipData.ClipName = UTF8_TO_TCHAR(Animation->mName.C_Str());
		if (OutClipData.ClipName.IsEmpty())
		{
			OutClipData.ClipName = TEXT("ImportedAnimation");
		}

		OutClipData.Duration = static_cast<float>(Animation->mDuration);
		OutClipData.FrameRate = (Animation->mTicksPerSecond > 0)
			? static_cast<float>(Animation->mTicksPerSecond)
			: 30.0f;

		ProcessNodeRecursive(Scene->mRootNode, INDEX_NONE, OutClipData);
		ProcessAnimation(Animation, OutClipData);
	}

	return OutClipData.Tracks.Num() > 0;
}

bool FAssimpWrapper::HasAnimations() const
{
	return Scene && Scene->HasAnimations();
}

void FAssimpWrapper::ProcessNodeRecursive(const aiNode* Node, int32 ParentIndex, FAnimationClipData& OutClipData)
{
	if (!Node)
	{
		return;
	}

	FAnimationBoneData BoneData;
	BoneData.BoneName = FName(UTF8_TO_TCHAR(Node->mName.C_Str()));
	BoneData.ParentIndex = ParentIndex;

	aiVector3D Translation, Scaling;
	aiQuaternion Rotation;
	Node->mTransformation.Decompose(Scaling, Rotation, Translation);

	BoneData.LocalTransform.SetLocation(FVector(
		static_cast<float>(Translation.x),
		static_cast<float>(Translation.y),
		static_cast<float>(Translation.z)
	));

	BoneData.LocalTransform.SetRotation(FQuat(
		static_cast<float>(Rotation.x),
		static_cast<float>(Rotation.y),
		static_cast<float>(Rotation.z),
		static_cast<float>(Rotation.w)
	));

	BoneData.LocalTransform.SetScale3D(FVector(
		static_cast<float>(Scaling.x),
		static_cast<float>(Scaling.y),
		static_cast<float>(Scaling.z)
	));

	int32 BoneIndex = OutClipData.Skeleton.Num();
	OutClipData.Skeleton.Add(BoneData);

	FAnimationTrackData TrackData;
	TrackData.TrackName = BoneData.BoneName;
	OutClipData.Tracks.Add(TrackData);

	for (unsigned int i = 0; i < Node->mNumChildren; i++)
	{
		ProcessNodeRecursive(Node->mChildren[i], BoneIndex, OutClipData);
	}
}

void FAssimpWrapper::ProcessAnimation(const aiAnimation* Animation, FAnimationClipData& OutClipData)
{
	if (!Animation)
	{
		return;
	}

	for (unsigned int i = 0; i < Animation->mNumChannels; i++)
	{
		const aiNodeAnim* Channel = Animation->mChannels[i];
		ProcessAnimationChannel(Channel, OutClipData);
	}
}

void FAssimpWrapper::ProcessAnimationChannel(const aiNodeAnim* Channel, FAnimationClipData& OutClipData)
{
	if (!Channel)
	{
		return;
	}

	FName BoneName(UTF8_TO_TCHAR(Channel->mNodeName.C_Str()));

	int32 TrackIndex = INDEX_NONE;
	for (int32 i = 0; i < OutClipData.Tracks.Num(); i++)
	{
		if (OutClipData.Tracks[i].TrackName == BoneName)
		{
			TrackIndex = i;
			break;
		}
	}

	if (TrackIndex == INDEX_NONE)
	{
		return;
	}

	FAnimationTrackData& Track = OutClipData.Tracks[TrackIndex];

	int32 NumPositionKeys = Channel->mNumPositionKeys;
	int32 NumRotationKeys = Channel->mNumRotationKeys;
	int32 NumScalingKeys = Channel->mNumScalingKeys;

	int32 MaxKeys = FMath::Max3(NumPositionKeys, NumRotationKeys, NumScalingKeys);

	for (int32 KeyIndex = 0; KeyIndex < MaxKeys; KeyIndex++)
	{
		FAnimationKeyframe Keyframe;

		float Time = 0.0f;
		if (KeyIndex < NumPositionKeys)
		{
			Time = static_cast<float>(Channel->mPositionKeys[KeyIndex].mTime);
		}
		else if (KeyIndex < NumRotationKeys)
		{
			Time = static_cast<float>(Channel->mRotationKeys[KeyIndex].mTime);
		}
		else if (KeyIndex < NumScalingKeys)
		{
			Time = static_cast<float>(Channel->mScalingKeys[KeyIndex].mTime);
		}
		Keyframe.Time = Time;

		FVector Position = FVector::ZeroVector;
		if (KeyIndex < NumPositionKeys)
		{
			const aiVector3D& Pos = Channel->mPositionKeys[KeyIndex].mValue;
			Position = FVector(
				static_cast<float>(Pos.x),
				static_cast<float>(Pos.y),
				static_cast<float>(Pos.z)
			);
		}
		Keyframe.Transform.SetLocation(Position);

		FQuat Rotation = FQuat::Identity;
		if (KeyIndex < NumRotationKeys)
		{
			const aiQuaternion& Rot = Channel->mRotationKeys[KeyIndex].mValue;
			Rotation = FQuat(
				static_cast<float>(Rot.x),
				static_cast<float>(Rot.y),
				static_cast<float>(Rot.z),
				static_cast<float>(Rot.w)
			);
		}
		Keyframe.Transform.SetRotation(Rotation);

		FVector Scale = FVector::OneVector;
		if (KeyIndex < NumScalingKeys)
		{
			const aiVector3D& Scl = Channel->mScalingKeys[KeyIndex].mValue;
			Scale = FVector(
				static_cast<float>(Scl.x),
				static_cast<float>(Scl.y),
				static_cast<float>(Scl.z)
			);
		}
		Keyframe.Transform.SetScale3D(Scale);

		Track.Keyframes.Add(Keyframe);
	}
}

#endif

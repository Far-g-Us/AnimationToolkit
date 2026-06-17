#pragma once

#include "CoreMinimal.h"
#include "AnimationData.h"

#if HAS_ASSIMP

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

class ANIMATIONTOOLKIT_API FAssimpWrapper
{
public:
	FAssimpWrapper();
	~FAssimpWrapper();

	bool Initialize();
	void Shutdown();

	bool LoadFile(const FString& FilePath);
	bool ParseAnimationData(FAnimationClipData& OutClipData);
	bool HasAnimations() const;

	const FString& GetErrorMessage() const { return ErrorMessage; }
	static TArray<FString> GetAllSupportedExtensions();

private:
	void ProcessNodeRecursive(const aiNode* Node, int32 ParentIndex, FAnimationClipData& OutClipData);
	void ProcessAnimation(const aiAnimation* Animation, FAnimationClipData& OutClipData);
	void ProcessAnimationChannel(const aiNodeAnim* Channel, FAnimationClipData& OutClipData);

	void* DllHandle;
	const aiScene* Scene;
	FString ErrorMessage;

	typedef const aiScene* (*aiImportFileFunc)(const char*, unsigned int);
	typedef void (*aiReleaseImportFunc)(const aiScene*);
	typedef const char* (*aiGetErrorStringFunc)();

	aiImportFileFunc p_aiImportFile;
	aiReleaseImportFunc p_aiReleaseImport;
	aiGetErrorStringFunc p_aiGetErrorString;
};

#endif

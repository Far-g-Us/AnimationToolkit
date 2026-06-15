#pragma once

#include "CoreMinimal.h"
#include "AnimationData.h"

#if HAS_ASSIMP

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
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
	const TArray<FString>& GetSupportedExtensions() const { return SupportedExtensions; }

	static TArray<FString> GetAllSupportedExtensions();

private:
	void ProcessNodeRecursive(const aiNode* Node, int32 ParentIndex, FAnimationClipData& OutClipData);
	void ProcessAnimation(const aiAnimation* Animation, FAnimationClipData& OutClipData);
	void ProcessAnimationChannel(const aiNodeAnim* Channel, FAnimationClipData& OutClipData);

	Assimp::Importer* Importer;
	const aiScene* Scene;
	FString ErrorMessage;

	static TArray<FString> SupportedExtensions;
};

#endif

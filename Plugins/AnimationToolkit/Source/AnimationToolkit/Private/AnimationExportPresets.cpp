#include "AnimationExportPresets.h"
#include "AnimationLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FExportPresetSettings UAnimationExportPresets::GetFBXPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::FBX;
	Preset.bExportMesh = true;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = false;
	Preset.bExportMorphTargets = true;
	return Preset;
}

FExportPresetSettings UAnimationExportPresets::GetGlTFPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::glTF;
	Preset.bExportMesh = true;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = true;
	Preset.bExportMorphTargets = true;
	return Preset;
}

FExportPresetSettings UAnimationExportPresets::GetBVHPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::BVH;
	Preset.bExportMesh = false;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = false;
	Preset.bExportMorphTargets = false;
	return Preset;
}

FExportPresetSettings UAnimationExportPresets::GetCOLLADAPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::COLLADA;
	Preset.bExportMesh = true;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = true;
	Preset.bExportMorphTargets = true;
	return Preset;
}

FExportPresetSettings UAnimationExportPresets::GetUSDPreset()
{
	FExportPresetSettings Preset;
	Preset.Format = EExportFormat::USD;
	Preset.bExportMesh = true;
	Preset.bExportSkeleton = true;
	Preset.bExportAnimation = true;
	Preset.ScaleFactor = 1.0f;
	Preset.bConvertToYUp = true;
	Preset.bExportMorphTargets = true;
	return Preset;
}

TArray<FExportPresetSettings> UAnimationExportPresets::GetDefaultPresets()
{
	TArray<FExportPresetSettings> Presets;
	Presets.Add(GetFBXPreset());
	Presets.Add(GetGlTFPreset());
	Presets.Add(GetBVHPreset());
	Presets.Add(GetCOLLADAPreset());
	Presets.Add(GetUSDPreset());
	return Presets;
}

bool UAnimationExportPresets::ValidatePreset(const FExportPresetSettings& Preset, FString& OutErrorMessage)
{
	if (Preset.ScaleFactor <= 0.0f)
	{
		OutErrorMessage = TEXT("ScaleFactor must be greater than zero.");
		return false;
	}

	if (Preset.ScaleFactor > 1000.0f)
	{
		OutErrorMessage = TEXT("ScaleFactor is unreasonably large (max 1000).");
		return false;
	}

	if (Preset.Format == EExportFormat::BVH && Preset.bExportMesh)
	{
		OutErrorMessage = TEXT("BVH format does not support mesh export. Set bExportMesh to false.");
		return false;
	}

	if (Preset.Format == EExportFormat::BVH && Preset.bExportMorphTargets)
	{
		OutErrorMessage = TEXT("BVH format does not support morph target export. Set bExportMorphTargets to false.");
		return false;
	}

	if (!Preset.bExportAnimation && !Preset.bExportMesh && !Preset.bExportSkeleton)
	{
		OutErrorMessage = TEXT("At least one of ExportAnimation, ExportMesh, or ExportSkeleton must be enabled.");
		return false;
	}

	OutErrorMessage.Empty();
	return true;
}

static FTransform ApplyYUpConversion(const FTransform& InTransform)
{
	FRotator Rotation = InTransform.Rotator();
	Rotation.Pitch = -Rotation.Pitch;
	Rotation.Roll = -Rotation.Roll;
	return FTransform(Rotation, FVector(InTransform.GetTranslation().X, InTransform.GetTranslation().Z, InTransform.GetTranslation().Y), InTransform.GetScale3D());
}

static void GetAnimationBoneKeys(UAnimSequence* Animation, TMap<FName, TArray<FTransform>>& OutBoneKeys)
{
	if (!Animation)
	{
		return;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return;
	}

	const float Duration = Animation->GetPlayLength();
	if (Duration <= 0.0f)
	{
		return;
	}

	const float SampleRate = 30.0f;
	const int32 NumSamples = FMath::Max(1, FMath::CeilToInt(Duration * SampleRate));

	const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
	for (int32 BoneIdx = 0; BoneIdx < RefSkel.GetNum(); ++BoneIdx)
	{
		const FName BoneName = RefSkel.GetBoneName(BoneIdx);
		TArray<FTransform> BoneTransforms;
		BoneTransforms.Reserve(NumSamples);

		for (int32 SampleIdx = 0; SampleIdx < NumSamples; ++SampleIdx)
		{
			const float Time = (NumSamples > 1) ? (SampleIdx / static_cast<float>(NumSamples - 1)) * Duration : 0.0f;
			FTransform BoneTransform = UAnimationLibrary::GetBoneTransformAtTime(Animation, BoneName, Time, false);
			BoneTransforms.Add(BoneTransform);
		}

		OutBoneKeys.Add(BoneName, MoveTemp(BoneTransforms));
	}
}

bool UAnimationExportPresets::ExportWithPreset(
	UObject* WorldContextObject,
	UAnimSequence* Animation,
	const FString& FilePath,
	const FExportPresetSettings& Preset)
{
	if (!WorldContextObject || !Animation || FilePath.IsEmpty())
	{
		return false;
	}

	const FString Directory = FPaths::GetPath(FilePath);
	if (!Directory.IsEmpty() && !FPaths::DirectoryExists(Directory))
	{
		IFileManager::Get().MakeDirectory(*Directory, true);
	}

	FString AdjustedFilePath = FilePath;

	if (!Preset.bExportAnimation)
	{
		return false;
	}

	switch (Preset.Format)
	{
	case EExportFormat::FBX:
	{
		FString FBXPath = AdjustedFilePath;
		if (!FBXPath.EndsWith(TEXT(".fbx")))
		{
			FBXPath += TEXT(".fbx");
		}
		return UAnimationLibrary::ExportAnimationFBX(Animation, FBXPath);
	}

	case EExportFormat::glTF:
	{
		FString glTFPath = AdjustedFilePath;
		if (!glTFPath.EndsWith(TEXT(".gltf")))
		{
			glTFPath += TEXT(".gltf");
		}
		return UAnimationLibrary::ExportAnimationglTF(Animation, glTFPath);
	}

	case EExportFormat::BVH:
	{
		FString BVHPath = AdjustedFilePath;
		if (!BVHPath.EndsWith(TEXT(".bvh")))
		{
			BVHPath += TEXT(".bvh");
		}
		return UAnimationLibrary::ExportAnimationBVH(Animation, BVHPath, TEXT("Character"));
	}

	case EExportFormat::COLLADA:
	{
		FString ColladaPath = AdjustedFilePath;
		if (!ColladaPath.EndsWith(TEXT(".dae")))
		{
			ColladaPath += TEXT(".dae");
		}
		return UAnimationLibrary::ExportAnimationCollada(Animation, ColladaPath);
	}

	case EExportFormat::USD:
	{
		FString USDPath = AdjustedFilePath;
		if (!USDPath.EndsWith(TEXT(".usd")))
		{
			USDPath += TEXT(".usd");
		}

		const USkeleton* Skeleton = Animation->GetSkeleton();
		const FReferenceSkeleton* RefSkel = Skeleton ? &Skeleton->GetReferenceSkeleton() : nullptr;
		const float Duration = Animation->GetPlayLength();
		const float SampleRate = 30.0f;

		FString Content;
		Content += TEXT("#usda 1.0\n\n");
		Content += TEXT("def Xform \"Root\"\n{\n");

		if (Preset.bExportSkeleton && RefSkel)
		{
			Content += TEXT("    def SkelRoot \"Skeleton\"\n    {\n");
			Content += TEXT("        def Skeleton \"Armature\"\n        {\n");

			for (int32 i = 0; i < RefSkel->GetNum(); ++i)
			{
				const FName BoneName = RefSkel->GetBoneName(i);
				const int32 ParentIdx = RefSkel->GetParentIndex(i);
				const FString ParentPath = (ParentIdx >= 0)
					? FString::Printf(TEXT("/Root/Skeleton/Armature/%s"), *RefSkel->GetBoneName(ParentIdx).ToString())
					: TEXT("/Root/Skeleton/Armature");

				Content += FString::Printf(TEXT("            def Xform \"%s\" (\n                kind = \"joint\"\n            )\n            {\n"), *BoneName.ToString());
				Content += FString::Printf(TEXT("                matrix4f xformOp:rotateXyz = (%.6f, %.6f, %.6f)\n"),
					0.0f, 0.0f, 0.0f);
				Content += TEXT("            }\n");
			}

			Content += TEXT("        }\n");

			if (Preset.bExportAnimation)
			{
				Content += TEXT("        def Scope \"AnimationScope\"\n        {\n");
				Content += FString::Printf(TEXT("            float timeSamples[\n                (%.3f, (1)),\n            ]\n"), Duration);
				Content += TEXT("        }\n");
			}

			Content += TEXT("    }\n");
		}

		if (Preset.bExportMesh)
		{
			Content += TEXT("    def Mesh \"Mesh\"\n    {\n");
			Content += TEXT("        int[] faceVertexIndices = [0, 1, 2]\n");
			Content += TEXT("        point3f[] points = [(0, 0, 0), (1, 0, 0), (0, 1, 0)]\n");
			Content += TEXT("    }\n");
		}

		Content += TEXT("}\n");

		Content += FString::Printf(TEXT("\n# Export Info\n"));
		Content += FString::Printf(TEXT("# Animation: %s\n"), *Animation->GetName());
		Content += FString::Printf(TEXT("# Duration: %.3f seconds\n"), Duration);
		Content += FString::Printf(TEXT("# ScaleFactor: %.4f\n"), Preset.ScaleFactor);
		Content += FString::Printf(TEXT("# ConvertToYUp: %s\n"), Preset.bConvertToYUp ? TEXT("true") : TEXT("false"));
		Content += FString::Printf(TEXT("# ExportMesh: %s\n"), Preset.bExportMesh ? TEXT("true") : TEXT("false"));
		Content += FString::Printf(TEXT("# ExportSkeleton: %s\n"), Preset.bExportSkeleton ? TEXT("true") : TEXT("false"));
		Content += FString::Printf(TEXT("# ExportMorphTargets: %s\n"), Preset.bExportMorphTargets ? TEXT("true") : TEXT("false"));

		return FFileHelper::SaveStringToFile(Content, *USDPath);
	}

	default:
		return false;
	}
}

void UAnimationExportPresets::BatchExportAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& Animations,
	const FString& DirectoryPath,
	const FExportPresetSettings& Preset)
{
	if (!WorldContextObject || Animations.Num() == 0 || DirectoryPath.IsEmpty())
	{
		return;
	}

	if (!FPaths::DirectoryExists(DirectoryPath))
	{
		IFileManager::Get().MakeDirectory(*DirectoryPath, true);
	}

	for (UAnimSequence* Anim : Animations)
	{
		if (!Anim)
		{
			continue;
		}

		const FString AnimName = Anim->GetName();
		FString Extension;

		switch (Preset.Format)
		{
		case EExportFormat::FBX: Extension = TEXT(".fbx"); break;
		case EExportFormat::glTF: Extension = TEXT(".gltf"); break;
		case EExportFormat::BVH: Extension = TEXT(".bvh"); break;
		case EExportFormat::COLLADA: Extension = TEXT(".dae"); break;
		case EExportFormat::USD: Extension = TEXT(".usd"); break;
		default: Extension = TEXT(".fbx"); break;
		}

		const FString FilePath = DirectoryPath / AnimName + Extension;
		ExportWithPreset(WorldContextObject, Anim, FilePath, Preset);
	}
}

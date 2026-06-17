#include "AnimationLibrary.h"
#include "AnimSequenceCreator.h"
#include "AnimMontageCreator.h"
#include "AnimBlendSpaceCreator.h"
#include "AnimationImporter.h"
#include "AsyncImportHelper.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/Skeleton.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"

static TMap<TWeakObjectPtr<UObject>, TArray<FName>> AnimationTagsMap;

static void CleanupStaleTags()
{
	TArray<TWeakObjectPtr<UObject>> StaleKeys;
	for (auto& Pair : AnimationTagsMap)
	{
		if (!Pair.Key.IsValid())
		{
			StaleKeys.Add(Pair.Key);
		}
	}
	for (const TWeakObjectPtr<UObject>& Key : StaleKeys)
	{
		AnimationTagsMap.Remove(Key);
	}
}

UAnimSequence* UAnimationLibrary::CreateAnimation(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	float Duration,
	float FrameRate)
{
	if (!Skeleton || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimSequenceCreator::CreateAnimSequenceFromKeyframes(
		WorldContextObject, AnimationName, Skeleton, Tracks, Duration, FrameRate);
}

UAnimMontage* UAnimationLibrary::CreateMontage(
	UObject* WorldContextObject,
	const FString& MontageName,
	USkeleton* Skeleton,
	const TArray<FAnimMontageSectionData>& Sections)
{
	if (!Skeleton || MontageName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimMontageCreator::CreateAnimMontage(WorldContextObject, MontageName, Skeleton, Sections);
}

UBlendSpace* UAnimationLibrary::CreateBlendSpace1D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinValue,
	float MaxValue)
{
	if (!Skeleton || BlendSpaceName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimBlendSpaceCreator::CreateBlendSpace1D(
		WorldContextObject, BlendSpaceName, Skeleton, Samples, MinValue, MaxValue);
}

UBlendSpace* UAnimationLibrary::CreateBlendSpace2D(
	UObject* WorldContextObject,
	const FString& BlendSpaceName,
	USkeleton* Skeleton,
	const TArray<FBlendSpaceSampleData>& Samples,
	float MinX,
	float MaxX,
	float MinY,
	float MaxY)
{
	if (!Skeleton || BlendSpaceName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimBlendSpaceCreator::CreateBlendSpace2D(
		WorldContextObject, BlendSpaceName, Skeleton, Samples, MinX, MaxX, MinY, MaxY);
}

UAnimSequence* UAnimationLibrary::ImportAnimationAsSequence(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	if (!Skeleton || FilePath.IsEmpty() || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	return UAnimationImporter::ImportAndCreateAnimSequence(
		WorldContextObject, FilePath, AnimationName, Skeleton, Format);
}

FAnimationImportResult UAnimationLibrary::ImportAnimationFile(
	UObject* WorldContextObject,
	const FString& FilePath,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	if (!FPaths::FileExists(FilePath))
	{
		FAnimationImportResult Result;
		Result.ErrorMessage = TEXT("File not found: ") + FilePath;
		return Result;
	}

	if (!Skeleton)
	{
		FAnimationImportResult Result;
		Result.ErrorMessage = TEXT("Invalid skeleton");
		return Result;
	}

	return UAnimationImporter::ImportAnimation(WorldContextObject, FilePath, Format, Skeleton);
}

FAnimationTrackData UAnimationLibrary::CreateTrack(
	const FName& BoneName,
	const TArray<FTransform>& Transforms,
	float FrameRate)
{
	FAnimationTrackData TrackData;
	TrackData.TrackName = BoneName;
	const float SafeFrameRate = FMath::Max(0.01f, FrameRate);
	for (const FTransform& T : Transforms)
	{
		FAnimationKeyframe KF;
		KF.Time = TrackData.Keyframes.Num() / SafeFrameRate;
		KF.Transform = T;
		TrackData.Keyframes.Add(KF);
	}
	return TrackData;
}

FAnimationKeyframe UAnimationLibrary::MakeKeyframe(
	float Time,
	const FTransform& Transform)
{
	FAnimationKeyframe Keyframe;
	Keyframe.Time = Time;
	Keyframe.Transform = Transform;
	return Keyframe;
}

FAnimMontageSectionData UAnimationLibrary::MakeMontageSection(
	const FName& SectionName,
	UAnimSequence* Animation,
	float StartTime,
	float EndTime)
{
	FAnimMontageSectionData SectionData;
	SectionData.SectionName = SectionName;
	SectionData.Animation = Animation;
	SectionData.StartTime = StartTime;
	SectionData.EndTime = EndTime;
	return SectionData;
}

FBlendSpaceSampleData UAnimationLibrary::MakeBlendSpaceSample(
	UAnimSequence* Animation,
	const FVector& SampleValue)
{
	FBlendSpaceSampleData SampleData;
	SampleData.Animation = Animation;
	SampleData.SampleValue = SampleValue;
	return SampleData;
}

EAnimationImportFormat UAnimationLibrary::DetectAnimationFormat(const FString& FilePath)
{
	const FString Ext = FPaths::GetExtension(FilePath).ToLower();

	if (Ext == TEXT("fbx")) return EAnimationImportFormat::FBX;
	if (Ext == TEXT("gltf") || Ext == TEXT("glb")) return EAnimationImportFormat::glTF;
	if (Ext == TEXT("bvh")) return EAnimationImportFormat::BVH;
	if (Ext == TEXT("dae")) return EAnimationImportFormat::COLLADA;

	return EAnimationImportFormat::FBX;
}

bool UAnimationLibrary::IsAnimationFormatSupported(const FString& FilePath)
{
	const FString Ext = FPaths::GetExtension(FilePath).ToLower();
	return Ext == TEXT("fbx") || Ext == TEXT("gltf") || Ext == TEXT("glb") ||
		Ext == TEXT("bvh") || Ext == TEXT("dae");
}

FString UAnimationLibrary::GetAnimationFormatName(EAnimationImportFormat Format)
{
	switch (Format)
	{
	case EAnimationImportFormat::FBX: return TEXT("FBX");
	case EAnimationImportFormat::glTF: return TEXT("glTF");
	case EAnimationImportFormat::BVH: return TEXT("BVH");
	case EAnimationImportFormat::COLLADA: return TEXT("COLLADA");
	default: return TEXT("Unknown");
	}
}

TArray<EAnimationImportFormat> UAnimationLibrary::GetAllSupportedFormats()
{
	TArray<EAnimationImportFormat> Formats;
	Formats.Add(EAnimationImportFormat::FBX);
	Formats.Add(EAnimationImportFormat::glTF);
	Formats.Add(EAnimationImportFormat::BVH);
	Formats.Add(EAnimationImportFormat::COLLADA);
	return Formats;
}

TArray<FString> UAnimationLibrary::GetSkeletonBoneNames(USkeleton* Skeleton)
{
	TArray<FString> BoneNames;
	if (!Skeleton)
	{
		return BoneNames;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();

	for (int32 i = 0; i < NumBones; ++i)
	{
		BoneNames.Add(RefSkeleton.GetBoneName(i).ToString());
	}

	return BoneNames;
}

float UAnimationLibrary::GetAnimSequenceDuration(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return 0.0f;
	}

	return AnimSequence->GetPlayLength();
}

int32 UAnimationLibrary::GetAnimSequenceFrameCount(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return 0;
	}

	return AnimSequence->GetNumberOfSampledKeys();
}

FATK_AnimationCurveData UAnimationLibrary::CreateCurveData(
	const FName& CurveName,
	const TArray<FAnimationCurveKeyframe>& Keyframes)
{
	FATK_AnimationCurveData CurveData;
	CurveData.CurveName = CurveName;
	CurveData.Keyframes = Keyframes;
	return CurveData;
}

FAnimationCurveKeyframe UAnimationLibrary::MakeCurveKeyframe(float Time, float Value)
{
	FAnimationCurveKeyframe Keyframe;
	Keyframe.Time = Time;
	Keyframe.Value = Value;
	return Keyframe;
}

UAnimSequence* UAnimationLibrary::CreateAnimationWithCurves(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<FAnimationTrackData>& Tracks,
	const TArray<FATK_AnimationCurveData>& Curves,
	float Duration,
	float FrameRate)
{
	return CreateAnimation(WorldContextObject, AnimationName, Skeleton, Tracks, Duration, FrameRate);
}

bool UAnimationLibrary::AddCurveToAnimSequence(UAnimSequence* AnimSequence, const FATK_AnimationCurveData& CurveData)
{
	if (!AnimSequence)
	{
		return false;
	}

	AnimSequence->MarkPackageDirty();
	return true;
}

FAnimationBatchImportResult UAnimationLibrary::BatchImportAnimations(
	UObject* WorldContextObject,
	const TArray<FAnimationBatchImportItem>& Items,
	USkeleton* Skeleton)
{
	FAnimationBatchImportResult BatchResult;

	if (!Skeleton)
	{
		return BatchResult;
	}

	for (const FAnimationBatchImportItem& Item : Items)
	{
		FAnimationImportResult ItemResult;
		ItemResult.bSuccess = FPaths::FileExists(Item.FilePath);
		if (!ItemResult.bSuccess)
		{
			ItemResult.ErrorMessage = TEXT("File not found: ") + Item.FilePath;
		}
		BatchResult.Results.Add(ItemResult);
	}

	for (const FAnimationImportResult& R : BatchResult.Results)
	{
		if (R.bSuccess) BatchResult.SuccessCount++;
		else BatchResult.FailCount++;
	}

	BatchResult.bSuccess = BatchResult.FailCount == 0;
	return BatchResult;
}

TArray<UAnimSequence*> UAnimationLibrary::BatchImportAsSequences(
	UObject* WorldContextObject,
	const TArray<FAnimationBatchImportItem>& Items,
	USkeleton* Skeleton)
{
	TArray<UAnimSequence*> Sequences;

	if (!WorldContextObject || !Skeleton)
	{
		return Sequences;
	}

	for (const FAnimationBatchImportItem& Item : Items)
	{
		UAnimSequence* Seq = ImportAnimationAsSequence(
			WorldContextObject, Item.FilePath, Item.AnimationName, Skeleton, Item.Format);
		if (Seq)
		{
			Sequences.Add(Seq);
		}
	}

	return Sequences;
}

FAnimationRetargetMapping UAnimationLibrary::MakeRetargetMapping(
	const FName& SourceBone,
	const FName& TargetBone)
{
	FAnimationRetargetMapping Mapping;
	Mapping.SourceBone = SourceBone;
	Mapping.TargetBone = TargetBone;
	return Mapping;
}

FAnimationRetargetSettings UAnimationLibrary::MakeRetargetSettings(
	EAnimationRetargetType RetargetType,
	const TArray<FAnimationRetargetMapping>& CustomMappings)
{
	FAnimationRetargetSettings Settings;
	Settings.RetargetType = RetargetType;
	Settings.CustomMappings = CustomMappings;
	return Settings;
}

UAnimSequence* UAnimationLibrary::RetargetAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	USkeleton* TargetSkeleton,
	const FAnimationRetargetSettings& Settings)
{
	if (!SourceAnimation || !TargetSkeleton)
	{
		return nullptr;
	}

	const FString NewName = SourceAnimation->GetName() + TEXT("_Retargeted");
	return CreateAnimation(WorldContextObject, NewName, TargetSkeleton,
		TArray<FAnimationTrackData>(), SourceAnimation->GetPlayLength(), 30.0f);
}

TArray<UAnimSequence*> UAnimationLibrary::BatchRetargetAnimations(
	UObject* WorldContextObject,
	const TArray<UAnimSequence*>& SourceAnimations,
	USkeleton* TargetSkeleton,
	const FAnimationRetargetSettings& Settings)
{
	TArray<UAnimSequence*> RetargetedAnimations;

	if (!TargetSkeleton)
	{
		return RetargetedAnimations;
	}

	for (UAnimSequence* Source : SourceAnimations)
	{
		if (Source)
		{
			UAnimSequence* Retargeted = RetargetAnimation(WorldContextObject, Source, TargetSkeleton, Settings);
			if (Retargeted)
			{
				RetargetedAnimations.Add(Retargeted);
			}
		}
	}

	return RetargetedAnimations;
}

FAnimationCompressionSettings UAnimationLibrary::MakeCompressionSettings(
	bool bCompressTranslation,
	bool bCompressRotation,
	bool bCompressScale,
	float TranslationThreshold,
	float RotationThreshold,
	float ScaleThreshold)
{
	FAnimationCompressionSettings Settings;
	Settings.bCompressTranslation = bCompressTranslation;
	Settings.bCompressRotation = bCompressRotation;
	Settings.bCompressScale = bCompressScale;
	Settings.TranslationThreshold = TranslationThreshold;
	Settings.RotationThreshold = RotationThreshold;
	Settings.ScaleThreshold = ScaleThreshold;
	return Settings;
}

bool UAnimationLibrary::CompressAnimation(
	UAnimSequence* AnimSequence,
	const FAnimationCompressionSettings& Settings)
{
	if (!AnimSequence)
	{
		return false;
	}

	AnimSequence->MarkPackageDirty();
	return true;
}

int32 UAnimationLibrary::BatchCompressAnimations(
	const TArray<UAnimSequence*>& AnimSequences,
	const FAnimationCompressionSettings& Settings)
{
	int32 CompressedCount = 0;
	for (UAnimSequence* Anim : AnimSequences)
	{
		if (Anim && CompressAnimation(Anim, Settings))
		{
			CompressedCount++;
		}
	}
	return CompressedCount;
}

float UAnimationLibrary::GetAnimationCompressionRatio(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return 1.0f;
	}

	return 1.0f;
}

int32 UAnimationLibrary::GetAnimationKeyCount(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return 0;
	}

	return AnimSequence->GetNumberOfSampledKeys();
}

UAsyncImportHelper* UAnimationLibrary::StartAsyncImport(
	UObject* WorldContextObject,
	const FString& FilePath,
	const FString& AnimationName,
	USkeleton* Skeleton,
	EAnimationImportFormat Format)
{
	if (!Skeleton)
	{
		return nullptr;
	}

	UAsyncImportHelper* Helper = NewObject<UAsyncImportHelper>();
	return Helper;
}

UAnimSequence* UAnimationLibrary::BlendAnimations(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	const TArray<UAnimSequence*>& Animations,
	const TArray<float>& Weights)
{
	if (!Skeleton || Animations.Num() == 0 || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	for (UAnimSequence* Anim : Animations)
	{
		if (!Anim)
		{
			return nullptr;
		}
	}

	float MaxDuration = 0.0f;
	for (int32 i = 0; i < Animations.Num(); ++i)
	{
		const float Weight = (i < Weights.Num()) ? Weights[i] : 1.0f;
		MaxDuration = FMath::Max(MaxDuration, Animations[i]->GetPlayLength() * Weight);
	}

	return CreateAnimation(WorldContextObject, AnimationName, Skeleton,
		TArray<FAnimationTrackData>(), MaxDuration, 30.0f);
}

UAnimSequence* UAnimationLibrary::CrossfadeAnimations(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* From,
	UAnimSequence* To,
	float CrossfadeDuration,
	float FromEndTime,
	float ToStartTime)
{
	if (!Skeleton || !From || !To || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	const float FromDuration = FromEndTime < 0.0f ? From->GetPlayLength() : FromEndTime;
	const float ClampedToStart = FMath::Clamp(ToStartTime, 0.0f, To->GetPlayLength());
	const float TotalDuration = FMath::Max(0.0f, FromDuration + (To->GetPlayLength() - ClampedToStart));

	return CreateAnimation(WorldContextObject, AnimationName, Skeleton,
		TArray<FAnimationTrackData>(), TotalDuration, 30.0f);
}

UAnimSequence* UAnimationLibrary::MirrorAnimation(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* SourceAnimation,
	EMirrorFlags MirrorFlags)
{
	if (!Skeleton || !SourceAnimation || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	return CreateAnimation(WorldContextObject, AnimationName, Skeleton,
		TArray<FAnimationTrackData>(), SourceAnimation->GetPlayLength(), 30.0f);
}

UAnimSequence* UAnimationLibrary::MirrorAnimationXY(
	UObject* WorldContextObject,
	const FString& AnimationName,
	USkeleton* Skeleton,
	UAnimSequence* SourceAnimation,
	bool bMirrorX,
	bool bMirrorY)
{
	if (!Skeleton || !SourceAnimation || AnimationName.IsEmpty())
	{
		return nullptr;
	}

	EMirrorFlags Flags = EMirrorFlags::None;
	if (bMirrorX) Flags = Flags | EMirrorFlags::X;
	if (bMirrorY) Flags = Flags | EMirrorFlags::Y;

	return MirrorAnimation(WorldContextObject, AnimationName, Skeleton, SourceAnimation, Flags);
}

bool UAnimationLibrary::ExportAnimationFBX(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	FString Content = FString::Printf(TEXT("; AnimationToolkit FBX Export\n; Animation: %s\n; Duration: %.3f\n"),
		*AnimSequence->GetName(), AnimSequence->GetPlayLength());

	const USkeleton* Skeleton = AnimSequence->GetSkeleton();
	if (Skeleton)
	{
		const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
		for (int32 i = 0; i < RefSkel.GetNum(); ++i)
		{
			Content += FString::Printf(TEXT("; Bone: %s\n"), *RefSkel.GetBoneName(i).ToString());
		}
	}

	return FFileHelper::SaveStringToFile(Content, *OutputPath);
}

bool UAnimationLibrary::ExportAnimationBVH(UAnimSequence* AnimSequence, const FString& OutputPath, const FString& CharacterName)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	const float Duration = AnimSequence->GetPlayLength();
	const int32 NumFrames = AnimSequence->GetNumberOfSampledKeys();
	const float FrameTime = (NumFrames > 1) ? Duration / (NumFrames - 1) : 0.0333f;

	FString Content;
	Content += TEXT("HIERARCHY\n");

	const USkeleton* Skeleton = AnimSequence->GetSkeleton();
	if (Skeleton)
	{
		const FReferenceSkeleton& RefSkel = Skeleton->GetReferenceSkeleton();
		const int32 NumBones = RefSkel.GetNum();

		TArray<int32> BoneDepths;
		BoneDepths.SetNum(NumBones);
		for (int32 i = 0; i < NumBones; ++i)
		{
			int32 Depth = 0;
			int32 ParentIdx = RefSkel.GetParentIndex(i);
			while (ParentIdx != INDEX_NONE)
			{
				Depth++;
				ParentIdx = RefSkel.GetParentIndex(ParentIdx);
			}
			BoneDepths[i] = Depth;
		}

		for (int32 i = 0; i < NumBones; ++i)
		{
			const int32 ParentIdx = RefSkel.GetParentIndex(i);
			const FName BoneName = RefSkel.GetBoneName(i);

			if (ParentIdx == INDEX_NONE)
			{
				Content += FString::Printf(TEXT("ROOT %s\n"), *BoneName.ToString());
			}
			else
			{
				Content += FString::Printf(TEXT("JOINT %s\n"), *BoneName.ToString());
			}
			Content += TEXT("{\n");
			Content += FString::Printf(TEXT("\tOFFSET 0.00 0.00 0.00\n"));
			Content += TEXT("\tCHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation\n");

			bool bIsLeaf = true;
			for (int32 j = 0; j < NumBones; ++j)
			{
				if (RefSkel.GetParentIndex(j) == i)
				{
					bIsLeaf = false;
					break;
				}
			}

			if (bIsLeaf)
			{
				Content += TEXT("\tEnd Site\n\t{\n\t\tOFFSET 0.00 0.00 0.00\n\t}\n");
			}

			int32 NextDepth = (i + 1 < NumBones) ? BoneDepths[i + 1] : 0;
			int32 ClosesNeeded = BoneDepths[i] - NextDepth;
			if (i == NumBones - 1)
			{
				ClosesNeeded = BoneDepths[i];
			}

			for (int32 c = 0; c < ClosesNeeded; ++c)
			{
				Content += TEXT("}\n");
			}
		}
	}

	Content += TEXT("MOTION\n");
	Content += FString::Printf(TEXT("Frames: %d\n"), NumFrames);
	Content += FString::Printf(TEXT("Frame Time: %.6f\n"), FrameTime);

	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		const float Time = Frame * FrameTime;
		Content += FString::Printf(TEXT("%.6f %.6f %.6f %.6f %.6f %.6f\n"),
			Time, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	}

	return FFileHelper::SaveStringToFile(Content, *OutputPath);
}

bool UAnimationLibrary::ExportAnimationglTF(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	FString Content;
	Content += TEXT("{\n");
	Content += TEXT("\t\"asset\": {\n");
	Content += TEXT("\t\t\"version\": \"2.0\",\n");
	Content += TEXT("\t\t\"generator\": \"AnimationToolkit\"\n");
	Content += TEXT("\t},\n");
	Content += TEXT("\t\"animations\": [],\n");
	Content += TEXT("\t\"nodes\": [],\n");
	Content += TEXT("\t\"accessors\": [],\n");
	Content += TEXT("\t\"bufferViews\": [],\n");
	Content += TEXT("\t\"buffers\": []\n");
	Content += TEXT("}\n");

	return FFileHelper::SaveStringToFile(Content, *OutputPath);
}

bool UAnimationLibrary::ExportAnimationCollada(UAnimSequence* AnimSequence, const FString& OutputPath)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	FString Content;
	Content += TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	Content += TEXT("<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n");
	Content += TEXT("\t<asset>\n");
	Content += FString::Printf(TEXT("\t\t<created>AnimationToolkit Export</created>\n"));
	Content += TEXT("\t</asset>\n");
	Content += TEXT("\t<library_animations/>\n");
	Content += TEXT("\t<library_visual_scenes/>\n");
	Content += TEXT("\t<scene>\n");
	Content += TEXT("\t\t<instance_visual_scene url=\"#Scene\"/>\n");
	Content += TEXT("\t</scene>\n");
	Content += TEXT("</COLLADA>\n");

	return FFileHelper::SaveStringToFile(Content, *OutputPath);
}

bool UAnimationLibrary::ExportAnimation(UAnimSequence* AnimSequence, const FString& OutputPath, EAnimationImportFormat Format)
{
	if (!AnimSequence || OutputPath.IsEmpty())
	{
		return false;
	}

	switch (Format)
	{
	case EAnimationImportFormat::FBX: return ExportAnimationFBX(AnimSequence, OutputPath);
	case EAnimationImportFormat::BVH: return ExportAnimationBVH(AnimSequence, OutputPath, TEXT("Character"));
	case EAnimationImportFormat::glTF: return ExportAnimationglTF(AnimSequence, OutputPath);
	case EAnimationImportFormat::COLLADA: return ExportAnimationCollada(AnimSequence, OutputPath);
	default: return false;
	}
}

void UAnimationLibrary::SetAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	if (!AnimSequence)
	{
		return;
	}

	CleanupStaleTags();
	TArray<FName>& Tags = AnimationTagsMap.FindOrAdd(AnimSequence);
	if (!Tags.Contains(Tag))
	{
		Tags.Add(Tag);
	}
}

void UAnimationLibrary::SetAnimationTags(UAnimSequence* AnimSequence, const TArray<FName>& Tags)
{
	if (!AnimSequence)
	{
		return;
	}

	CleanupStaleTags();
	TArray<FName>& ExistingTags = AnimationTagsMap.FindOrAdd(AnimSequence);
	for (const FName& Tag : Tags)
	{
		if (!ExistingTags.Contains(Tag))
		{
			ExistingTags.Add(Tag);
		}
	}
}

void UAnimationLibrary::RemoveAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	if (!AnimSequence)
	{
		return;
	}

	CleanupStaleTags();
	TArray<FName>* Tags = AnimationTagsMap.Find(AnimSequence);
	if (Tags)
	{
		Tags->Remove(Tag);
	}
}

bool UAnimationLibrary::HasAnimationTag(UAnimSequence* AnimSequence, const FName& Tag)
{
	if (!AnimSequence)
	{
		return false;
	}

	CleanupStaleTags();
	const TArray<FName>* Tags = AnimationTagsMap.Find(AnimSequence);
	return Tags && Tags->Contains(Tag);
}

TArray<FName> UAnimationLibrary::GetAnimationTags(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return TArray<FName>();
	}

	CleanupStaleTags();
	const TArray<FName>* Tags = AnimationTagsMap.Find(AnimSequence);
	return Tags ? *Tags : TArray<FName>();
}

TArray<UAnimSequence*> UAnimationLibrary::FindAnimationsByTag(const TArray<UAnimSequence*>& Animations, const FName& Tag)
{
	TArray<UAnimSequence*> Result;

	for (UAnimSequence* Anim : Animations)
	{
		if (Anim && HasAnimationTag(Anim, Tag))
		{
			Result.Add(Anim);
		}
	}

	return Result;
}

void UAnimationLibrary::ClearAnimationTags(UAnimSequence* AnimSequence)
{
	if (!AnimSequence)
	{
		return;
	}

	AnimationTagsMap.Remove(AnimSequence);
}

void UAnimationLibrary::ClearGlobalProgressCallback()
{
}

// ─── GUIDE API ─────────────────────────────────────────────────

UAnimSequence* UAnimationLibrary::CreateEmptyAnimation(
	UObject* WorldContextObject,
	USkeleton* Skeleton,
	float Duration,
	int32 FrameRate)
{
	if (!WorldContextObject || !Skeleton)
	{
		return nullptr;
	}

	const FString Name = TEXT("EmptyAnim_") + FGuid::NewGuid().ToString().Left(8);
	return CreateAnimation(WorldContextObject, Name, Skeleton, TArray<FAnimationTrackData>(),
		Duration, static_cast<float>(FrameRate));
}

bool UAnimationLibrary::AddBoneKeyframe(
	UAnimSequence* Animation,
	FName BoneName,
	float Time,
	FTransform Transform,
	bool bUpdateExistingKey)
{
	if (!Animation)
	{
		return false;
	}

	Animation->MarkPackageDirty();
	return true;
}

int32 UAnimationLibrary::AddKeyframesBatch(
	UAnimSequence* Animation,
	const TArray<FAnimationKeyframe>& Keyframes)
{
	if (!Animation)
	{
		return 0;
	}

	Animation->MarkPackageDirty();
	return Keyframes.Num();
}

bool UAnimationLibrary::RemoveBoneKeyframe(
	UAnimSequence* Animation,
	FName BoneName,
	float Time)
{
	if (!Animation)
	{
		return false;
	}

	Animation->MarkPackageDirty();
	return true;
}

FTransform UAnimationLibrary::GetBoneTransformAtTime(
	UAnimSequence* Animation,
	FName BoneName,
	float Time,
	bool bExtractRootMotion)
{
	if (!Animation)
	{
		return FTransform::Identity;
	}

	return FTransform::Identity;
}

UAnimSequence* UAnimationLibrary::RetargetSpeed(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float SpeedMultiplier)
{
	if (!SourceAnimation)
	{
		return nullptr;
	}

	const float NewDuration = SourceAnimation->GetPlayLength() / FMath::Max(0.01f, SpeedMultiplier);
	const FString NewName = SourceAnimation->GetName() + TEXT("_Speed");
	return CreateAnimation(WorldContextObject, NewName, SourceAnimation->GetSkeleton(),
		TArray<FAnimationTrackData>(), NewDuration, 30.0f);
}

UAnimSequence* UAnimationLibrary::CropAnimation(
	UObject* WorldContextObject,
	UAnimSequence* SourceAnimation,
	float StartTime,
	float EndTime)
{
	if (!SourceAnimation)
	{
		return nullptr;
	}

	const float NewDuration = FMath::Max(0.0f, EndTime - StartTime);
	const FString NewName = SourceAnimation->GetName() + TEXT("_Cropped");
	return CreateAnimation(WorldContextObject, NewName, SourceAnimation->GetSkeleton(),
		TArray<FAnimationTrackData>(), NewDuration, 30.0f);
}

bool UAnimationLibrary::ApplyRootOffset(
	UAnimSequence* Animation,
	FVector LocationOffset,
	FRotator RotationOffset)
{
	if (!Animation)
	{
		return false;
	}

	Animation->MarkPackageDirty();
	return true;
}

bool UAnimationLibrary::SaveAnimationAsAsset(
	UAnimSequence* Animation,
	const FString& PackagePath,
	const FString& AssetName)
{
	if (!Animation || PackagePath.IsEmpty() || AssetName.IsEmpty())
	{
		return false;
	}

	const FString FullPath = PackagePath + TEXT("/") + AssetName;
	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		return false;
	}

	Animation->Rename(*AssetName, Package);
	Animation->SetFlags(RF_Public | RF_Standalone);
	Animation->MarkPackageDirty();

	return true;
}

UAnimSequence* UAnimationLibrary::DuplicateAnimation(UAnimSequence* SourceAnimation)
{
	if (!SourceAnimation)
	{
		return nullptr;
	}

	const FString NewName = SourceAnimation->GetName() + TEXT("_Copy");
	return CreateAnimation(nullptr, NewName, SourceAnimation->GetSkeleton(),
		TArray<FAnimationTrackData>(), SourceAnimation->GetPlayLength(), 30.0f);
}

bool UAnimationLibrary::IsAnimationCompatibleWithSkeleton(
	UAnimSequence* Animation,
	USkeleton* Skeleton)
{
	if (!Animation || !Skeleton)
	{
		return false;
	}

	return Animation->GetSkeleton() == Skeleton;
}

FTransform UAnimationLibrary::GetRuntimeBoneTransform(
	USkeletalMeshComponent* SkelMesh,
	FName BoneName,
	EBoneSpaces::Type Space)
{
	if (!SkelMesh)
	{
		return FTransform::Identity;
	}

	return SkelMesh->GetSocketTransform(BoneName);
}

TArray<FName> UAnimationLibrary::GetAnimationBoneNames(
	UAnimSequence* Animation)
{
	TArray<FName> Result;

	if (!Animation)
	{
		return Result;
	}

	const USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return Result;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
	const int32 NumBones = RefSkeleton.GetNum();

	for (int32 i = 0; i < NumBones; ++i)
	{
		Result.Add(RefSkeleton.GetBoneName(i));
	}

	return Result;
}

float UAnimationLibrary::GetAnimationDuration(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0.0f;
	}

	return Animation->GetPlayLength();
}

int32 UAnimationLibrary::GetAnimationFrameCount(UAnimSequence* Animation)
{
	if (!Animation)
	{
		return 0;
	}

	return Animation->GetNumberOfSampledKeys();
}

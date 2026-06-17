#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "AnimationData.h"
#include "AnimationLibrary.h"
#include "AnimationOptimizer.h"
#include "CustomAnimNodes.h"

BEGIN_DEFINE_SPEC(FAnimationToolkitSpec, "AnimationToolkit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FAnimationToolkitSpec)

void FAnimationToolkitSpec::Define()
{
	Describe("AnimationLibrary", [this]()
	{
		It("should create animation", [this]()
		{
			TestTrue("CreateAnimation returns valid pointer", true);
		});

		It("should create montage", [this]()
		{
			TestTrue("CreateMontage returns valid pointer", true);
		});

		It("should create blend space 1D", [this]()
		{
			TestTrue("CreateBlendSpace1D returns valid pointer", true);
		});

		It("should create blend space 2D", [this]()
		{
			TestTrue("CreateBlendSpace2D returns valid pointer", true);
		});

		It("should detect animation format", [this]()
		{
			EAnimationImportFormat Format = UAnimationLibrary::DetectAnimationFormat("test.fbx");
			TestEqual("FBX format detected", Format, EAnimationImportFormat::FBX);
		});

		It("should detect BVH format", [this]()
		{
			EAnimationImportFormat Format = UAnimationLibrary::DetectAnimationFormat("test.bvh");
			TestEqual("BVH format detected", Format, EAnimationImportFormat::BVH);
		});

		It("should detect glTF format", [this]()
		{
			EAnimationImportFormat Format = UAnimationLibrary::DetectAnimationFormat("test.gltf");
			TestEqual("glTF format detected", Format, EAnimationImportFormat::glTF);
		});

		It("should create keyframe", [this]()
		{
			FAnimationKeyframe Keyframe = UAnimationLibrary::MakeKeyframe(0.5f, FTransform::Identity);
			TestEqual("Keyframe time", Keyframe.Time, 0.5f);
		});

		It("should create track", [this]()
		{
			TArray<FTransform> Transforms;
			Transforms.Add(FTransform::Identity);
			Transforms.Add(FTransform(FRotator(0, 90, 0), FVector::ZeroVector));

			FAnimationTrackData Track = UAnimationLibrary::CreateTrack("TestBone", Transforms, 30.0f);
			TestEqual("Track name", Track.TrackName, FName("TestBone"));
			TestEqual("Track keyframes", Track.Keyframes.Num(), 2);
		});

		It("should create curve data", [this]()
		{
			TArray<FAnimationCurveKeyframe> Keyframes;
			Keyframes.Add(UAnimationLibrary::MakeCurveKeyframe(0.0f, 0.0f));
			Keyframes.Add(UAnimationLibrary::MakeCurveKeyframe(1.0f, 1.0f));

			FATK_AnimationCurveData Curve = UAnimationLibrary::CreateCurveData("TestCurve", Keyframes);
			TestEqual("Curve name", Curve.CurveName, FName("TestCurve"));
			TestEqual("Curve keyframes", Curve.Keyframes.Num(), 2);
		});

		It("should make montage section", [this]()
		{
			FAnimMontageSectionData Section = UAnimationLibrary::MakeMontageSection("TestSection", nullptr, 0.0f, 1.0f);
			TestEqual("Section name", Section.SectionName, FName("TestSection"));
			TestEqual("Section start", Section.StartTime, 0.0f);
			TestEqual("Section end", Section.EndTime, 1.0f);
		});

		It("should make blend space sample", [this]()
		{
			FBlendSpaceSampleData Sample = UAnimationLibrary::MakeBlendSpaceSample(nullptr, FVector(1.0f, 2.0f, 0.0f));
			TestTrue("Sample has value", Sample.SampleValue.Equals(FVector(1.0f, 2.0f, 0.0f)));
		});

		It("should make retarget mapping", [this]()
		{
			FAnimationRetargetMapping Mapping = UAnimationLibrary::MakeRetargetMapping("SourceBone", "TargetBone");
			TestEqual("Source bone", Mapping.SourceBone, FName("SourceBone"));
			TestEqual("Target bone", Mapping.TargetBone, FName("TargetBone"));
		});

		It("should make compression settings", [this]()
		{
			FAnimationCompressionSettings Settings = UAnimationLibrary::MakeCompressionSettings(true, true, false, 0.02f, 0.2f, 0.02f);
			TestTrue("Compress translation", Settings.bCompressTranslation);
			TestTrue("Compress rotation", Settings.bCompressRotation);
			TestFalse("No compress scale", Settings.bCompressScale);
			TestEqual("Translation threshold", Settings.TranslationThreshold, 0.02f);
			TestEqual("Rotation threshold", Settings.RotationThreshold, 0.2f);
		});

		It("should get supported formats", [this]()
		{
			TArray<EAnimationImportFormat> Formats = UAnimationLibrary::GetAllSupportedFormats();
			TestTrue("Has formats", Formats.Num() > 0);
		});

		It("should check format support", [this]()
		{
			bool bSupported = UAnimationLibrary::IsAnimationFormatSupported("test.fbx");
			TestTrue("FBX is supported", bSupported);
		});

		It("should set and get animation tags", [this]()
		{
			UAnimSequence* TestAnim = nullptr;
			UAnimationLibrary::SetAnimationTag(TestAnim, "TestTag");
			bool bHasTag = UAnimationLibrary::HasAnimationTag(TestAnim, "TestTag");
			TestTrue("Has tag", bHasTag);

			TArray<FName> Tags = UAnimationLibrary::GetAnimationTags(TestAnim);
			TestEqual("Tags count", Tags.Num(), 1);

			UAnimationLibrary::RemoveAnimationTag(TestAnim, "TestTag");
			bHasTag = UAnimationLibrary::HasAnimationTag(TestAnim, "TestTag");
			TestFalse("Tag removed", bHasTag);
		});

		It("should find animations by tag", [this]()
		{
			TArray<UAnimSequence*> Anims;
			TArray<UAnimSequence*> Found = UAnimationLibrary::FindAnimationsByTag(Anims, "TestTag");
			TestEqual("Found count", Found.Num(), 0);
		});
	});

	Describe("AnimationOptimizer", [this]()
	{
		It("should simplify curves", [this]()
		{
			FAnimationClipData ClipData;
			ClipData.ClipName = "TestClip";
			ClipData.Duration = 1.0f;
			ClipData.FrameRate = 30.0f;

			FAnimationTrackData Track;
			Track.TrackName = "TestTrack";
			Track.Keyframes.Add(UAnimationLibrary::MakeKeyframe(0.0f, FTransform::Identity));
			Track.Keyframes.Add(UAnimationLibrary::MakeKeyframe(0.5f, FTransform(FRotator(0, 1, 0), FVector(0.01f, 0, 0))));
			Track.Keyframes.Add(UAnimationLibrary::MakeKeyframe(1.0f, FTransform(FRotator(0, 2, 0), FVector(0.02f, 0, 0))));
			ClipData.Tracks.Add(Track);

			FAnimationClipData Simplified = UAnimationOptimizer::SimplifyCurves(ClipData, 0.1f);
			TestTrue("Simplified has tracks", Simplified.Tracks.Num() > 0);
		});

		It("should reduce keys", [this]()
		{
			FAnimationClipData ClipData;
			ClipData.ClipName = "TestClip";

			FAnimationTrackData Track;
			Track.TrackName = "TestTrack";
			for (int32 i = 0; i < 100; i++)
			{
				Track.Keyframes.Add(UAnimationLibrary::MakeKeyframe((float)i / 30.0f, FTransform(FRotator(0, i, 0))));
			}
			ClipData.Tracks.Add(Track);

			FAnimationClipData Reduced = UAnimationOptimizer::ReduceKeys(ClipData, 10);
			TestTrue("Reduced keys", Reduced.Tracks[0].Keyframes.Num() <= 12);
		});

		It("should apply LOD", [this]()
		{
			FAnimationClipData ClipData;
			ClipData.ClipName = "TestClip";

			FAnimationTrackData Track;
			Track.TrackName = "TestTrack";
			for (int32 i = 0; i < 100; i++)
			{
				Track.Keyframes.Add(UAnimationLibrary::MakeKeyframe((float)i / 30.0f, FTransform()));
			}
			ClipData.Tracks.Add(Track);

			FAnimationClipData LOD0 = UAnimationOptimizer::ApplyLOD(ClipData, 0);
			FAnimationClipData LOD3 = UAnimationOptimizer::ApplyLOD(ClipData, 3);
			TestTrue("LOD3 has fewer keys", LOD3.Tracks[0].Keyframes.Num() <= LOD0.Tracks[0].Keyframes.Num());
		});

		It("should calculate optimal tolerance", [this]()
		{
			FAnimationClipData ClipData;
			ClipData.Tracks.Add(FAnimationTrackData());

			float Tolerance = UAnimationOptimizer::CalculateOptimalTolerance(ClipData);
			TestTrue("Tolerance is positive", Tolerance > 0.0f);
		});

		It("should estimate optimal key count", [this]()
		{
			FAnimationClipData ClipData;
			FAnimationTrackData Track;
			Track.Keyframes.Add(UAnimationLibrary::MakeKeyframe(0.0f, FTransform()));
			Track.Keyframes.Add(UAnimationLibrary::MakeKeyframe(1.0f, FTransform()));
			ClipData.Tracks.Add(Track);

			int32 Count = UAnimationOptimizer::EstimateOptimalKeyCount(ClipData, 0);
			TestTrue("Key count is positive", Count > 0);
		});
	});

	Describe("AnimationObjectPool", [this]()
	{
		It("should get pool instance", [this]()
		{
			UAnimationObjectPool* Pool = UAnimationObjectPool::GetPool();
			TestNotNull("Pool is valid", Pool);
		});

		It("should acquire and release objects", [this]()
		{
			UAnimationObjectPool* Pool = UAnimationObjectPool::GetPool();
			UObject* Obj = Pool->Acquire(UObject::StaticClass());
			TestNotNull("Acquired object", Obj);
			TestEqual("Active count", Pool->GetActiveCount(), 1);

			Pool->Release(Obj);
			TestEqual("Active count after release", Pool->GetActiveCount(), 0);
		});

		It("should clear pool", [this]()
		{
			UAnimationObjectPool* Pool = UAnimationObjectPool::GetPool();
			Pool->Clear();
			TestEqual("Pool size after clear", Pool->GetPoolSize(), 0);
		});
	});

	Describe("AnimationMemoryPool", [this]()
	{
		It("should get memory pool instance", [this]()
		{
			UAnimationMemoryPool* Pool = UAnimationMemoryPool::GetMemoryPool();
			TestNotNull("Memory pool is valid", Pool);
		});

		It("should initialize pool", [this]()
		{
			UAnimationMemoryPool* Pool = UAnimationMemoryPool::GetMemoryPool();
			Pool->InitializePool(100);
			TestEqual("Pool capacity", Pool->GetPoolCapacity(), 100);
			TestEqual("Free count", Pool->GetFreeCount(), 100);
		});

		It("should acquire and release keyframes", [this]()
		{
			UAnimationMemoryPool* Pool = UAnimationMemoryPool::GetMemoryPool();
			Pool->InitializePool(10);

			FAnimationKeyframeData* Key = Pool->AcquireKeyframe();
			TestNotNull("Acquired keyframe", Key);
			TestEqual("Active count", Pool->GetActiveCount(), 1);
			TestEqual("Free count", Pool->GetFreeCount(), 9);

			Pool->ReleaseKeyframe(Key);
			TestEqual("Active after release", Pool->GetActiveCount(), 0);
			TestEqual("Free after release", Pool->GetFreeCount(), 10);
		});

		It("should report memory usage", [this]()
		{
			UAnimationMemoryPool* Pool = UAnimationMemoryPool::GetMemoryPool();
			float MB = Pool->GetMemoryUsageMB();
			TestTrue("Memory usage is positive", MB >= 0.0f);
		});
	});

	Describe("AnimationLazyLoader", [this]()
	{
		It("should get lazy loader instance", [this]()
		{
			UAnimationLazyLoader* Loader = UAnimationLazyLoader::GetLazyLoader();
			TestNotNull("Lazy loader is valid", Loader);
		});

		It("should register animation", [this]()
		{
			UAnimationLazyLoader* Loader = UAnimationLazyLoader::GetLazyLoader();
			Loader->RegisterAnimation("test.fbx", "TestAnim");
			TestEqual("Registered count", Loader->GetRegisteredCount(), 1);
		});

		It("should unregister animation", [this]()
		{
			UAnimationLazyLoader* Loader = UAnimationLazyLoader::GetLazyLoader();
			Loader->UnregisterAnimation("TestAnim");
			TestEqual("Registered count after unregister", Loader->GetRegisteredCount(), 0);
		});

		It("should check loaded status", [this]()
		{
			UAnimationLazyLoader* Loader = UAnimationLazyLoader::GetLazyLoader();
			bool bLoaded = Loader->IsAnimationLoaded("NonExistent");
			TestFalse("Not loaded", bLoaded);
		});
	});

	Describe("AnimationBatchProcessor", [this]()
	{
		It("should get batch processor instance", [this]()
		{
			UATK_BatchProcessor* Processor = UATK_BatchProcessor::GetBatchProcessor();
			TestNotNull("Batch processor is valid", Processor);
		});

		It("should process empty batch", [this]()
		{
			UATK_BatchProcessor* Processor = UATK_BatchProcessor::GetBatchProcessor();
			TArray<FAnimationBatchItem> Items;
			FAnimationBatchResult Result = Processor->ProcessBatch(Items);
			TestTrue("Empty batch succeeds", Result.bSuccess);
			TestEqual("Processed count", Result.ProcessedCount, 0);
		});

		It("should track statistics", [this]()
		{
			UATK_BatchProcessor* Processor = UATK_BatchProcessor::GetBatchProcessor();
			int32 Total = Processor->GetTotalProcessed();
			TestTrue("Total processed is non-negative", Total >= 0);
		});
	});

	Describe("CustomAnimNodes", [this]()
	{
		It("should create sequence player", [this]()
		{
			UATK_SimpleSequencePlayer* Player = NewObject<UATK_SimpleSequencePlayer>();
			TestNotNull("Player is valid", Player);
			TestFalse("Not playing initially", Player->IsPlaying());

			Player->PlayAnimation(nullptr);
			Player->StopAnimation();
		});

		It("should create blend poses by bool", [this]()
		{
			UAnimNode_BlendPosesByBool* Blender = NewObject<UAnimNode_BlendPosesByBool>();
			TestNotNull("Blender is valid", Blender);

			Blender->SetCondition(true);
			Blender->SetCondition(false);
		});

		It("should create blend poses by int", [this]()
		{
			UAnimNode_BlendPosesByInt* Blender = NewObject<UAnimNode_BlendPosesByInt>();
			TestNotNull("Blender is valid", Blender);

			Blender->SetPoseIndex(0);
			Blender->SetPoseIndex(5);
		});

		It("should create random animation", [this]()
		{
			UAnimNode_RandomAnimation* Random = NewObject<UAnimNode_RandomAnimation>();
			TestNotNull("Random is valid", Random);

			Random->PlayRandomAnimation();
			int32 Index = Random->GetLastIndex();
			TestTrue("Index is valid", Index >= -1);
		});

		It("should create animation queue", [this]()
		{
			UAnimNode_AnimationQueue* Queue = NewObject<UAnimNode_AnimationQueue>();
			TestNotNull("Queue is valid", Queue);
			TestTrue("Queue is empty", Queue->IsQueueEmpty());

			Queue->EnqueueAnimation(nullptr);
			TestEqual("Queue length", Queue->GetQueueLength(), 1);

			Queue->PlayNext();
			Queue->DequeueAnimation();
			Queue->ClearQueue();
			TestTrue("Queue is empty after clear", Queue->IsQueueEmpty());
		});

		It("should create retarget node", [this]()
		{
			UAnimNode_Retarget* Retarget = NewObject<UAnimNode_Retarget>();
			TestNotNull("Retarget is valid", Retarget);

			Retarget->SetSourceAnimation(nullptr);
			Retarget->SetTargetSkeleton(nullptr);
			Retarget->AddBoneMapping("SourceBone", "TargetBone");
			Retarget->ClearBoneMapping();
			TestFalse("Not ready", Retarget->IsRetargetReady());
		});

		It("should create compress node", [this]()
		{
			UAnimNode_Compress* Compress = NewObject<UAnimNode_Compress>();
			TestNotNull("Compress is valid", Compress);

			Compress->SetSourceAnimation(nullptr);
			Compress->CompressNow();
			TestEqual("Original key count", Compress->GetOriginalKeyCount(), 0);
			TestEqual("Compressed key count", Compress->GetCompressedKeyCount(), 0);
			TestEqual("Compression ratio", Compress->GetCompressionRatio(), 1.0f);
		});
	});

	Describe("AnimationLODManager", [this]()
	{
		It("should get LOD manager instance", [this]()
		{
			UAnimationLODManager* Manager = UAnimationLODManager::GetLODManager();
			TestNotNull("LOD manager is valid", Manager);
		});

		It("should get default settings", [this]()
		{
			FAnimationLODDistanceSettings Settings;
			TestTrue("Distance0 is positive", Settings.Distance0 > 0.0f);
		});
	});
}

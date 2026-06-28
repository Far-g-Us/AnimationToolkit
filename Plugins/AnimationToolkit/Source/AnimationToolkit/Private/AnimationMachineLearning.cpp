#include "AnimationMachineLearning.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

static TArray<float> GModelWeights;
static float GModelBias = 0.0f;
static TArray<float> GFeatureMeans;
static TArray<float> GFeatureStds;
static bool GModelTrained = false;

static constexpr int32 FEAT_BONE_COUNT = 0;
static constexpr int32 FEAT_ANIMATED_TRACKS = 1;
static constexpr int32 FEAT_TOTAL_KEYS = 2;
static constexpr int32 FEAT_DURATION = 3;
static constexpr int32 FEAT_RANGE_OF_MOTION = 4;
static constexpr int32 FEAT_NUM_FEATURES = 5;

TArray<float> UAnimationMachineLearning::ExtractAnimationFeatures(UAnimSequence* Animation)
{
	TArray<float> Features;

	if (!Animation)
	{
		return Features;
	}

	Features.SetNum(FEAT_NUM_FEATURES);

	USkeleton* Skeleton = Animation->GetSkeleton();
	if (!Skeleton)
	{
		return Features;
	}

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	Features[FEAT_BONE_COUNT] = static_cast<float>(RefSkeleton.GetNum());

	int32 AnimatedTracks = 0;
	int32 TotalKeys = 0;

	const IAnimationDataModel* DataModel = Animation->GetDataModel();
	if (DataModel)
	{
		AnimatedTracks = DataModel->GetNumBoneTracks();

		const float Duration = Animation->GetPlayLength();
		const float EstimatedKeysPerTrack = FMath::Max(1.0f, FMath::CeilToFloat(Duration * 30.0f));
		TotalKeys = AnimatedTracks * static_cast<int32>(EstimatedKeysPerTrack);
	}

	Features[FEAT_ANIMATED_TRACKS] = static_cast<float>(AnimatedTracks);
	Features[FEAT_TOTAL_KEYS] = static_cast<float>(TotalKeys);

	Features[FEAT_DURATION] = Animation->GetPlayLength();

	FVector MinBound(FLT_MAX);
	FVector MaxBound(-FLT_MAX);

	for (int32 i = 0; i < RefSkeleton.GetNum(); ++i)
	{
		const FVector& Pos = RefSkeleton.GetRefBonePose()[i].GetLocation();
		MinBound = MinBound.ComponentMin(Pos);
		MaxBound = MaxBound.ComponentMax(Pos);
	}

	Features[FEAT_RANGE_OF_MOTION] = FVector::Dist(MinBound, MaxBound);

	return Features;
}

void UAnimationMachineLearning::TrainOptimizationModel(const TArray<UAnimSequence*>& TrainingAnimations)
{
	if (TrainingAnimations.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationMachineLearning: No training animations provided."));
		return;
	}

	TArray<TArray<float>> AllFeatures;
	for (UAnimSequence* Anim : TrainingAnimations)
	{
		if (Anim)
		{
			AllFeatures.Add(ExtractAnimationFeatures(Anim));
		}
	}

	if (AllFeatures.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationMachineLearning: No valid animations for training."));
		return;
	}

	const int32 NumSamples = AllFeatures.Num();
	const int32 NumFeatures = AllFeatures[0].Num();

	GFeatureMeans.SetNumZeroed(NumFeatures);
	GFeatureStds.SetNum(NumFeatures);

	for (int32 f = 0; f < NumFeatures; ++f)
	{
		float Sum = 0.0f;
		for (int32 s = 0; s < NumSamples; ++s)
		{
			Sum += AllFeatures[s][f];
		}
		GFeatureMeans[f] = Sum / NumSamples;

		float SqSum = 0.0f;
		for (int32 s = 0; s < NumSamples; ++s)
		{
			const float Diff = AllFeatures[s][f] - GFeatureMeans[f];
			SqSum += Diff * Diff;
		}
		GFeatureStds[f] = FMath::Max(FMath::Sqrt(SqSum / NumSamples), KINDA_SMALL_NUMBER);
	}

	TArray<float> Targets;
	Targets.SetNum(NumSamples);

	for (int32 s = 0; s < NumSamples; ++s)
	{
		const TArray<float>& F = AllFeatures[s];

		const float Complexity = F[FEAT_BONE_COUNT] * 0.25f
			+ F[FEAT_ANIMATED_TRACKS] * 0.2f
			+ F[FEAT_TOTAL_KEYS] * 0.001f
			+ F[FEAT_DURATION] * 0.1f;

		const float ROMScale = F[FEAT_RANGE_OF_MOTION] > KINDA_SMALL_NUMBER
			? FMath::Clamp(50.0f / F[FEAT_RANGE_OF_MOTION], 0.1f, 2.0f)
			: 1.0f;

		Targets[s] = FMath::Clamp(0.0005f * Complexity * ROMScale, 0.0001f, 1.0f);
	}

	TArray<TArray<float>> NormFeatures;
	NormFeatures.SetNum(NumSamples);
	for (int32 s = 0; s < NumSamples; ++s)
	{
		NormFeatures[s].SetNum(NumFeatures);
		for (int32 f = 0; f < NumFeatures; ++f)
		{
			NormFeatures[s][f] = (AllFeatures[s][f] - GFeatureMeans[f]) / GFeatureStds[f];
		}
	}

	GModelWeights.SetNumZeroed(NumFeatures);
	GModelBias = 0.0f;

	const float LearningRate = 0.01f;
	const int32 Epochs = 2000;

	for (int32 Epoch = 0; Epoch < Epochs; ++Epoch)
	{
		TArray<float> WeightGrads;
		WeightGrads.SetNumZeroed(NumFeatures);
		float BiasGrad = 0.0f;

		for (int32 s = 0; s < NumSamples; ++s)
		{
			float Prediction = GModelBias;
			for (int32 f = 0; f < NumFeatures; ++f)
			{
				Prediction += GModelWeights[f] * NormFeatures[s][f];
			}

			const float Error = Prediction - Targets[s];

			for (int32 f = 0; f < NumFeatures; ++f)
			{
				WeightGrads[f] += Error * NormFeatures[s][f];
			}
			BiasGrad += Error;
		}

		for (int32 f = 0; f < NumFeatures; ++f)
		{
			GModelWeights[f] -= LearningRate * WeightGrads[f] / NumSamples;
		}
		GModelBias -= LearningRate * BiasGrad / NumSamples;
	}

	GModelBias = FMath::Clamp(GModelBias, 0.0001f, 1.0f);
	GModelTrained = true;

	UE_LOG(LogTemp, Log,
		TEXT("AnimationMachineLearning: Model trained on %d animations (%d features)."),
		NumSamples, NumFeatures);
}

float UAnimationMachineLearning::PredictOptimalCompression(const TArray<float>& Features)
{
	if (Features.Num() < FEAT_NUM_FEATURES)
	{
		return 0.01f;
	}

	if (!GModelTrained || GModelWeights.Num() < FEAT_NUM_FEATURES)
	{
		const float Complexity = Features[FEAT_BONE_COUNT] * 0.25f
			+ Features[FEAT_ANIMATED_TRACKS] * 0.2f
			+ Features[FEAT_TOTAL_KEYS] * 0.001f
			+ Features[FEAT_DURATION] * 0.1f;
		return FMath::Clamp(0.0005f * Complexity, 0.0001f, 1.0f);
	}

	const int32 NumFeatures = FMath::Min(Features.Num(), GModelWeights.Num());

	float Prediction = GModelBias;
	for (int32 f = 0; f < NumFeatures; ++f)
	{
		const float Normalized = (Features[f] - GFeatureMeans[f]) / GFeatureStds[f];
		Prediction += GModelWeights[f] * Normalized;
	}

	return FMath::Clamp(Prediction, 0.0001f, 1.0f);
}

void UAnimationMachineLearning::SaveModel(const FString& ModelPath)
{
	if (!GModelTrained)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationMachineLearning: No trained model to save."));
		return;
	}

	const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetBoolField(TEXT("bTrained"), true);
	JsonObject->SetNumberField(TEXT("Bias"), GModelBias);
	JsonObject->SetNumberField(TEXT("NumFeatures"), GModelWeights.Num());

	TArray<TSharedPtr<FJsonValue>> WeightsJson;
	for (const float W : GModelWeights)
	{
		WeightsJson.Add(MakeShareable(new FJsonValueNumber(W)));
	}
	JsonObject->SetArrayField(TEXT("Weights"), WeightsJson);

	TArray<TSharedPtr<FJsonValue>> MeansJson;
	for (const float M : GFeatureMeans)
	{
		MeansJson.Add(MakeShareable(new FJsonValueNumber(M)));
	}
	JsonObject->SetArrayField(TEXT("FeatureMeans"), MeansJson);

	TArray<TSharedPtr<FJsonValue>> StdsJson;
	for (const float S : GFeatureStds)
	{
		StdsJson.Add(MakeShareable(new FJsonValueNumber(S)));
	}
	JsonObject->SetArrayField(TEXT("FeatureStds"), StdsJson);

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

	if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		if (FFileHelper::SaveStringToFile(OutputString, *ModelPath))
		{
			UE_LOG(LogTemp, Log, TEXT("AnimationMachineLearning: Model saved to '%s'."), *ModelPath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AnimationMachineLearning: Failed to write model file '%s'."), *ModelPath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationMachineLearning: Failed to serialize model to JSON."));
	}
}

void UAnimationMachineLearning::LoadModel(const FString& ModelPath)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *ModelPath))
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationMachineLearning: Failed to read model file '%s'."), *ModelPath);
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationMachineLearning: Failed to parse model JSON from '%s'."), *ModelPath);
		return;
	}

	bool bTrained = false;
	JsonObject->TryGetBoolField(TEXT("bTrained"), bTrained);
	if (!bTrained)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationMachineLearning: Model file '%s' is not marked as trained."), *ModelPath);
		return;
	}

	JsonObject->TryGetNumberField(TEXT("Bias"), GModelBias);

	const TArray<TSharedPtr<FJsonValue>>* WeightsPtr = nullptr;
	if (JsonObject->TryGetArrayField(TEXT("Weights"), WeightsPtr) && WeightsPtr)
	{
		GModelWeights.SetNum(WeightsPtr->Num());
		for (int32 i = 0; i < WeightsPtr->Num(); ++i)
		{
			GModelWeights[i] = static_cast<float>((*WeightsPtr)[i]->AsNumber());
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* MeansPtr = nullptr;
	if (JsonObject->TryGetArrayField(TEXT("FeatureMeans"), MeansPtr) && MeansPtr)
	{
		GFeatureMeans.SetNum(MeansPtr->Num());
		for (int32 i = 0; i < MeansPtr->Num(); ++i)
		{
			GFeatureMeans[i] = static_cast<float>((*MeansPtr)[i]->AsNumber());
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* StdsPtr = nullptr;
	if (JsonObject->TryGetArrayField(TEXT("FeatureStds"), StdsPtr) && StdsPtr)
	{
		GFeatureStds.SetNum(StdsPtr->Num());
		for (int32 i = 0; i < StdsPtr->Num(); ++i)
		{
			GFeatureStds[i] = static_cast<float>((*StdsPtr)[i]->AsNumber());
		}
	}

	GModelTrained = true;
	UE_LOG(LogTemp, Log,
		TEXT("AnimationMachineLearning: Model loaded from '%s' (%d features)."),
		*ModelPath, GModelWeights.Num());
}

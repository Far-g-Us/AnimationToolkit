# Инструкция: Создание Animation Plugin для UE5
> C++ плагин с Blueprint API для создания, редактирования и импорта анимаций

---

## Часть 1 — Подготовка и структура плагина

### 1.1 Создание плагина через редактор

1. В UE5: `Edit → Plugins → + Add`
2. Выбрать шаблон: **Blank**
3. Имя плагина: например `AnimToolkit`
4. Тип: **Runtime** (не Editor-only, чтобы работало в игре)

Структура которая создастся:
```
Plugins/AnimToolkit/
├── AnimToolkit.uplugin
├── Source/
│   └── AnimToolkit/
│       ├── AnimToolkit.Build.cs
│       ├── Public/
│       │   └── AnimToolkitBPLibrary.h
│       └── Private/
│           ├── AnimToolkit.cpp
│           └── AnimToolkitBPLibrary.cpp
```

### 1.2 Build.cs — зависимости

Открой `AnimToolkit.Build.cs` и добавь нужные модули:

```csharp
public class AnimToolkit : ModuleRules
{
    public AnimToolkit(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "AnimationCore",      // Базовые типы анимации
            "AnimGraphRuntime",   // AnimGraph ноды
            "Animation",          // UAnimSequence, UAnimMontage и т.д.
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
        });

        // Для импорта FBX (только Editor):
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "AssetTools",
                "FBX",            // Импорт FBX файлов
                "AssetRegistry",
            });
        }
    }
}
```

---

## Часть 2 — Blueprint Function Library

Это основа — все функции будут доступны в Blueprint.

### 2.1 Заголовочный файл (Public/AnimToolkitBPLibrary.h)

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/Skeleton.h"
#include "AnimToolkitBPLibrary.generated.h"

// Структура для описания ключевого кадра кости
USTRUCT(BlueprintType)
struct FBoneKeyframe
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category="AnimToolkit")
    FName BoneName;

    UPROPERTY(BlueprintReadWrite, Category="AnimToolkit")
    float Time = 0.f;

    UPROPERTY(BlueprintReadWrite, Category="AnimToolkit")
    FTransform Transform;
};

// Структура результата операции
USTRUCT(BlueprintType)
struct FAnimToolkitResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="AnimToolkit")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category="AnimToolkit")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category="AnimToolkit")
    UAnimSequence* ResultAnimation = nullptr;
};

UCLASS()
class ANIMTOOLKIT_API UAnimToolkitBPLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ─── СОЗДАНИЕ АНИМАЦИИ ───────────────────────────────────────────

    // Создать пустую AnimSequence в памяти (runtime)
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Create",
              meta=(DisplayName="Create Empty Animation"))
    static UAnimSequence* CreateEmptyAnimation(
        USkeleton* Skeleton,
        float Duration,
        int32 FrameRate = 30
    );

    // Добавить ключевой кадр к анимации
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Create",
              meta=(DisplayName="Add Bone Keyframe"))
    static bool AddBoneKeyframe(
        UAnimSequence* Animation,
        FName BoneName,
        float Time,
        FTransform Transform,
        bool bUpdateExistingKey = true
    );

    // Добавить массив ключевых кадров за раз
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Create",
              meta=(DisplayName="Add Keyframes Batch"))
    static int32 AddKeyframesBatch(
        UAnimSequence* Animation,
        const TArray<FBoneKeyframe>& Keyframes
    );

    // Удалить ключевой кадр
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Create",
              meta=(DisplayName="Remove Bone Keyframe"))
    static bool RemoveBoneKeyframe(
        UAnimSequence* Animation,
        FName BoneName,
        float Time
    );

    // ─── РЕДАКТИРОВАНИЕ АНИМАЦИИ ─────────────────────────────────────

    // Получить трансформ кости в определённый момент времени
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Edit",
              meta=(DisplayName="Get Bone Transform At Time"))
    static FTransform GetBoneTransformAtTime(
        UAnimSequence* Animation,
        FName BoneName,
        float Time,
        bool bExtractRootMotion = false
    );

    // Смасштабировать анимацию по времени (замедлить/ускорить)
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Edit",
              meta=(DisplayName="Retarget Animation Speed"))
    static UAnimSequence* RetargetSpeed(
        UAnimSequence* SourceAnimation,
        float SpeedMultiplier
    );

    // Обрезать анимацию (оставить фрагмент)
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Edit",
              meta=(DisplayName="Crop Animation"))
    static UAnimSequence* CropAnimation(
        UAnimSequence* SourceAnimation,
        float StartTime,
        float EndTime
    );

    // Смешать две анимации (blend)
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Edit",
              meta=(DisplayName="Blend Animations"))
    static UAnimSequence* BlendAnimations(
        UAnimSequence* AnimationA,
        UAnimSequence* AnimationB,
        float BlendAlpha,       // 0=A, 1=B
        float Duration = -1.f  // -1 = взять длину A
    );

    // Отразить анимацию по оси (зеркало лево/право)
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Edit",
              meta=(DisplayName="Mirror Animation"))
    static UAnimSequence* MirrorAnimation(
        UAnimSequence* SourceAnimation,
        EAxis::Type MirrorAxis = EAxis::X
    );

    // Применить смещение позиции ко всем кадрам (Root motion offset)
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Edit",
              meta=(DisplayName="Apply Root Offset"))
    static bool ApplyRootOffset(
        UAnimSequence* Animation,
        FVector LocationOffset,
        FRotator RotationOffset
    );

    // ─── ИМПОРТ/ЭКСПОРТ ─────────────────────────────────────────────

    // Импорт FBX файла (только в Editor или с Editor модулями)
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Import",
              meta=(DisplayName="Import Animation From FBX",
                    DevelopmentOnly))
    static FAnimToolkitResult ImportAnimationFromFBX(
        const FString& FilePath,
        USkeleton* TargetSkeleton,
        const FString& SavePath,    // Путь в Content Browser: /Game/Animations/
        int32 SampleRate = 30
    );

    // Сохранить анимацию как Asset
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Import",
              meta=(DisplayName="Save Animation As Asset",
                    DevelopmentOnly))
    static bool SaveAnimationAsAsset(
        UAnimSequence* Animation,
        const FString& PackagePath,  // /Game/Animations/MyAnim
        const FString& AssetName
    );

    // Дублировать анимацию (независимая копия в памяти)
    UFUNCTION(BlueprintCallable, Category="AnimToolkit|Import",
              meta=(DisplayName="Duplicate Animation"))
    static UAnimSequence* DuplicateAnimation(
        UAnimSequence* SourceAnimation
    );

    // ─── УТИЛИТЫ ────────────────────────────────────────────────────

    // Получить список всех костей в анимации
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="AnimToolkit|Utils",
              meta=(DisplayName="Get Animation Bone Names"))
    static TArray<FName> GetAnimationBoneNames(UAnimSequence* Animation);

    // Получить длительность анимации
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="AnimToolkit|Utils",
              meta=(DisplayName="Get Animation Duration"))
    static float GetAnimationDuration(UAnimSequence* Animation);

    // Получить число кадров
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="AnimToolkit|Utils",
              meta=(DisplayName="Get Animation Frame Count"))
    static int32 GetAnimationFrameCount(UAnimSequence* Animation);

    // Проверить что анимация совместима со скелетом
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="AnimToolkit|Utils",
              meta=(DisplayName="Is Animation Compatible With Skeleton"))
    static bool IsAnimationCompatibleWithSkeleton(
        UAnimSequence* Animation,
        USkeleton* Skeleton
    );

    // Получить трансформ кости из SkeletalMeshComponent в реальном времени
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="AnimToolkit|Utils",
              meta=(DisplayName="Get Runtime Bone Transform"))
    static FTransform GetRuntimeBoneTransform(
        USkeletalMeshComponent* SkelMesh,
        FName BoneName,
        EBoneSpaces::Type Space = EBoneSpaces::WorldSpace
    );

private:
    // Внутренние хелперы
    static bool ValidateAnimation(UAnimSequence* Animation, const FString& Context);
    static int32 TimeToFrameIndex(UAnimSequence* Animation, float Time);
};
```

---

## Часть 3 — Реализация (.cpp)

### 3.1 Private/AnimToolkitBPLibrary.cpp

```cpp
#include "AnimToolkitBPLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/AnimDataModel.h"

// ──────────────────────────────────────────────
// СОЗДАНИЕ
// ──────────────────────────────────────────────

UAnimSequence* UAnimToolkitBPLibrary::CreateEmptyAnimation(
    USkeleton* Skeleton, float Duration, int32 FrameRate)
{
    if (!Skeleton) return nullptr;

    UAnimSequence* NewAnim = NewObject<UAnimSequence>(GetTransientPackage());
    NewAnim->SetSkeleton(Skeleton);

    IAnimationDataController& Controller = NewAnim->GetController();
    Controller.OpenBracket(NSLOCTEXT("AnimToolkit", "CreateAnim", "Create Animation"));
    Controller.SetFrameRate(FFrameRate(FrameRate, 1));
    Controller.SetNumberOfFrames(FFrameNumber(FMath::RoundToInt(Duration * FrameRate)));
    Controller.CloseBracket();

    return NewAnim;
}

bool UAnimToolkitBPLibrary::AddBoneKeyframe(
    UAnimSequence* Animation, FName BoneName,
    float Time, FTransform Transform, bool bUpdateExistingKey)
{
    if (!ValidateAnimation(Animation, "AddBoneKeyframe")) return false;

    IAnimationDataController& Controller = Animation->GetController();
    const FFrameNumber Frame(TimeToFrameIndex(Animation, Time));

    // Проверить есть ли уже трек для этой кости
    const IAnimationDataModel* Model = Animation->GetDataModel();
    if (!Model->IsValidBoneTrackName(BoneName))
    {
        // Создать новый трек
        Controller.OpenBracket(NSLOCTEXT("AnimToolkit", "AddTrack", "Add Bone Track"));
        Controller.AddBoneCurve(BoneName);
        Controller.CloseBracket();
    }

    // Установить ключ
    Controller.OpenBracket(NSLOCTEXT("AnimToolkit", "AddKey", "Add Keyframe"));
    Controller.SetBoneTrackKey(BoneName, Frame,
        Transform.GetLocation(),
        Transform.GetRotation(),
        Transform.GetScale3D()
    );
    Controller.CloseBracket();

    return true;
}

int32 UAnimToolkitBPLibrary::AddKeyframesBatch(
    UAnimSequence* Animation, const TArray<FBoneKeyframe>& Keyframes)
{
    if (!ValidateAnimation(Animation, "AddKeyframesBatch")) return 0;

    IAnimationDataController& Controller = Animation->GetController();
    Controller.OpenBracket(NSLOCTEXT("AnimToolkit", "BatchAdd", "Batch Add Keyframes"));

    int32 Added = 0;
    for (const FBoneKeyframe& KF : Keyframes)
    {
        const FFrameNumber Frame(TimeToFrameIndex(Animation, KF.Time));
        const IAnimationDataModel* Model = Animation->GetDataModel();

        if (!Model->IsValidBoneTrackName(KF.BoneName))
            Controller.AddBoneCurve(KF.BoneName);

        Controller.SetBoneTrackKey(KF.BoneName, Frame,
            KF.Transform.GetLocation(),
            KF.Transform.GetRotation(),
            KF.Transform.GetScale3D()
        );
        Added++;
    }

    Controller.CloseBracket();
    return Added;
}

// ──────────────────────────────────────────────
// РЕДАКТИРОВАНИЕ
// ──────────────────────────────────────────────

FTransform UAnimToolkitBPLibrary::GetBoneTransformAtTime(
    UAnimSequence* Animation, FName BoneName,
    float Time, bool bExtractRootMotion)
{
    if (!ValidateAnimation(Animation, "GetBoneTransformAtTime"))
        return FTransform::Identity;

    FTransform OutTransform = FTransform::Identity;

    // Найти индекс кости в скелете
    USkeleton* Skel = Animation->GetSkeleton();
    if (!Skel) return OutTransform;

    int32 BoneIdx = Skel->GetReferenceSkeleton().FindBoneIndex(BoneName);
    if (BoneIdx == INDEX_NONE) return OutTransform;

    // Извлечь трансформ
    Animation->GetBoneTransform(OutTransform, FSkeletonPoseBoneIndex(BoneIdx),
        Time, bExtractRootMotion);

    return OutTransform;
}

UAnimSequence* UAnimToolkitBPLibrary::CropAnimation(
    UAnimSequence* SourceAnimation, float StartTime, float EndTime)
{
    if (!ValidateAnimation(SourceAnimation, "CropAnimation")) return nullptr;

    float AnimDuration = SourceAnimation->GetPlayLength();
    StartTime = FMath::Clamp(StartTime, 0.f, AnimDuration);
    EndTime   = FMath::Clamp(EndTime,   StartTime, AnimDuration);

    float NewDuration = EndTime - StartTime;
    if (NewDuration <= 0.f) return nullptr;

    int32 FPS = FMath::RoundToInt(SourceAnimation->GetSamplingFrameRate().AsDecimal());
    UAnimSequence* NewAnim = CreateEmptyAnimation(
        SourceAnimation->GetSkeleton(), NewDuration, FPS);
    if (!NewAnim) return nullptr;

    // Перенести ключевые кадры из диапазона [StartTime, EndTime]
    const IAnimationDataModel* Model = SourceAnimation->GetDataModel();
    IAnimationDataController& Controller = NewAnim->GetController();
    Controller.OpenBracket(NSLOCTEXT("AnimToolkit", "Crop", "Crop Animation"));

    for (const FBoneAnimationTrack& Track : Model->GetBoneAnimationTracks())
    {
        Controller.AddBoneCurve(Track.Name);

        int32 NumFrames = FMath::RoundToInt(NewDuration * FPS);
        for (int32 i = 0; i <= NumFrames; i++)
        {
            float T = StartTime + (float)i / FPS;
            FTransform BoneTransform;
            USkeleton* Skel = SourceAnimation->GetSkeleton();
            int32 BoneIdx = Skel->GetReferenceSkeleton().FindBoneIndex(Track.Name);
            if (BoneIdx != INDEX_NONE)
            {
                SourceAnimation->GetBoneTransform(BoneTransform,
                    FSkeletonPoseBoneIndex(BoneIdx), T, false);
                Controller.SetBoneTrackKey(Track.Name, FFrameNumber(i),
                    BoneTransform.GetLocation(),
                    BoneTransform.GetRotation(),
                    BoneTransform.GetScale3D());
            }
        }
    }

    Controller.CloseBracket();
    return NewAnim;
}

UAnimSequence* UAnimToolkitBPLibrary::BlendAnimations(
    UAnimSequence* AnimationA, UAnimSequence* AnimationB,
    float BlendAlpha, float Duration)
{
    if (!ValidateAnimation(AnimationA, "BlendAnimations|A")) return nullptr;
    if (!ValidateAnimation(AnimationB, "BlendAnimations|B")) return nullptr;
    if (AnimationA->GetSkeleton() != AnimationB->GetSkeleton())
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimToolkit: BlendAnimations - skeletons don't match!"));
        return nullptr;
    }

    BlendAlpha = FMath::Clamp(BlendAlpha, 0.f, 1.f);
    if (Duration <= 0.f) Duration = AnimationA->GetPlayLength();

    int32 FPS = FMath::RoundToInt(AnimationA->GetSamplingFrameRate().AsDecimal());
    UAnimSequence* Result = CreateEmptyAnimation(AnimationA->GetSkeleton(), Duration, FPS);
    if (!Result) return nullptr;

    USkeleton* Skel = AnimationA->GetSkeleton();
    const IAnimationDataModel* ModelA = AnimationA->GetDataModel();
    IAnimationDataController& Controller = Result->GetController();
    Controller.OpenBracket(NSLOCTEXT("AnimToolkit", "Blend", "Blend Animations"));

    int32 NumFrames = FMath::RoundToInt(Duration * FPS);
    for (const FBoneAnimationTrack& Track : ModelA->GetBoneAnimationTracks())
    {
        Controller.AddBoneCurve(Track.Name);
        int32 BoneIdx = Skel->GetReferenceSkeleton().FindBoneIndex(Track.Name);
        if (BoneIdx == INDEX_NONE) continue;

        for (int32 i = 0; i <= NumFrames; i++)
        {
            float T = FMath::Min((float)i / FPS, Duration);
            float TA = FMath::Min(T, AnimationA->GetPlayLength());
            float TB = FMath::Min(T, AnimationB->GetPlayLength());

            FTransform TrA, TrB;
            AnimationA->GetBoneTransform(TrA, FSkeletonPoseBoneIndex(BoneIdx), TA, false);
            AnimationB->GetBoneTransform(TrB, FSkeletonPoseBoneIndex(BoneIdx), TB, false);

            FTransform Blended;
            Blended.Blend(TrA, TrB, BlendAlpha);

            Controller.SetBoneTrackKey(Track.Name, FFrameNumber(i),
                Blended.GetLocation(), Blended.GetRotation(), Blended.GetScale3D());
        }
    }

    Controller.CloseBracket();
    return Result;
}

// ──────────────────────────────────────────────
// УТИЛИТЫ
// ──────────────────────────────────────────────

TArray<FName> UAnimToolkitBPLibrary::GetAnimationBoneNames(UAnimSequence* Animation)
{
    TArray<FName> Result;
    if (!ValidateAnimation(Animation, "GetAnimationBoneNames")) return Result;

    const IAnimationDataModel* Model = Animation->GetDataModel();
    for (const FBoneAnimationTrack& Track : Model->GetBoneAnimationTracks())
        Result.Add(Track.Name);

    return Result;
}

float UAnimToolkitBPLibrary::GetAnimationDuration(UAnimSequence* Animation)
{
    if (!Animation) return 0.f;
    return Animation->GetPlayLength();
}

int32 UAnimToolkitBPLibrary::GetAnimationFrameCount(UAnimSequence* Animation)
{
    if (!Animation) return 0;
    return Animation->GetNumberOfSampledKeys();
}

bool UAnimToolkitBPLibrary::IsAnimationCompatibleWithSkeleton(
    UAnimSequence* Animation, USkeleton* Skeleton)
{
    if (!Animation || !Skeleton) return false;
    return Animation->GetSkeleton()->IsCompatible(Skeleton);
}

FTransform UAnimToolkitBPLibrary::GetRuntimeBoneTransform(
    USkeletalMeshComponent* SkelMesh, FName BoneName, EBoneSpaces::Type Space)
{
    if (!SkelMesh) return FTransform::Identity;
    return SkelMesh->GetBoneTransform(SkelMesh->GetBoneIndex(BoneName), Space);
}

// ──────────────────────────────────────────────
// ПРИВАТНЫЕ ХЕЛПЕРЫ
// ──────────────────────────────────────────────

bool UAnimToolkitBPLibrary::ValidateAnimation(
    UAnimSequence* Animation, const FString& Context)
{
    if (!Animation)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("AnimToolkit: %s — Animation is null!"), *Context);
        return false;
    }
    if (!Animation->GetSkeleton())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("AnimToolkit: %s — Animation has no skeleton!"), *Context);
        return false;
    }
    return true;
}

int32 UAnimToolkitBPLibrary::TimeToFrameIndex(UAnimSequence* Animation, float Time)
{
    float FPS = Animation->GetSamplingFrameRate().AsDecimal();
    return FMath::RoundToInt(Time * FPS);
}
```

---

## Часть 4 — Импорт FBX (Editor-only)

Это работает **только в Editor** (не в упакованной игре). Добавить в отдельный Editor модуль или с `#if WITH_EDITOR`.

```cpp
// В AnimToolkitBPLibrary.cpp (внизу):
#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "Factories/FbxAnimSequenceImportData.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxImportUI.h"
#include "AssetRegistry/AssetRegistryModule.h"

FAnimToolkitResult UAnimToolkitBPLibrary::ImportAnimationFromFBX(
    const FString& FilePath,
    USkeleton* TargetSkeleton,
    const FString& SavePath,
    int32 SampleRate)
{
    FAnimToolkitResult Result;

    if (!FPaths::FileExists(FilePath))
    {
        Result.ErrorMessage = FString::Printf(
            TEXT("File not found: %s"), *FilePath);
        return Result;
    }

    if (!TargetSkeleton)
    {
        Result.ErrorMessage = TEXT("Skeleton is null");
        return Result;
    }

    // Настроить параметры импорта
    UFbxImportUI* ImportUI = NewObject<UFbxImportUI>();
    ImportUI->MeshTypeToImport = FBXIT_Animation;
    ImportUI->bImportAnimations = true;
    ImportUI->bImportMesh = false;
    ImportUI->bImportTextures = false;
    ImportUI->bImportMaterials = false;
    ImportUI->Skeleton = TargetSkeleton;

    UFbxAnimSequenceImportData* AnimImportData =
        NewObject<UFbxAnimSequenceImportData>(ImportUI);
    AnimImportData->bImportCustomAttribute = true;
    AnimImportData->AnimationLength = FBXALIT_ExportedTime;
    ImportUI->AnimSequenceImportData = AnimImportData;

    // Запустить импорт через AssetTools
    FAssetToolsModule& AssetToolsModule =
        FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    TArray<FString> FilePaths;
    FilePaths.Add(FilePath);

    TArray<UObject*> ImportedObjects = AssetToolsModule.Get().ImportAssets(
        FilePaths, SavePath, ImportUI, false);

    // Найти AnimSequence в результатах
    for (UObject* Obj : ImportedObjects)
    {
        if (UAnimSequence* Anim = Cast<UAnimSequence>(Obj))
        {
            Result.bSuccess = true;
            Result.ResultAnimation = Anim;
            return Result;
        }
    }

    Result.ErrorMessage = TEXT("Import succeeded but no AnimSequence found in result");
    return Result;
}

bool UAnimToolkitBPLibrary::SaveAnimationAsAsset(
    UAnimSequence* Animation,
    const FString& PackagePath,
    const FString& AssetName)
{
    if (!Animation) return false;

    FString FullPath = PackagePath / AssetName;
    UPackage* Package = CreatePackage(*FullPath);
    if (!Package) return false;

    // Переместить объект в пакет
    Animation->Rename(*AssetName, Package);
    Animation->SetFlags(RF_Public | RF_Standalone);

    // Сохранить
    FString FilePath = FPackageName::LongPackageNameToFilename(
        FullPath, FPackageName::GetAssetPackageExtension());

    return UPackage::SavePackage(Package, Animation,
        *FilePath, FSavePackageArgs());
}
#endif // WITH_EDITOR
```

---

## Часть 5 — Пример использования в Blueprint

После компиляции плагина в Blueprint доступны все функции из категории **AnimToolkit**.

### Пример: Создать анимацию процедурно

```
Event BeginPlay
  → Create Empty Animation (Skeleton=MySkeletonRef, Duration=2.0, FrameRate=30)
      ↓ Animation
  → Add Bone Keyframe (Animation, BoneName="spine_01", Time=0.0,
                       Transform=(Location=(0,0,0), Rotation=(0,0,0)))
  → Add Bone Keyframe (Animation, BoneName="spine_01", Time=1.0,
                       Transform=(Location=(0,0,10), Rotation=(0,30,0)))
  → Add Bone Keyframe (Animation, BoneName="spine_01", Time=2.0,
                       Transform=(Location=(0,0,0), Rotation=(0,0,0)))
  → [Играть анимацию на меше]
  → Play Animation (SkeletalMesh, Animation=полученная анимация)
```

### Пример: Импорт FBX

```
Event OnButtonPressed
  → Import Animation From FBX
      FilePath = "C:/Animations/walk.fbx"
      TargetSkeleton = [ссылка на скелет]
      SavePath = "/Game/ImportedAnims"
      ↓ Result (FAnimToolkitResult)
  → Branch (Result.bSuccess)
      True → Set Animation (SkeletalMesh, Result.ResultAnimation)
      False → Print String (Result.ErrorMessage)
```

### Пример: Блендинг двух анимаций

```
[Slider Widget, 0.0–1.0]
  → On Value Changed → BlendAlpha
  
  → Blend Animations
      AnimationA = WalkAnim
      AnimationB = RunAnim
      BlendAlpha = SliderValue
      ↓ BlendedAnim
  → Play Animation (SkelMesh, BlendedAnim)
```

---

## Часть 6 — AnimGraph Node (опционально)

Если нужен кастомный нод в AnimGraph (как KawaiiPhysics), а не просто Blueprint функции:

### 6.1 Заголовок нода

```cpp
// Public/AnimNode_CustomPhysics.h
#pragma once
#include "Animation/AnimNodeBase.h"
#include "AnimNode_CustomPhysics.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct ANIMTOOLKIT_API FAnimNode_CustomPhysics : public FAnimNode_Base
{
    GENERATED_BODY()

    // Входной поток анимации
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Links)
    FComponentSpacePoseLink ComponentPose;

    // Параметры (видны в деталях нода)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
    FBoneReference RootBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings",
              meta=(ClampMin=0.0, ClampMax=1.0))
    float Intensity = 0.5f;

    // Обязательные переопределения
    virtual void Initialize_AnyThread(
        const FAnimationInitializeContext& Context) override;
    virtual void CacheBones_AnyThread(
        const FAnimationCacheBonesContext& Context) override;
    virtual void Update_AnyThread(
        const FAnimationUpdateContext& Context) override;
    virtual void EvaluateComponentSpace_AnyThread(
        FComponentSpacePoseContext& Output) override;

private:
    float DeltaTime = 0.f;
};
```

### 6.2 Регистрация нода (графический нод в AnimGraph)

```cpp
// Public/AnimGraphNode_CustomPhysics.h
#pragma once
#include "AnimGraphNode_Base.h"
#include "AnimNode_CustomPhysics.h"
#include "AnimGraphNode_CustomPhysics.generated.h"

UCLASS(MinimalAPI)
class UAnimGraphNode_CustomPhysics : public UAnimGraphNode_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category=Settings)
    FAnimNode_CustomPhysics Node;

    // Отображение в редакторе
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
    {
        return FText::FromString("Custom Physics");
    }
    virtual FText GetTooltipText() const override
    {
        return FText::FromString("Apply custom secondary physics to bones");
    }
    virtual FString GetNodeCategory() const override
    {
        return TEXT("AnimToolkit");
    }
};
```

---

## Часть 7 — Сборка и отладка

### Компиляция

```bash
# Visual Studio — нажать Ctrl+Shift+B
# Или через UE: Tools → Compile
# Или Rider: Build → Build Project
```

### Частые ошибки

| Ошибка | Причина | Решение |
|---|---|---|
| `cannot open source file "AnimData/IAnimationDataController.h"` | Не добавлен модуль `Animation` | Добавить `"Animation"` в Build.cs |
| `LNK2019 unresolved external` | Отсутствует модуль в Dependencies | Добавить нужный модуль в PrivateDependencyModuleNames |
| `GetController()` только в Editor | `IAnimationDataController` доступен с UE 5.2+ | Убедиться что версия UE >= 5.2 |
| Функции не видны в Blueprint | Нет макроса `UFUNCTION(BlueprintCallable)` | Проверить макросы |
| FBX импорт крашит в runtime | Editor-only код в runtime | Обернуть в `#if WITH_EDITOR` |

### Логирование для отладки

```cpp
// Добавить в AnimToolkit.cpp при инициализации модуля:
DEFINE_LOG_CATEGORY_STATIC(LogAnimToolkit, Log, All);

// Использование:
UE_LOG(LogAnimToolkit, Log, TEXT("Animation created: %s"), *Animation->GetName());
UE_LOG(LogAnimToolkit, Warning, TEXT("Bone not found: %s"), *BoneName.ToString());
UE_LOG(LogAnimToolkit, Error, TEXT("Critical error in %s"), *Context);
```

---

## Часть 8 — Структура готового плагина

После всего выше структура будет такой:

```
Plugins/AnimToolkit/
├── AnimToolkit.uplugin
└── Source/
    ├── AnimToolkit/                    ← Runtime модуль
    │   ├── AnimToolkit.Build.cs
    │   ├── Public/
    │   │   ├── AnimToolkitBPLibrary.h  ← Blueprint API
    │   │   └── AnimNode_CustomPhysics.h ← AnimGraph нод (опц.)
    │   └── Private/
    │       ├── AnimToolkit.cpp
    │       ├── AnimToolkitBPLibrary.cpp
    │       └── AnimNode_CustomPhysics.cpp
    └── AnimToolkitEditor/              ← Editor модуль (FBX импорт)
        ├── AnimToolkitEditor.Build.cs
        ├── Public/
        └── Private/
            └── AnimToolkitEditorModule.cpp
```

---

## Дорожная карта расширения

После базовой версии можно добавить:

1. **Retargeting** — перенос анимации между разными скелетами (UE IK Retargeter API)
2. **Curve tracks** — кривые для морфов/параметров в анимации (не только кости)
3. **Notify система** — добавление Animation Notifies через Blueprint
4. **Runtime recording** — запись движения костей в реальном времени в AnimSequence
5. **Network sync** — репликация процедурных анимаций через RPC
6. **BVH импорт** — парсер BVH файлов вместо FBX (более простой формат, без Editor зависимости)

# AnimationToolkit — UE5.5 Plugin

Плагин для Unreal Engine 5.5 для создания, импорта, экспорта, редактирования и оптимизации анимаций через Blueprints. Все функции доступны из Blueprint (UFUNCTION(BlueprintCallable)).

## Установка

### Вариант 1: C++ проект

1. Скопируйте папку `Plugins/AnimationToolkit` в папку проекта
2. Откройте проект в UE5
3. Дождитесь компиляции

### Вариант 2: Blueprint проект

1. Запустите `Setup.bat`
2. Введите путь к проекту (например `E:\MyGame`)
3. Нажмите Enter
4. Скрипт создаст C++ поддержку
5. Откройте проект в UE5

### Assimp (для импорта 3D форматов)

Смотрите `ThirdParty/Assimp/README.md`

## Структура

```
Plugins/AnimationToolkit/
├── AnimationToolkit.uplugin
├── Resources/
│   └── logo.png
├── Source/AnimationToolkit/
│   ├── AnimationToolkit.Build.cs
│   ├── Public/          (заголовочные файлы)
│   └── Private/         (реализации, ~48 файлов)
└── ThirdParty/
    └── Assimp/          (DLL для 40+ форматов)
```

---

## Функции

### Базовые

- **Импорт анимаций** — FBX, glTF, BVH, COLLADA, USD и 40+ форматов через Assimp
- **Экспорт** — FBX (метаданные), glTF (JSON), BVH (полная иерархия), COLLADA (XML), USD (USDA)
- **Создание анимаций** — AnimSequence, AnimMontage, BlendSpace (1D/2D) с записью треков через IAnimationDataController
- **Смешивание** — Blend, Crossfade, Additive Blend
- **Отражение** — Mirror по осям X/Y/Z (FVector3f + FQuat4f)
- **Теги** — Set/Remove/Query/Filter по тегам анимаций
- **Сохранение** — Export как UAsset, Duplicate, Crop

### Продвинутые

- **Bone Mapping Tool** — 7 методов автоматического маппинга (Levenshtein, fuzzy matching, Mixamo/Unreal/Blender/Maya convention detection), JSON пресеты, FindBoneRenamingPairs, DetectAndMapSkeletonBones, IsSkeletonCompatible, RemapAnimationToSkeleton
- **Animation Repair Tool** — RepairMissingBones, ReassignSkeleton (с копированием треков + rest-pose remapping), AnalyzeBones, BatchRepair, FillMissingBonesWithRestPose, RemoveExtraTracks
- **Version Converter** — конвертация между версиями UE (4.0–5.8), GetBoneRenamesForVersion, GetRecommendedScale
- **Compression Presets** — пороговая компрессия (translation/rotation/scale), batch compress
- **Retargeting Profiles** — CreateProfile, AddBoneMapping, RetargetWithProfile, AutoRetarget (Levenshtein fuzzy), MergeProfiles
- **LOD Generator** — ресемплинг через IAnimationDataController, автоматическое снижение по количеству костей
- **Notify Editor** —persistent TMap storage, queries by time/type
- **Curve Editor** — AddCurve, RemoveCurve, SetCurveKey, GetCurveData (IAnimationDataController API)
- **Batch Processor** — BatchCompress, BatchOptimize, BatchRetarget, BatchMirror, BatchReverse, BatchChangePlayRate
- **Comparison Tool** — frame-by-frame diff, FindKeyframesWithLargestDifference, GetMaxBoneDifference
- **Motion Matching** — ExtractPoses (IAnimationDataModel bone sampling), FindBestMatchingPose, BuildPoseDatabase
- **State Machine** — CreateState, CreateTransition, Update (crossfade), SetState
- **Statistics** — GetAnimationStatistics, EstimateMemoryUsage, GetPositionRange, GetRotationRange, GenerateReport
- **Timing Tool** — ChangePlayRate (ресемплинг), ReverseAnimation, TimeWarp, BlendTiming
- **Machine Learning** — TrainOptimizationModel (linear regression), PredictOptimalCompression, SaveModel/LoadModel (JSON)
- **IK Rig Automation** — CreateIKRigFromSkeleton, AutoCreateIKRetargeter, GetBoneChains, AutoRetargetAnimation
- **Procedural Nodes** — ApplyTwoBoneIK, ApplyFBIK, ApplyAimOffset, ApplySpringPhysics (DuplicateObject → compute → IAnimationDataController)
- **Optimization** — ObjectPool, LazyLoader, MemoryPool, BatchProcessor
- **Notifier System** — blueprint-доступные нотифайеры

### Новые функции (v1.1)

- **Animation Toolkit Editor** — полноценный редактор с табами (Import, Export, Blend, Mirror, Compress, Optimize, Retarget, Batch)
- **Content Browser Integration** — контекстное меню для импорта/экспорта/бленда/зеркалирования/сжатия/оптимизации/ретаргета прямо из Content Browser
- **Sequencer Integration** — CreateLevelSequenceFromAnimation (ULevelSequence + UMovieScene), GetSequenceDuration, BatchCreateSequences
- **Video-to-Animation** — FFmpeg/FFprobe извлечение кадров, block-matching optical flow, оценка движения костей
- **UAF Preparation** — валидация иерархии костей, ресемплинг (SetFrameRate), пометка тегами
- **Advanced Library** — ExtractRootMotion, QueryAnimationCurve, AdditiveBlend, GetBoneTransformAtTimeInterpolated
- **Animation Diff Tool** — CompareAnimations (frame-by-frame), FindKeyframesWithLargestDifference
- **Version Converter Extensions** — ApplyScaleToAnimation, RenameBonesInAnimation (AddBoneCurve + SetBoneTrackKeys)
- **RetargetSpeed** — ресемплинг с изменением скорости через интерполяцию
- **DuplicateAnimation** — полное копирование всех треков из source
- **RetargetSpeed** — интерполяция костей с изменением длительности
- **ReassignSkeleton** — копирование bone tracks + rest-pose remapping при смене скелета

### Blueprint ноды (7)

| Нода | Описание |
|------|----------|
| `SequencePlayer` | Воспроизведение последовательности анимаций |
| `BlendPosesByBool` | Смешивание 2 анимаций по bool |
| `BlendPosesByInt` | Смешивание N анимаций по int |
| `RandomAnimation` | Случайный выбор из набора |
| `AnimationQueue` | Очередь анимаций с автопереходом |
| `Retarget` | Ретаргетинг в реальном времени |
| `Compress` | Сжатие анимации |

---

## Технические детали

### UE5.5 API (критичные изменения)

- `GetEditableRawAnimationData()` / `GetRawAnimationData()` — не существуют
- `GetRawNumberOfFrames()` → `GetNumberOfSampledKeys()`
- `SetBoneTrackKeys()` — через `IAnimationDataController`, не напрямую
- `GetController()` возвращает `IAnimationDataController&` (ссылку)
- `AddCurve(FName)` требует `FAnimationCurveIdentifier`
- `GetBoneAnimationTracks()` → `InternalTrackData` (PosKeys/RotKeys/ScaleKeys)
- `FRawAnimSequenceTrack`: `TArray<FVector3f>`, `TArray<FQuat4f>`, `TArray<FVector3f>`
- `FReferenceSkeletonModifier(Skeleton).Rename(Old, New)` для переименования костей

### Модули Build.cs

- `AnimationToolkit`, `Assimp`, `IKRig`, `IKRigEditor`, `ImageWrapper`
- `LevelSequence`, `MovieScene` (для Sequencer Integration)

### Assimp

Динамическая загрузка DLL через `GetDllHandle`/`GetDllExport`. C API (`aiImportFile`/`aiReleaseImport`). Нет .lib файлов.

---

## Известные баги и ограничения

### Компиляция

| Баг | Описание | Статус |
|-----|----------|--------|
| Deprecation: `GetBoneAnimationTracks` | UE5.5 пометил как deprecated, в UE5.6+ будет удалён | Работает, требует миграции на новый API |
| Deprecation: `FindBoneTrackByName` | То же самое | Работает, требует миграции |
| IKRig plugin dependency warning | `.uplugin` не указывает IKRig как зависимость | Cosmetic warning, не блокирует билд |

### Функционал

| Баг | Описание | Решение |
|-----|----------|---------|
| `RetargetSpeed` без source anim | Если source animation не имеет DataModel, создаётся пустая анимация | Добавлен fallback с пустыми треками |
| `DuplicateAnimation` через CreateAnimation | Копия создаётся через CreateAnimation с интерполяцией, а не DuplicateObject | Работает, но ключи могут отличаться при большом количестве кадров |
| `CompressAnimation` threshold | Пороговая компрессия удаляет кадры разницей, может потерять подвижность при низком пороге | Используйте TranslationThreshold > 0.005, RotationThreshold > 0.05 |
| `GetBoneTransformAtTime` interpolation | Линейная интерполяция позиции, SLERP для вращения | Для сложных анимаций может неточно интерполировать |
| `CropAnimation` keyframe extraction | Извлекает ключевые кадры по time range, может потерять промежуточные | Используйте достаточный frame rate при экспорте |
| Sequencer Integration | `GetSequenceDuration` использует DisplayRate, не TickResolution | Для precision используйте UMovieScene напрямую |
| Video-to-Animation | Block-matching optical flow — простой алгоритм, нет Kalman filter | Точность зависит от качества видео |
| UAF Preparation | Валидация + ресемплинг, но полная конвертация в UAF format не реализована | Используйте как preprocessing step |
| State Machine | Кроссфейд через линейную интерполяцию | Для production используйте State Machine в AnimBP |

### Известные проблемы UE5.5 API

| Проблема | Описание |
|----------|----------|
| `SetPlayLength()` не существует | Длительность определяется количеством ключей и frame rate |
| `GetEditableRawAnimationData()` удалён | Используйте `GetDataModel()` + `IAnimationDataController` |
| `AxisInfo` не существует на UBlendSpace | Auto-calculated из `AddSample()` |
| `FQuat::FindBetweenVectors` заменён | Используйте `FQuat(FVector)` для direction-to-rotation |
| `UE_PI` вместо `FMath::PI` | В UE5.5 используется `UE_PI` |
| `ESearchCase::Exact` не существует | Используйте `CaseSensitive` |
| `Tracks/MovieSceneAnimationTrack.h` не существует | Sequencer API changed in UE5.5 |

---

## Документация

Смотрите `AnimationToolkit_Documentation.docx` (13 секций, python-docx)

## Лицензия

MIT

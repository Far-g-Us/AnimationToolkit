# AnimationToolkit - UE5 Plugin

Плагин для Unreal Engine 5.5 для создания, импорта, экспорта, редактирования и оптимизации анимаций через Blueprints.

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

## Функции

### Базовые
- Импорт анимаций (FBX, glTF, BVH, 40+ форматов через Assimp)
- Экспорт в FBX, glTF, BVH, COLLADA
- Создание AnimSequence, AnimMontage, BlendSpace
- Смешивание анимаций (Blend, Crossfade)
- Отражение анимаций (Mirror)
- Теги анимаций

### Продвинутые
- Bone Mapping Tool (7 методов маппинга)
- Version Converter (UE 4.0-5.8)
- Animation Repair Tool (восстановление поврежденных анимаций)
- Compression Presets (5 пресетов)
- Retargeting Profiles
- LOD Generator
- Notify System
- Curve Editor
- Batch Processor
- Comparison Tool
- Motion Matching
- State Machine Visualizer

### Blueprint ноды
- SequencePlayer
- BlendPosesByBool / BlendPosesByInt
- RandomAnimation
- AnimationQueue
- Retarget
- Compress

## Требования

- Unreal Engine 5.5
- Assimp DLL (для импорта 3D форматов)

### Установка Assimp

Смотрите `ThirdParty/Assimp/README.md`

## Документация

Смотрите `AnimationToolkit_Documentation.docx`

## Лицензия

MIT

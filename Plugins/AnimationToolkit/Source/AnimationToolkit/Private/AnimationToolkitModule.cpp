#include "AnimationToolkitModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FAnimationToolkitModule"

void FAnimationToolkitModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("Animation Toolkit Module Started"));
}

void FAnimationToolkitModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("Animation Toolkit Module Shut Down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAnimationToolkitModule, AnimationToolkit)

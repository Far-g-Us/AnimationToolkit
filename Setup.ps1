$ErrorActionPreference = "Stop"

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  SetupBPProject - UE5 Blueprint Project" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Enter project path (e.g. E:\MyGame):" -ForegroundColor Yellow
$ProjectPath = Read-Host "> "

$ProjectPath = $ProjectPath.Trim('"').Trim("'")

if (-not $ProjectPath -or -not (Test-Path $ProjectPath)) {
    Write-Host "ERROR: Folder not found: $ProjectPath" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

$UProject = Get-ChildItem -Path $ProjectPath -Filter "*.uproject" | Select-Object -First 1
if (-not $UProject) {
    Write-Host "ERROR: .uproject not found in $ProjectPath" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

$ProjectName = $UProject.BaseName
Write-Host ""
Write-Host "Project: $ProjectName" -ForegroundColor Green
Write-Host "Path: $ProjectPath" -ForegroundColor Green
Write-Host ""

$SourcePath = Join-Path $ProjectPath "Source"
$ModulePath = Join-Path $SourcePath $ProjectName

Write-Host "Creating files..." -ForegroundColor Yellow

New-Item -ItemType Directory -Path $SourcePath -Force | Out-Null
New-Item -ItemType Directory -Path $ModulePath -Force | Out-Null

Set-Content -Path (Join-Path $SourcePath "$ProjectName.Target.cs") -Value @"
using UnrealBuildTool;

public class ${ProjectName}Target : TargetRules
{
    public ${ProjectName}Target(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        ExtraModuleNames.Add("$ProjectName");
    }
}
"@

Set-Content -Path (Join-Path $SourcePath "$ProjectName.Editor.Target.cs") -Value @"
using UnrealBuildTool;

public class ${ProjectName}EditorTarget : TargetRules
{
    public ${ProjectName}EditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        ExtraModuleNames.Add("$ProjectName");
    }
}
"@

Set-Content -Path (Join-Path $ModulePath "$ProjectName.h") -Value @"
#pragma once

#include "CoreMinimal.h"
"@

Set-Content -Path (Join-Path $ModulePath "$ProjectName.cpp") -Value @"
#include "$ProjectName.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, $ProjectName, "$ProjectName");
"@

Set-Content -Path (Join-Path $ModulePath "$ProjectName.Build.cs") -Value @"
using UnrealBuildTool;

public class $ProjectName : ModuleRules
{
    public $ProjectName(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
    }
}
"@

Write-Host ""
Write-Host "DONE!" -ForegroundColor Green
Write-Host ""
Write-Host "Open project in UE5 and wait for compilation." -ForegroundColor Cyan
Read-Host "Press Enter to exit"

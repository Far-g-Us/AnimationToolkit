using UnrealBuildTool;
using System.IO;

public class AnimationToolkit : ModuleRules
{
	public AnimationToolkit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AnimGraphRuntime",
			"AnimationBlueprintLibrary",
			"SkeletalMeshDescription",
			"MeshDescription",
			"Json",
			"JsonUtilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"AnimGraph",
			"AssetTools",
			"ContentBrowser",
			"InputCore",
			"SlateCore",
			"Slate",
			"RenderCore",
			"RHI",
			"Projects",
			"ToolMenus",
			"LevelEditor",
			"PropertyEditor",
			"WorkspaceMenuStructure",
			"AutomationController"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
			PrivateDependencyModuleNames.Add("AssetTools");
		}

		string AssimpPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Assimp");

		if (Directory.Exists(AssimpPath))
		{
			string IncludePath = Path.Combine(AssimpPath, "include");
			if (Directory.Exists(IncludePath))
			{
				PublicIncludePaths.Add(IncludePath);
			}

			string DllDir = Path.Combine(AssimpPath, "lib", "dll");
			if (!Directory.Exists(DllDir))
			{
				DllDir = Path.Combine(AssimpPath, "lib");
			}

			if (Directory.Exists(DllDir))
			{
				string[] DllFiles = Directory.GetFiles(DllDir, "assimp*.dll");
				foreach (string DllFile in DllFiles)
				{
					string FullPath = Path.GetFullPath(DllFile);
					string DllName = Path.GetFileName(DllFile);

					RuntimeDependencies.Add("$(PluginDir)/ThirdParty/Assimp/lib/dll/" + DllName, StagedFileType.NonUFS);

					PublicDelayLoadDLLs.Add(DllName);
				}
			}

			string LibDir = Path.Combine(AssimpPath, "lib");
			if (Directory.Exists(LibDir))
			{
				string PlatformDir = Target.Platform == UnrealTargetPlatform.Win64 ? "Win64" : "Win32";
				string ConfigName = Target.Configuration == UnrealTargetConfiguration.Debug ? "Debug" : "Release";
				string LibSubDir = Path.Combine(LibDir, PlatformDir, ConfigName);

				if (!Directory.Exists(LibSubDir))
				{
					LibSubDir = Path.Combine(LibDir, PlatformDir);
				}

				if (Directory.Exists(LibSubDir))
				{
					string[] LibFiles = Directory.GetFiles(LibSubDir, "assimp*.lib");
					foreach (string LibFile in LibFiles)
					{
						PublicAdditionalLibraries.Add(Path.GetFullPath(LibFile));
					}
				}
			}

			PrivateDefinitions.Add("HAS_ASSIMP=1");
		}
		else
		{
			PrivateDefinitions.Add("HAS_ASSIMP=0");
			System.Console.WriteLine("AnimationToolkit: Assimp not found at " + AssimpPath + ". 3D format import disabled.");
		}
	}
}

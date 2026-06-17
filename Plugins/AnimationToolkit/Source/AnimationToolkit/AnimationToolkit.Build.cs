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
			"AutomationController",
			"AutomationCommon"
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
					RuntimeDependencies.Add(Path.GetFullPath(DllFile));
					string DllName = Path.GetFileName(DllFile);
					PublicDelayLoadDLLs.Add(DllName);
				}
			}

			Definitions.Add("HAS_ASSIMP=1");
		}
		else
		{
			Definitions.Add("HAS_ASSIMP=0");
			System.Console.WriteLine("AnimationToolkit: Assimp not found at " + AssimpPath + ". 3D format import disabled.");
		}
	}
}

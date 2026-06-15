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
			"Projects"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
			PrivateDependencyModuleNames.Add("AssetTools");
		}

		// Assimp ThirdParty integration
		string AssimpPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Assimp");

		if (Directory.Exists(AssimpPath))
		{
			string IncludePath = Path.Combine(AssimpPath, "include");
			if (Directory.Exists(IncludePath))
			{
				PublicIncludePaths.Add(IncludePath);
			}

			string LibPath = Path.Combine(AssimpPath, "lib");
			string PlatformPath = "";

			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				PlatformPath = "Win64";
			}
			else if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				PlatformPath = "Mac";
			}
			else if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				PlatformPath = "Linux";
			}

			string ConfigPath = (Target.Configuration == UnrealTargets.Debug ||
				Target.Configuration == UnrealTargets.DebugGame) ? "Debug" : "Release";

			string FullLibPath = Path.Combine(LibPath, PlatformPath, ConfigPath);

			if (Directory.Exists(FullLibPath))
			{
				string LibFile = Path.Combine(FullLibPath, "assimp-vc143-mt.lib");
				if (File.Exists(LibFile))
				{
					PublicAdditionalLibraries.Add(LibFile);
				}
			}

			string DllPath = Path.Combine(LibPath, "dll");
			if (Directory.Exists(DllPath) && Target.Platform == UnrealTargetPlatform.Win64)
			{
				string DllFile = Path.Combine(DllPath, "assimp-vc143-mt.dll");
				if (File.Exists(DllFile))
				{
					RuntimeDependencies.Add(DllFile);
					PublicDelayLoadDLLs.Add("assimp-vc143-mt.dll");
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

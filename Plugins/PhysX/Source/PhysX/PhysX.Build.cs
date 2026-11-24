
using UnrealBuildTool;
using System.IO;

public class PhysX : ModuleRules
{
	public PhysX(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;

		PublicDefinitions.Add("USING_PHYSX=1");
		PublicDefinitions.Add("PHYSX_SDK_VERSION=\"5.6.1\"");
		PrivateIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "Landscape" });

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new[] { "AssetRegistry", "MeshDescription", "StaticMeshDescription", "UnrealEd" });
		}

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));

			string PluginPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
			string BinariesPath = Path.Combine(PluginPath, "Binaries", "Win64");
			string LibPath = Path.Combine(ModuleDirectory, "lib");

			PublicRuntimeLibraryPaths.Add(BinariesPath);

			string[] RuntimeDlls =
			{
				"PhysXCommon_64.dll",
				"PhysX_64.dll",
				"PhysXCooking_64.dll",
				"PhysXFoundation_64.dll",
				"PVDRuntime_64.dll"
			};

			foreach (string RuntimeDll in RuntimeDlls)
			{
				PublicDelayLoadDLLs.Add(RuntimeDll);
				RuntimeDependencies.Add(Path.Combine(BinariesPath, RuntimeDll));
			}

			string[] ImportLibraries =
			{
				"PhysXCommon_64.lib",
				"PhysX_64.lib",
				"PhysXCooking_64.lib",
				"PhysXFoundation_64.lib",
				"PhysXExtensions_static_64.lib",
				"PhysXPvdSDK_static_64.lib",
				"PVDRuntime_64.lib"
			};

			foreach (string ImportLibrary in ImportLibraries)
			{
				PublicAdditionalLibraries.Add(Path.Combine(LibPath, ImportLibrary));
			}
		}
		else
		{
			throw new BuildException("The PhysX plugin currently supports Win64 only.");
		}
	}
}

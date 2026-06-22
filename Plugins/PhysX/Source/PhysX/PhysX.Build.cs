
//https://docs.unrealengine.com/5.3/en-US/integrating-third-party-libraries-into-unreal-engine/
using UnrealBuildTool;
using System.IO;

public class PhysX : ModuleRules
{
	public PhysX(ReadOnlyTargetRules Target) : base(Target)
	{
		//Type = ModuleType.External;
		//PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Add any macros that need to be set
		PublicDefinitions.Add("USING_PHYSX=1");
        //PublicDefinitions.Add("_WIN64");
        //PublicDefinitions.Add("WIN64");

        PublicDependencyModuleNames.AddRange(new[] {"Core", "CoreUObject", "Engine", "Landscape", "UnrealEd" }); 

	//if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
	if (Target.Platform == UnrealTargetPlatform.Win64)
	{
		// Add any include paths for the plugin
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));

		// Get the plugin's root directory (two levels up from Source/PhysX)
		string PluginPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
		string BinariesPath = Path.Combine(PluginPath, "Binaries", "Win64");

		// Add the plugin's Binaries folder to the DLL search path
		PublicRuntimeLibraryPaths.Add(BinariesPath);

		
		// Add delay load DLLs (ONLY the DLL name, no path!)
		PublicDelayLoadDLLs.Add("PhysXCommon_64.dll");
		PublicDelayLoadDLLs.Add("PhysX_64.dll");
		PublicDelayLoadDLLs.Add("PhysXCooking_64.dll");
		PublicDelayLoadDLLs.Add("PhysXFoundation_64.dll");

		// Add runtime dependencies from the plugin's Binaries folder
		// This ensures DLLs are staged during packaging
		RuntimeDependencies.Add(Path.Combine(BinariesPath, "PhysXCommon_64.dll"));
		RuntimeDependencies.Add(Path.Combine(BinariesPath, "PhysX_64.dll"));
		RuntimeDependencies.Add(Path.Combine(BinariesPath, "PhysXCooking_64.dll"));
		RuntimeDependencies.Add(Path.Combine(BinariesPath, "PhysXFoundation_64.dll"));

		// Add import libraries or static libraries
		string LibPath = Path.Combine(ModuleDirectory, "lib");
		PublicAdditionalLibraries.Add(Path.Combine(LibPath, "PhysXCommon_64.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(LibPath, "PhysX_64.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(LibPath, "PhysXCooking_64.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(LibPath, "PhysXFoundation_64.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(LibPath, "PhysXExtensions_static_64.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(LibPath, "PhysXPvdSDK_static_64.lib"));
	}
}
}
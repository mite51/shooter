
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

		PublicDependencyModuleNames.AddRange(new[] {"Core", "CoreUObject", "Engine", "Landscape" }); 

		//if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add any include paths for the plugin
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));

			// Add any import libraries or static libraries

			PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "dll", "PhysXCommon_64.dll"));
			PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "dll", "PhysX_64.dll"));
			
			PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "dll", "PhysXCooking_64.dll"));
			PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "dll", "PhysXFoundation_64.dll"));
			//PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "dll", "PhysXExtensions_static_64.dll"));

			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "PhysXCommon_64.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "PhysX_64.lib"));
			
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "PhysXCooking_64.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "PhysXFoundation_64.lib"));
			//PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "PhysXExtensions_static_64.lib"));


		}

	}
}
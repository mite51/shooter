
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

		PublicDependencyModuleNames.AddRange(new[] {"Core", "CoreUObject", "Engine"});

		//if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add any include paths for the plugin
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));

			// Add any import libraries or static libraries
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "PhysX_64.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "PhysXCommon_64.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "PhysXFoundation_64.lib"));

			//PublicDelayLoadDLLs.Add("foo.dll")
		}
/*
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Private"),
				
				// ... add other private include paths required here ...
			}
		);
*/
	}
}
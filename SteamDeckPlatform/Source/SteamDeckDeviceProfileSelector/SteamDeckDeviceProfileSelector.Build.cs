// MIT License - Copyright (c) Juniper Bouchard

using UnrealBuildTool;

public class SteamDeckDeviceProfileSelector : ModuleRules
{
	public SteamDeckDeviceProfileSelector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"Engine",
		});
    }
}

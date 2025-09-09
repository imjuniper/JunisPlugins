// MIT License - Copyright (c) Juniper Bouchard

using UnrealBuildTool;

public class SteamDeckConfig : ModuleRules
{
	public SteamDeckConfig(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"SteamShared"
		});
		
        AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
    }
}

// MIT License - Copyright (c) Juniper Bouchard

using UnrealBuildTool;

public class NekoSteam : ModuleRules
{
	public NekoSteam(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"SteamShared"
		});
		
		AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
	}
}

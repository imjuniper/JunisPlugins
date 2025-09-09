// MIT License - Copyright (c) Juniper Bouchard

using UnrealBuildTool;

public class NekoUtils : ModuleRules
{
	public NekoUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"CommonUI",
			"Engine",
			"GameplayTags",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CommonInput",
			"EngineSettings",
			"UMG",
			"Slate",
		});
	}
}
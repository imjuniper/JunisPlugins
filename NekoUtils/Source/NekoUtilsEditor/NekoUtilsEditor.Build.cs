// MIT License - Copyright (c) Juniper Bouchard

using UnrealBuildTool;

public class NekoUtilsEditor : ModuleRules
{
	public NekoUtilsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"EditorSubsystem",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Engine",
			"UnrealEd",
		});
	}
}

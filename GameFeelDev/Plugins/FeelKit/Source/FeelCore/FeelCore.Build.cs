// Copyright 2026 Billo. All Rights Reserved.

using UnrealBuildTool;

public class FeelCore : ModuleRules
{
	public FeelCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Hard rule: stock modules only.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine",
			"GameplayTags",
			// FKey appears in public headers (Feel Switch keys).
			"InputCore",
			// Widget targets and widget delivery appear in public headers.
			"Slate",
			"SlateCore",
			"UMG",
		});
	}
}

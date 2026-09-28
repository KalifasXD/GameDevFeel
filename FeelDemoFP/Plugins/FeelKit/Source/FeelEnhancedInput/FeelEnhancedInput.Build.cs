// Copyright 2026 Billo. All Rights Reserved.

using UnrealBuildTool;

/** Optional FeelKit module: plays recipes from Enhanced Input actions (TRG-003). Kept out of FeelCore so FeelCore depends only on stock modules. */
public class FeelEnhancedInput : ModuleRules
{
	public FeelEnhancedInput(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"FeelCore",
			"GameplayTags",
		});
	}
}

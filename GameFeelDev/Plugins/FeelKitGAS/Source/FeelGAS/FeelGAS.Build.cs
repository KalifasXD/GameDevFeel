// Copyright 2026 Billo. All Rights Reserved.

using UnrealBuildTool;

/** FeelKit GAS add-on: Gameplay Cue notifies that play recipes. Kept out of FeelKit so projects without GAS never enable it. */
public class FeelGAS : ModuleRules
{
	public FeelGAS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"FeelCore",
			"GameplayAbilities",
			"GameplayTags",
		});
	}
}

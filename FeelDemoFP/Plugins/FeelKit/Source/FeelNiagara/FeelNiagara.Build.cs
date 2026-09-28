// Copyright 2026 Billo. All Rights Reserved.

using UnrealBuildTool;

/** Optional FeelKit module for Niagara particle steps. Kept out of FeelCore so FeelCore depends only on stock modules. */
public class FeelNiagara : ModuleRules
{
	public FeelNiagara(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"FeelCore",
			"GameplayTags",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Niagara",
		});
	}
}

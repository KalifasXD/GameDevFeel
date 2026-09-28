// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameFeelDev : ModuleRules
{
	public GameFeelDev(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
			"FeelCore",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Development tools in editor builds only (FeelCreateProjectTool.cpp and the DiagFeel playthroughs).
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "GameProjectGeneration", "UnrealEd", "MessageLog", "AssetRegistry", "LevelEditor" });
		}

		PublicIncludePaths.AddRange(new string[] {
			"GameFeelDev",
			"GameFeelDev/Variant_Platforming",
			"GameFeelDev/Variant_Platforming/Animation",
			"GameFeelDev/Variant_Combat",
			"GameFeelDev/Variant_Combat/AI",
			"GameFeelDev/Variant_Combat/Animation",
			"GameFeelDev/Variant_Combat/Gameplay",
			"GameFeelDev/Variant_Combat/Interfaces",
			"GameFeelDev/Variant_Combat/UI",
			"GameFeelDev/Variant_SideScrolling",
			"GameFeelDev/Variant_SideScrolling/AI",
			"GameFeelDev/Variant_SideScrolling/Gameplay",
			"GameFeelDev/Variant_SideScrolling/Interfaces",
			"GameFeelDev/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

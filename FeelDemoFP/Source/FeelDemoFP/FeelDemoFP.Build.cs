// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FeelDemoFP : ModuleRules
{
	public FeelDemoFP(ReadOnlyTargetRules Target) : base(Target)
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
			// FeelKit: the demo hooks (look for "FeelKit" comments).
			"FeelCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"FeelDemoFP",
			"FeelDemoFP/Variant_Horror",
			"FeelDemoFP/Variant_Horror/UI",
			"FeelDemoFP/Variant_Shooter",
			"FeelDemoFP/Variant_Shooter/AI",
			"FeelDemoFP/Variant_Shooter/UI",
			"FeelDemoFP/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

// Copyright 2026 Billo. All Rights Reserved.

using UnrealBuildTool;

public class FeelEditor : ModuleRules
{
	public FeelEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"FeelCore",
			"AdvancedPreviewScene",
			"AssetDefinition",
			"AssetRegistry",
			"AssetTools",
			"UMG",
			"UMGEditor",
			"BlueprintGraph",
			"ClassViewer",
			"ContentBrowser",
			"ContentBrowserData",
			"DesktopPlatform",
			"DeveloperSettings",
			"GameplayTags",
			"GraphEditor",
			"InputCore",
			"Json",
			"JsonUtilities",
			"MessageLog",
			"Projects",
			"PropertyEditor",
			"RHI",
			"RenderCore",
			"ImageWrapper",
			"Slate",
			"SlateCore",
			"ToolMenus",
				"ToolWidgets",
			"UnrealEd",
			"WorkspaceMenuStructure",
		});
	}
}

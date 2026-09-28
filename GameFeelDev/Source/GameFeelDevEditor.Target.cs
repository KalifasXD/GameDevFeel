using UnrealBuildTool;

public class GameFeelDevEditorTarget : TargetRules
{
	public GameFeelDevEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("GameFeelDev");
	}
}

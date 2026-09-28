using UnrealBuildTool;

public class GameFeelDevTarget : TargetRules
{
	public GameFeelDevTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("GameFeelDev");
	}
}

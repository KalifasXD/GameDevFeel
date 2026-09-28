param([string]$Package = "B:\NewUE5Project\Build\FKPkg_56", [string]$Engine = "B:\UE_5.6", [string]$Association = "", [string]$Root = "B:\NewUE5Project\Build\GASProof")
# Proves the GAS add-on setup (D-076) with the packaged plugin (run package_plugin.ps1 first). Makes two new C++ projects:
#   NoGAS:   FeelKit only. GameplayAbilities and the add-on must be off, although the add-on folder sits inside FeelKit.
#   WithGAS: FeelKit plus Extras/FeelKitGAS copied into Plugins. Both must be on, the project must build and the
#            Gameplay Cue test (copied from the development add-on) must pass.
# Each project carries a test Proof.Plugins that asks the plugin manager what is enabled and which modules are loaded.
# Other engines: -Engine D:\EpicGames\UE_5.8 -Association 5.8 -Package <that package> -Root <own folder>.
$root = $Root
$engine = $Engine
$association = if ($Association -ne "") { $Association } else { (Get-Content "B:\NewUE5Project\GameFeelDev\GameFeelDev.uproject" -Raw | ConvertFrom-Json).EngineAssociation }
$gasTest = "B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Extras\FeelKitGAS\Source\FeelGAS\Private\Tests\FeelGASTests.cpp"
$logs = "B:\NewUE5Project\Build\Logs"

function Write-Utf8([string]$Path, [string]$Text) {
	New-Item -ItemType Directory -Force (Split-Path $Path) | Out-Null
	[System.IO.File]::WriteAllText($Path, $Text, (New-Object System.Text.UTF8Encoding($false)))
}

function New-ProofProject([string]$Name, [bool]$WithGas) {
	$dir = "$root\$Name"
	if (Test-Path $dir) { Remove-Item -Recurse -Force $dir }
	robocopy $Package "$dir\Plugins\FeelKit" /E /NFL /NDL /NJH /NJS /NP | Out-Null
	if ($WithGas) { robocopy "$Package\Extras\FeelKitGAS" "$dir\Plugins\FeelKitGAS" /E /NFL /NDL /NJH /NJS /NP | Out-Null }

	Write-Utf8 "$dir\$Name.uproject" @"
{
	"FileVersion": 3,
	"EngineAssociation": "$association",
	"Modules": [ { "Name": "$Name", "Type": "Runtime", "LoadingPhase": "Default" } ]
}
"@
	foreach ($kind in @("Game", "Editor")) {
		$suffix = if ($kind -eq "Editor") { "Editor" } else { "" }
		Write-Utf8 "$dir\Source\$Name$suffix.Target.cs" @"
using UnrealBuildTool;

public class $Name${suffix}Target : TargetRules
{
	public $Name${suffix}Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.$kind;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("$Name");
	}
}
"@
	}
	$gasDeps = if ($WithGas) { '"FeelGAS", "GameplayAbilities", ' } else { '' }
	$expect = if ($WithGas) { 1 } else { 0 }
	Write-Utf8 "$dir\Source\$Name\$Name.Build.cs" @"
using UnrealBuildTool;

public class $Name : ModuleRules
{
	public $Name(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "GameplayTags", "Projects", "FeelCore", $gasDeps });
		PublicDefinitions.Add("PROOF_EXPECT_GAS=$expect");
	}
}
"@
	Write-Utf8 "$dir\Source\$Name\$Name.cpp" @"
#include "Modules/ModuleManager.h"
#include "Misc/AutomationTest.h"
#include "Interfaces/IPluginManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, $Name, "$Name");

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProofPluginsTest, "Proof.Plugins", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FProofPluginsTest::RunTest(const FString& Parameters)
{
	auto Enabled = [](const TCHAR* Plugin)
	{
		const TSharedPtr<IPlugin> Found = IPluginManager::Get().FindPlugin(Plugin);
		return Found.IsValid() && Found->IsEnabled();
	};
	const bool bFeelKit = Enabled(TEXT("FeelKit"));
	const bool bAddOn = Enabled(TEXT("FeelKitGAS"));
	const bool bGas = Enabled(TEXT("GameplayAbilities"));
	const bool bFeelCore = FModuleManager::Get().IsModuleLoaded(TEXT("FeelCore"));
	const bool bGasModule = FModuleManager::Get().IsModuleLoaded(TEXT("GameplayAbilities"));
	const bool bFeelGasModule = FModuleManager::Get().IsModuleLoaded(TEXT("FeelGAS"));
	AddInfo(FString::Printf(TEXT("PROOF ${Name}: FeelKit plugin on %d, FeelCore loaded %d, FeelKitGAS plugin on %d, FeelGAS loaded %d, GameplayAbilities plugin on %d, GameplayAbilities loaded %d"),
		bFeelKit, bFeelCore, bAddOn, bFeelGasModule, bGas, bGasModule));
	TestTrue(TEXT("FeelKit is on"), bFeelKit);
	TestTrue(TEXT("FeelCore is loaded"), bFeelCore);
	TestEqual(TEXT("FeelKitGAS plugin on"), bAddOn, PROOF_EXPECT_GAS == 1);
	TestEqual(TEXT("FeelGAS module loaded"), bFeelGasModule, PROOF_EXPECT_GAS == 1);
	TestEqual(TEXT("GameplayAbilities plugin on"), bGas, PROOF_EXPECT_GAS == 1);
	TestEqual(TEXT("GameplayAbilities module loaded"), bGasModule, PROOF_EXPECT_GAS == 1);
	return true;
}
#endif
"@
	if ($WithGas) { Copy-Item $gasTest "$dir\Source\$Name\FeelGASTests.cpp" }
	return $dir
}

foreach ($case in @(@{ Name = "NoGAS"; Gas = $false }, @{ Name = "WithGAS"; Gas = $true })) {
	$name = $case.Name
	$dir = New-ProofProject $name $case.Gas
	$build = & "$engine/Engine/Build/BatchFiles/Build.bat" "${name}Editor" Win64 Development -Project="$dir\$name.uproject" -WaitMutex 2>&1
	$result = ($build | Select-String "Result:").Line
	$warnings = @($build | Select-String ": warning|: error")
	"PROOF ${name} build: $result, warnings and errors: $($warnings.Count)"
	$warnings | Select-Object -First 5 | ForEach-Object { "PROOF   " + $_.Line.Trim() }

	$log = "$logs\gas_proof_$(Split-Path $root -Leaf)_$name.log"
	& "$engine/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$dir\$name.uproject" -ExecCmds="Automation RunTests Proof+FeelKit.GAS; Quit" -TestExit="Automation Test Queue Empty" -unattended -nullrhi -nosplash -abslog="$log" | Out-Null
	Select-String -Path $log -Pattern "PROOF |Test Completed" | ForEach-Object { $_.Line -replace '^.*?(PROOF|Test Completed)', '$1' }
	$mounted = @(Select-String -Path $log -Pattern "Mounting (Engine|Project) plugin (GameplayAbilities|FeelKit|FeelKitGAS)\b" | ForEach-Object { $_.Line -replace '^.*Mounting ', '' })
	"PROOF ${name} plugins mounted: $($mounted -join ', ')"
	"PROOF ${name} .uproject: " + ((Get-Content "$dir\$name.uproject" -Raw) -replace '\s+', ' ')
}

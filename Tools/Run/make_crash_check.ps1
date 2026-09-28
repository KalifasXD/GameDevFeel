param([switch]$SkipBuild)
# Builds the projects for checking by hand that Epic's 5.6 template levels crash in 5.7 and 5.8 with or without FeelKit.
# For each of UE 5.7 and 5.8 it makes, under Build\CrashCheck\:
#   UE<v>_Epic56_ThirdPerson and UE<v>_Epic56_FirstPerson: Epic's untouched 5.6 templates (no FeelKit), opened in <v>
#   UE<v>_Epic<v>_ThirdPerson and UE<v>_Epic<v>_FirstPerson: Epic's own <v> templates (no FeelKit)
# Changes to the copies, all needed to open them in the newer engine or to test them automatically:
#   - EngineAssociation set to the engine; the 5.6 templates get the engine's current build settings and include order
#     (what the editor's project upgrade does);
#   - the 5.6 First Person template gets the one-line Shooter fix Epic made in their later template (it does not
#     compile on 5.7 and later otherwise);
#   - a small test file StockPIESmoke.cpp (tests Stock.PIESmoke and Stock.PIESmokeViewport) and an editor-only
#     UnrealEd/LevelEditor dependency, so the same Play can be started automatically.
# The engine's identifier for .uproject files: the ID the engine registered for itself under
# HKCU\Software\Epic Games\Unreal Engine\Builds when there is one, otherwise the launcher name ("5.8").
function Get-EngineAssociation([string]$EngineDir, [string]$Version) {
	$wanted = $EngineDir.TrimEnd('/', '\').Replace('\', '/').ToLower()
	$builds = Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue
	if ($builds) {
		foreach ($entry in $builds.PSObject.Properties) {
			if ($entry.Name -like "{*" -and ([string]$entry.Value).TrimEnd('/', '\').Replace('\', '/').ToLower() -eq $wanted) { return $entry.Name }
		}
	}
	return $Version
}
$root = "B:\NewUE5Project\Build\CrashCheck"
$engines = @{ "5.7" = "D:\EpicGames\UE_5.7"; "5.8" = "D:\EpicGames\UE_5.8" }
$smoke = @'
// Starts Play In Editor on the loaded map, lets it run for 8 seconds and ends it. Stock.PIESmoke plays in a new window,
// Stock.PIESmokeViewport plays in the level viewport like the Play button.
#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Editor.h"
#include "LevelEditor.h"
#include "IAssetViewport.h"
#include "Modules/ModuleManager.h"
#include "Misc/AutomationTest.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FStockPIEWait, double, Start);
bool FStockPIEWait::Update()
{
	if (FPlatformTime::Seconds() - Start < 8.0) { return false; }
	UE_LOG(LogTemp, Display, TEXT("STOCK PIE ran 8 seconds, world %s"), GEditor->PlayWorld ? *GEditor->PlayWorld->GetName() : TEXT("none"));
	GEditor->RequestEndPlayMap();
	return true;
}

static void StartStockPIE(bool bViewport)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = bViewport ? EPlayModeType::PlayMode_InViewPort : EPlayModeType::PlayMode_InEditorFloating;
	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	if (bViewport)
	{
		FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		Params.DestinationSlateViewport = LevelEditor.GetFirstActiveViewport();
		UE_LOG(LogTemp, Display, TEXT("STOCK PIE in the level viewport: %s"), Params.DestinationSlateViewport.IsSet() && Params.DestinationSlateViewport.GetValue().IsValid() ? TEXT("yes") : TEXT("no viewport found"));
	}
	GEditor->RequestPlaySession(Params);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStockPIESmoke, "Stock.PIESmoke", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FStockPIESmoke::RunTest(const FString& Parameters)
{
	StartStockPIE(false);
	ADD_LATENT_AUTOMATION_COMMAND(FStockPIEWait(FPlatformTime::Seconds()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStockPIESmokeViewport, "Stock.PIESmokeViewport", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FStockPIESmokeViewport::RunTest(const FString& Parameters)
{
	StartStockPIE(true);
	ADD_LATENT_AUTOMATION_COMMAND(FStockPIEWait(FPlatformTime::Seconds()));
	return true;
}
#endif
'@

function Write-Utf8([string]$Path, [string]$Text) { [System.IO.File]::WriteAllText($Path, $Text, (New-Object System.Text.UTF8Encoding($false))) }

foreach ($v in @("5.7", "5.8")) {
	$engine = $engines[$v]
	$tag = $v -replace '\.', ''
	foreach ($source in @("5.6", $v)) {
		$sourceEngine = if ($source -eq "5.6") { "B:\UE_5.6" } else { $engine }
		$sourceTag = $source -replace '\.', ''
		foreach ($template in @("ThirdPerson", "FirstPerson")) {
			$module = "TP_$template"
			$dst = "$root\UE${tag}_Epic${sourceTag}_$template"
			if (Test-Path $dst) { Remove-Item -Recurse -Force $dst }
			robocopy "$sourceEngine\Templates\$module" $dst /E /XD Media /NFL /NDL /NJH /NJS /NP | Out-Null
			$uproject = "$dst\$module.uproject"
			$text = [System.IO.File]::ReadAllText($uproject)
			if ($text -match '"EngineAssociation"') { $text = $text -replace '"EngineAssociation":\s*"[^"]*"', ('"EngineAssociation": "' + (Get-EngineAssociation $engine $v) + '"') }
			else { $text = $text -replace '"FileVersion":\s*3,', ('"FileVersion": 3,' + "`r`n`t" + '"EngineAssociation": "' + (Get-EngineAssociation $engine $v) + '",') }
			Write-Utf8 $uproject $text
			if ($source -eq "5.6") {
				foreach ($target in Get-ChildItem "$dst\Source" -Filter *.Target.cs) {
					$code = [System.IO.File]::ReadAllText($target.FullName) -replace 'BuildSettingsVersion\.V\d+', 'BuildSettingsVersion.Latest' -replace 'EngineIncludeOrderVersion\.Unreal5_\d+', 'EngineIncludeOrderVersion.Latest'
					Write-Utf8 $target.FullName $code
				}
				$shooter = "$dst\Source\$module\Variant_Shooter\AI\ShooterStateTreeUtility.cpp"
				if (Test-Path $shooter) {
					$code = [System.IO.File]::ReadAllText($shooter).Replace('FInstanceDataType* LambdaInstanceData = WeakContext.MakeStrongExecutionContext().GetInstanceDataPtr<FInstanceDataType>();', 'const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();' + "`r`n" + '				FInstanceDataType* LambdaInstanceData = StrongContext.GetInstanceDataPtr<FInstanceDataType>();')
					Write-Utf8 $shooter $code
				}
			}
			$buildCs = "$dst\Source\$module\$module.Build.cs"
			$code = [System.IO.File]::ReadAllText($buildCs).Replace('PrivateDependencyModuleNames.AddRange(new string[] { });', 'PrivateDependencyModuleNames.AddRange(new string[] { });' + "`r`n`t`t" + 'if (Target.bBuildEditor) { PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "LevelEditor" }); }')
			Write-Utf8 $buildCs $code
			Write-Utf8 "$dst\Source\$module\StockPIESmoke.cpp" $smoke
			if (-not $SkipBuild) {
				$out = & "$engine\Engine\Build\BatchFiles\Build.bat" "${module}Editor" Win64 Development -Project="$uproject" -WaitMutex 2>&1
				"CRASHCHECK built UE$v Epic$source $template : " + ($out | Select-String "Result:").Line
				$out | Select-String ": error" | Select-Object -First 5 | ForEach-Object { "CRASHCHECK   " + $_.Line.Trim() }
			}
		}
	}
}

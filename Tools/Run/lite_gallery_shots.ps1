param([switch]$Keep)
# Editor pictures of FeelKit Lite for its store gallery. Takes the Lite source that package_plugin.ps1 -Edition Lite
# staged (Build\Stage\Lite_56\FeelKit), adds the capture diagnostic DiagFeel.GalleryEditorShots and its helper from the
# development plugin (both use only what Lite contains), builds a new C++ project with it on UE 5.6 and runs the
# diagnostic in a rendering session. The project is called MyGame, as a buyer's might be; its name shows in the Content
# Browser picture. Pictures: Build\LiteShots\Saved\FeelKit\Manual\Gallery_*.png, copied to Build\LiteGallery. The
# project is deleted afterwards unless -Keep. Close the editor first.
$stage = "B:\NewUE5Project\Build\Stage\Lite_56\FeelKit"
$tests = "B:\NewUE5Project\GameFeelDev\Plugins\FeelKit\Source\FeelEditor\Private\Tests"
$dir = "B:\NewUE5Project\Build\LiteShots"
$out = "B:\NewUE5Project\Build\LiteGallery"
$logs = "B:\NewUE5Project\Build\Logs"
$name = "MyGame"

function Write-Utf8([string]$Path, [string]$Text) {
	New-Item -ItemType Directory -Force (Split-Path $Path) | Out-Null
	[System.IO.File]::WriteAllText($Path, $Text, (New-Object System.Text.UTF8Encoding($false)))
}

if (-not (Test-Path "$stage\FeelKit.uplugin")) { "LITESHOTS no Lite stage; run package_plugin.ps1 -Edition Lite first"; exit 1 }
if (Test-Path $dir) { Remove-Item -Recurse -Force $dir }
robocopy $stage "$dir\Plugins\FeelKit" /E /XD Binaries Intermediate /NFL /NDL /NJH /NJS /NP | Out-Null
New-Item -ItemType Directory -Force "$dir\Plugins\FeelKit\Source\FeelEditor\Private\Tests" | Out-Null
Copy-Item "$tests\FeelManualCapture.h", "$tests\FeelGalleryEditorShots.cpp" "$dir\Plugins\FeelKit\Source\FeelEditor\Private\Tests"
$association = (Get-Content "B:\NewUE5Project\GameFeelDev\GameFeelDev.uproject" -Raw | ConvertFrom-Json).EngineAssociation
Write-Utf8 "$dir\$name.uproject" @"
{
	"FileVersion": 3,
	"EngineAssociation": "$association",
	"Modules": [ { "Name": "$name", "Type": "Runtime", "LoadingPhase": "Default" } ],
	"Plugins": [ { "Name": "FeelKit", "Enabled": true } ]
}
"@
foreach ($kind in @("Game", "Editor")) {
	$suffix = if ($kind -eq "Editor") { "Editor" } else { "" }
	Write-Utf8 "$dir\Source\$name$suffix.Target.cs" @"
using UnrealBuildTool;

public class $name${suffix}Target : TargetRules
{
	public $name${suffix}Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.$kind;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("$name");
	}
}
"@
}
Write-Utf8 "$dir\Source\$name\$name.Build.cs" @"
using UnrealBuildTool;

public class $name : ModuleRules
{
	public $name(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
	}
}
"@
Write-Utf8 "$dir\Source\$name\$name.cpp" @"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, $name, "$name");
"@

$build = & "B:/UE_5.6/Engine/Build/BatchFiles/Build.bat" "${name}Editor" Win64 Development -Project="$dir\$name.uproject" -WaitMutex 2>&1
"LITESHOTS build: " + ($build | Select-String "Result:").Line
$build | Select-String ": warning|: error" | Select-Object -First 10 | ForEach-Object { "LITESHOTS   " + $_.Line.Trim() }
if (-not ($build | Select-String "Result: Succeeded")) { "LITESHOTS build failed; Build\LiteGallery left as it was"; exit 1 }

$log = "$logs\lite_gallery_shots.log"
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$dir\$name.uproject" -ExecCmds="Automation RunTests DiagFeel.GalleryEditorShots" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
Select-String -Path $log -Pattern "Result=\{|MANUALSHOT|LogAutomationController: Error" | ForEach-Object { "LITESHOTS " + ($_.Line -replace '^.*?(Test Completed|MANUALSHOT|LogAutomationController: )', '$1') }

$taken = @(Get-ChildItem "$dir\Saved\FeelKit\Manual\Gallery_*.png" -ErrorAction SilentlyContinue)
if ($taken.Count -eq 0) { "LITESHOTS no pictures taken; Build\LiteGallery left as it was"; exit 1 }
if (Test-Path $out) { Remove-Item -Recurse -Force $out }
New-Item -ItemType Directory -Force $out | Out-Null
$taken | Copy-Item -Destination $out
"LITESHOTS pictures: " + ((Get-ChildItem $out).Name -join ", ")
if (-not $Keep) { Remove-Item -Recurse -Force $dir }

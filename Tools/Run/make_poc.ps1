param(
	[string]$Engine = "D:\EpicGames\UE_5.8",
	[string]$Version = "5.8",
	[switch]$WithTests
)
# Proof that FeelKit works on a newer engine the way a buyer gets it: a fresh project from Epic's own Third Person
# template of that engine, FeelKit from that engine's Fab package (package_plugin.ps1), and the Platformer demo built
# on top by the same kit script as in GameFeelDev (build_platformer_kit.py, no template code changes).
# -WithTests uses a package that keeps FeelKit's tests, so the Platformer playthrough can check it automatically.
# Output: Build\UE<tag>_PoC\TP_ThirdPerson (or Build\UE<tag>_PoC_Tests\TP_ThirdPerson).
$tag = $Version -replace '\.', ''
$suffix = if ($WithTests) { "_Tests" } else { "" }
$root = "B:\NewUE5Project\Build\UE${tag}_PoC$suffix"
$project = "$root\TP_ThirdPerson"
$uproject = "$project\TP_ThirdPerson.uproject"
$logs = "B:\NewUE5Project\Build\Logs"
$run = "B:\NewUE5Project\Tools\Run"

function Write-Utf8([string]$Path, [string]$Text) { [System.IO.File]::WriteAllText($Path, $Text, (New-Object System.Text.UTF8Encoding($false))) }
function Get-EngineAssociation([string]$EngineDir, [string]$Fallback) {
	$wanted = $EngineDir.TrimEnd('/', '\').Replace('\', '/').ToLower()
	$builds = Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue
	if ($builds) {
		foreach ($entry in $builds.PSObject.Properties) {
			if ($entry.Name -like "{*" -and ([string]$entry.Value).TrimEnd('/', '\').Replace('\', '/').ToLower() -eq $wanted) { return $entry.Name }
		}
	}
	return $Fallback
}

# 1. The package.
$package = "B:\NewUE5Project\Build\FKPkg_$tag$suffix"
if ($WithTests) { & "$run\package_plugin.ps1" -Engine $Engine -Version $Version -KeepTests -Out $package | Select-String "BUILD|test files" }
elseif (-not (Test-Path "$package\FeelKit.uplugin")) { & "$run\package_plugin.ps1" -Engine $Engine -Version $Version | Select-String "BUILD" }

# 2. A fresh project from the engine's own template, with FeelKit in its Plugins folder.
if (Test-Path $root) { Remove-Item -Recurse -Force $root }
robocopy "$Engine\Templates\TP_ThirdPerson" $project /E /XD Media /NFL /NDL /NJH /NJS /NP | Out-Null
robocopy $package "$project\Plugins\FeelKit" /E /XD Intermediate /NFL /NDL /NJH /NJS /NP | Out-Null
$text = [System.IO.File]::ReadAllText($uproject) -replace '"EngineAssociation":\s*"[^"]*"', ('"EngineAssociation": "' + (Get-EngineAssociation $Engine $Version) + '"')
Write-Utf8 $uproject $text
"POC project $project"

# 3. Build it.
$out = & "$Engine\Engine\Build\BatchFiles\Build.bat" TP_ThirdPersonEditor Win64 Development -Project="$uproject" -WaitMutex 2>&1
"POC build: " + ($out | Select-String "Result:").Line + ", warnings and errors: " + @($out | Select-String ": warning|: error").Count

# 4. The Platformer demo, built by the kit script.
$log = "$logs\poc_kit_ue$tag$suffix.log"
$env:FEELKIT_PROJECT = $project
& "$Engine\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $uproject -run=pythonscript -script="B:\NewUE5Project\Tools\Unreal\build_platformer_kit.py" -unattended -nosplash -nullrhi "-abslog=$log" | Out-Null
Remove-Item Env:FEELKIT_PROJECT
Select-String -Path $log -Pattern "PLATKIT" | ForEach-Object { "POC " + ($_.Line -replace '^.*PLATKIT ', '') }

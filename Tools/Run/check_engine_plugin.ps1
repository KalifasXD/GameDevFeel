param(
	[string]$Engine = "D:\EpicGames\UE_5.8",
	[string]$Version = "5.8",
	[switch]$KeepInstalled
)
# Release check (C-006, TS-005, TS-006): FeelKit installed the way Fab installs it, as an engine plugin, used by a
# Blueprint-only project made from Epic's Third Person Blueprint template, then that project packaged for Windows.
# Installs Build\FKPkg_<tag> (no tests; with Binaries and Intermediate, which hold the prebuilt game libraries Epic's
# build delivers to buyers and which a Blueprint-only project needs to package) to <Engine>\Engine\Plugins\Marketplace\FeelKit and removes it
# at the end unless -KeepInstalled. The user allowed this install on 2026-09-24.
$tag = $Version -replace '\.', ''
$package = "B:\NewUE5Project\Build\FKPkg_$tag"
$install = "$Engine\Engine\Plugins\Marketplace\FeelKit"
$root = "B:\NewUE5Project\Build\UE${tag}_BPOnly"
$project = "$root\TP_ThirdPersonBP"
$uproject = "$project\TP_ThirdPersonBP.uproject"
$logs = "B:\NewUE5Project\Build\Logs"

function Write-Utf8([string]$Path, [string]$Text) { [System.IO.File]::WriteAllText($Path, $Text, (New-Object System.Text.UTF8Encoding($false))) }
function Get-EngineAssociation([string]$EngineDir, [string]$Fallback) {
	$wanted = $EngineDir.TrimEnd('/', '\').Replace('\', '/').ToLower()
	$builds = Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue
	if ($builds) { foreach ($entry in $builds.PSObject.Properties) { if ($entry.Name -like "{*" -and ([string]$entry.Value).TrimEnd('/', '\').Replace('\', '/').ToLower() -eq $wanted) { return $entry.Name } } }
	return $Fallback
}

# 1. Install as an engine plugin.
if (Test-Path $install) { Remove-Item -Recurse -Force $install }
robocopy $package $install /E /NFL /NDL /NJH /NJS /NP | Out-Null
"CHECK installed $package to $install"

# 2. A Blueprint-only project that enables FeelKit (and Python, only for the check script).
if (Test-Path $root) { Remove-Item -Recurse -Force $root }
robocopy "$Engine\Templates\TP_ThirdPersonBP" $project /E /XD Media /NFL /NDL /NJH /NJS /NP | Out-Null
$json = Get-Content $uproject -Raw | ConvertFrom-Json
$json.EngineAssociation = Get-EngineAssociation $Engine $Version
$json.Plugins += [pscustomobject]@{ Name = "FeelKit"; Enabled = $true }
$json.Plugins += [pscustomobject]@{ Name = "PythonScriptPlugin"; Enabled = $true }
Write-Utf8 $uproject ($json | ConvertTo-Json -Depth 8)
"CHECK Blueprint-only project $project (no Source folder: $(-not (Test-Path "$project\Source")))"

# 3. Open it: FeelKit must load from the engine folder, and a library recipe must load.
$py = "$root\check_feelkit.py"
Write-Utf8 $py @'
import unreal
plugin_dir = unreal.Paths.convert_relative_path_to_full(unreal.Paths.engine_plugins_dir())
recipe = unreal.load_asset('/FeelKit/Library/Impact/FR_Impact_HeavyHit')
unreal.log_warning('CHECKPY FeelRecipe class: {0}'.format(unreal.FeelRecipe.static_class().get_path_name()))
unreal.log_warning('CHECKPY library recipe loaded: {0}, tracks {1}'.format(recipe is not None, len(recipe.get_editor_property('tracks')) if recipe else 0))
'@
$log = "$logs\bponly_open_ue$tag.log"
& "$Engine\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $uproject -run=pythonscript -script="$py" -unattended -nosplash -nullrhi "-abslog=$log" | Out-Null
Select-String -Path $log -Pattern "CHECKPY|Mounting Engine plugin FeelKit|Plugin 'FeelKit'" | ForEach-Object { "CHECK " + ($_.Line -replace '^.*?(CHECKPY|Mounting|Plugin)', '$1') }

# 4. Package the Blueprint-only project for Windows (Shipping).
$pkglog = "$logs\bponly_package_ue$tag.log"
& "$Engine\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="$uproject" -platform=Win64 -clientconfig=Shipping -build -cook -stage -pak -archive -archivedirectory="$root\Packaged" -unattended -utf8output *> $pkglog
"CHECK package exit code: $LASTEXITCODE"
Select-String -Path $pkglog -Pattern "BUILD SUCCESSFUL|BUILD FAILED|temporary target|temp target|: error|Warning: .*FeelKit|Error: " | Select-Object -First 15 | ForEach-Object { "CHECK   " + $_.Line.Trim() }
$exe = Get-ChildItem "$root\Packaged" -Recurse -Filter *.exe -ErrorAction SilentlyContinue | Select-Object -First 1
"CHECK packaged game: $(if ($exe) { $exe.FullName } else { 'none' })"
$feelInPak = Select-String -Path $pkglog -Pattern "FeelCore|FeelKit" -Quiet
"CHECK FeelKit mentioned in the packaging log: $feelInPak"

# 5. Remove the engine install again.
if (-not $KeepInstalled) {
	Remove-Item -Recurse -Force $install
	$market = Split-Path $install
	if (-not (Get-ChildItem $market -ErrorAction SilentlyContinue)) { Remove-Item $market }
	"CHECK removed $install"
}

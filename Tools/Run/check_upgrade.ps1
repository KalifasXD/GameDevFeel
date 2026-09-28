param([string]$Version = "5.6")
# Moving a project from FeelKit Lite to FeelKit Pro: a Blueprint-only project with the Lite package saves two recipes of
# its own (a copy of a library recipe, and a new recipe with one track built by script), then the Lite plugin folder is
# replaced by the Pro package and the same project loads them again. Every track must keep its step and settings.
# Uses Build\FKPkg_Lite_<tag> and Build\FKPkg_Pro_<tag>; the project in Build\UpgradeCheck is deleted afterwards.
$engines = @{ "5.6" = "B:\UE_5.6"; "5.7" = "D:\EpicGames\UE_5.7"; "5.8" = "D:\EpicGames\UE_5.8" }
$tag = $Version -replace '\.', ''
$engine = $engines[$Version]
$dir = "B:\NewUE5Project\Build\UpgradeCheck"
$logs = "B:\NewUE5Project\Build\Logs"

function Write-Utf8([string]$Path, [string]$Text) {
	New-Item -ItemType Directory -Force (Split-Path $Path) | Out-Null
	[System.IO.File]::WriteAllText($Path, $Text, (New-Object System.Text.UTF8Encoding($false)))
}
function Get-EngineAssociation([string]$EngineDir, [string]$Fallback) {
	$wanted = $EngineDir.TrimEnd('/', '\').Replace('\', '/').ToLower()
	$builds = Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue
	if ($builds) { foreach ($entry in $builds.PSObject.Properties) { if ($entry.Name -like "{*" -and ([string]$entry.Value).TrimEnd('/', '\').Replace('\', '/').ToLower() -eq $wanted) { return $entry.Name } } }
	return $Fallback
}

if (Test-Path $dir) { Remove-Item -Recurse -Force $dir }
robocopy "B:\NewUE5Project\Build\FKPkg_Lite_$tag" "$dir\Plugins\FeelKit" /E /NFL /NDL /NJH /NJS /NP | Out-Null
$association = if ($Version -eq "5.6") { (Get-Content "B:\NewUE5Project\GameFeelDev\GameFeelDev.uproject" -Raw | ConvertFrom-Json).EngineAssociation } else { Get-EngineAssociation $engine $Version }
Write-Utf8 "$dir\UpgradeCheck.uproject" @"
{
	"FileVersion": 3,
	"EngineAssociation": "$association",
	"Plugins": [
		{ "Name": "FeelKit", "Enabled": true },
		{ "Name": "PythonScriptPlugin", "Enabled": true }
	]
}
"@

Write-Utf8 "$dir\make.py" @'
import unreal
unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(["/FeelKit", "/Game"], True)
tools = unreal.AssetToolsHelpers.get_asset_tools()
copy = unreal.EditorAssetLibrary.duplicate_asset("/FeelKit/Library/Impact/FR_Impact_HeavyHit", "/Game/MyRecipes/MyHeavyHit")
recipe = tools.create_asset("MyShake", "/Game/MyRecipes", unreal.FeelRecipe, None)
track = unreal.FeelTrack()
step = unreal.new_object(unreal.FeelStep_ProceduralShake, outer=recipe)
step.set_editor_property("frequency", 17.0)
track.set_editor_property("step", step)
track.set_editor_property("duration", 0.4)
track.set_editor_property("start_time", 0.1)
recipe.set_editor_property("tracks", [track])
unreal.EditorAssetLibrary.save_loaded_asset(copy)
unreal.EditorAssetLibrary.save_loaded_asset(recipe)
for path in ("/Game/MyRecipes/MyHeavyHit", "/Game/MyRecipes/MyShake"):
    r = unreal.load_asset(path)
    steps = [t.get_editor_property("step").get_class().get_name() for t in r.get_editor_property("tracks")]
    unreal.log_warning("UPGRADE saved in Lite {0}: {1}".format(path, steps))
'@
Write-Utf8 "$dir\load.py" @'
import unreal
for path in ("/Game/MyRecipes/MyHeavyHit", "/Game/MyRecipes/MyShake"):
    r = unreal.load_asset(path)
    if r is None:
        unreal.log_warning("UPGRADE loaded in Pro {0}: FAILED".format(path))
        continue
    tracks = r.get_editor_property("tracks")
    steps = [t.get_editor_property("step").get_class().get_name() if t.get_editor_property("step") else "None" for t in tracks]
    unreal.log_warning("UPGRADE loaded in Pro {0}: {1}".format(path, steps))
shake = unreal.load_asset("/Game/MyRecipes/MyShake")
if shake:
    t = shake.get_editor_property("tracks")[0]
    unreal.log_warning("UPGRADE MyShake track: start {0:.2f}, duration {1:.2f}, frequency {2:.2f}".format(
        t.get_editor_property("start_time"), t.get_editor_property("duration"), t.get_editor_property("step").get_editor_property("frequency")))
unreal.log_warning("UPGRADE Pro field visible after the move: parameters {0}".format(len(shake.get_editor_property("parameters")) if shake else "n/a"))
'@

$exe = "$engine\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
& $exe "$dir\UpgradeCheck.uproject" -run=pythonscript -script="$dir\make.py" -unattended -nosplash -nullrhi "-abslog=$logs\upgrade_lite_$tag.log" | Out-Null
Select-String -Path "$logs\upgrade_lite_$tag.log" -Pattern "UPGRADE |Error" | ForEach-Object { $_.Line -replace '^.*?(UPGRADE|Error)', '$1' } | Select-Object -Unique

# The move: Lite out, Pro in, same project.
Remove-Item -Recurse -Force "$dir\Plugins\FeelKit"
robocopy "B:\NewUE5Project\Build\FKPkg_Pro_$tag" "$dir\Plugins\FeelKit" /E /NFL /NDL /NJH /NJS /NP | Out-Null
& $exe "$dir\UpgradeCheck.uproject" -run=pythonscript -script="$dir\load.py" -unattended -nosplash -nullrhi "-abslog=$logs\upgrade_pro_$tag.log" | Out-Null
Select-String -Path "$logs\upgrade_pro_$tag.log" -Pattern "UPGRADE |LogLinker: Warning|Error:" | ForEach-Object { $_.Line -replace '^.*?(UPGRADE|LogLinker|Error)', '$1' } | Select-Object -Unique
Remove-Item -Recurse -Force $dir

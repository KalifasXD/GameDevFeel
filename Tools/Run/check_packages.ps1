param(
	[string[]]$Editions = @("Lite", "Pro"),
	[string[]]$Versions = @("5.6", "5.7", "5.8"),
	[switch]$Keep
)
# Loads each package made by package_all.ps1 (Build\FKPkg_<Edition>_<tag>, with the binaries BuildPlugin built) in a new
# Blueprint-only project on its own engine and runs Tools/Unreal/check_package.py: plugin on, the classes and Blueprint
# nodes of that edition and no others, every /FeelKit asset loads, every recipe track has its step, the comfort menu
# loads, Pro-only fields hidden in Lite, sample assets nothing uses. The projects in Build\PkgCheck are deleted afterwards
# unless -Keep. Summary: Build\Logs\check_packages.log
$engines = @{ "5.6" = "B:\UE_5.6"; "5.7" = "D:\EpicGames\UE_5.7"; "5.8" = "D:\EpicGames\UE_5.8" }
$logs = "B:\NewUE5Project\Build\Logs"
$summary = "$logs\check_packages.log"
$root = "B:\NewUE5Project\Build\PkgCheck"

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

"check_packages started $(Get-Date -Format s)" | Set-Content $summary
foreach ($edition in $Editions) {
	foreach ($version in $Versions) {
		$tag = $version -replace '\.', ''
		$engine = $engines[$version]
		$package = "B:\NewUE5Project\Build\FKPkg_${edition}_$tag"
		$dir = "$root\${edition}_$tag"
		if (Test-Path $dir) { Remove-Item -Recurse -Force $dir }
		robocopy $package "$dir\Plugins\FeelKit" /E /NFL /NDL /NJH /NJS /NP | Out-Null
		$association = if ($version -eq "5.6") { (Get-Content "B:\NewUE5Project\GameFeelDev\GameFeelDev.uproject" -Raw | ConvertFrom-Json).EngineAssociation } else { Get-EngineAssociation $engine $version }
		Write-Utf8 "$dir\PkgCheck.uproject" @"
{
	"FileVersion": 3,
	"EngineAssociation": "$association",
	"Plugins": [
		{ "Name": "FeelKit", "Enabled": true },
		{ "Name": "PythonScriptPlugin", "Enabled": true }
	]
}
"@
		$py = "$dir\check_package.py"
		Write-Utf8 $py ((Get-Content "B:\NewUE5Project\Tools\Unreal\check_package.py" -Raw).Replace("@EDITION@", $edition))
		$log = "$logs\check_package_${edition}_$tag.log"
		& "$engine\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$dir\PkgCheck.uproject" -run=pythonscript -script="$py" -unattended -nosplash -nullrhi "-abslog=$log" | Out-Null
		"$edition ${version}:" | Add-Content $summary
		Select-String -Path $log -Pattern "CHECKPKG " | ForEach-Object { "    " + ($_.Line -replace '^.*CHECKPKG ', '') } | Select-Object -Unique | Add-Content $summary
		$loadWarnings = @(Select-String -Path $log -Pattern "LogLinker: Warning|Failed to load|LogUObjectGlobals: Warning|Error: " | Where-Object { $_.Line -notmatch "CHECKPKG" -and $_.Line -notmatch "LogWindows: Failed to load '\w+\.dll'" })
		"    load warnings and errors in the log: $($loadWarnings.Count)" | Add-Content $summary
		$loadWarnings | Select-Object -First 5 | ForEach-Object { "      " + $_.Line.Trim() } | Add-Content $summary
		if (-not $Keep) { Remove-Item -Recurse -Force $dir }
	}
}
if (-not $Keep -and (Test-Path $root) -and -not (Get-ChildItem $root)) { Remove-Item $root }
"check_packages finished $(Get-Date -Format s)" | Add-Content $summary
Get-Content $summary

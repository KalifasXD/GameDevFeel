param(
	[string]$Engine = "B:\UE_5.6",
	[string]$Version = "5.6",
	[string]$Out = "",
	[ValidateSet("Pro", "Lite")]
	[string]$Edition = "Pro",
	[string]$FabUrl = "",
	[switch]$KeepTests
)
# Packages FeelKit the way it ships (D-075: no tests) for one engine version (Fab wants one upload per version, 4.2.2.d).
# Copies the plugin to a short staging path without Binaries and Intermediate, removes every Tests folder from the copy
# (FeelKit and the GAS add-on in Extras) unless -KeepTests, turns the copy into the source of one edition
# (Tools/Run/make_edition.py: Pro keeps everything, Lite leaves out the Pro-only code and content), sets "EngineVersion"
# in the .uplugin files to <Version>.0, then runs a strict BuildPlugin with that engine. The development copy keeps its
# tests and stays on 5.6.
# Examples: package_plugin.ps1   /   package_plugin.ps1 -Engine D:\EpicGames\UE_5.8 -Version 5.8 -Edition Lite
$tag = $Version -replace '\.', ''
if ($Out -eq "") { $Out = "B:\NewUE5Project\Build\FKPkg_${Edition}_$tag" }
$src = "B:\NewUE5Project\GameFeelDev\Plugins\FeelKit"
$stage = "B:\NewUE5Project\Build\Stage\${Edition}_$tag\FeelKit"
$log = "B:\NewUE5Project\Build\Logs\package_plugin_${Edition}_$tag.log"

if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
robocopy $src $stage /E /XD Binaries Intermediate /NFL /NDL /NJH /NJS /NP | Out-Null
if (-not $KeepTests) {
	$tests = Get-ChildItem $stage -Recurse -Directory -Filter Tests | Where-Object { $_.FullName -match "\\Source\\" }
	foreach ($dir in $tests) { Remove-Item -Recurse -Force $dir.FullName }
	"PACKAGE removed test folders: $($tests.Count)"
}
python "B:\NewUE5Project\Tools\Run\make_edition.py" --edition $Edition --stage $stage
if ($LASTEXITCODE -ne 0) { "PACKAGE edition step failed"; exit 1 }
foreach ($descriptor in @("$stage\FeelKit.uplugin", "$stage\Extras\FeelKitGAS\FeelKitGAS.uplugin")) {
	if (-not (Test-Path $descriptor)) { continue }
	$text = [System.IO.File]::ReadAllText($descriptor)
	$text = $text -replace '"EngineVersion":\s*"[^"]*"', ('"EngineVersion": "' + $Version + '.0"')
	# FabURL (Fab 4.3.6.c): the launcher link of the listing, known once the listing exists.
	if ($FabUrl -ne "" -and $descriptor -like "*\FeelKit.uplugin") {
		$text = $text -replace '\s*"FabURL":\s*"[^"]*",', ''
		$text = $text -replace '("MarketplaceURL":\s*"[^"]*",)', ('$1' + "`n`t" + '"FabURL": "' + $FabUrl + '",')
	}
	[System.IO.File]::WriteAllText($descriptor, $text, (New-Object System.Text.UTF8Encoding($false)))
}
"PACKAGE $Edition, engine $Engine, EngineVersion $Version.0"

if (Test-Path $Out) { Remove-Item -Recurse -Force $Out }
& "$Engine/Engine/Build/BatchFiles/RunUAT.bat" BuildPlugin -Plugin="$stage\FeelKit.uplugin" -Package="$Out" -TargetPlatforms=Win64 -StrictIncludes -Unattended *> $log
"PACKAGE BuildPlugin exit code: $LASTEXITCODE"
Select-String -Path $log -Pattern "BUILD SUCCESSFUL|BUILD FAILED|warning C|error C|: warning|: error" | Select-Object -First 25 | ForEach-Object { "PACKAGE " + $_.Line.Trim() }

$left = @(Get-ChildItem $Out -Recurse -File -ErrorAction SilentlyContinue | Where-Object { $_.FullName -match "\\Tests\\" })
"PACKAGE test files in the package: $($left.Count)"
$gas = Test-Path "$Out\Extras\FeelKitGAS\FeelKitGAS.uplugin"
"PACKAGE GAS add-on in the package: $gas"
if (Test-Path "$Out\FeelKit.uplugin") {
	$uplugin = Get-Content "$Out\FeelKit.uplugin" -Raw
	"PACKAGE FeelKit.uplugin mentions GameplayAbilities: $($uplugin -match 'GameplayAbilities')"
	$size = (Get-ChildItem $Out -Recurse -File | Where-Object { $_.FullName -notmatch "\\(Binaries|Intermediate)\\" } | Measure-Object Length -Sum).Sum
	"PACKAGE size without Binaries and Intermediate: {0:N1} MB" -f ($size / 1MB)
}

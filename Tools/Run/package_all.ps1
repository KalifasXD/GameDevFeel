param(
	[string[]]$Editions = @("Lite", "Pro"),
	[string]$FabUrlLite = "",
	[string]$FabUrlPro = ""
)
# Packages both editions for every supported engine (package_plugin.ps1 six times) and prints one summary line each.
# Output folders: B:\NewUE5Project\Build\FKPkg_<Edition>_<56|57|58>. Summary: Build\Logs\package_all.log
# After the Fab listings exist: package_all.ps1 -FabUrlLite <link> -FabUrlPro <link>, then make_uploads.ps1.
$engines = @(
	@{ Path = "B:\UE_5.6"; Version = "5.6" },
	@{ Path = "D:\EpicGames\UE_5.7"; Version = "5.7" },
	@{ Path = "D:\EpicGames\UE_5.8"; Version = "5.8" }
)
$summary = "B:\NewUE5Project\Build\Logs\package_all.log"
"package_all started $(Get-Date -Format s)" | Set-Content $summary
foreach ($edition in $Editions) {
	$fabUrl = if ($edition -eq "Lite") { $FabUrlLite } else { $FabUrlPro }
	foreach ($engine in $engines) {
		$lines = & "B:\NewUE5Project\Tools\Run\package_plugin.ps1" -Engine $engine.Path -Version $engine.Version -Edition $edition -FabUrl $fabUrl
		$ok = ($lines -match "BUILD SUCCESSFUL").Count -gt 0
		$tag = $engine.Version -replace '\.', ''
		$warnings = @(Select-String -Path "B:\NewUE5Project\Build\Logs\package_plugin_${edition}_$tag.log" -Pattern "warning C|: warning" | Where-Object { $_.Line -notmatch "WarningsAsErrors" }).Count
		"$edition $($engine.Version): $(if ($ok) { 'BUILD SUCCESSFUL' } else { 'BUILD FAILED' }), warnings $warnings" | Add-Content $summary
		$lines | Where-Object { $_ -match "error|size" } | ForEach-Object { "    $_" } | Add-Content $summary
	}
}
"package_all finished $(Get-Date -Format s)" | Add-Content $summary
Get-Content $summary

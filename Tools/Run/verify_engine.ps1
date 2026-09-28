param(
	[Parameter(Mandatory = $true)][string]$Engine,
	[Parameter(Mandatory = $true)][string]$Version,
	[switch]$SkipCopy,
	[switch]$SkipBuild
)
# Checks FeelKit on another engine version without touching the 5.6 projects (their content stays saved in 5.6).
# Copies GameFeelDev and FeelDemoFP (with the development plugin and its tests, no Binaries, Intermediate, Saved or
# DerivedDataCache) to Build\UE<tag>\, points the copies at the engine, builds both editor targets, then runs the
# automation tests, the editor window tests and every demo playthrough. Logs: Build\Logs\*_ue<tag>.log.
# Example: verify_engine.ps1 -Engine D:\EpicGames\UE_5.8 -Version 5.8
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
$tag = $Version -replace '\.', ''
$root = "B:\NewUE5Project\Build\UE$tag"
$logs = "B:\NewUE5Project\Build\Logs"
$editor = "$Engine\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$projects = @("GameFeelDev", "FeelDemoFP")

if (-not $SkipCopy) {
	foreach ($name in $projects) {
		$dst = "$root\$name"
		if (Test-Path $dst) { Remove-Item -Recurse -Force $dst }
		robocopy "B:\NewUE5Project\$name" $dst /E /XD Binaries Intermediate Saved DerivedDataCache .vs /XF *.sln /NFL /NDL /NJH /NJS /NP | Out-Null
		$uproject = "$dst\$name.uproject"
		$text = [System.IO.File]::ReadAllText($uproject)
		$text = $text -replace '"EngineAssociation":\s*"[^"]*"', ('"EngineAssociation": "' + (Get-EngineAssociation $Engine $Version) + '"')
		[System.IO.File]::WriteAllText($uproject, $text, (New-Object System.Text.UTF8Encoding($false)))
		# What the editor's project upgrade does: the copies use the engine's current build settings and include order.
		foreach ($target in Get-ChildItem "$dst\Source" -Filter *.Target.cs) {
			$code = [System.IO.File]::ReadAllText($target.FullName)
			$code = $code -replace 'BuildSettingsVersion\.V\d+', 'BuildSettingsVersion.Latest' -replace 'EngineIncludeOrderVersion\.Unreal5_\d+', 'EngineIncludeOrderVersion.Latest'
			[System.IO.File]::WriteAllText($target.FullName, $code, (New-Object System.Text.UTF8Encoding($false)))
		}
		# Each Fab upload carries its engine's version in the .uplugin (4.2.2.d); an unattended editor skips a plugin made for another version.
		foreach ($descriptor in Get-ChildItem "$dst\Plugins" -Recurse -Filter *.uplugin) {
			$json = [System.IO.File]::ReadAllText($descriptor.FullName) -replace '"EngineVersion":\s*"[^"]*"', ('"EngineVersion": "' + $Version + '.0"')
			[System.IO.File]::WriteAllText($descriptor.FullName, $json, (New-Object System.Text.UTF8Encoding($false)))
		}
		# Epic's 5.6 Shooter template calls GetInstanceDataPtr on a temporary strong context, which 5.7 and later forbid;
		# their own later template keeps the context in a local first. Same change here, in the copy only.
		$shooter = "$dst\Source\$name\Variant_Shooter\AI\ShooterStateTreeUtility.cpp"
		if (Test-Path $shooter) {
			$code = [System.IO.File]::ReadAllText($shooter)
			$code = $code.Replace('FInstanceDataType* LambdaInstanceData = WeakContext.MakeStrongExecutionContext().GetInstanceDataPtr<FInstanceDataType>();', 'const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();' + "`r`n" + '				FInstanceDataType* LambdaInstanceData = StrongContext.GetInstanceDataPtr<FInstanceDataType>();')
			[System.IO.File]::WriteAllText($shooter, $code, (New-Object System.Text.UTF8Encoding($false)))
		}
		"VERIFY copied $name to $dst"
	}
}

if (-not $SkipBuild) {
	foreach ($name in $projects) {
		$out = & "$Engine\Engine\Build\BatchFiles\Build.bat" "${name}Editor" Win64 Development -Project="$root\$name\$name.uproject" -WaitMutex 2>&1
		$out | Out-File "$logs\build_${name}_ue$tag.log"
		$warnings = @($out | Select-String ": warning|: error")
		"VERIFY build $name on ${Version}: " + ($out | Select-String "Result:").Line + ", warnings and errors: $($warnings.Count)"
		$warnings | Select-Object -First 20 | ForEach-Object { "VERIFY   " + $_.Line.Trim() }
	}
}

function Invoke-Run([string]$Project, [string]$Map, [string]$Tests, [string]$Label, [switch]$NullRhi) {
	$log = "$logs\${Label}_ue$tag.log"
	$cli = @("$root\$Project\$Project.uproject")
	if ($Map -ne "") { $cli += $Map }
	$cli += @("-ExecCmds=Automation RunTests $Tests", "-TestExit=Automation Test Queue Empty", "-unattended", "-nosplash", "-nosound", "-abslog=$log")
	if ($NullRhi) { $cli += "-nullrhi" }
	& $editor @cli | Out-Null
	$ok = (Select-String -Path $log -Pattern "Result=\{Success\}").Count
	$fail = (Select-String -Path $log -Pattern "Result=\{Fail").Count
	$pie = (Select-String -Path $log -Pattern "PIE warnings and errors: (\d+)" | ForEach-Object { $_.Matches[0].Groups[1].Value }) -join ","
	"VERIFY $Label on ${Version}: passed $ok, failed $fail" + $(if ($pie -ne "") { ", PIE warnings and errors: $pie" } else { "" })
	Select-String -Path $log -Pattern "Result=\{Fail|LogAutomationController: Error|Ensure condition|Fatal error|Assertion failed" | Select-Object -First 10 | ForEach-Object { "VERIFY   " + $_.Line.Trim() }
}

Invoke-Run "GameFeelDev" "" "FeelKit" "tests" -NullRhi
Invoke-Run "GameFeelDev" "" "FeelKit.Editor.DetailsKeepFocusAfterEdit+FeelKit.Editor.RecipeBrowserUI+FeelKit.Editor.PreviewMute+FeelKit.Editor.LibraryOpensFromContentBrowser+FeelKit.Editor.FeelSwitchInPlay" "uitests"
Invoke-Run "GameFeelDev" "/Game/Variant_Combat/Lvl_Combat" "DiagFeel.ARPGPlaythrough" "arpg"
Invoke-Run "GameFeelDev" "/Game/Variant_Combat/Lvl_Combat" "DiagFeel.ARPGGuardPlaythrough" "arpg_guard"
Invoke-Run "GameFeelDev" "/Game/Variant_Platforming/Lvl_Platforming" "DiagFeel.PlatformerPlaythrough" "platformer"
Invoke-Run "FeelDemoFP" "/Game/Variant_Shooter/Lvl_Shooter" "DiagFeel.ShooterPlaythrough" "shooter"
Invoke-Run "FeelDemoFP" "/Game/Variant_Horror/Lvl_Horror" "DiagFeel.HorrorPlaythrough" "horror"

param([string]$Name = "switch", [string]$Map = "/Game/Variant_Combat/Lvl_Combat")
$sp = "B:\NewUE5Project\Build\Logs"
$log = "$sp\$Name.log"
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" $Map -ExecCmds="Automation RunTests FeelKit.Editor.FeelSwitchInPlay" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
Select-String -Path $log -Pattern "Result=\{|Feel Switch picture|LogAutomationController: Error|Expected|LogFeel\w*: (Warning|Error)" | ForEach-Object { $_.Line -replace '^.*?(LogAutomationController: |LogFeel)', '$1' }

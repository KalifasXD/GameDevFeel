param([string]$Name = "tests", [string]$Filter = "FeelKit")
$sp = "B:\NewUE5Project\Build\Logs"
$log = "$sp\$Name.log"
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" -ExecCmds="Automation RunTests $Filter" -TestExit="Automation Test Queue Empty" -unattended -nullrhi -nosplash -nosound -abslog="$log" | Out-Null
"Success: " + (Select-String -Path $log -Pattern "Result=\{Success\}").Count
"Fail: " + (Select-String -Path $log -Pattern "Result=\{Fail").Count
Select-String -Path $log -Pattern "Result=\{Fail|LogAutomationController: Error|Ensure condition|LogPython: Warning|LogFeel\w*: (Warning|Error)|Fatal error|Assertion failed" | ForEach-Object { $_.Line } | Select-Object -First 40

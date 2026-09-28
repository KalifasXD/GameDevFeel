param([string]$Name = "arpg")
$sp = "B:\NewUE5Project\Build\Logs"
$log = "$sp\$Name.log"
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" /Game/Variant_Platforming/Lvl_Platforming -ExecCmds="Automation RunTests DiagFeel.PlatformerPlaythrough" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
Select-String -Path $log -Pattern "PLATFORMER |Result=\{|fully simulated" | ForEach-Object { $_.Line -replace '^.*?(PLATFORMER|Test Completed|Attempting)', '$1' }

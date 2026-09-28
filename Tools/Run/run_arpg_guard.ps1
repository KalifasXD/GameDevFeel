param([string]$Name = "arpg_guard")
# Plays the Action/RPG guards: blocks, guard breaks, parries, and hits with and without a guard (DiagFeel.ARPGGuardPlaythrough).
$sp = "B:\NewUE5Project\Build\Logs"
$log = "$sp\$Name.log"
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" /Game/Variant_Combat/Lvl_Combat -ExecCmds="Automation RunTests DiagFeel.ARPGGuardPlaythrough" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
Select-String -Path $log -Pattern "GUARD |Result=\{" | ForEach-Object { $_.Line -replace '^.*?(GUARD|Test Completed)', '$1' }

param([string]$Name = "shoot")
$sp = "B:\NewUE5Project\Build\Logs"
$log = "$sp\$Name.log"
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/FeelDemoFP/FeelDemoFP.uproject" /Game/Variant_Horror/Lvl_Horror -ExecCmds="Automation RunTests DiagFeel.HorrorPlaythrough" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
Select-String -Path $log -Pattern "HORROR |Result=\{" | ForEach-Object { $_.Line -replace '^.*?(HORROR|Test Completed)', '$1' }

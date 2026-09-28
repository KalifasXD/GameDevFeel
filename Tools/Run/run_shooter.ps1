param([string]$Name = "shoot")
$sp = "B:\NewUE5Project\Build\Logs"
$log = "$sp\$Name.log"
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/FeelDemoFP/FeelDemoFP.uproject" /Game/Variant_Shooter/Lvl_Shooter -ExecCmds="Automation RunTests DiagFeel.ShooterPlaythrough" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
Select-String -Path $log -Pattern "SHOOTER |Result=\{" | ForEach-Object { $_.Line -replace '^.*?(SHOOTER|Test Completed)', '$1' }

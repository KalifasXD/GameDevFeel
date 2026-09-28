param([string]$Name = "platformer_landing")
# Drops the Platforming character from several heights and reports what FeelKit adds to the camera (DiagFeel.PlatformerLanding).
$sp = "B:\NewUE5Project\Build\Logs"
$log = "$sp\$Name.log"
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" /Game/Variant_Platforming/Lvl_Platforming -ExecCmds="Automation RunTests DiagFeel.PlatformerLanding" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
Select-String -Path $log -Pattern "LAND |Result=\{" | ForEach-Object { $_.Line -replace '^.*?(LAND|Test Completed)', '$1' }

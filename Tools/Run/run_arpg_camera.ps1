param([string]$Name = "arpg_camera", [string]$Setups = "", [switch]$FeelOff, [switch]$NoPictures)
# Measures Action/RPG camera setups (DiagFeel.ARPGCameraStudy). -Setups "arm,side,height,fov;..." measures only those and saves pictures.
$sp = "B:\NewUE5Project\Build\Logs"
$log = "$sp\$Name.log"
$extra = @()
if ($Setups -ne "") { $extra += "-FeelCameraSetups=`"$Setups`"" }
if ($FeelOff) { $extra += "-FeelCameraFeelOff" }
if ($NoPictures) { $extra += "-FeelCameraNoPictures" }
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" /Game/Variant_Combat/Lvl_Combat -ExecCmds="Automation RunTests DiagFeel.ARPGCameraStudy" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" @extra | Out-Null
Select-String -Path $log -Pattern "CAMERA |Result=\{" | ForEach-Object { $_.Line -replace '^.*?(CAMERA|Test Completed)', '$1' }

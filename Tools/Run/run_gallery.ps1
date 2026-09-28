# Store gallery pictures of the four demo levels: runs the playthrough diagnostics with FEELKIT_GALLERY set, so each one
# also saves HighResShot pictures (3840 x 2160, scene without interface) at its moments to <project>/Saved/FeelKit/Gallery.
# The Action/RPG run plays its combo twice, FeelKit on and then off. Needs a rendering session; close the editor first.
$env:FEELKIT_GALLERY = "1"
$logs = "B:\NewUE5Project\Build\Logs"
$runs = @(
	@{ Project = "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject"; Map = "/Game/Variant_Combat/Lvl_Combat"; Test = "ARPGPlaythrough" },
	@{ Project = "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject"; Map = "/Game/Variant_Platforming/Lvl_Platforming"; Test = "PlatformerPlaythrough" },
	@{ Project = "B:/NewUE5Project/FeelDemoFP/FeelDemoFP.uproject"; Map = "/Game/Variant_Shooter/Lvl_Shooter"; Test = "ShooterPlaythrough" },
	@{ Project = "B:/NewUE5Project/FeelDemoFP/FeelDemoFP.uproject"; Map = "/Game/Variant_Horror/Lvl_Horror"; Test = "HorrorPlaythrough" }
)
foreach ($run in $runs) {
	$log = "$logs\gallery_$($run.Test).log"
	& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" $run.Project $run.Map -ExecCmds="Automation RunTests DiagFeel.$($run.Test)" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
	"$($run.Test): " + ((Select-String -Path $log -Pattern "Test Completed. Result=\{\w+\}" | Select-Object -Last 1).Line -replace '^.*(Result=\{\w+\}).*$', '$1')
	Select-String -Path $log -Pattern "GALLERY " | ForEach-Object { "  " + ($_.Line -replace '^.*GALLERY ', '') }
}
Remove-Item Env:FEELKIT_GALLERY

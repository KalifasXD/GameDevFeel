param([string]$Name = "uitests")
$sp = "B:\NewUE5Project\Build\Logs"
$log = "$sp\$Name.log"
& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" -ExecCmds="Automation RunTests FeelKit.Editor.DetailsKeepFocusAfterEdit+FeelKit.Editor.RecipeBrowserUI+FeelKit.Editor.PreviewMute+FeelKit.Editor.LibraryOpensFromContentBrowser+FeelKit.Editor.FeelSwitchInPlay+FeelKit.Editor.ComfortMenuInPlay+FeelKit.Editor.ShowDebugInPlay" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
Select-String -Path $log -Pattern "Result=\{" | ForEach-Object { $_.Line -replace '^.*?Test Completed', 'Test Completed' }

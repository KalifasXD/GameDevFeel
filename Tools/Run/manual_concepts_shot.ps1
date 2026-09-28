# The Feel Map picture of the manual's chapter 2 (DiagFeel.ManualShotsConcepts). Its example rows use the gameplay tags
# Hit.Heavy and Hit.Critical, which are not FeelKit's: they are added to GameFeelDev's tag list for this run only, and the
# list is put back byte for byte afterwards. Needs a rendering session; close the editor first.
$ini = "B:\NewUE5Project\GameFeelDev\Config\DefaultGameplayTags.ini"
$backup = "$ini.manualshot"
$log = "B:\NewUE5Project\Build\Logs\manualshots_concepts.log"
$before = (Get-FileHash $ini).Hash
Copy-Item $ini $backup -Force
try {
	Add-Content $ini "`r`n+GameplayTagList=(Tag=`"Hit.Heavy`",DevComment=`"`")`r`n+GameplayTagList=(Tag=`"Hit.Critical`",DevComment=`"`")"
	& "B:/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "B:/NewUE5Project/GameFeelDev/GameFeelDev.uproject" -ExecCmds="Automation RunTests DiagFeel.ManualShotsConcepts" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nosound -abslog="$log" | Out-Null
}
finally {
	Copy-Item $backup $ini -Force
	Remove-Item $backup
}
Select-String -Path $log -Pattern "Test Completed|MANUALSHOT|LogAutomationController: Error" | ForEach-Object { $_.Line -replace '^.*?(Test Completed|MANUALSHOT|LogAutomationController: )', '$1' }
"Tag list restored: " + ((Get-FileHash $ini).Hash -eq $before)

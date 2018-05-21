Set WshShell = WScript.CreateObject("WScript.Shell")
'WScript.echo "running Minilog..."

Function SendKeysTo (process, keys, wait)									'sends key to app
    WshShell.AppActivate(process.ProcessID)
    WScript.Sleep wait
    WshShell.SendKeys keys
End Function


Set minilogn= WshShell.Exec(".\Minilog\Minilog.exe ZMnichtoptimiert.tbl")	'open minilog with parameter and save id
Set minilogo= WshShell.Exec(".\Minilog\Minilog.exe ZMoptimiert.tbl")		'open minilog with parameter and save id

WScript.Sleep 3000															'Wait long for Minilog to configure first time
SendKeysTo minilogn, "{F10}", 100											'enter key only mode
SendKeysTo minilogn, "{r}", 100												'open Run menu
SendKeysTo minilogn, "{m}", 100												'start Minimize
WScript.Sleep 1000															'wait for minimize to finish
SendKeysTo minilogo, "{F10}", 100											'enter key only mode
SendKeysTo minilogo, "{r}", 100												'open Run menu
SendKeysTo minilogo, "{m}", 100												'start Minimize
WScript.Sleep 1000															'wait for minimize to finish
SendKeysTo minilogo, "%{F4}", 100											'close output window
SendKeysTo minilogo, "%{F4}", 100											'close Minilog window
SendKeysTo minilogn, "%{F4}", 100											'close output window
SendKeysTo minilogn, "%{F4}", 100											'close Minilog window
WScript.echo "Finished!"
Set WshShell = WScript.CreateObject("WScript.Shell")
'WScript.echo "running Minilog..."

Function SendKeysTo (process, keys, wait)
    WshShell.AppActivate(process.ProcessID)
    WScript.Sleep wait
    WshShell.SendKeys keys
End Function


Set minilogn= WshShell.Exec(".\Minilog\Minilog.exe ZMnichtoptimiert.tbl")
Set minilogo= WshShell.Exec(".\Minilog\Minilog.exe ZMoptimiert.tbl")

WScript.Sleep 3000					'Wait long for Minilog to configure first time
SendKeysTo minilogn, "{F9}", 300
WScript.Sleep 500
SendKeysTo minilogo, "{F9}", 300
WScript.Sleep 500
SendKeysTo minilogo, "%{F4}", 300
SendKeysTo minilogo, "%{F4}", 300
SendKeysTo minilogn, "%{F4}", 300
SendKeysTo minilogn, "%{F4}", 300
WScript.echo "Finished!"
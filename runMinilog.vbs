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
SendKeysTo minilogn, "{F9}", 100
WScript.Sleep 1000
SendKeysTo minilogo, "{F9}", 100
WScript.Sleep 1000
SendKeysTo minilogo, "%{F4}", 100
SendKeysTo minilogo, "%{F4}", 100
SendKeysTo minilogn, "%{F4}", 100
SendKeysTo minilogn, "%{F4}", 100
WScript.echo "Finished!"
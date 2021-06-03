Sub Main
	xsh.Session.LogFilePath = "F:\Workspace\ONT\Logs\wlan_cafe01.log"
	xsh.Session.StartLog

	Dim count
	Do While true
		count = count + 1
		
		'*** Send ***
		xsh.Screen.Send("echo 'commit lan thu: " + CStr(count) + "'")
		xsh.Screen.Send(VbCr)
		xsh.Session.Sleep(1000)
		
		xsh.Screen.Send("tcapi commit WLan11ac")
		xsh.Screen.Send(VbCr)
		xsh.Session.Sleep(1000 * 60)
		
		xsh.Screen.Send("iwpriv rai0 show wdev_bss_info=a4:f4:c2:ca:fe:01")
		xsh.Screen.Send(VbCr)
		xsh.Session.Sleep(1000)
		
		'*** Get, Clear ***
		Dim ScreenRow, ReadLine, Items
		
		ScreenRow = xsh.Screen.CurrentRow - 1
		ReadLine = xsh.Screen.Get(ScreenRow, 1, ScreenRow, 40)
		Items = Split(ReadLine, ":", -1)
		
		If (Trim(Items(1)) <> "3") Then
			Dim MsgProp
			MsgProp = "Loop count:" + CStr(count) + "\n"
			MsgProp = MsgProp + "State:" + Items(1) + "\n"
			xsh.Dialog.MsgBox(MsgProp)
			Exit Do
		End If
	Loop
	
	xsh.Session.StopLog
End Sub
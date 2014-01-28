#include <malloc.h>
#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "powsavt.h"
#include "misc.h"

INT_PTR CALLBACK powersave_timer_dlg(HWND dlg,UINT msg,WPARAM w,LPARAM l)
{
	int num;
	BOOL ok;
	POWSAVT_DLG_PARAM *ptr;

	ptr = (POWSAVT_DLG_PARAM*)GetWindowLongPtr(dlg,DWLP_USER);

	switch(msg)
	{
	case WM_CLOSE: /* 0x0010 */
		free(ptr);
		EndDialog(dlg,IDCANCEL);
		return TRUE;

#if 0
	case WM_HELP: /* 0x0053 */
		if(((const HELPINFO*)l)->cbSize >= sizeof(HELPINFO))
		{
			if(((const HELPINFO*)l)->iContextType == HELPINFO_WINDOW)
			{
				//memset(&ti,0,sizeof(TOOLINFO));
				//ti.cbSize = sizeof(TOOLINFO);
				//ti.uFlags = TTF_IDISHWND;

				//ti.uId = (UINT_PTR)GetDlgItem(dlg,((const HELPINFO*)l)->iCtrlId);
				SendMessage(ptr->ToolTip,TTM_POPUP,0,0);
			}
		}
		break;
#endif

	case WM_INITDIALOG: /* 0x0110 */
		/*ptr = HeapAlloc(((const POWSAVT_DLG_PARAM*)l)->Heap,HEAP_ZERO_MEMORY,sizeof(POWSAVT_DLG_PARAM));*/
		ptr = calloc(1,sizeof(POWSAVT_DLG_PARAM));
		if(!ptr)
		{
			return FALSE;
		}

		*ptr = *(const POWSAVT_DLG_PARAM*)l;
		SetWindowLongPtr(dlg,DWLP_USER,(LONG_PTR)ptr);

		/* 通常 */
		SendDlgItemMessage(dlg,IDC_POWERSAVE_TIMER_UD,UDM_SETRANGE32,1,32767);
		SystemParametersInfo(SPI_GETPOWEROFFTIMEOUT,0,&num,0);
		SetDlgItemInt(dlg,IDC_POWERSAVE_TIMER,num,FALSE);
		/* 省電力 */
		SendDlgItemMessage(dlg,IDC_LOWPOWERSAVE_TIMER_UD,UDM_SETRANGE32,1,32767);
		SystemParametersInfo(SPI_GETLOWPOWERTIMEOUT,0,&num,0);
		SetDlgItemInt(dlg,IDC_LOWPOWERSAVE_TIMER,num,FALSE);

		CenteringWindowToParent(dlg,ptr->Parent);

		return TRUE;

	case WM_COMMAND: /* 0x0111 */
		switch(LOWORD(w))
		{
		case IDOK:
			num = GetDlgItemInt(dlg,IDC_POWERSAVE_TIMER,&ok,FALSE);
			if(ok)
				SystemParametersInfo(SPI_SETPOWEROFFTIMEOUT,num,NULL,0);
			num = GetDlgItemInt(dlg,IDC_LOWPOWERSAVE_TIMER,&ok,FALSE);
			if(ok)
				SystemParametersInfo(SPI_SETLOWPOWERTIMEOUT,num,NULL,0);
			EndDialog(dlg,IDOK);
			break;
		case IDCANCEL:
			EndDialog(dlg,IDCANCEL);
			break;
		}
		return TRUE;
	}
	return FALSE;
}

void ShowPowerSaveTimerDialog(HWND parent,HINSTANCE instance)
{
	POWSAVT_DLG_PARAM p;

	p.Instance = instance;
	p.Parent = parent;

	DialogBoxParam(instance,MAKEINTRESOURCE(IDD_POWERSAVE_TIMER),parent,powersave_timer_dlg,(LPARAM)&p);
}

#include <crtdefs.h>
#include <crtdbg.h>
#include <string.h>
#include <malloc.h>
#include <tchar.h>
#include <Windows.h>
#include <CommCtrl.h>
#include "misc.h"
#include "resource.h"
#include "SizeDialog.h"

SizeDialog::SizeDialog(HWND hwnd)
{
	Debug(TEXT("SizeDialog: Initialize"));
	p_ref = 1;

	p_dialog = hwnd;
	p_parent = (HWND)GetWindowLongPtr(p_dialog, GWLP_HWNDPARENT);
}

SizeDialog::~SizeDialog()
{
	Debug(TEXT("SizeDialog: Destroy"));
	_ASSERT(p_ref == 0);
	if(p_caption_text)
	{
		free(p_caption_text);
	}
}

intptr_t SizeDialog::AddRef()
{
	return InterlockedIncrement(&p_ref);
}

intptr_t SizeDialog::Release()
{
	InterlockedDecrement(&p_ref);
	if(p_ref == 0)
	{
		delete this;
	}
	return p_ref;
}

INT_PTR CALLBACK SizeDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SizeDialog * _this;
	INT_PTR r;

#if defined(ZEROFILL_LOCAL_VARS)
	_this = NULL;
	r = 0;
#endif

	switch(uMsg)
	{
	case WM_DESTROY:
		_this = (SizeDialog *)GetProp(hwndDlg, TEXT("DialogClassPtr"));
		_ASSERT(_this != NULL);
		_this->do_destroy();
		r = TRUE;
		break;

	case WM_NOTIFY:
		_this = (SizeDialog *)GetProp(hwndDlg, TEXT("DialogClassPtr"));
		_ASSERT(_this != NULL);
		_this->do_notify((NMHDR *)lParam);
		r = TRUE;
		break;

	case WM_INITDIALOG:
		// プロパティシートとして作成した場合、LPARAMはPROPSHEETPAGE構造体となる。
		_this = new SizeDialog(hwndDlg);
		_ASSERT(_this != NULL);
		_this->do_initdialog(((const PROPSHEETPAGE *)lParam)->lParam);
		r = TRUE;
		break;

	case WM_COMMAND:
		_this = (SizeDialog *)GetProp(hwndDlg, TEXT("DialogClassPtr"));
		if(_this)
		{
			_this->do_command(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			r = TRUE;
		}
		else
		{
			r = FALSE;
		}
		break;

	default:
		r = FALSE;
		break;
	}
	return r;
}

void SizeDialog::do_destroy()
{
	Release();
	RemoveProp(p_dialog, TEXT("DialogClassPtr"));
}

void SizeDialog::do_initdialog(LPARAM l)
{
	LRESULT item;
	int ival;
	ENUMMONITORPARAM mp;
	RECT r;
	bool client_metrics;

#if defined(ZEROFILL_LOCAL_VARS)
	item = 0;
	ival = 0;
	memset(&mp, 0, sizeof(ENUMMONITORPARAM));
	memset(&r, 0, sizeof(RECT));
	client_metrics = false;
#endif

	SetProp(p_dialog,TEXT("DialogClassPtr"), (HANDLE)this);
	p_do_apply = false;

	p_target = ((const SIZEDIALOGPARAM *)l)->target;
	p_caption = GetDlgItem(p_dialog, IDC_CAPTION);
	p_xpos = GetDlgItem(p_dialog, IDC_SPIN_XPOS);
	p_ypos = GetDlgItem(p_dialog, IDC_SPIN_YPOS);
	p_width = GetDlgItem(p_dialog, IDC_SPIN_WIDTH);
	p_height = GetDlgItem(p_dialog, IDC_SPIN_HEIGHT);
	p_monitor = GetDlgItem(p_dialog, IDC_DISPLAY);
	p_callback = ((const SIZEDIALOGPARAM *)l)->callback;
	p_callback_param = ((const SIZEDIALOGPARAM *)l)->callback_param;

	SendMessage(p_caption, EM_LIMITTEXT, 1023, 0);
	SendDlgItemMessage(p_dialog, IDC_XPOS,EM_LIMITTEXT, 15, 0);
	SendDlgItemMessage(p_dialog, IDC_YPOS,EM_LIMITTEXT, 15, 0);
	SendDlgItemMessage(p_dialog, IDC_WIDTH,EM_LIMITTEXT, 15, 0);
	SendDlgItemMessage(p_dialog, IDC_HEIGHT,EM_LIMITTEXT, 15, 0);

	mp.combobox = p_monitor;
	item = SendMessage(p_monitor, CB_ADDSTRING, 0, (LPARAM)TEXT("すべてのモニタ"));
	SendMessage(p_monitor, CB_SETITEMDATA, item, 0);

	mp.current_monitor = MonitorFromWindow(p_target, MONITOR_DEFAULTTONEAREST);
	EnumDisplayMonitors(NULL, NULL, enum_monitors, (LPARAM)&mp);

	/* 仮想画面の幅と左端の座標 */
	ival = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	SendMessage(p_xpos, UDM_SETRANGE32, (WPARAM)(intptr_t)-32768, 32767);
	SendMessage(p_width, UDM_SETRANGE32, 0, ival);

	/* 仮想画面の高さと上端の座標 */
	ival = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	SendMessage(p_ypos, UDM_SETRANGE32, (WPARAM)(intptr_t)-32768, 32767);
	SendMessage(p_height, UDM_SETRANGE32, 0, ival);

	client_metrics = ((0x8000 & GetAsyncKeyState(VK_SHIFT)) == 0x8000);
	if(client_metrics)
	{
		GetClientRectAsScreenPos(p_target, &r);
	}
	else
	{
		GetWindowRect(p_target, &r);
	}

	p_pos.x = r.left;
	p_pos.y = r.top;
	p_size.cx = r.right - r.left;
	p_size.cy = r.bottom - r.top;

	SendMessage(p_xpos, UDM_SETPOS32, 0, p_pos.x);
	SendMessage(p_ypos, UDM_SETPOS32, 0, p_pos.y);
	SendMessage(p_width, UDM_SETPOS32, 0, p_size.cx);
	SendMessage(p_height, UDM_SETPOS32, 0, p_size.cy);

	if(client_metrics)
	{
		SendDlgItemMessage(p_dialog, IDC_POS_CLIENT, BM_SETCHECK, BST_CHECKED, 0);
		SendDlgItemMessage(p_dialog, IDC_SIZE_CLIENT, BM_SETCHECK, BST_CHECKED, 0);
	}

	p_caption_text = (TCHAR *)calloc(1024, sizeof(TCHAR));
	_ASSERT(p_caption_text);
	if(p_caption_text)
	{
		GetWindowText(p_target, p_caption_text, 1024);
		SetWindowText(p_caption, p_caption_text);
	}
}

void SizeDialog::do_notify(NMHDR * notify_header)
{
	POINT pos;
	SIZE size;
	ULONG flags;
	TCHAR * new_caption;
	int n;

#if defined(ZEROFILL_LOCAL_VARS)
	memset(&pos, 0, sizeof(POINT));
	memset(&size, 0, sizeof(SIZE));
	//flags = 0;
	new_caption = NULL;
	n = 0;
#endif

	flags = 0;
	if(notify_header->code == PSN_APPLY)
	{
		if(p_do_apply)
		{
			pos.x = (int)SendMessage(p_xpos, UDM_GETPOS32, 0, 0);
			pos.y = (int)SendMessage(p_ypos, UDM_GETPOS32, 0, 0);
			size.cx = (int)SendMessage(p_width, UDM_GETPOS32, 0, 0);
			size.cy = (int)SendMessage(p_height, UDM_GETPOS32, 0, 0);

			if(IsDlgButtonChecked(p_dialog, IDC_POS_CLIENT) == BST_CHECKED)
			{
				flags |= SDAF_CLIENT_POS;
			}

			if(IsDlgButtonChecked(p_dialog, IDC_SIZE_CLIENT) == BST_CHECKED)
			{
				flags |= SDAF_CLIENT_SIZE;
			}

			n = GetWindowTextLength(p_caption);
			if(n > 0)
			{
				new_caption = (TCHAR *)calloc(++n, sizeof(TCHAR));;
				_ASSERT(new_caption);
				if(new_caption)
				{
					GetWindowText(p_caption, new_caption, n);
					if(_tcscmp(new_caption, p_caption_text))
					{
						flags |= SDAF_SETCAPTION;
					}
				}
			}
			else
				new_caption = nullptr;

			if(p_callback)
			{
				(*p_callback)(p_callback_param, flags, &pos, &size, new_caption);
			}

			p_pos = pos;
			p_size = size;

			if(new_caption)
			{
				_tcsncpy_s(p_caption_text, 1024, new_caption, _TRUNCATE);
				free(new_caption);
			}

			p_do_apply = false;
		}
	}
}

void SizeDialog::do_command(WORD id, WORD code, HWND window_from)
{
	LRESULT item;
	HMONITOR mon;
	RECT r;
	ULONG flags;
	TCHAR * buffer;
	TCHAR minibuffer[16];
	int val;
	int val_o;

#if defined(ZEROFILL_LOCAL_VARS)
	item = 0;
	HMONITOR = NULL;
	memset(&r, 0, sizeof(RECT));
	//flags = 0;
	buffer = NULL;
	memset(minibuffer, 0, sizeof(TCHAR) * 16);
	val = 0;
	val_o = 0;
#endif

	flags = 0;

	if(code == EN_CHANGE)
	{
		switch(id)
		{
		case IDC_CAPTION:
			buffer = (TCHAR *)calloc(1024, sizeof(TCHAR));
			if(buffer)
			{
				GetWindowText(window_from, buffer, 1024);
				p_do_apply = _tcscmp(buffer, p_caption_text) != 0;
				free(buffer);
			}
			break;
		case IDC_XPOS:
			val_o = p_pos.x;
			goto getval;
		case IDC_YPOS:
			val_o = p_pos.y;
			goto getval;
		case IDC_WIDTH:
			val_o = p_size.cx;
			goto getval;
		case IDC_HEIGHT:
			val_o = p_size.cy;
			goto getval;
getval:
			GetWindowText(window_from, minibuffer, 16);
			val = _ttoi(minibuffer);
			p_do_apply = (val != val_o);
			break;
		}
		if(p_do_apply)
		{
			SendMessage(p_parent, PSM_CHANGED, (WPARAM)p_dialog, 0);
		}
		else
		{
			SendMessage(p_parent, PSM_UNCHANGED, (WPARAM)p_dialog, 0);
		}

		return;
	}

	switch(id)
	{
	case IDC_UPLEFT:
		flags = CWPF_UP | CWPF_LEFT;
		break;
	case IDC_UP:
		flags = CWPF_UP;
		break;
	case IDC_UPRIGHT:
		flags = CWPF_RIGHT | CWPF_UP;
		break;
	case IDC_LEFT:
		flags = CWPF_LEFT;
		break;
	case IDC_CENTER:
		flags = CWPF_MIDDLE | CWPF_CENTER;
		break;
	case IDC_RIGHT:
		flags = CWPF_RIGHT;
		break;
	case IDC_DOWNLEFT:
		flags = CWPF_DOWN | CWPF_LEFT;
		break;
	case IDC_DOWN:
		flags = CWPF_DOWN;
		break;
	case IDC_DOWNRIGHT:
		flags = CWPF_DOWN | CWPF_RIGHT;
		break;
	case IDC_FULLWIDTH:
		flags = CWPF_FULLWIDTH;
		break;
	case IDC_FULLHEIGHT:
		flags = CWPF_FULLHEIGHT;
		break;
	}

	r.left = (LONG)SendMessage(p_xpos, UDM_GETPOS32, 0, 0);
	r.top = (LONG)SendMessage(p_ypos, UDM_GETPOS32, 0, 0);
	r.right = (LONG)SendMessage(p_width, UDM_GETPOS32, 0, 0);
	r.bottom = (LONG)SendMessage(p_height, UDM_GETPOS32, 0, 0);

	item = SendMessage(p_monitor, CB_GETCURSEL, 0, 0);
	mon = (HMONITOR)SendMessage(p_monitor, CB_GETITEMDATA, item, 0);

	calc_window_position(flags, mon, &r);

	SendMessage(p_xpos, UDM_SETPOS32, 0, r.left);
	SendMessage(p_ypos, UDM_SETPOS32, 0, r.top);
	SendMessage(p_width, UDM_SETPOS32, 0, r.right);
	SendMessage(p_height, UDM_SETPOS32, 0, r.bottom);
}

void SizeDialog::calc_window_position(ULONG flags, HMONITOR monitor, RECT * r)
{
	MONITORINFO mi;

#if defined(ZEROFILL_LOCAL_VARS)
	mi = NULL;
#endif

	if(monitor)
	{
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(monitor, &mi);
	}
	else
	{
		mi.rcWork.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
		mi.rcWork.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
		mi.rcWork.right = GetSystemMetrics(SM_CXVIRTUALSCREEN) - mi.rcWork.left;
		mi.rcWork.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) - mi.rcWork.top;
	}

	if(flags & CWPF_FULLWIDTH)
	{
		r->left = mi.rcWork.left;
		r->right = mi.rcWork.right;
	}

	if(flags & CWPF_FULLHEIGHT)
	{
		r->top = mi.rcWork.top;
		r->bottom = mi.rcWork.bottom;
	}

	switch(flags & CWPF_YPOS_MASK)
	{
	case CWPF_UP:
		r->top = mi.rcWork.top;
		break;
	case CWPF_MIDDLE:
		r->top = ((mi.rcWork.bottom - mi.rcWork.top) - r->bottom) / 2;
		break;
	case CWPF_DOWN:
		r->top = mi.rcWork.bottom - r->bottom;
		break;
	}
	switch(flags & CWPF_XPOS_MASK)
	{
	case CWPF_LEFT:
		r->left = mi.rcWork.left;
		break;
	case CWPF_CENTER:
		r->left = ((mi.rcWork.right - mi.rcWork.left) - r->right) / 2;
		break;
	case CWPF_RIGHT:
		r->left = mi.rcWork.right - r->right;
		break;
	}
}

BOOL CALLBACK SizeDialog::enum_monitors(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	UNREFERENCED_PARAMETER(hdcMonitor);
	UNREFERENCED_PARAMETER(lprcMonitor);
	LRESULT item;
	MONITORINFOEX mi;

#if defined(ZEROFILL_LOCAL_VARS)
	item = 0;
	memset(&mi, 0, sizeof(MONITORINFOEX));
#endif

	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, (LPMONITORINFO)&mi);

	item = SendMessage(((const ENUMMONITORPARAM *)dwData)->combobox, CB_ADDSTRING, 0, (LPARAM)mi.szDevice);
	SendMessage(((const ENUMMONITORPARAM *)dwData)->combobox, CB_SETITEMDATA, item, (LPARAM)hMonitor);

	if(hMonitor == ((const ENUMMONITORPARAM *)dwData)->current_monitor)
	{
		SendMessage(((const ENUMMONITORPARAM *)dwData)->combobox, CB_SETCURSEL, item, 0);
	}

	return TRUE;
}

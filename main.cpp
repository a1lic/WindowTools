#include "manifests.h"
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include "misc.h"
#include "powsavt.h"
#include "main.h"
#include "WindowList.h"
#include "WindowStatusBar.h"
#include "resource.h"

#if 0
/* dymcimp.c */
extern "C" void InitializeDelayedLoadEntryPoints();
extern "C" void FreeDelayedLoadEntryPoints();
#endif

struct THREAD_PARAM
{
	HINSTANCE instance;
	int       show;
};
typedef struct THREAD_PARAM THREAD_PARAM;


bool MainWindow::on_create()
{
	SetLastError(0);
	if(SetWindowLongPtr(p_window,0,(LONG_PTR)this) == 0)
	{
		if(GetLastError() != 0)
		{
			return false;
		}
	}

	p_menu = GetMenu(p_window);
	p_window_menu = LoadMenu(p_instance,MAKEINTRESOURCE(IDM_OP));

	p_list = new WindowList(p_window,1,NULL);
	p_status = new WindowStatusBar(p_window,2);

	SetTimer(p_window,398,1000,NULL);
	SetTimer(p_window,37564,1000,NULL);

	p_list->Update();

	return true;
}

void MainWindow::on_destroy()
{
	KillTimer(p_window,37564);
	KillTimer(p_window,398);

	if(p_window_menu)
	{
		DestroyMenu(p_window_menu);
	}
	if(p_status)
	{
		delete p_status;
	}
	if(p_list)
	{
		delete p_list;
	}
	PostQuitMessage(0);
}

void MainWindow::on_size(unsigned int width,unsigned int height)
{
	HWND listview;

	SendMessage(p_status->GetWindowHandle(),WM_SIZE,MAKELONG(width,height),0);

	listview = p_list->GetWindowHandle();
	SetWindowPos(listview,NULL,0,0,width,height - p_status->GetBarHeight(),SWP_NOACTIVATE | SWP_NOZORDER);
	ListView_Arrange(listview,LVA_DEFAULT);
}

void MainWindow::on_winini_change()
{
	RECT r[2];

	GetWindowRect(p_window,&r[0]);
	GetClientRect(p_window,&r[1]);
	OffsetRect(&r[0],-(r[0].left),-(r[0].top));

	p_min_client_area.x = 256 + (r[0].right - r[1].right);
	p_min_client_area.y = 256 + (r[0].bottom - r[1].bottom);
}

void MainWindow::on_get_minmax_info(MINMAXINFO *m)
{
	m->ptMinTrackSize = p_min_client_area;
}

bool MainWindow::on_contextmenu(HWND wnd,unsigned int x,unsigned int y)
{
	HMENU context_menu;

	if(wnd == p_list->GetWindowHandle())
	{
		context_menu = GetSubMenu(p_window_menu,0);
		if(context_menu)
		{
			p_list->HandleContextMenu(context_menu);
		}
		return true;
	}

	return false;
}

void MainWindow::on_command(unsigned int id,unsigned int type,HWND cwindow)
{
	DWORD exstyle;
	bool is_menu;

	is_menu = (type == 0);

	if(is_menu)
	{
		switch(id)
		{
		case IDC_VIEW_NORMALICON:
			p_list->SetViewMode(LVS_ICON);
			break;

		case IDC_VIEW_SMALLICON:
			p_list->SetViewMode(LVS_SMALLICON);
			break;

		case IDC_VIEW_LIST:
			p_list->SetViewMode(LVS_LIST);
			break;

		case IDC_VIEW_REPORT:
			p_list->SetViewMode(LVS_REPORT);
			break;

		case IDC_VIEW_TILE:
			if(OSFeatureTest(OSF_LISTVIEW_NT51))
			{
				p_list->SetViewMode(LV_VIEW_TILE);
			}
			break;

		case IDC_VIEW_TOGGLE_INVISIBLE:
			p_list->ToggleStylesMask(WS_VISIBLE);
			p_list->Update();
			break;

		case IDC_VIEW_TOGGLE_TOOLBARS:
			p_list->ToggleStylesMask(WS_POPUP);
			p_list->ToggleExtendStylesMask(WS_EX_TOOLWINDOW);
			p_list->Update();
			break;

		case IDC_VIEW_TOGGLE_NOTITLE:
			p_list->ShowNoTitleWindow();
			p_list->Update();
			break;

		case IDC_OPTION_TOPMOST:
			exstyle = (DWORD)GetWindowLongPtr(p_window,GWL_EXSTYLE);
			SetWindowPos(p_window,(exstyle & WS_EX_TOPMOST) ? HWND_NOTOPMOST : HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
			break;

		case IDC_OPTION_TOGGLEIMEBAR:
			SystemParametersInfo(SPI_SETSHOWIMEUI,p_toggle.ime_ui ? FALSE : TRUE,NULL,0);
			break;

		case IDC_OPTION_TOGGLECURSHADOW:
			SystemParametersInfo(SPI_SETCURSORSHADOW,0,(PVOID)(p_toggle.cursor_shadow ? FALSE : TRUE),0);
			break;

		case IDC_OPTION_TOGGLEHOTTRACK:
			SystemParametersInfo(SPI_SETHOTTRACKING,0,(PVOID)(p_toggle.hot_tracking ? FALSE : TRUE),SPIF_SENDWININICHANGE);
			break;

		case IDC_OPTION_TOGGLEWINDOWSHADOW:
			SystemParametersInfo(SPI_SETDROPSHADOW,0,(PVOID)(p_toggle.window_shadow ? FALSE : TRUE),SPIF_SENDWININICHANGE);
			break;

		case IDC_OPTION_TOGGLEMONITORPOWER:
			SystemParametersInfo(SPI_SETPOWEROFFACTIVE,p_toggle.monitor_power ? 0 : 1,NULL,0);
			break;

		case IDC_OPTION_TOGGLELOWMONITORPOWER:
			SystemParametersInfo(SPI_SETLOWPOWERACTIVE,p_toggle.monitor_power_low ? 0 : 1,NULL,0);
			break;

		case IDC_OPTION_POWERSAVE_TIMER:
			ShowPowerSaveTimerDialog(p_window,p_instance);
			break;

		case IDC_OPTION_PREVENT_POWERSAVE:
			if(p_toggle.prevent_power_save)
			{
				KillTimer(p_window,36115);
			}
			else
			{
				SetTimer(p_window,36115,10000,NULL);
			}
			p_toggle.prevent_power_save = !p_toggle.prevent_power_save;
			break;

		case IDC_OPTION_DISABLE_RESET:
			if(!SetThreadExecutionState(ES_CONTINUOUS))
			{
				MessageBox(p_window,TEXT("省電力タイマーのリセットの無効化に失敗しました。"),NULL,MB_ICONWARNING);
			}
			break;

		case IDC_OPTION_TOGGLEVANISH:
			SystemParametersInfo(SPI_SETMOUSEVANISH,0,(PVOID)(p_toggle.cursor_vanish ? FALSE : TRUE),SPIF_SENDWININICHANGE);
			break;

		case IDC_OPTION_PAUSE:
			if(!p_toggle.pause_update)
			{
				KillTimer(p_window,37564);
			}
			else
			{
				SetTimer(p_window,37564,1000,NULL);
			}
			p_toggle.pause_update = !p_toggle.pause_update;
			break;

		case IDC_HELP_ABOUT:
			DialogBoxParam(p_instance,MAKEINTRESOURCE(IDD_ABOUT),p_window,AboutDialogProc,0);
			break;

		case IDC_OPTION_TOGGLELEFTMENU:
			SystemParametersInfo(SPI_SETMENUDROPALIGNMENT, 0, (PVOID)(p_toggle.left_menu ? FALSE : TRUE), SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
			break;
		}
	}
}

void MainWindow::on_timer(UINT_PTR id)
{
	ULONG res;
	unsigned int c;
	TCHAR timer_res_text[32];

	switch(id)
	{
	case 398:
		if(!QueryTimerResolutions(NULL,NULL,&res))
		{
			if(p_timer_res != res)
			{
				p_timer_res = res;
				_sntprintf_s(timer_res_text,32,_TRUNCATE,_T("タイマーの解像度:%u00 ns"),res);
				p_status->SetText(0,timer_res_text);
			}
		}
		break;

	case 36115:
		SetThreadExecutionState(ES_DISPLAY_REQUIRED);
		break;

	case 37564:
#if 0
		id = (int)GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
		_sntprintf_s(caption,64,_TRUNCATE,_T("Window Tools - %lu gdi objs"),(DWORD)id);
		SetWindowText(window,caption);
#endif
		if(!IsIconic(p_window))
		{
			p_list->Update();
			c = p_list->GetItemCount();
			if(c != p_item_count)
			{
				p_item_count = c;
				_sntprintf_s(timer_res_text,32,_TRUNCATE,_T("%u のウィンドウ"),p_item_count);
				p_status->SetText(1,timer_res_text);
			}
		}
		break;
	}
}

void MainWindow::on_enter_menu_loop()
{
	MENUITEMINFO item_info;
	DWORD style,exstyle;

	memset(&item_info,0,sizeof(MENUITEMINFO));

	item_info.cbSize = sizeof(MENUITEMINFO);
	item_info.fMask = MIIM_STATE;

	style = p_list->GetStylesMask();
	exstyle = p_list->GetExtendStylesMask();

	item_info.fState = (style & WS_VISIBLE) ? MFS_UNCHECKED : MFS_CHECKED;
	SetMenuItemInfo(p_menu,IDC_VIEW_TOGGLE_INVISIBLE,FALSE,&item_info);

	item_info.fState = ((style & WS_POPUP) && (exstyle & WS_EX_TOOLWINDOW)) ? MFS_UNCHECKED : MFS_CHECKED;
	SetMenuItemInfo(p_menu,IDC_VIEW_TOGGLE_TOOLBARS,FALSE,&item_info);

	item_info.fState = p_list->IsShowNoTitleWindow() ? MFS_CHECKED : MFS_UNCHECKED;
	SetMenuItemInfo(p_menu,IDC_VIEW_TOGGLE_NOTITLE,FALSE,&item_info);

	if(!OSFeatureTest(OSF_LISTVIEW_NT51))
	{
		item_info.fState = MFS_GRAYED;
		SetMenuItemInfo(p_menu,IDC_VIEW_TILE,FALSE,&item_info);
	}

	p_toggle.ime_ui = TO_BOOLEAN(GetSysParametersBoolean(SPI_GETSHOWIMEUI));
	p_toggle.cursor_shadow = TO_BOOLEAN(GetSysParametersBoolean(SPI_GETCURSORSHADOW));
	p_toggle.hot_tracking = TO_BOOLEAN(GetSysParametersBoolean(SPI_GETHOTTRACKING));
	p_toggle.window_shadow = TO_BOOLEAN(GetSysParametersBoolean(SPI_GETDROPSHADOW));
	p_toggle.monitor_power = TO_BOOLEAN(GetSysParametersBoolean(SPI_GETPOWEROFFACTIVE));
	p_toggle.monitor_power_low = TO_BOOLEAN(GetSysParametersBoolean(SPI_GETLOWPOWERACTIVE));
	p_toggle.cursor_vanish = TO_BOOLEAN(GetSysParametersBoolean(SPI_GETMOUSEVANISH));
	p_toggle.left_menu = TO_BOOLEAN(GetSysParametersBoolean(SPI_GETMENUDROPALIGNMENT));
	exstyle = (DWORD)GetWindowLongPtr(p_window,GWL_EXSTYLE);

	CheckMenuItem2(p_menu,IDC_OPTION_TOPMOST,exstyle & WS_EX_TOPMOST);
	CheckMenuItem2(p_menu,IDC_OPTION_TOGGLEIMEBAR,p_toggle.ime_ui);
	CheckMenuItem2(p_menu,IDC_OPTION_TOGGLECURSHADOW,p_toggle.cursor_shadow);
	CheckMenuItem2(p_menu,IDC_OPTION_TOGGLEHOTTRACK,p_toggle.hot_tracking);
	CheckMenuItem2(p_menu,IDC_OPTION_TOGGLEWINDOWSHADOW,p_toggle.window_shadow);
	CheckMenuItem2(p_menu,IDC_OPTION_TOGGLEMONITORPOWER,p_toggle.monitor_power);
	CheckMenuItem2(p_menu,IDC_OPTION_TOGGLELOWMONITORPOWER,p_toggle.monitor_power_low);
	CheckMenuItem2(p_menu,IDC_OPTION_PREVENT_POWERSAVE,p_toggle.prevent_power_save);
	CheckMenuItem2(p_menu,IDC_OPTION_TOGGLEVANISH,p_toggle.cursor_vanish);
	CheckMenuItem2(p_menu,IDC_OPTION_PAUSE,p_toggle.pause_update);
	CheckMenuItem2(p_menu, IDC_OPTION_TOGGLELEFTMENU, p_toggle.left_menu);
}

LRESULT CALLBACK MainWindow::window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MainWindow *_this;
	LRESULT     r;

	_this = (MainWindow*)GetWindowLongPtr(hwnd, 0);
	r = 0;

	switch(uMsg)
	{
	case WM_CREATE: // 0x0001
		_this = (MainWindow*)((const CREATESTRUCT*)lParam)->lpCreateParams;
		_this->p_window = hwnd;
		if(!_this->on_create())
		{
			r = -1;
		}
		break;

	case WM_DESTROY: // 0x0002
		_this->on_destroy();
		break;

	case WM_SIZE: // 0x0005
		_this->on_size(LOWORD(lParam), HIWORD(lParam));

		break;

	case WM_SETFOCUS: // 0x0007
		_this->p_list->GiveFocus();
		break;

	case WM_SETTINGCHANGE: // 0x001A
		_this->on_winini_change();
		break;

	case WM_GETMINMAXINFO: // 0x0024
		if(_this)
		{
			_this->on_get_minmax_info((MINMAXINFO*)lParam);
		}
		break;

	case WM_NOTIFY: // 0x004E
		if((((const NMHDR*)lParam)->idFrom == 1))
		{
			r = _this->p_list->Notify((const NMHDR*)lParam);
		}
		break;

	case WM_CONTEXTMENU: // 0x007B
		if(!_this->on_contextmenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam)))
		{
			r = DefWindowProc(hwnd, WM_CONTEXTMENU, wParam, lParam);
		}
		break;

	case WM_COMMAND: // 0x0111
		_this->on_command(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;

	case WM_TIMER: // 0x112
		_this->on_timer((UINT_PTR)wParam);
		break;

	case WM_LBUTTONUP: // 0x0202
	case WM_RBUTTONUP: // 0x0205
		_this->p_list->EndDragMode();
		break;

	case WM_ENTERMENULOOP: // 0x0211
		_this->on_enter_menu_loop();
		break;

	default:
		r = DefWindowProc(hwnd, uMsg, wParam, lParam);
		break;
	}

	return r;
}

MainWindow::MainWindow(HINSTANCE instance)
{
	p_instance = instance;
	p_show_window = 0;

	//p_pos.x       = 0;
	//p_pos.y       = 0;
	//p_size.cx     = 0;
	//p_size.cy     = 0;

	p_list        = NULL;
	p_status      = NULL;
	p_menu        = NULL;
	p_window_menu = NULL;

	p_min_client_area.x = 0;
	p_min_client_area.y = 0;

	p_timer_res  = 0;
	p_item_count = 0;

	p_toggle.ime_ui             = false;
	p_toggle.cursor_shadow      = false;
	p_toggle.hot_tracking       = false;
	p_toggle.window_shadow      = false;
	p_toggle.monitor_power      = false;
	p_toggle.monitor_power_low  = false;
	p_toggle.prevent_power_save = false;
	p_toggle.cursor_vanish      = false;
	p_toggle.pause_update       = false;
}

MainWindow::~MainWindow()
{
	SendMessage(p_window, WM_CLOSE, 0, 0);
}

HWND MainWindow::ShowWindow2(int show_window)
{
	p_window = CreateWindowEx(
		0,
		TEXT("Window Tools"),
		TEXT("Window Tools"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		p_instance,
		this);

	if(p_window)
	{
		ShowWindow(p_window, show_window);
	}
	return p_window;
}

ATOM MainWindow::RegisterClass2(const WNDCLASSEX *wc)
{
	WNDCLASSEX window_class;

	window_class           = window_class_t;
	window_class.hInstance = wc->hInstance;
	window_class.hIcon     = wc->hIcon;
	window_class.hIconSm   = wc->hIconSm;

	return RegisterClassEx(&window_class);
}

const WNDCLASSEX MainWindow::window_class_t = {
	/* cbSize        */ sizeof(WNDCLASSEX),
	/* style         */ 0,
	/* lpfnWndProc   */ MainWindow::window_proc,
	/* cbClsExtra    */ 0,
	/* cbWndExtra    */ sizeof(void*), /* ポインタを格納する領域を確保する */
	/* hInstance     */ NULL,
	/* hIcon         */ NULL,
	/* hCursor       */ NULL,
	/* hbrBackground */ NULL, //(HBRUSH)(1 + COLOR_3DFACE), // クライアント領域を子ウィンドウがすべて覆い尽くす場合はNULLで良い
	/* lpszMenuName  */ MAKEINTRESOURCE(IDM_VIEW),
	/* lpszClassName */ TEXT("Window Tools"),
	/* hIconSm       */ NULL};

LRESULT CALLBACK kbd_watchdog(int nCode, WPARAM wParam, LPARAM lParam)
{
	static unsigned long long last_exec;
	unsigned long long        current_time;
	FILETIME                  current;

	if(nCode == HC_ACTION)
	{
		GetSystemTimeAsFileTime(&current);
		current_time = ((unsigned long long)current.dwHighDateTime << 32) + current.dwLowDateTime;

		if(current_time >= last_exec + 10000000ULL)
		{
			last_exec = current_time;
		}
		else
		{
			return CallNextHookEx(NULL,HC_ACTION,wParam,lParam);
		}

		switch(wParam)
		{
		case VK_NONCONVERT:
			LocalHeapDump();
			break;

		case VK_CONVERT:
			DumpAllHeapsStatus();
			break;
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

unsigned __stdcall main_window_thread(void *arg)
{
	MSG         msg;
	MainWindow *rarg;
	HWND        window;
	HHOOK       hook;

	rarg = new MainWindow(((const THREAD_PARAM*)arg)->instance);

	window = rarg->ShowWindow2(((const THREAD_PARAM*)arg)->show);
	if(window)
	{
		hook = SetWindowsHookEx(WH_KEYBOARD, kbd_watchdog, NULL, GetCurrentThreadId());

		SendMessage(window, WM_SETTINGCHANGE, 0, 0);

		// BOOLは符号付き整数なのでこのような比較でも問題はない
		while(GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		UnhookWindowsHookEx(hook);
	}

	delete rarg;
	return (unsigned)(msg.wParam & 0xFF);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX   wc;
	THREAD_PARAM p;
	ATOM         catom;
#if _WIN32_WINNT >= 0x600  && defined(ENABLE_THUMBNAIL)
	ATOM         thumb_atom;
#endif
	//STARTUPINFO  StartupInfo;
	//PROCESS_INFORMATION ProcessInfo;
	//BOOL         Result;
	//TCHAR        *Cmd;
	int          quit_code;
	bool         loaded_oem_normal_icon;
	bool         loaded_oem_small_icon;

	/*InitializeDelayedLoadEntryPoints();*/
	LocalHeapInitialize();
	InitCommonControls();

#if 0
	memset(&StartupInfo, 0, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);
	Cmd = new TCHAR[MAX_PATH];
	_tcscpy_s(Cmd, MAX_PATH, _T("winver.exe"));
	Result = ::CreateProcess(nullptr, Cmd, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &StartupInfo, &ProcessInfo);
	if(Result)
	{
		::CloseHandle(ProcessInfo.hProcess);
		::CloseHandle(ProcessInfo.hThread);
	}
	delete [] Cmd;
#endif

	ntdll = LoadLibrary(TEXT("NTDLL.DLL"));
	if(ntdll)
	{
		u_NtQueryTimerResolution.FuncPointer = GetProcAddress(ntdll, "NtQueryTimerResolution");
	}

	// アイコンをリソースから読み込む
	// Windows95、WindowsNT4以降では小さいアイコンも読み込む必要がある
	wc.hInstance = hInstance;
	// LoadIconのアイコンサイズはSM_CXICON、SM_CYICON固定なので、LoadImage+GetSystemMetricsでサイズを指定して読み込む
	wc.hIcon     = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, GetSystemMetrics(SM_CXICON),   GetSystemMetrics(SM_CYICON),   0);
	if(wc.hIcon == NULL)
	{
		wc.hIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_SAMPLE), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_SHARED);
		loaded_oem_normal_icon = (wc.hIcon != NULL);
	}
	else
	{
		loaded_oem_normal_icon = false;
	}
	wc.hIconSm   = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	if(wc.hIconSm == NULL)
	{
		wc.hIconSm = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_SAMPLE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		loaded_oem_small_icon = (wc.hIconSm != NULL);
	}
	else
	{
		loaded_oem_small_icon = false;
	}

	catom = MainWindow::RegisterClass2(&wc);

#if _WIN32_WINNT >= 0x600 && defined(ENABLE_THUMBNAIL)
	thumb_atom = Thumbnail::RegisterClass2(&wc);
#endif

	p.instance = hInstance;
	p.show     = nCmdShow;

	quit_code = (int)main_window_thread(&p);

	// 後始末
#if _WIN32_WINNT >= 0x600 && defined(ENABLE_THUMBNAIL)
	UnregisterClass((LPCTSTR)thumb_atom,hInstance);
#endif

	UnregisterClass((LPCTSTR)catom,hInstance);

	// アイコンの削除
	if(loaded_oem_small_icon)
	{
		DestroyIcon(wc.hIconSm);
	}
	if(loaded_oem_normal_icon)
	{
		DestroyIcon(wc.hIcon);
	}

	if(ntdll)
	{
		FreeLibrary(ntdll);
	}

	LocalHeapDestroy();
	/*FreeDelayedLoadEntryPoints();*/

	return quit_code;
}

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <tchar.h>
#include <windows.h>
//#include <psapi.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include "misc.h"

HINSTANCE ntdll;
union NTQUERYTIMERRESOLUTION u_NtQueryTimerResolution;

DWORD feature_flags;
HANDLE main_heap;

void Debug(PCTSTR fmt, ...)
{
	va_list ap;
	TCHAR * debug_msg;

	if(!IsDebuggerPresent())
	{
		return;
	}

	debug_msg = (TCHAR *)calloc(4096, sizeof(TCHAR));
	if(!debug_msg)
	{
		return;
	}

	va_start(ap, fmt);
	_vstprintf_s(debug_msg, 4095, fmt, ap);
	va_end(ap);

	_tcscat_s(debug_msg, 4096, _T("\n"));

	OutputDebugString(debug_msg);
	free(debug_msg);
}

int MessageBoxf(HWND parent, PCTSTR title, DWORD flags, PCTSTR fmt, ...)
{
	va_list ap;
	TCHAR * msg;
	int messagebox_result;
	BOOLEAN static_buffer;
	TCHAR msg_s[64];

	msg = (TCHAR *)calloc(4096, sizeof(TCHAR));
	if(msg)
	{
		static_buffer = FALSE;
	}
	else
	{
		static_buffer = TRUE;
		msg = msg_s;
	}

	va_start(ap, fmt);
	_vstprintf_s(msg, static_buffer ? 64 : 4096, fmt, ap);
	va_end(ap);

	messagebox_result = MessageBox(parent, msg, title, flags);

	if(!static_buffer)
	{
		free(msg);
	}

	return messagebox_result;
}

int ListViewGetSelectingIndex(HWND lvd)
{
	int i;

	i = ListView_GetSelectedCount(lvd);
	if(i == 0)
		return -1;

	i = ListView_GetSelectionMark(lvd);
	return i;
}

LPARAM ListViewGetLPARAM(HWND listview,int item)
{
	LVITEM i;

	memset(&i,0,sizeof(LVITEM));
	i.mask = LVIF_PARAM;
	i.iItem = item;

	if(!ListView_GetItem(listview,&i))
	{
		return 0;
	}
	return i.lParam;
}

void ListViewUpdateItemString(HWND listview,int item,int sub_item,PCTSTR text)
{
	TCHAR *tmp_text;

	tmp_text = (TCHAR*)calloc(1024,sizeof(TCHAR));
	if(tmp_text)
	{
		ListView_GetItemText(listview,item,sub_item,tmp_text,1024);
		if(_tcscmp(tmp_text,text))
		{
			ListView_SetItemText(listview,item,sub_item,(PTSTR)text);
		}
	}
	else
	{
		ListView_SetItemText(listview,item,sub_item,(PTSTR)text);
	}
}

DWORD GetNtDDIVersionOfPlatform()
{
	OSVERSIONINFOEX version;
	DWORD ddi;

	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
#pragma warning(push)
#pragma warning(disable:4996)
	GetVersionEx((OSVERSIONINFO*)&version);
#pragma warning(pop)

	ddi = version.dwMajorVersion & 0xFF;
	ddi = (ddi << 8) | (version.dwMinorVersion & 0xFF);
	ddi = (ddi << 8) | (version.wServicePackMajor & 0xFF);
	ddi = (ddi << 8) | (version.wServicePackMinor & 0xFF);

	return ddi;
}

BOOLEAN OSFeatureTest(DWORD flags)
{
	DWORD v;

	if(feature_flags != OSF_OK)
	{
		v = GetNtDDIVersionOfPlatform();
		feature_flags = OSF_OK;
		if(v >= 0x40A0000) /* Windows98 */
		{
			feature_flags |= OSF_MON_98;
		}
		if(v >= NTDDI_WIN2K)
		{
			feature_flags |= OSF_GUI_NT5;
		}
		if(v >= NTDDI_WINXP)
		{
			feature_flags |= (OSF_GUI_NT51 | OSF_LISTVIEW_NT51);
		}
		if(v >= NTDDI_VISTA)
		{
			feature_flags |= OSF_LISTVIEW_NT60;
		}
	}
	return ((feature_flags & flags) == flags);
}

void CenteringWindow(HWND window,const POINT *pos)
{
	/* ウィンドウの中心点がposで指定した位置に来るように移動する */
	HMONITOR monitor;
	POINT p;
	RECT dlg_rect;
	MONITORINFO mon_info;

	p = *pos;
	/* 指定した点のあるモニタ */
	if(OSFeatureTest(OSF_MON_98))
	{
		monitor = MonitorFromPoint(p,MONITOR_DEFAULTTONEAREST);
	}
	else
	{
		monitor = NULL;
	}

	if(monitor)
	{
		/* そのモニタに映っている作業領域の大きさを調べる */
		mon_info.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(monitor,&mon_info);
	}
	else
	{
		/* モニタのハンドルが取れなかった場合はプライマリモニタと仮定 */
		SystemParametersInfo(SPI_GETWORKAREA,0,&mon_info.rcWork,0);
	}

	InflateRect(&mon_info.rcWork,-8,-8);

	/* ダイアログの大きさ */
	GetWindowRect(window,&dlg_rect);
	OffsetRect(&dlg_rect,-dlg_rect.left,-dlg_rect.top);

	/* 大きさの半分を座標から減ずる */
	p.x -= dlg_rect.right / 2;
	p.y -= dlg_rect.bottom / 2;

	/* はみ出る場合は収まるようにする */
	if((p.x + dlg_rect.right) > mon_info.rcWork.right)
	{
		p.x = mon_info.rcWork.right - dlg_rect.right;
	}
	if((p.y + dlg_rect.bottom) > mon_info.rcWork.bottom)
	{
		p.y = mon_info.rcWork.bottom - dlg_rect.bottom;
	}

	if(p.x < mon_info.rcWork.left)
	{
		p.x = mon_info.rcWork.left;
	}
	if(p.y < mon_info.rcWork.top)
	{
		p.y = mon_info.rcWork.top;
	}

	SetWindowPos(window,NULL,p.x,p.y,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
}

void CenteringWindowToCursor(HWND window)
{
	POINT p;

	GetCursorPos(&p);
	CenteringWindow(window,&p);
}

void CenteringWindowToParent(HWND window,HWND parent)
{
	RECT parent_rect;
	POINT p;

	if(GetWindowRect(parent,&parent_rect))
	{
		p.x = parent_rect.left + (parent_rect.right - parent_rect.left) / 2;
		p.y = parent_rect.top + (parent_rect.bottom - parent_rect.top) / 2;
		CenteringWindow(window,&p);
	}
}

BOOLEAN GetSysParametersBoolean(int index)
{
	BOOL value;

	if(!SystemParametersInfo(index,0,&value,0))
	{
		if(IsDebuggerPresent())
		{
			__debugbreak();
		}
		value = TRUE;
	}
	return (value != FALSE);
}

void CheckMenuItem2(HMENU menu,int id,BOOLEAN checked)
{
	CheckMenuItem(menu,id,MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
}

/*
void CheckMenuItem3(HMENU menu,int id,BOOLEAN checked)
{
	
	m.hbmpChecked = MFT_RADIOCHECK;
	CheckMenuItem(menu,id,MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
}
*/

ULONG QueryTimerResolutions(ULONG *pmin,ULONG *pmax,ULONG *pcur)
{
	ULONG min,max,cur,r;

	if(!u_NtQueryTimerResolution.FuncPointer)
	{
		return 0xC0000002UL; /* STATUS_NOT_IMPLEMENTED */
	}

	r = u_NtQueryTimerResolution.Invoke(&min,&max,&cur);

	if(!r)
	{
		if(pmin)
		{
			*pmin = min;
		}
		if(pmax)
		{
			*pmax = max;
		}
		if(pcur)
		{
			*pcur = cur;
		}
	}

	return r;
}

INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	INT_PTR r;
	OSVERSIONINFOEX version;
	TCHAR *str;
	TCHAR *newstr;

	r = FALSE;
	switch(uMsg)
	{
	case WM_CLOSE:
		EndDialog(hwndDlg,0);
		r = TRUE;
		break;

	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hwndDlg,0);
			r = TRUE;
		}
		break;

	case WM_INITDIALOG:
		CenteringWindowToParent(hwndDlg,(HWND)GetWindowLongPtr(hwndDlg,GWLP_HWNDPARENT));
		memset(&version, 0, sizeof(OSVERSIONINFOEX));
		version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
#pragma warning(push)
#pragma warning(disable:4996)
		GetVersionEx((OSVERSIONINFO *)&version);
#pragma warning(pop)
		str = malloc(sizeof(TCHAR) * 1024);
		newstr = malloc(sizeof(TCHAR) * 256);
		GetDlgItemText(hwndDlg, 33, str, 1024);
		_stprintf_s(newstr, 256, _T("\r\nRunning in Windows version %d.%d.%d."), version.dwMajorVersion, version.dwMinorVersion, version.dwBuildNumber);
		_tcscat_s(str, 1024, newstr);
		SetDlgItemText(hwndDlg, 33, str);
		free(newstr);
		free(str);
		r = TRUE;
	}
	return r;
}

BOOLEAN GetProcessNameFromId(DWORD id,PTSTR name,size_t name_s)
{
	BOOLEAN r;
	HANDLE s;
	PROCESSENTRY32 info;
	BOOL rw;
	DWORD name_size;
	BOOL qfpin_result;
#if (NTDDI_VERSION < NTDDI_VISTA)
	BOOL (WINAPI *qfpin)(HANDLE, DWORD, LPTSTR, PDWORD);

	HMODULE kernel32 = GetModuleHandle(TEXT("KERNEL32.DLL"));
#if defined(UNICODE)
	qfpin = GetProcAddress(kernel32, "QueryFullProcessImageNameW");
#else
	qfpin = GetProcAddress(kernel32, "QueryFullProcessImageNameA");
#endif
	if(qfpin == NULL)
	{
		goto LegacyMethod;
	}
#endif

	DWORD ntddi = GetNtDDIVersionOfPlatform();
	if(ntddi < NTDDI_VISTA)
	{
		goto LegacyMethod;
	}

	s = OpenProcess((ntddi >= NTDDI_VISTA) ? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_QUERY_INFORMATION, FALSE, id);
	if(s == NULL)
	{
		goto LegacyMethod;
	}

	name_size = (DWORD)name_s;
#if (NTDDI_VERSION >= NTDDI_VISTA)
	qfpin_result = QueryFullProcessImageName(s, PROCESS_NAME_NATIVE, name, &name_size);
#else
	qfpin_result = (*qfpin)(s, PROCESS_NAME_NATIVE, name, name_size);
#endif
	CloseHandle(s);
	if(qfpin_result)
	{
		return TRUE;
	}

LegacyMethod:
	r = FALSE;
	s = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(s)
	{
		info.dwSize = sizeof(PROCESSENTRY32);
		for(rw = Process32First(s,&info); rw; rw = Process32Next(s,&info))
		{
			if(info.th32ProcessID == id)
			{
				_tcsncpy_s(name,name_s,info.szExeFile,_TRUNCATE);
				r = TRUE;
				break;
			}
		}
		CloseHandle(s);
	}
	return r;
}

void CleanupListView(HWND window,HANDLE heap)
{
}

void GetClientRectAsScreenPos(HWND window,RECT *rc)
{
	POINT pos;

	pos.x = 0;
	pos.y = 0;
	ClientToScreen(window,&pos);
	GetClientRect(window,rc);

	rc->left = pos.x;
	rc->top = pos.y;
	rc->right += pos.x;
	rc->bottom += pos.y;
}

void LocalHeapInitialize()
{
	static const ULONG two = 2UL;

	main_heap = HeapCreate(0,0,0);
	HeapSetInformation(main_heap,HeapCompatibilityInformation,(PVOID)&two,sizeof(ULONG));
}

void LocalHeapDestroy()
{
	if(HeapDestroy(main_heap))
	{
		main_heap = NULL;
	}
}

void* LocalHeapAlloc(DWORD size)
{
	return HeapAlloc(main_heap,HEAP_ZERO_MEMORY,size);
}

void* LocalHeapReAlloc(DWORD size,void *pointer)
{
	return HeapReAlloc(main_heap,HEAP_ZERO_MEMORY,pointer,size);
}

void LocalHeapFree(void *pointer)
{
	HeapFree(main_heap,0,pointer);
}

void LocalHeapDump()
{
	DumpHeapStatus(main_heap);
}

void DumpHeapStatus(HANDLE heap)
{
	PROCESS_HEAP_ENTRY heap_entry;
	unsigned int i;

	if(!IsDebuggerPresent())
	{
		return;
	}

	if(HeapLock(heap))
	{
		Debug(TEXT("Dumping heap(%p) informations..."),heap);
		i = 0;
		memset(&heap_entry,0,sizeof(PROCESS_HEAP_ENTRY));
		while(HeapWalk(heap,&heap_entry))
		{
			if(heap_entry.wFlags & PROCESS_HEAP_REGION)
			{
				Debug(
					TEXT("%5lu: %p(%12lu,%3u) %3u %04X %12lu %12lu %p-%p"),
					i++,
					heap_entry.lpData,
					heap_entry.cbData,
					heap_entry.cbOverhead,
					heap_entry.iRegionIndex,
					heap_entry.wFlags,
					heap_entry.Region.dwCommittedSize,
					heap_entry.Region.dwUnCommittedSize,
					heap_entry.Region.lpFirstBlock,
					heap_entry.Region.lpLastBlock);
			}
			else if(heap_entry.wFlags & PROCESS_HEAP_ENTRY_MOVEABLE)
			{
				Debug(
					TEXT("%5lu: %p(%12lu,%3u) %3u %04X %p %08X %08X %08X"),
					i++,
					heap_entry.lpData,
					heap_entry.cbData,
					heap_entry.cbOverhead,
					heap_entry.iRegionIndex,
					heap_entry.wFlags,
					heap_entry.Block.hMem,
					heap_entry.Block.dwReserved[0],
					heap_entry.Block.dwReserved[1],
					heap_entry.Block.dwReserved[2]);
			}
			else
			{
				Debug(
					TEXT("%5lu: %p(%12lu,%3u) %3u %04X"),
					i++,
					heap_entry.lpData,
					heap_entry.cbData,
					heap_entry.cbOverhead,
					heap_entry.iRegionIndex,
					heap_entry.wFlags);
			}
		}
		HeapUnlock(heap);
	}
}

void DumpAllHeapsStatus()
{
	HANDLE *heap_list;
	DWORD i,count;

	heap_list = VirtualAlloc(NULL,4096 * sizeof(HANDLE),MEM_COMMIT,PAGE_READWRITE);
	if(heap_list)
	{
		count = GetProcessHeaps(4096,heap_list);
		if(count > 0UL)
		{
			for(i = 0UL; i < count; i++)
			{
				DumpHeapStatus(heap_list[i]);
			}
		}
		VirtualFree(heap_list,0,MEM_RELEASE);
	}
}

void SnapWindow(HWND hwnd, RECT *prc, int Margin, HWND hwndExclude)
{
	HMONITOR hMonitor;
	RECT rc;
	SnapWindowInfo Info;
	int XOffset, YOffset;

	hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if(hMonitor != NULL)
	{
		MONITORINFO mi;

		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMonitor, &mi);
		rc = mi.rcMonitor;
	}
	else
	{
		rc.left = 0;
		rc.top = 0;
		rc.right = GetSystemMetrics(SM_CXSCREEN);
		rc.bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	Info.hwnd = hwnd;
	Info.rcOriginal = *prc;
	Info.rcNearest.left = rc.left - prc->left;
	Info.rcNearest.top = rc.top - prc->top;
	Info.rcNearest.right = rc.right - prc->right;
	Info.rcNearest.bottom = rc.bottom - prc->bottom;
	Info.hwndExclude = hwndExclude;
	EnumWindows(SnapWindowProc, (LPARAM)&Info);
	if(abs(Info.rcNearest.left)<abs(Info.rcNearest.right) || Info.rcNearest.left == Info.rcNearest.right)
		XOffset = Info.rcNearest.left;
	else if(abs(Info.rcNearest.left)>abs(Info.rcNearest.right))
		XOffset = Info.rcNearest.right;
	else
		XOffset = 0;
	if(abs(Info.rcNearest.top)<abs(Info.rcNearest.bottom) || Info.rcNearest.top == Info.rcNearest.bottom)
		YOffset = Info.rcNearest.top;
	else if(abs(Info.rcNearest.top)>abs(Info.rcNearest.bottom))
		YOffset = Info.rcNearest.bottom;
	else
		YOffset = 0;
	if(abs(XOffset) <= Margin)
		prc->left += XOffset;
	if(abs(YOffset) <= Margin)
		prc->top += YOffset;
	prc->right = prc->left + (Info.rcOriginal.right - Info.rcOriginal.left);
	prc->bottom = prc->top + (Info.rcOriginal.bottom - Info.rcOriginal.top);
}

BOOL CALLBACK SnapWindowProc(HWND hwnd, LPARAM lParam)
{
	SnapWindowInfo *pInfo = (SnapWindowInfo*)lParam;
	RECT rc;
	RECT rcEdge;

	if(!(IsWindowVisible(hwnd) && hwnd != pInfo->hwnd && hwnd != pInfo->hwndExclude))
	{
		return TRUE;
	}

	GetWindowRect(hwnd, &rc);
	if(rc.right>rc.left && rc.bottom>rc.top)
	{
		if(rc.top<pInfo->rcOriginal.bottom && rc.bottom>pInfo->rcOriginal.top)
		{
			if(abs(rc.left - pInfo->rcOriginal.right)<abs(pInfo->rcNearest.right))
			{
				rcEdge.left = rc.left;
				rcEdge.right = rc.left;
				rcEdge.top = max(rc.top, pInfo->rcOriginal.top);
				rcEdge.bottom = min(rc.bottom, pInfo->rcOriginal.bottom);
				if(IsWindowEdgeVisible(hwnd, GetTopWindow(GetDesktopWindow()), &rcEdge, pInfo->hwnd))
					pInfo->rcNearest.right = rc.left - pInfo->rcOriginal.right;
			}
			if(abs(rc.right - pInfo->rcOriginal.left)<abs(pInfo->rcNearest.left))
			{
				rcEdge.left = rc.right;
				rcEdge.right = rc.right;
				rcEdge.top = max(rc.top, pInfo->rcOriginal.top);
				rcEdge.bottom = min(rc.bottom, pInfo->rcOriginal.bottom);
				if(IsWindowEdgeVisible(hwnd, GetTopWindow(GetDesktopWindow()), &rcEdge, pInfo->hwnd))
					pInfo->rcNearest.left = rc.right - pInfo->rcOriginal.left;
			}
		}
		if(rc.left<pInfo->rcOriginal.right && rc.right>pInfo->rcOriginal.left)
		{
			if(abs(rc.top - pInfo->rcOriginal.bottom)<abs(pInfo->rcNearest.bottom))
			{
				rcEdge.left = max(rc.left, pInfo->rcOriginal.left);
				rcEdge.right = min(rc.right, pInfo->rcOriginal.right);
				rcEdge.top = rc.top;
				rcEdge.bottom = rc.top;
				if(IsWindowEdgeVisible(hwnd, GetTopWindow(GetDesktopWindow()), &rcEdge, pInfo->hwnd))
					pInfo->rcNearest.bottom = rc.top - pInfo->rcOriginal.bottom;
			}
			if(abs(rc.bottom - pInfo->rcOriginal.top)<abs(pInfo->rcNearest.top))
			{
				rcEdge.left = max(rc.left, pInfo->rcOriginal.left);
				rcEdge.right = min(rc.right, pInfo->rcOriginal.right);
				rcEdge.top = rc.bottom;
				rcEdge.bottom = rc.bottom;
				if(IsWindowEdgeVisible(hwnd, GetTopWindow(GetDesktopWindow()), &rcEdge, pInfo->hwnd))
					pInfo->rcNearest.top = rc.bottom - pInfo->rcOriginal.top;
			}
		}
	}
	return TRUE;
}

BOOLEAN IsWindowEdgeVisible(HWND hwnd, HWND hwndTop, const RECT *pRect, HWND hwndTarget)
{
	RECT rc, rcEdge;
	HWND hwndNext;

	if(hwndTop == hwnd || hwndTop == NULL)
		return TRUE;
	GetWindowRect(hwndTop, &rc);
	hwndNext = GetNextWindow(hwndTop, GW_HWNDNEXT);
	if(hwndTop == hwndTarget || !IsWindowVisible(hwndTop) || rc.left == rc.right || rc.top == rc.bottom)
		return IsWindowEdgeVisible(hwnd, hwndNext, pRect, hwndTarget);
	if(pRect->top == pRect->bottom)
	{
		if(rc.top <= pRect->top && rc.bottom>pRect->top)
		{
			if(rc.left <= pRect->left && rc.right >= pRect->right)
				return FALSE;
			if(rc.left <= pRect->left && rc.right>pRect->left)
			{
				rcEdge = *pRect;
				rcEdge.right = min(rc.right, pRect->right);
				return IsWindowEdgeVisible(hwnd, hwndNext, &rcEdge, hwndTarget);
			}
			else if(rc.left>pRect->left && rc.right >= pRect->right)
			{
				rcEdge = *pRect;
				rcEdge.left = rc.left;
				return IsWindowEdgeVisible(hwnd, hwndNext, &rcEdge, hwndTarget);
			}
			else if(rc.left>pRect->left && rc.right<pRect->right)
			{
				rcEdge = *pRect;
				rcEdge.right = rc.left;
				if(IsWindowEdgeVisible(hwnd, hwndNext, &rcEdge, hwndTarget))
					return TRUE;
				rcEdge.left = rc.right;
				rcEdge.right = pRect->right;
				return IsWindowEdgeVisible(hwnd, hwndNext, &rcEdge, hwndTarget);
			}
		}
	}
	else
	{
		if(rc.left <= pRect->left && rc.right>pRect->left)
		{
			if(rc.top <= pRect->top && rc.bottom >= pRect->bottom)
				return FALSE;
			if(rc.top <= pRect->top && rc.bottom>pRect->top)
			{
				rcEdge = *pRect;
				rcEdge.bottom = min(rc.bottom, pRect->bottom);
				return IsWindowEdgeVisible(hwnd, hwndNext, &rcEdge, hwndTarget);
			}
			else if(rc.top>pRect->top && rc.bottom >= pRect->bottom)
			{
				rcEdge = *pRect;
				rcEdge.top = rc.top;
				return IsWindowEdgeVisible(hwnd, hwndNext, &rcEdge, hwndTarget);
			}
			else if(rc.top>pRect->top && rc.bottom<pRect->bottom)
			{
				rcEdge = *pRect;
				rcEdge.bottom = rc.top;
				if(IsWindowEdgeVisible(hwnd, hwndNext, &rcEdge, hwndTarget))
					return TRUE;
				rcEdge.top = rc.bottom;
				rcEdge.bottom = pRect->bottom;
				return IsWindowEdgeVisible(hwnd, hwndNext, &rcEdge, hwndTarget);
			}
		}
	}
	return IsWindowEdgeVisible(hwnd, hwndNext, pRect, hwndTarget);
}

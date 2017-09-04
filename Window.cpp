#include <tchar.h>
#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>
#include "Window.h"
#include "WindowList.h"
#include "Thumbnail.h"
#include "misc.h"

Window::Window(HWND handle_arg)
{
	LARGE_INTEGER pc;
	HWND t_handle;

	QueryPerformanceCounter(&pc);
	item_time_stamp = pc.QuadPart;

	is_unicode_window = TO_BOOLEAN(IsWindowUnicode(handle_arg));
	GetWindowThreadProcessId(handle_arg, &process);
	e_icon = NULL;

	handle = handle_arg;
	depth = 0;
	if(GetStyle() & WS_CHILD)
	{
		parent_handle = (HWND)GetWindowLongPtr(handle, GWLP_HWNDPARENT);
		t_handle = parent_handle;
		while(GetWindowLongPtr(t_handle, GWL_STYLE) & WS_CHILD)
		{
			depth++;
			t_handle = (HWND)GetWindowLongPtr(t_handle, GWLP_HWNDPARENT);
			if(!t_handle)
			{
				break;
			}
		}
	}
	else
	{
		parent_handle = NULL;
	}
	w_icon = NULL;
	w_icon_small = NULL;

#if _WIN32_WINNT >= 0x600 && defined(ENABLE_THUMBNAIL)
	thumb = new Thumbnail(handle,GetModuleHandle(NULL));
#endif

	UpdateIcon();
}

Window::~Window()
{
#if _WIN32_WINNT >= 0x600 && defined(ENABLE_THUMBNAIL)
	if(thumb)
	{
		thumb->Release();
	}
#endif
}

void Window::UpdateIcon()
{
	HICON icon;

	icon = get_icon_handle(false);
	if(icon != w_icon)
	{
		w_icon = icon;
	}

	icon = get_icon_handle(true);
	if(icon != w_icon_small)
	{
		w_icon_small = icon;
	}
}

INT_PTR Window::SendMsg(UINT message, WPARAM w, LPARAM l)
{
	return SendMessage(handle, message, w, l);
}

bool Window::SendMsg(UINT message, WPARAM w, LPARAM l, UINT time, DWORD_PTR * ret)
{
	DWORD_PTR dummy;
	return (bool)(SendMessageTimeout(handle, message, w, l, SMTO_ABORTIFHUNG, time, ret ? ret : &dummy) != 0);
}

void Window::PostMsg(UINT message, WPARAM w, LPARAM l)
{
	PostMessage(handle, message, w, l);
}

void Window::Iconic()
{
	ShowWindow(handle, SW_MINIMIZE);
}

void Window::Maximize()
{
	ShowWindow(handle, SW_MAXIMIZE);
}

void Window::Restore()
{
	ShowWindowAsync(handle, SW_RESTORE);
}

void Window::BringToTop()
{
	SetWindowPos(handle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void Window::SinkToBottom()
{
	SetWindowPos(handle, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

void Window::ZoomClient(float factor)
{
	POINT p;
	SIZE s;
	this->GetClientLocation(&p, &s);

	s.cx = (LONG)((float)s.cx * factor);
	s.cy = (LONG)((float)s.cy * factor);

	SIZE ns;
	do
	{
		this->ClientResize(&s);
		this->GetClientLocation(nullptr, &ns);
	}
	while(memcmp(&ns, &s, sizeof(SIZE)));
}

void Window::GetWindowLocation(POINT * pos, SIZE * size)
{
	RECT window_rect;

	GetWindowRect(handle, &window_rect);
	if(pos)
	{
		pos->x = window_rect.left;
		pos->y = window_rect.top;
	}
	if(size)
	{
		OffsetRect(&window_rect, -window_rect.left, -window_rect.top);
		size->cx = window_rect.right;
		size->cy = window_rect.bottom;
	}
}

void Window::GetClientLocation(POINT * pos, SIZE * size)
{
	RECT client_rect;

	GetClientRect(handle, &client_rect);
	if(TO_BOOLEAN(pos))
	{
		pos->x = 0;
		pos->y = 0;
		ClientToScreen(handle, pos);
	}
	if(TO_BOOLEAN(size))
	{
		size->cx = client_rect.right;
		size->cy = client_rect.bottom;
	}
}

void Window::SetWindowLocation(const POINT * pos, const SIZE * size)
{
	int x, y;
	unsigned int w, h;
	UINT flags;

	flags = SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER;
	if(pos)
	{
		x = pos->x;
		y = pos->y;
		flags ^= SWP_NOMOVE;
	}
	if(size)
	{
		w = size->cx;
		h = size->cy;
		flags ^= SWP_NOSIZE;
	}
	SetWindowPos(handle, NULL, x, y, w, h, flags);
}

void Window::SetClientLocation(const POINT * pos, const SIZE * size)
{
	POINT npos, wpos, cpos;
	SIZE nsize, wsize, csize;

	GetWindowLocation(&wpos, &wsize);
	GetClientLocation(&cpos, &csize);

	if(pos)
	{
		npos.x = pos->x - (cpos.x - wpos.x);
		npos.y = pos->y - (cpos.y - wpos.y);
		Move(&npos);
	}
	if(size)
	{
		nsize.cx = size->cx + (wsize.cx - csize.cx);
		nsize.cy = size->cy + (wsize.cy - csize.cy);
		Resize(&nsize);
	}
}

void Window::Move(const POINT * pos)
{
	SetWindowLocation(pos, NULL);
}

void Window::ClientMove(const POINT * pos)
{
	SetClientLocation(pos, NULL);
}

void Window::Resize(const SIZE * size)
{
	SetWindowLocation(NULL, size);
}

void Window::ClientResize(const SIZE * size)
{
	SetClientLocation(NULL, size);
}

DWORD Window::GetStyle()
{
	return (DWORD)GetWindowLongPtr(handle, GWL_STYLE);
}

void Window::SetStyle(DWORD new_style)
{
	HANDLE t;
	API_WATCHDOG * awd;
	SET_WINDOW_LONGPTR_PARAMS * wlp;

	awd = (API_WATCHDOG *)LocalHeapAlloc(sizeof(API_WATCHDOG));
	if(awd)
	{
		wlp = (SET_WINDOW_LONGPTR_PARAMS *)LocalHeapAlloc(sizeof(SET_WINDOW_LONGPTR_PARAMS));
		if(wlp)
		{
			awd->proc = set_window_longptr_thread;
			awd->parameter = wlp;
			awd->timeout_sec = 30;
			wlp->hWnd = handle;
			wlp->nIndex = GWL_STYLE;
			wlp->dwNewLong = new_style;

			t = CreateThread(NULL, 0, api_watchdog, awd, 0, NULL);
			if(t)
			{
				CloseHandle(t);
			}
		}
		else
		{
			LocalHeapFree(awd);
		}
	}
}

DWORD Window::GetExtendStyle()
{
	return (DWORD)GetWindowLongPtr(handle, GWL_EXSTYLE);
}

void Window::SetExtendStyle(DWORD new_extend_style)
{
	HANDLE t;
	API_WATCHDOG * awd;
	SET_WINDOW_LONGPTR_PARAMS * wlp;

	awd = (API_WATCHDOG *)LocalHeapAlloc(sizeof(API_WATCHDOG));
	if(awd)
	{
		wlp = (SET_WINDOW_LONGPTR_PARAMS *)LocalHeapAlloc(sizeof(SET_WINDOW_LONGPTR_PARAMS));
		if(wlp)
		{
			awd->proc = set_window_longptr_thread;
			awd->parameter = wlp;
			awd->timeout_sec = 30;
			wlp->hWnd = handle;
			wlp->nIndex = GWL_EXSTYLE;
			wlp->dwNewLong = new_extend_style;

			t = CreateThread(NULL, 0, api_watchdog, awd, 0, NULL);
			if(t)
			{
				CloseHandle(t);
			}
		}
		else
		{
			LocalHeapFree(awd);
		}
	}
}

unsigned int Window::GetCaptionLength()
{
	int n;
	n = GetWindowTextLength(handle);
	if(n < 0)
	{
		n = 0;
	}
	return (unsigned int)n;
}

PTSTR Window::GetCaption()
{
	int n;
	PTSTR caption;

	n = GetWindowTextLength(handle);
	if(n > 0)
	{
		n++;
		if(caption = (PTSTR)LocalHeapAlloc(sizeof(TCHAR) * n))
		{
			GetWindowText(handle, caption, n);
		}
	}
	else
	{
		caption = (PTSTR)LocalHeapAlloc(sizeof(TCHAR));
	}
	return caption;
}

void Window::SetCaption(PCTSTR new_caption)
{
	HANDLE t;
	API_WATCHDOG * awd;
	WM_SETTEXT_PARAMS * wst;
	TCHAR * new_caption_c;
	unsigned short l;

	awd = (API_WATCHDOG *)LocalHeapAlloc(sizeof(API_WATCHDOG));
	if(awd)
	{
		l = (unsigned short)_tcsnlen(new_caption, 0x7FFE) + 1;
		new_caption_c = (TCHAR *)LocalHeapAlloc(sizeof(TCHAR) * l);
		if(new_caption_c)
		{
			_tcsncpy_s(new_caption_c, l, new_caption, _TRUNCATE);
			wst = (WM_SETTEXT_PARAMS *)LocalHeapAlloc(sizeof(WM_SETTEXT_PARAMS));
			if(wst)
			{
				awd->proc = set_caption_thread;
				awd->parameter = wst;
				awd->timeout_sec = 30;

				wst->hWnd = handle;
				wst->lParam = new_caption_c;

				t = CreateThread(NULL, 0, api_watchdog, awd, 0, NULL);
				if(t)
				{
					CloseHandle(t);
				}
			}
			else
			{
				LocalHeapFree(new_caption_c);
			}
		}
		else
		{
			LocalHeapFree(awd);
		}
	}
}

PTSTR Window::GetClassName2()
{
	int n;
	TCHAR tmp_class_name[256];
	PTSTR class_name;

	if(n = GetClassName(handle, tmp_class_name, 256))
	{
		n++;
		if(class_name = (PTSTR)LocalHeapAlloc(sizeof(TCHAR) * n))
		{
			_tcsncpy_s(class_name, n, tmp_class_name, _TRUNCATE);
		}
	}
	else
	{
		class_name = (PTSTR)LocalHeapAlloc(sizeof(TCHAR));
	}

	return class_name;
}

void Window::GetDispInfo(NMLVDISPINFO * info,WindowList * list)
{
	char mode;
	if(info->item.mask & LVIF_IMAGE)
	{
		mode = list->GetViewMode();
		if(w_icon && ((mode == LVS_ICON) || (mode == LV_VIEW_TILE)))
		{
			ImageList_ReplaceIcon(list->GetImageList(false), 1, ((Window *)info->item.lParam)->GetIcon());
			info->item.iImage = 1;
		}
		else if(w_icon_small && (mode != LVS_ICON))
		{
			ImageList_ReplaceIcon(list->GetImageList(true), 1, ((Window *)info->item.lParam)->GetCaptionIcon());
			info->item.iImage = 1;
		}
		else
		{
			info->item.iImage = 0;
		}
	}
}

void Window::PopupThumbnail()
{
#if _WIN32_WINNT >= 0x600 && defined(ENABLE_THUMBNAIL)
	thumb->Popup();
#endif
}

HICON Window::get_icon_handle(bool small_icon)
{
	HICON icon;

	icon = NULL;

	if(small_icon)
	{
		if(SendMsg(WM_GETICON, ICON_SMALL, 0, 100,(PDWORD_PTR)&icon))
		{
			if(icon)
			{
				return icon;
			}
		}
#if _WIN32_WINNT >= 0x501
		if(SendMsg(WM_GETICON, ICON_SMALL2, 0, 100, (PDWORD_PTR)&icon))
		{
			if(icon)
			{
				return icon;
			}
		}
#endif
	}
	else
	{
		if(SendMsg(WM_GETICON, ICON_BIG, 0, 100, (PDWORD_PTR)&icon))
		{
			if(icon)
			{
				return icon;
			}
		}
	}

	if(icon = (HICON)GetClassLongPtr(handle, small_icon ? GCLP_HICONSM : GCLP_HICON))
	{
		return icon;
	}

	return NULL;
}

DWORD WINAPI Window::set_caption_thread(LPVOID lpParameter)
{
	SendMessage(((const WM_SETTEXT_PARAMS *)lpParameter)->hWnd, WM_SETTEXT, 0, (LPARAM)((const WM_SETTEXT_PARAMS *)lpParameter)->lParam);
	return 0;
}

DWORD WINAPI Window::set_window_longptr_thread(LPVOID lpParameter)
{
	DWORD old_style, new_style;

	if(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->nIndex == GWL_STYLE)
	{
		new_style = (DWORD)(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->dwNewLong & 0xFFFFFFFFUL);
		old_style = (DWORD)GetWindowLongPtr(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->hWnd,GWL_STYLE);
		if((new_style & WS_MAXIMIZE) && !(old_style & WS_MAXIMIZE))
		{
			ShowWindow(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->hWnd, SW_MAXIMIZE);
		}
		else if((new_style & WS_MINIMIZE) && !(old_style & WS_MINIMIZE))
		{
			ShowWindow(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->hWnd, SW_MINIMIZE);
		}
		else
		{
			ShowWindow(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->hWnd, SW_RESTORE);
		}
		new_style &= ~(WS_MAXIMIZE | WS_MINIMIZE);
	}
	else if(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->nIndex == GWL_EXSTYLE)
	{
		new_style = (DWORD)(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->dwNewLong & 0xFFFFFFFFUL);
		old_style = (DWORD)GetWindowLongPtr(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->hWnd, GWL_EXSTYLE);
		if((new_style & WS_EX_TOPMOST) && !(old_style & WS_EX_TOPMOST))
		{
			SetWindowPos(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
		}
		else if(!(new_style & WS_EX_TOPMOST) && (old_style & WS_EX_TOPMOST))
		{
			SetWindowPos(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
		}
		new_style &= ~WS_EX_TOPMOST;
	}

	SetWindowLongPtr(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->hWnd, ((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->nIndex, new_style);
	SetWindowPos(((const SET_WINDOW_LONGPTR_PARAMS *)lpParameter)->hWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
	return 0;
}

DWORD WINAPI Window::api_watchdog(LPVOID lpParameter)
{
	HANDLE t;

	t = CreateThread(NULL, 0, ((API_WATCHDOG *)lpParameter)->proc, ((API_WATCHDOG *)lpParameter)->parameter, 0, NULL);
	if(t)
	{
		while(WaitForSingleObject(t, 1000) != WAIT_OBJECT_0)
		{
			if(!--(((API_WATCHDOG *)lpParameter)->timeout_sec))
			{
				if(WaitForSingleObject(t, 0) != WAIT_OBJECT_0)
				{
					TerminateThread(t, 0);
					break;
				}
			}
		}
		CloseHandle(t);
	}
	if(((API_WATCHDOG *)lpParameter)->parameter)
	{
		LocalHeapFree(((API_WATCHDOG *)lpParameter)->parameter);
	}
	LocalHeapFree(lpParameter);
	return 0;
}

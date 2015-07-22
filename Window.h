#pragma once

#include <Windows.h>
#include <CommCtrl.h>

#if _WIN32_WINNT >= 0x600 && defined(ENABLE_THUMBNAIL)
#include "Thumbnail.h"
#endif

struct WM_SETTEXT_PARAMS
{
	HWND hWnd;
	LPCTSTR lParam;
};
typedef struct WM_SETTEXT_PARAMS WM_SETTEXT_PARAMS;

struct SET_WINDOW_LONGPTR_PARAMS
{
	HWND hWnd;
	int nIndex;
	LONG_PTR dwNewLong;
};
typedef struct SET_WINDOW_LONGPTR_PARAMS SET_WINDOW_LONGPTR_PARAMS;

struct API_WATCHDOG
{
	LPTHREAD_START_ROUTINE proc;
	void * parameter;
	unsigned int timeout_sec;
};
typedef struct API_WATCHDOG API_WATCHDOG;

class Window
{
private:
	HWND handle;
	HWND parent_handle;
	unsigned char depth;
	HICON w_icon;
	HICON w_icon_small;
	HICON e_icon;
	INT64 item_time_stamp;
	DWORD process;
#if _WIN32_WINNT >= 0x600 && defined(ENABLE_THUMBNAIL)
	class Thumbnail * thumb;
#endif
	bool is_unicode_window;

public:
	Window(HWND);
	~Window();

	bool IsUnicodeWindow() { return is_unicode_window; }
	DWORD GetProcess(){return process;}
	INT64 GetTimeStamp(){return item_time_stamp;}
	HWND GetHandle(){return handle;}
	HWND GetParentHandle(){return parent_handle;}
	unsigned char GetDepth(){return depth;}
	HICON GetIcon(){return w_icon;}
	HICON GetCaptionIcon(){return w_icon_small;}
	HICON GetProcessIcon(){return e_icon;}

	INT_PTR SendMsg(UINT, WPARAM, LPARAM);
	bool SendMsg(UINT, WPARAM, LPARAM, UINT, DWORD_PTR *);
	void PostMsg(UINT, WPARAM, LPARAM);

	void UpdateIcon();

	void GetWindowLocation(POINT *, SIZE *);
	void GetClientLocation(POINT *, SIZE *);
	void SetWindowLocation(const POINT *, const SIZE *);
	void SetClientLocation(const POINT *, const SIZE *);
	void Move(const POINT *);
	void ClientMove(const POINT *);
	void Resize(const SIZE *);
	void ClientResize(const SIZE *);

	DWORD GetStyle();
	void SetStyle(DWORD);
	DWORD GetExtendStyle();
	void SetExtendStyle(DWORD);

	void Iconic();
	void Maximize();
	void Restore();
	void BringToTop();
	void SinkToBottom();

	unsigned int GetCaptionLength();
	PTSTR GetCaption();
	void SetCaption(PCTSTR);
	PTSTR GetClassName2();

	void GetDispInfo(NMLVDISPINFO *,class WindowList *);

	void PopupThumbnail();

	static DWORD WINAPI set_caption_thread(LPVOID);
	static DWORD WINAPI set_window_longptr_thread(LPVOID);
	static DWORD WINAPI api_watchdog(LPVOID);

private:
	HICON get_icon_handle(bool);
};

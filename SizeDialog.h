#ifndef SIZEDIALOG_H
#define SIZEDIALOG_H

#pragma once

#include <stddef.h>
#include <windows.h>

typedef void (CALLBACK *SIZEDIALOGAPPLY)(void*,ULONG,const POINT*,const SIZE*,const TCHAR*);

struct ENUMMONITORPARAM
{
	HWND combobox;
	HMONITOR current_monitor;
};
typedef struct ENUMMONITORPARAM ENUMMONITORPARAM;

struct SIZEDIALOGPARAM
{
	HWND target;
	SIZEDIALOGAPPLY callback;
	void *callback_param;
};
typedef struct SIZEDIALOGPARAM SIZEDIALOGPARAM;

// Flags for SIZEDIALOGAPPLY callback
#define SDAF_CLIENT_POS  0x1UL
#define SDAF_CLIENT_SIZE 0x2UL
#define SDAF_SETCAPTION  0x4UL

// Internal flags for calc_window_pos function
#define CWPF_UP         0x01UL
#define CWPF_MIDDLE     0x02UL
#define CWPF_DOWN       0x03UL
#define CWPF_YPOS_MASK  0x03UL
#define CWPF_LEFT       0x04UL
#define CWPF_CENTER     0x08UL
#define CWPF_RIGHT      0x0CUL
#define CWPF_XPOS_MASK  0x0CUL
#define CWPF_FULLWIDTH  0x10UL
#define CWPF_FULLHEIGHT 0x20UL

class SizeDialog
{
private:
	long p_ref;
	HWND p_dialog;
	HWND p_parent;
	HWND p_caption;
	HWND p_xpos;
	HWND p_ypos;
	HWND p_width;
	HWND p_height;
	HWND p_monitor;
	HWND p_target;
	POINT p_pos;
	SIZE p_size;
	POINT p_client_pos;
	SIZE p_client_size;
	TCHAR *p_caption_text;
	SIZEDIALOGAPPLY p_callback;
	void *p_callback_param;
	bool p_do_apply;
	bool p_is_client_pos;
	bool p_is_client_size;

private:
	SizeDialog(HWND);
	~SizeDialog();

public:
	// COMっぽく実装してみるが、QueryInterfaceは無かったことになった。
	intptr_t AddRef();
	intptr_t Release();

	static INT_PTR CALLBACK DialogProc(HWND,UINT,WPARAM,LPARAM);

private:
	// メッセージハンドラ
	void do_destroy();
	void do_notify(NMHDR*);
	void do_initdialog(LPARAM);
	void do_command(WORD,WORD,HWND);
	// 内部関数
	void calc_window_position(ULONG,HMONITOR,RECT*);
	static BOOL CALLBACK enum_monitors(HMONITOR,HDC,LPRECT,LPARAM);
};

#endif

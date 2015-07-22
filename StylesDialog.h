#pragma once

#include <stddef.h>
#include <Windows.h>
#include <CommCtrl.h>

typedef void (CALLBACK * STYLEDIALOGAPPLY)(void *, DWORD, DWORD);

struct STYLEBITMAP
{
	PCTSTR name;
	DWORD style_bit;
};
typedef struct STYLEBITMAP STYLEBITMAP;

struct STYLESDIALOGPARAM
{
	DWORD style;
	DWORD extend_style;
	STYLEDIALOGAPPLY callback;
	void *callback_param;
};
typedef struct STYLESDIALOGPARAM STYLESDIALOGPARAM;

class StylesDialog
{
private:
	intptr_t p_ref;
	HWND p_dialog;
	HWND p_parent;
	HWND p_style_list;
	HWND p_extend_style_list;
	DWORD p_style;
	DWORD p_extend_style;
	STYLEDIALOGAPPLY p_callback;
	void * p_callback_param;
	bool p_do_apply;
	static const STYLEBITMAP basic_window_styles[16];
	static const STYLEBITMAP extend_window_styles[16];
	static const STYLEBITMAP extend_window_styles_nt5[4];
	static const STYLEBITMAP extend_window_styles_nt51[1];
	static const LVCOLUMN dummycolumn;

private:
	StylesDialog(HWND);
	~StylesDialog();

public:
	// COMっぽく実装してみるが、QueryInterfaceは無かったことになった。
	intptr_t AddRef();
	intptr_t Release();

	static INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);

private:
	void do_destroy();
	void do_notify(NMHDR *);
	void do_initdialog(LPARAM);
	void do_timer(UINT_PTR);
	//void do_command(WORD, WORD, HWND);
	void get_styles(DWORD *,DWORD *);
	static DWORD get_checked_styles(HWND);
	static void add_styles(HWND, DWORD, const STYLEBITMAP[], unsigned char);
};

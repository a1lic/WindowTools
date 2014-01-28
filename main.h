#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include "WindowList.h"
#include "WindowStatusBar.h"

struct MAINWINDOW__TOGGLE
{
	bool ime_ui;
	bool cursor_shadow;
	bool hot_tracking;
	bool window_shadow;
	bool monitor_power;
	bool monitor_power_low;
	bool prevent_power_save;
	bool cursor_vanish;
	bool pause_update;
};

class MainWindow
{
private:
	HINSTANCE p_instance;
	int p_show_window;
	HWND p_window;
	//POINT p_pos;
	//SIZE p_size;
	WindowList *p_list;
	WindowStatusBar *p_status;
	HMENU p_menu;
	HMENU p_window_menu;
	POINT p_min_client_area; //MINMAXINFO.ptMinTrackSizeに代入するためPOINTとする
	ULONG p_timer_res;
	unsigned int p_item_count;
	struct MAINWINDOW__TOGGLE p_toggle;

public:
	MainWindow(HINSTANCE);
	~MainWindow();
	HWND ShowWindow2(int);
	static ATOM RegisterClass2(const WNDCLASSEX*);

private:
	bool on_create();
	void on_destroy();
	void on_size(unsigned int,unsigned int);
	void on_winini_change();
	void on_get_minmax_info(MINMAXINFO*);
	bool on_contextmenu(HWND,unsigned int,unsigned int);
	void on_command(unsigned int,unsigned int,HWND);
	void on_timer(UINT_PTR);
	void on_enter_menu_loop();
	static LRESULT CALLBACK window_proc(HWND,UINT,WPARAM,LPARAM);
	static const WNDCLASSEX window_class_t;

};

#endif

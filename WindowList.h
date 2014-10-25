#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <windows.h>
#include <commctrl.h>
#include "Window.h"

#if !defined(_UXTHEME_H)
typedef HRESULT (WINAPI SETWINDOWTHEME)(HWND, LPCWSTR, LPCWSTR);
#endif

enum WINDOWLIST_SORT_MODE : unsigned char
{
	WLSM_NOSORT,WLSM_ASC,WLSM_DSC,WLSM_ENDNUM
};
//typedef enum WINDOWLIST_SORT_MODE WINDOWLIST_SORT_MODE;

struct WINDOWLIST_ENUM_CHILDREN
{
	class WindowList *_this;
	unsigned char depth;
	int insert_index;
};
typedef struct WINDOWLIST_ENUM_CHILDREN WINDOWLIST_ENUM_CHILDREN;

class WindowList
{
private:
	HWND listview;
	HWND parent;
	HANDLE lock;
	HIMAGELIST normal_icons;
	HIMAGELIST caption_icons;
	DWORD process_id;
	bool stop_update;
	WINDOWLIST_SORT_MODE sort_mode;
	char view_mode;
	bool show_no_title_window;
	int sort_column;
	int sorted_column;
	TCHAR tmp[2][1024];
	HMENU sort_menu;
	HCURSOR cursor_no;
	DWORD styles_mask;
	DWORD extend_styles_mask;
	HWND target_parent;

public:
	WindowList(HWND,UINT,HWND);
	~WindowList();

	HWND GetWindowHandle(){return listview;}
	bool IsShowNoTitleWindow() {return show_no_title_window;}
	bool ShowNoTitleWindow() {return (show_no_title_window = !show_no_title_window);}
	bool ShowNoTitleWindow(bool f) {return (show_no_title_window = f);}
	int GetSortColumn() {return sort_column;};
	DWORD GetStylesMask() {return styles_mask;}
	void SetStylesMask(DWORD mask) {styles_mask = mask;}
	void AddStylesMask(DWORD mask) {styles_mask |= mask;}
	void ClearStylesMask(DWORD mask) {styles_mask &= ~mask;}
	void ToggleStylesMask(DWORD mask) {styles_mask ^= mask;}
	DWORD GetExtendStylesMask() {return extend_styles_mask;}
	void SetExtendStylesMask(DWORD mask) {extend_styles_mask = mask;}
	void AddExtendStylesMask(DWORD mask) {extend_styles_mask |= mask;}
	void ClearExtendStylesMask(DWORD mask) {extend_styles_mask &= ~mask;}
	void ToggleExtendStylesMask(DWORD mask) {extend_styles_mask ^= mask;}
	HWND GetTargetWindow() {return NULL;}
	void SetTargetWindow(HWND target) {}

	HWND GetHandle();
	void GiveFocus();
	char GetViewMode();
	void SetViewMode(char);
	void Update();
	void BringToTop();
	void SinkToBottom();
	void ShowProperty();
	HIMAGELIST GetImageList(bool);
	LRESULT Notify(const NMHDR*);
	void Sort(int);
	void Sort(int,WINDOWLIST_SORT_MODE);
	void EndDragMode();
	unsigned int GetItemCount();
	void HandleContextMenu(HMENU);

private:
	void delete_all_items();
	void delete_invalid_items();
	int find_next_selected_item(int,Window**);
	int find_window_item(HWND,Window**);
	int add(Window*);
	LPARAM item_param(int);
	bool check_style(HWND,DWORD*,DWORD*);
	void get_info_tip(NMLVGETINFOTIP*);
	static BOOL CALLBACK update_child_list(HWND,LPARAM);
	static BOOL CALLBACK update_window_list(HWND,LPARAM);
	static int CALLBACK sort_items(LPARAM,LPARAM,LPARAM);

	static const LVCOLUMN columns[7];
#if _WIN32_WINNT >= 0x501
	static const LVGROUP grp_unicode;
	static const LVGROUP grp_ansi;
	static const UINT show_columns[4];
	static const LVTILEVIEWINFO tile_info;
	static const LVTILEINFO tile_info2;
#if _WIN32_WINNT >= 0x600
	static const int show_columns_formats[4];
#endif
#endif
};
#endif

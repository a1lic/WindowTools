#ifndef WINDOWPROP_H
#define WINDOWPROP_H

#include <windows.h>
#include "Window.h"

class WindowConfig
{
private:
	class Window *item;
	HICON icon_dup;
public:
	WindowConfig(class Window*);
	//~WindowProp();
	void Config(HWND);

private:

	void get_styles(DWORD*,DWORD*);
	static void CALLBACK set_size(void*,ULONG,const POINT*,const SIZE*,const TCHAR*);
	static void CALLBACK set_style(void*,DWORD,DWORD);
	static const PROPSHEETHEADER psht;
	static const PROPSHEETPAGE pst_size;
	static const PROPSHEETPAGE pst_style;
};

#endif /* !WINDOWPROP_H */

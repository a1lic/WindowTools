#include <windows.h>
#include <commctrl.h>

Toolbar::Toolbar(HWND parent,UINT id)
{
	HINSTANCE instance;

	instance = (HINSTANCE)GetWindowLongPtr(parent,GWLP_HINSTANCE);
	p_statusbar = CreateWindowEx(0,TOOLBARCLASSNAME,NULL,WS_CHILD | WS_VISIBLE,0,0,0,0,parent,(HMENU)id,instance,NULL);

	
}

Toolbar::~Toolbar()
{
	SendMessage(p_toolbar,WM_CLOSE,0,0);
}

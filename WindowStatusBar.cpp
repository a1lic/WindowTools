#include <Windows.h>
#include <CommCtrl.h>
#include "WindowStatusBar.h"

WindowStatusBar::WindowStatusBar(HWND parent, UINT id)
{
	HINSTANCE instance;

	instance = (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE);
	p_statusbar = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, parent, reinterpret_cast<HMENU>(static_cast<intptr_t>(id)), instance, NULL);

	if(p_statusbar)
	{
		SendMessage(p_statusbar, SB_SETPARTS, 3, (LPARAM)width_list);
	}
}

WindowStatusBar::~WindowStatusBar()
{
	SendMessage(p_statusbar, WM_CLOSE, 0, 0);
}

unsigned int WindowStatusBar::GetBarHeight()
{
	RECT rc;

	memset(&rc, 0, sizeof(RECT));
	GetWindowRect(p_statusbar, &rc);
	return rc.bottom - rc.top;
}

void WindowStatusBar::SetText(unsigned char panel, PCTSTR text)
{
	SendMessage(p_statusbar, SB_SETTEXT, panel, (LPARAM)text);
}

const int WindowStatusBar::width_list[3] = {180,360,-1};

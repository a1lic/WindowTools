#pragma once

#include <Windows.h>

class WindowStatusBar
{
private:
	HWND p_statusbar;

public:
	WindowStatusBar(HWND,UINT);
	~WindowStatusBar();

	HWND GetWindowHandle() {return p_statusbar;}
	unsigned int GetBarHeight();
	void SetText(unsigned char, PCTSTR);

private:
	static const int width_list[3];
};

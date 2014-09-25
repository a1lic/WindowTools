#pragma once
#include <tchar.h>
#include <Windows.h>

#define CONFIG_COORD_LEFT  TEXT("左")
#define CONFIG_COORD_TOP   TEXT("上")
#define CONFIG_SIZE_WIDTH  TEXT("幅")
#define CONFIG_SIZE_HEIGHT TEXT("嵩")

class ApplicationConfig
{
protected:
	HKEY LocalRegistry;
public:
	ApplicationConfig();
	ApplicationConfig(const TCHAR * const FileName);
	~ApplicationConfig();
	void GetWindowPosition(POINT * const Coord);
	void SetWindowPosition(const POINT * const Coord);
	void GetWindowSize(SIZE * const Metric);
	void SetWindowSize(const SIZE * const Metric);
protected:
	bool ReadDWORD(const TCHAR * const Name, DWORD * const Value);
	void WriteDWORD(const TCHAR * const Name, const DWORD Value);
};

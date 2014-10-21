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
	bool Read8(const TCHAR * const Name, unsigned char * const Value);
	void Write8(const TCHAR * const Name, const unsigned char Value);
	bool Read16(const TCHAR * const Name, unsigned short * const Value);
	void Write16(const TCHAR * const Name, const unsigned short Value);
	bool Read32(const TCHAR * const Name, unsigned long * const Value);
	void Write32(const TCHAR * const Name, const unsigned long Value);
	bool Read64(const TCHAR * const Name, unsigned long long * const Value);
	void Write64(const TCHAR * const Name, const unsigned long long Value);
	bool Read(const TCHAR * const Name, unsigned char ** const Value, const size_t ValueSize, size_t * const ValueSizeGot);
	void Write(const TCHAR * const Name, unsigned char * const Value, const size_t ValueSize);
};

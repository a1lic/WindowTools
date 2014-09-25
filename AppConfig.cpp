#include <tchar.h>
#include <Windows.h>

#include "AppConfig.h"
#include "misc.h"

ApplicationConfig::ApplicationConfig()
{
	this->LocalRegistry = nullptr;
}

ApplicationConfig::ApplicationConfig(const TCHAR * const FileName)
{
	LONG LResult;

	LResult = ::RegLoadAppKey(FileName, &this->LocalRegistry, KEY_READ | KEY_WRITE, 0, 0);
	if(LResult != ERROR_SUCCESS)
	{
		::Debug(TEXT("RegLoadAppKey(%s) failed with %u.\n"), LResult);
		this->LocalRegistry = nullptr;
	}
}

ApplicationConfig::~ApplicationConfig()
{
	if(this->LocalRegistry != nullptr)
	{
		::RegCloseKey(this->LocalRegistry);
	}
}

void ApplicationConfig::GetWindowPosition(POINT * const Coord)
{
	DWORD Value;

	if(this->ReadDWORD(CONFIG_COORD_LEFT, &Value))
	{
		Coord->x = (LONG)Value;
	}
	if(this->ReadDWORD(CONFIG_COORD_TOP, &Value))
	{
		Coord->y = (LONG)Value;
	}
}

void ApplicationConfig::SetWindowPosition(const POINT * const Coord)
{
	this->WriteDWORD(CONFIG_COORD_LEFT, Coord->x);
	this->WriteDWORD(CONFIG_COORD_TOP, Coord->y);
}

void ApplicationConfig::GetWindowSize(SIZE * const Metric)
{
	DWORD Value;

	if(this->ReadDWORD(CONFIG_SIZE_WIDTH, &Value))
	{
		Metric->cx = (LONG)Value;
	}
	if(this->ReadDWORD(CONFIG_SIZE_HEIGHT, &Value))
	{
		Metric->cy = (LONG)Value;
	}
}

void ApplicationConfig::SetWindowSize(const SIZE * const Metric)
{
	this->WriteDWORD(CONFIG_SIZE_WIDTH, Metric->cx);
	this->WriteDWORD(CONFIG_SIZE_HEIGHT, Metric->cy);
}

bool ApplicationConfig::ReadDWORD(const TCHAR * const Name, DWORD * const Value)
{
	LONG LResult;
	DWORD ValueType;
	DWORD ValueSize;
	DWORD ValueDWORD;

	if(this->LocalRegistry == nullptr)
	{
		return false;
	}

	ValueSize = sizeof(DWORD);
	LResult = ::RegQueryValueEx(this->LocalRegistry, Name, nullptr, &ValueType, reinterpret_cast<LPBYTE>(&ValueDWORD), &ValueSize);
	if(LResult == ERROR_SUCCESS)
	{
		*Value = ValueDWORD;
	}

	return (LResult == ERROR_SUCCESS);
}

void ApplicationConfig::WriteDWORD(const TCHAR * const Name, const DWORD Value)
{
	LONG LResult;

	if(this->LocalRegistry == nullptr)
	{
		return;
	}

	LResult = ::RegSetValueEx(this->LocalRegistry, Name, 0, REG_DWORD, reinterpret_cast<const BYTE *>(&Value), sizeof(DWORD));
	if(LResult != ERROR_SUCCESS)
	{
		::Debug(TEXT("Can not store value at %s with %u (%u).\n"), Name, Value, LResult);
	}
}

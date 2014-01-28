#include <ntstatus.h>
#define WIN32_NO_STATUS
/*#include <assert.h>*/
#include <tchar.h>
#include <windows.h>
#include "dyntdll.h"

typedef NTSTATUS (NTAPI * NTSETTIMERRESOLUTION)(ULONG, BOOLEAN, PULONG);
typedef NTSTATUS (NTAPI * NTQUERYTIMERRESOLUTION)(PULONG, PULONG, PULONG);

HMODULE                ntdll;
NTSETTIMERRESOLUTION   _NtSetTimerResolution;
NTQUERYTIMERRESOLUTION _NtQueryTimerResolution;

void InitializeNtDllRuntimeLinker()
{
	ntdll = LoadLibrary(TEXT("NTDLL.DLL"));
	if(ntdll)
	{
		_NtSetTimerResolution   = (NTSETTIMERRESOLUTION)GetProcAddress(ntdll, "ZwSetTimerResolution");
		_NtQueryTimerResolution = (NTQUERYTIMERRESOLUTION)GetProcAddress(ntdll, "ZwQueryTimerResolution");
	}
}

void UninitializeNtDllRuntimeLinker()
{
	_NtSetTimerResolution   = NULL;
	_NtQueryTimerResolution = NULL;
	FreeLibrary(ntdll);
}

NTSTATUS NTAPI NtSetTimerResolution(ULONG resolution, BOOLEAN set, PULONG previous_resolution)
{
	if(!_NtSetTimerResolution)
	{
		return STATUS_NOT_IMPLEMENTED;
	}
	return (*_NtSetTimerResolution)(resolution, set, previous_resolution);
}

NTSTATUS NTAPI NtQueryTimerResolution(PULONG rmin, PULONG rmax, PULONG rcurrent)
{
	if(!_NtQueryTimerResolution)
	{
		return STATUS_NOT_IMPLEMENTED;
	}
	return (*_NtQueryTimerResolution)(rmin, rmax, rcurrent);
}

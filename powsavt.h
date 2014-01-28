#ifndef POWSAVT_H
#define POWSAVT_H

#include <tchar.h>
#include <windows.h>

#if !defined(EXTERN_C)
#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

struct POWSAVT_DLG_PARAM
{
	HINSTANCE Instance;
	HWND Parent;
};
typedef struct POWSAVT_DLG_PARAM POWSAVT_DLG_PARAM;

EXTERN_C void ShowPowerSaveTimerDialog(HWND,HINSTANCE);

#endif

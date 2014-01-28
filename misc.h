#ifndef MISC_H
#define MISC_H

#include <windows.h>

#if !defined(EXTERN_C)
#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

struct UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
};
typedef struct UNICODE_STRING UNICODE_STRING;
/* OS Feature bit mask */
#define OSF_MON_98         0x00000001L
#define OSF_GUI_NT5        0x00000002L
#define OSF_GUI_NT51       0x00000004L
#define OSF_LISTVIEW_NT51  0x00000100L
#define OSF_LISTVIEW_NT60  0x00000200L
#define OSF_OK             0x80000000L

/* Constant value for heap handle argument of CleanupListView function */
#define CRT_HEAP ((HANDLE)-1)

EXTERN_C HINSTANCE ntdll;
EXTERN_C union NTQUERYTIMERRESOLUTION {FARPROC FuncPointer; LONG (WINAPI *Invoke)(PULONG,PULONG,PULONG);} u_NtQueryTimerResolution;

EXTERN_C void Debug(PCTSTR, ...);
EXTERN_C int MessageBoxf(HWND,PCTSTR,DWORD,PCTSTR, ...);
EXTERN_C int ListViewGetSelectingIndex(HWND);
EXTERN_C LPARAM ListViewGetLPARAM(HWND,int);
EXTERN_C void ListViewUpdateItemString(HWND,int,int,PCTSTR);
EXTERN_C DWORD GetNtDDIVersionOfPlatform();
EXTERN_C BOOLEAN OSFeatureTest(DWORD);
EXTERN_C BOOLEAN GetSysParametersBoolean(int);
EXTERN_C void CheckMenuItem2(HMENU,int,BOOLEAN);
EXTERN_C void CenteringWindow(HWND,const POINT*);
EXTERN_C void CenteringWindowToCursor(HWND);
EXTERN_C void CenteringWindowToParent(HWND,HWND);
EXTERN_C ULONG QueryTimerResolutions(ULONG*,ULONG*,ULONG*);
EXTERN_C INT_PTR CALLBACK AboutDialogProc(HWND,UINT,WPARAM,LPARAM);
EXTERN_C BOOLEAN GetProcessNameFromId(DWORD,PTSTR,size_t);
EXTERN_C void CleanupListView(HWND,HANDLE);
EXTERN_C void GetClientRectAsScreenPos(HWND,RECT*);

/* _beginthreadexではなくCreateThreadで作成したスレッドのためのメモリアロケータ関数 */
EXTERN_C void LocalHeapInitialize();
EXTERN_C void LocalHeapDestroy();
EXTERN_C void* LocalHeapAlloc(DWORD);
EXTERN_C void* LocalHeapReAlloc(DWORD,void*);
EXTERN_C void LocalHeapFree(void*);

EXTERN_C void LocalHeapDump();
EXTERN_C void DumpHeapStatus(HANDLE heap);
EXTERN_C void DumpAllHeapsStatus();

#define TO_BOOLEAN(e) ((uintptr_t)(e) != 0)

#endif

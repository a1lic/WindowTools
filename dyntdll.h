#ifndef DYNTDLL_H
#define DYNTDLL_H
#pragma once

#if !defined(EXTERN_C)
#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

/* NTSTATUS is as long int, Must define NTSTATUS type before include ntstatus.h */
typedef LONG NTSTATUS;
/* NTAPI is as __stdcall */
#define NTAPI __stdcall

#include <ntstatus.h>
#ifndef WIN32_NO_STATUS
#define WIN32_NO_STATUS
#endif
#include <windows.h>

EXTERN_C void InitializeNtDllRuntimeLinker();
EXTERN_C void UninitializeNtDllRuntimeLinker();
EXTERN_C NTSTATUS NTAPI NtSetTimerResolution(ULONG, BOOLEAN, PULONG);
EXTERN_C NTSTATUS NTAPI NtQueryTimerResolution(PULONG, PULONG, PULONG);

#endif

#pragma once

#if defined(_M_ARM)
#if defined(_ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE)
#undef _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE
#endif
#define _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE 1
#endif

#define OEMRESOURCE
#define WIN32_LEAN_AND_MEAN

#if defined(_DLL)
// Cランタイム関数を呼び出すとき、スタブではなくlibのエクスポートを使用する
#if defined(_CRTIMP)
#undef _CRTIMP
#endif
#define _CRTIMP __declspec(dllimport)
#endif

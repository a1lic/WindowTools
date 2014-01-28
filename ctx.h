#ifndef CTX_H
#define CTX_H

#include <windows.h>

#if !defined(EXTERN_C)
#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

EXTERN_C HANDLE CreateContextFromManifest(HINSTANCE);

#endif

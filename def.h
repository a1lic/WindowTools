#pragma once
#include <stdlib.h>
// Allocate UTF-32 string buffer (use U prefix)
#define U32_ALLOC(c) (char32_t*)calloc(c, sizeof(char32_t))
// Allocate UTF-16 string buffer (use u prefix)
#define U16_ALLOC(c) (char16_t*)calloc(c, sizeof(char16_t))
#if defined(_NATIVE_WCHAR_T_DEFINED)
// Allocate wide-char string buffer (use L prefix)
#define WCHAR_ALLOC(c) (wchar_t*)calloc(c, sizeof(wchar_t))
#else
// Allocate typedef-ed wide-char string buffer (use L prefix)
#define WCHAR_ALLOC(c) (unsigned short*)calloc(c, sizeof(unsigned short))
#endif
// Allocate forced wide-char string buffer (use L prefix)
#define WCHAR_N_ALLOC(c) (__wchar_t*)calloc(c, sizeof(__wchar_t))
#if defined(_CHAR_UNSIGNED)
// Allocate string buffer (use u8 prefix or unprefix)
#define CHAR_ALLOC(c) (unsigned char*)calloc(c, sizeof(unsigned char))
#else
// Allocate ascii/multi bytes string buffer (use u8 prefix or unprefix)
#define CHAR_ALLOC(c) (char*)calloc(c, sizeof(char))
#endif
// Release UTF-32 string buffer
#define U32_FREE(c) free(c)
// Release UTF-16 string buffer
#define U16_FREE(c) free(c)
// Release wide-char string buffer
#define WCHAR_FREE(c) free(c)
// Release ascii/multi bytes string buffer
#define CHAR_FREE(c) free(c)
#if defined(UNICODE)
// Allocate TCHAR buffer
#define TCHAR_ALLOC WCHAR_ALLOC
// Release TCHAR buffer
#define TCHAR_FREE WCHAR_FREE
#else
// Allocate TCHAR buffer
#define TCHAR_ALLOC CHAR_ALLOC
// Release TCHAR buffer
#define TCHAR_FREE CHAR_FREE
#endif
// Allocate array of struct
#define STRUCT_ARRAY_ALLOC(s, e) (s*)calloc(e, sizeof(s))
// Allocate buffer for struct
#define STRUCT_ALLOC(s) (s*)calloc(1, sizeof(s))
// Release struct buffer
#define STRUCT_FREE(s) free(s)
// Allocate array of type
#define ARRAY_ALLOC(t, e) (t*)calloc(e, sizeof(t))
// Release array
#define ARRAY_FREE(s) free(s)

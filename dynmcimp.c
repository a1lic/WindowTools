#include <tchar.h>
#include <windows.h>

HMODULE kernel32;

PVOID WINAPI IslEncodePointer(PVOID ptr)
{
	return (PVOID)(~(ULONG_PTR)ptr);
}
typedef PVOID (WINAPI ENCODEPOINTER)(PVOID);
#if defined(_WIN64)
ENCODEPOINTER *__imp_EncodePointer = IslEncodePointer;
#else
extern ENCODEPOINTER *_imp__EncodePointer;
#endif

PVOID WINAPI IslDecodePointer(PVOID ptr)
{
	return (PVOID)(~(ULONG_PTR)ptr);
}
typedef PVOID (WINAPI DECODEPOINTER)(PVOID);
#if defined(_WIN64)
DECODEPOINTER *__imp_DecodePointer = IslDecodePointer;
#else
extern DECODEPOINTER *_imp__DecodePointer;
#endif

void InitializeDelayedLoadEntryPoints()
{
	kernel32 = LoadLibrary(TEXT("KERNEL32.DLL"));
	if(kernel32)
	{
		__imp_EncodePointer = (ENCODEPOINTER*)GetProcAddress(kernel32, "EncodePointer");
		if(!__imp_EncodePointer)
		{
			__imp_EncodePointer = IslEncodePointer;
		}
		__imp_DecodePointer = (DECODEPOINTER*)GetProcAddress(kernel32, "DecodePointer");
		if(!__imp_DecodePointer)
		{
			__imp_DecodePointer = IslDecodePointer;
		}
	}
}

void FreeDelayedLoadEntryPoints()
{
	__imp_EncodePointer = IslEncodePointer;
	__imp_DecodePointer = IslDecodePointer;
	if(kernel32)
	{
		FreeLibrary(kernel32);
		kernel32 = NULL;
	}
}

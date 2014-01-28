#include <string.h>
#include <tchar.h>
#include <windows.h>
#include "ctx.h"

typedef HANDLE (WINAPI CREATEACTCTX)(ACTCTX*);

/*
指定した実行ファイルのリソースからマニフェストをファイルに書き出す
CreateActCtxに渡す構造体のlpSourceにFilePathを指定する
コンテキストを作成したらFilePathをDeleteFileで削除する
*/

BOOLEAN ExportManifest(HINSTANCE Module, PTSTR FilePath, size_t FilePathSize)
{
	TCHAR   TempPath[MAX_PATH], TempFilePath[MAX_PATH];
	void   *ResourceData;
	HANDLE  TempFile;
	HRSRC   Resource;
	DWORD   ResourceSize;

	/* Type=24、ID=2のリソース(DLL用マニフェスト) */
	Resource = FindResource((HMODULE)Module, MAKEINTRESOURCE(2), MAKEINTRESOURCE(24));
	if(!Resource)
	{
		/* Type=24、ID=1のリソース(EXE用マニフェスト) */
		Resource = FindResource((HMODULE)Module, MAKEINTRESOURCE(1), MAKEINTRESOURCE(24));
		if(!Resource)
		{
			/* どちらもない場合は諦める */
			return FALSE;
		}
	}
	ResourceSize = SizeofResource((HMODULE)Module, Resource);
	if(!ResourceSize)
	{
		return FALSE;
	}
	ResourceData = (void*)LoadResource((HMODULE)Module, Resource);
	if(!ResourceData)
	{
		return FALSE;
	}

	GetTempPath(MAX_PATH, TempPath);
	GetTempFileName(TempPath, TEXT("MF"), 0, TempFilePath);

	TempFile = CreateFile(TempFilePath, FILE_WRITE_DATA, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if(TempFile == INVALID_HANDLE_VALUE)
	{
		FreeResource((HGLOBAL)ResourceData);
		return FALSE;
	}

	WriteFile(TempFile, ResourceData, ResourceSize, &ResourceSize, NULL);
	CloseHandle(TempFile);
	_tcsncpy_s(FilePath, FilePathSize, TempFilePath, _TRUNCATE);

	FreeResource((HGLOBAL)ResourceData);
	return TRUE;
}

/*
指定したモジュールのマニフェストからコンテキストを生成する
*/
HANDLE CreateContextFromManifest(HINSTANCE Module)
{
	TCHAR         TempFile[MAX_PATH];
	ACTCTX        Context;
	HANDLE        Handle;
	HINSTANCE     Kernel32;
	CREATEACTCTX *DCreateActCtx;

	Kernel32 = LoadLibrary(TEXT("KERNEL32.DLL"));
	if(!Kernel32)
	{
		return INVALID_HANDLE_VALUE;
	}

	DCreateActCtx = (CREATEACTCTX*)GetProcAddress(Kernel32, "CreateActCtx");
	if(!DCreateActCtx)
	{
		FreeLibrary(Kernel32);
		return INVALID_HANDLE_VALUE;
	}


	if(ExportManifest(Module, TempFile, MAX_PATH))
	{
		memset(&Context, 0, sizeof(ACTCTX));
		Context.cbSize = sizeof(ACTCTX);
		Context.lpSource = TempFile;

		Handle = (*DCreateActCtx)(&Context);

		DeleteFile(TempFile);
	}
	else
	{
		Handle = INVALID_HANDLE_VALUE;
	}

	FreeLibrary(Kernel32);
	return Handle;
}

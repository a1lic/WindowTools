//#define LEGACY_USE_LISTBOX

#include <tchar.h>
#include <windows.h>
#include <commctrl.h>
#include "WindowConfig.h"
#include "Window.h"
#include "SizeDialog.h"
#include "StylesDialog.h"
#include "misc.h"
#include "resource.h"

WindowConfig::WindowConfig(class Window *item)
{
	this->item = item;
}

void WindowConfig::Config(HWND parent)
{

	PROPSHEETHEADER psh;
	PROPSHEETPAGE ps[2];
	SIZEDIALOGPARAM size_param;
	STYLESDIALOGPARAM style_param;
	HICON icon;

	if(!parent)
	{
		return;
	}
	if(!item)
	{
		return;
	}

	size_param.target = item->GetHandle();
	size_param.callback = set_size;
	size_param.callback_param = this;

	style_param.style = item->GetStyle();
	style_param.extend_style = item->GetExtendStyle();
	style_param.callback = set_style;
	style_param.callback_param = this;

	ps[0] = pst_size;
	ps[0].hInstance = (HINSTANCE)GetWindowLongPtr(parent,GWLP_HINSTANCE);
	ps[0].lParam = (LPARAM)&size_param;

	ps[1] = pst_style;
	ps[1].hInstance = ps[0].hInstance;
	ps[1].lParam = (LPARAM)&style_param;

#if 0
	ps[2] = pst_layer;
	ps[2].hInstance = ud->Arguments.Instance;
	ps[2].lParam = (LPARAM)&cfg;
#endif

	icon = item->GetCaptionIcon();
	icon_dup = icon ? (HICON)CopyImage(icon,IMAGE_ICON,0,0,0) : NULL;
	if(!icon_dup)
	{
		psh.dwFlags &= ~PSH_USEHICON;
	}

	psh = psht;
	psh.hwndParent = parent;
	psh.hInstance = NULL;
	psh.hIcon = icon_dup;
	psh.pszCaption = item->GetCaption();
	psh.ppsp = ps;

	PropertySheet(&psh);

	LocalHeapFree((void*)psh.pszCaption);
	if(icon_dup)
	{
		if(!DestroyIcon(icon_dup))
		{
			MessageBoxf(parent,NULL,MB_ICONWARNING,TEXT("アイコンハンドルの解放に失敗しました(%lu)"),GetLastError());
			if(IsDebuggerPresent())
			{
				__debugbreak();
			}
		}
	}
}

void CALLBACK WindowConfig::set_size(void *_this,ULONG flags,const POINT *pt,const SIZE *size,const TCHAR *newstr)
{
	if(flags & SDAF_CLIENT_POS)
	{
		((WindowConfig*)_this)->item->ClientMove(pt);
	}
	else
	{
		((WindowConfig*)_this)->item->Move(pt);
	}
	if(flags & SDAF_CLIENT_SIZE)
	{
		((WindowConfig*)_this)->item->ClientResize(size);
	}
	else
	{
		((WindowConfig*)_this)->item->Resize(size);
	}
	if(flags & SDAF_SETCAPTION)
	{
		((WindowConfig*)_this)->item->SetCaption(newstr);
	}
}

void CALLBACK WindowConfig::set_style(void *_this,DWORD style,DWORD exstyle)
{
	((WindowConfig*)_this)->item->SetStyle(style);
	((WindowConfig*)_this)->item->SetExtendStyle(exstyle);
}

const PROPSHEETHEADER WindowConfig::psht = {
	/* dwSize      */ sizeof(PROPSHEETHEADER),
	/* dwFlags     */ PSH_PROPSHEETPAGE | PSH_PROPTITLE | PSH_USEHICON | PSH_NOCONTEXTHELP /*| PSH_MODELESS*/,
	/* hwndParent  */ NULL,
	/* hInstance   */ NULL,
	/* hIcon       */ NULL,
	/* pszCaption  */ NULL,
	/* nPages      */ 2,
	/* nStartPage  */ 0,
	/* ppsp        */ NULL,
	/* pfnCallback */ NULL};

const PROPSHEETPAGE WindowConfig::pst_size = {
	/* dwSize            */ sizeof(PROPSHEETPAGE),
	/* dwFlags           */ 0,
	/* hInstance         */ NULL,
	/* pszTemplate       */ MAKEINTRESOURCE(IDD_WINDOW_POS),
	/* hIcon             */ NULL,
	/* pszTitle          */ NULL,
	/* pfnDlgProc        */ SizeDialog::DialogProc,
	/* lParam            */ 0,
	/* pfnCallback       */ NULL,
	/* pcRefParent       */ NULL,
	/* pszHeaderTitle    */ NULL,
	/* pszHeaderSubTitle */ NULL};

const PROPSHEETPAGE WindowConfig::pst_style = {
	/* dwSize            */ sizeof(PROPSHEETPAGE),
	/* dwFlags           */ 0,
	/* hInstance         */ NULL,
	/* pszTemplate       */ MAKEINTRESOURCE(IDD_WINDOW_STYLE),
	/* hIcon             */ NULL,
	/* pszTitle          */ NULL,
	/* pfnDlgProc        */ StylesDialog::DialogProc,
	/* lParam            */ 0,
	/* pfnCallback       */ NULL,
	/* pcRefParent       */ NULL,
	/* pszHeaderTitle    */ NULL,
	/* pszHeaderSubTitle */ NULL};

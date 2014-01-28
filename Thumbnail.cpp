#include <windows.h>
#include <dwmapi.h>
#include "Thumbnail.h"

Thumbnail::Thumbnail(HWND target,HINSTANCE instance)
{
	p_target = target;
	p_instance = instance;
	p_count = 1;
	p_thumbnail_id = NULL;
	p_popup = NULL;
	p_show = false;
}

Thumbnail::~Thumbnail()
{
}

uintptr_t Thumbnail::AddRef()
{
	return ++p_count;
}

uintptr_t Thumbnail::Release()
{
	if((--p_count) == 0)
	{
		DestroyWindow(p_popup);
		delete this;
	}
	return p_count;
}

void Thumbnail::Popup()
{
	HWND popup;
	if(p_show)
	{
		return;
	}

	popup = CreateWindowEx(0,TEXT("Window Tools Thumbnail"),NULL,WS_POPUP,0,0,266,266,NULL,NULL,p_instance,this);
	if(popup)
	{
		ShowWindow(popup,SW_SHOW);
	}
}

void Thumbnail::Hide()
{
	PostMessage(p_popup,WM_CLOSE,0,0);
}

ATOM Thumbnail::RegisterClass2(const WNDCLASSEX *wc)
{
	WNDCLASSEX window_class;

	window_class = window_class_t;
	window_class.hInstance = wc->hInstance;

	return RegisterClassEx(&window_class);
}

void Thumbnail::on_create()
{
	HRESULT r;
	DWM_THUMBNAIL_PROPERTIES props;

	r = DwmRegisterThumbnail(p_popup,p_target,&p_thumbnail_id);
	if(r != S_OK)
	{
		p_thumbnail_id = NULL;
		return;
	}

	memset(&props,0,sizeof(DWM_THUMBNAIL_PROPERTIES));
	props.dwFlags = DWM_TNP_RECTDESTINATION;
	GetClientRect(p_popup,&props.rcDestination);

	DwmExtendFrameIntoClientArea(p_popup,&full_grass);
	DwmUpdateThumbnailProperties(p_thumbnail_id,&props);

	p_show = true;
	SetTimer(p_popup,1771104,10000,NULL);
}

void Thumbnail::on_destroy()
{
	HRESULT r;

	if(p_thumbnail_id)
	{
		r = DwmUnregisterThumbnail(p_thumbnail_id);
		if(r == S_OK)
		{
			p_thumbnail_id = NULL;
		}
	}
	p_show = false;
}

void Thumbnail::on_timer(UINT_PTR id)
{
	if(id == 1771104)
	{
		KillTimer(p_popup,1771104);
		PostMessage(p_popup,WM_CLOSE,0,0);
	}
}

LRESULT CALLBACK Thumbnail::window_proc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	Thumbnail *_this;
	LRESULT r;

	_this = (Thumbnail*)GetWindowLongPtr(hwnd,0);
	r = 0;

	switch(uMsg)
	{
	case WM_CREATE: // 0x0001
		_this = (Thumbnail*)((const CREATESTRUCT*)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd,0,(LONG_PTR)_this);
		_this->p_popup = hwnd;
		_this->on_create();
		break;

	case WM_DESTROY: // 0x0002
		_this->on_destroy();
		break;

	case WM_TIMER: // 0x112
		_this->on_timer((UINT_PTR)wParam);
		break;

	default:
		r = DefWindowProc(hwnd,uMsg,wParam,lParam);
		break;
	}

	return r;
}

const WNDCLASSEX Thumbnail::window_class_t = {
	/* cbSize        */ sizeof(WNDCLASSEX),
	/* style         */ 0,
	/* lpfnWndProc   */ Thumbnail::window_proc,
	/* cbClsExtra    */ 0,
	/* cbWndExtra    */ sizeof(void*), /* ポインタを格納する領域を確保する */
	/* hInstance     */ NULL,
	/* hIcon         */ NULL,
	/* hCursor       */ NULL,
	/* hbrBackground */ 0, //(HBRUSH)(1 + COLOR_3DFACE),
	/* lpszMenuName  */ NULL,
	/* lpszClassName */ TEXT("Window Tools Thumbnail"),
	/* hIconSm       */ 0};

const MARGINS Thumbnail::full_grass = {
	/* cxLeftWidth    */ -1,
	/* cxRightWidth   */ -1,
	/* cyTopHeight    */ -1,
	/* cyBottomHeight */ -1};

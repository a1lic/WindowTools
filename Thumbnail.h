#ifndef THUMBNAIL_H
#define THUMBNAIL_H
#pragma once

#include <crtdefs.h>
#include <windows.h>
#include <dwmapi.h>

class Thumbnail
{
private:
	HWND p_target;
	HINSTANCE p_instance;
	uintptr_t p_count;
	HTHUMBNAIL p_thumbnail_id;
	HWND p_popup;
	bool p_show;

public:
	Thumbnail(HWND,HINSTANCE);
private:
	~Thumbnail();
public:
	uintptr_t AddRef();
	uintptr_t Release();
	void Popup();
	void Hide();
	static ATOM RegisterClass2(const WNDCLASSEX*);

private:
	void on_create();
	void on_destroy();
	void on_timer(UINT_PTR);
	static LRESULT CALLBACK window_proc(HWND,UINT,WPARAM,LPARAM);
	static const WNDCLASSEX window_class_t;
	static const MARGINS full_grass;
};

#endif

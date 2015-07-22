#include <crtdefs.h>
#include <crtdbg.h>
#include <Windows.h>
#include "misc.h"
#include "resource.h"
#include "StylesDialog.h"

StylesDialog::StylesDialog(HWND hwnd)
{
	Debug(TEXT("StylesDialog: Initialize"));
	p_ref = 1;

	p_dialog = hwnd;
	p_parent = (HWND)GetWindowLongPtr(p_dialog, GWLP_HWNDPARENT);
}

StylesDialog::~StylesDialog()
{
	Debug(TEXT("StylesDialog: Destroy"));
	_ASSERT(p_ref == 0);
}

intptr_t StylesDialog::AddRef()
{
	return ++p_ref;
}

intptr_t StylesDialog::Release()
{
	p_ref--;
	if(p_ref == 0)
	{
		delete this;
	}
	return p_ref;
}

INT_PTR CALLBACK StylesDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	StylesDialog * _this;
	INT_PTR r;

#if defined(ZEROFILL_LOCAL_VARS)
	_this = NULL;
	r = 0;
#endif

	switch(uMsg)
	{
#if 0
	case WM_CLOSE:
		_this = (StylesDialog *)GetProp(hwndDlg, TEXT("DialogClassPtr"));
		_ASSERT(_this);
		_this->do_close();
		r = TRUE;
		break;
#endif

	case WM_DESTROY:
		_this = (StylesDialog *)GetProp(hwndDlg, TEXT("DialogClassPtr"));
		_ASSERT(_this);
		_this->do_destroy();
		r = TRUE;
		break;

	case WM_NOTIFY:
		_this = (StylesDialog *)GetProp(hwndDlg, TEXT("DialogClassPtr"));
		//_ASSERT(_this);
		if(_this)
		{
			_this->do_notify((NMHDR *)lParam);
			r = TRUE;
		}
		else
		{
			r = FALSE;
		}
		break;

	case WM_INITDIALOG:
		// プロパティシートとして作成した場合、LPARAMはPROPSHEETPAGE構造体となる。
		_this = new StylesDialog(hwndDlg);
		//_ASSERT(_this);
		_this->do_initdialog(((const PROPSHEETPAGE *)lParam)->lParam);
		r = TRUE;
		break;

	case WM_TIMER:
		_this = (StylesDialog *)GetProp(hwndDlg, TEXT("DialogClassPtr"));
		_ASSERT(_this);
		_this->do_timer((UINT_PTR)wParam);
		r = TRUE;
		break;

#if 0
	case WM_COMMAND:
		_this = (StylesDialog *)GetProp(hwndDlg, TEXT("DialogClassPtr"));
		_ASSERT(_this);
		if(_this)
		{
			_this->do_command(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			r = TRUE;
		}
		else
		{
			r = FALSE;
		}
		break;
#endif

	default:
		r = FALSE;
		break;
	}
	return r;
}

void StylesDialog::do_destroy()
{
	KillTimer(p_dialog, 1771104);
	//CleanupListView(p_style_list, NULL);
	//CleanupListView(p_extend_style_list, NULL);
	RemoveProp(p_dialog, TEXT("DialogClassPtr"));
	Release();
}

void StylesDialog::do_initdialog(LPARAM l)
{
#if !defined(UNICODE)
	LVCOLUMNA dummycolumn2;
#endif

#if defined(ZEROFILL_LOCAL_VARS)
#if !defined(UNICODE)
	memset(&dummycolumn2, 0, sizeof(LVCOLUMNA));
#endif
#endif

	// DWLP_USERDATAでもいいが…
	SetProp(p_dialog, TEXT("DialogClassPtr"), (HANDLE)this);
	p_do_apply = false;

	p_style = ((const STYLESDIALOGPARAM *)l)->style;
	p_extend_style = ((const STYLESDIALOGPARAM *)l)->extend_style;
	p_callback = ((const STYLESDIALOGPARAM *)l)->callback;
	p_callback_param = ((const STYLESDIALOGPARAM *)l)->callback_param;

	// 各コントロールのウィンドウハンドルを保持しておく。
	p_style_list = GetDlgItem(p_dialog, IDC_STYLE);
	p_extend_style_list = GetDlgItem(p_dialog, IDC_EXSTYLE);

	// リストビューにチェックボックスを付ける。
	ListView_SetExtendedListViewStyleEx(p_style_list, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
	ListView_SetExtendedListViewStyleEx(p_extend_style_list, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

	// ダミー列を追加する。
#if !defined(UNICODE)
	// Unicode版ではない時はLVCOLUMNが読み書きできないとアクセス違反が発生するため、ローカル変数にコピーする。
	dummycolumn2 = dummycolumn;
	ListView_InsertColumn(p_style_list, 0, &dummycolumn2);
	ListView_InsertColumn(p_extend_style_list, 0, &dummycolumn2);
#else
	ListView_InsertColumn(p_style_list, 0, &dummycolumn);
	ListView_InsertColumn(p_extend_style_list, 0, &dummycolumn);
#endif

	// 基本スタイルと拡張スタイルを追加する。
	add_styles(p_style_list, p_style,basic_window_styles, 16);
	add_styles(p_extend_style_list, p_extend_style,extend_window_styles, 16);

	if(OSFeatureTest(OSF_GUI_NT5))
	{
		// Windows 2000で使用できるスタイルを追加する。
		add_styles(p_extend_style_list, p_extend_style, extend_window_styles_nt5, 4);
	}

	if(OSFeatureTest(OSF_GUI_NT51))
	{
		// Windows XPで使用できるスタイルを追加する。
		add_styles(p_extend_style_list, p_extend_style, extend_window_styles_nt51, 1);
	}

	// ダミー列の幅を調整する。
	ListView_SetColumnWidth(p_style_list, 0, LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(p_extend_style_list, 0, LVSCW_AUTOSIZE_USEHEADER);

	// リストビューの変更を監視するタイマーを設定する。
	SetTimer(p_dialog, 1771104, 100, NULL);
}

void StylesDialog::do_notify(NMHDR * notify_header)
{
	DWORD style, exstyle;

#if defined(ZEROFILL_LOCAL_VARS)
	style = 0;
	exstyle = 0;
#endif

	if(notify_header->code == PSN_APPLY)
	{
		if(p_do_apply)
		{
			get_styles(&style, &exstyle);
			// コールバック関数を登録している場合はそれを呼び出す。
			if(p_callback)
			{
				(*p_callback)(p_callback_param, style, exstyle);
			}
			p_style = style;
			p_extend_style = exstyle;
			p_do_apply = false;
		}
	}
}

void StylesDialog::do_timer(UINT_PTR id)
{
	DWORD style, exstyle;

#if defined(ZEROFILL_LOCAL_VARS)
	style = 0;
	exstyle = 0;
#endif

	if(id == 1771104)
	{
		get_styles(&style, &exstyle);
		if(!((style == p_style) && (exstyle == p_extend_style)))
		{
			SendMessage(p_parent, PSM_CHANGED, (WPARAM)p_dialog, 0);
			p_do_apply = true;
		}
		else
		{
			SendMessage(p_parent, PSM_UNCHANGED, (WPARAM)p_dialog, 0);
			p_do_apply = false;
		}
	}
}

void StylesDialog::get_styles(DWORD * style,DWORD * exstyle)
{
	*style = get_checked_styles(p_style_list);
	*exstyle = get_checked_styles(p_extend_style_list);
}

DWORD StylesDialog::get_checked_styles(HWND listview)
{
	LPARAM lparam;
	DWORD style;
	unsigned char i, count;

#if defined(ZEROFILL_LOCAL_VARS)
	lparam = 0;
	//style = 0;
	i = 0;
	count = 0;
#endif

	style = 0;
	count = (unsigned char)ListView_GetItemCount(listview);
	for(i = 0; i < count; i++)
	{
		lparam = i;
		if(ListView_GetCheckState(listview, i))
		{
			lparam = ListViewGetLPARAM(listview, i);
			if(lparam)
			{
				style |= ((const STYLEBITMAP *)lparam)->style_bit;
			}
		}
	}
	return style;
}

void StylesDialog::add_styles(HWND listview, DWORD style, const STYLEBITMAP sinfo[], unsigned char count)
{
	LVITEM litem;
	unsigned char i, lindex;

#if defined(ZEROFILL_LOCAL_VARS)
	//memset(&litem, 0, sizeof(LVITEM));
	i = 0;
	lindex = 0;
#endif

	memset(&litem, 0, sizeof(LVITEM));
	litem.mask = LVIF_TEXT | LVIF_PARAM;
	for(i = 0; i < count; i++)
	{
		litem.pszText = (PTSTR)sinfo[i].name;
		litem.lParam = (LPARAM)&sinfo[i];

		lindex = (unsigned char)ListView_InsertItem(listview, &litem);

		if((style & sinfo[i].style_bit) == sinfo[i].style_bit)
		{
			ListView_SetCheckState(listview, lindex, TRUE);
		}
	}
}

// 基本スタイル
const STYLEBITMAP StylesDialog::basic_window_styles[16] = {
	{TEXT("最大化ボタン"), WS_TABSTOP},
	{TEXT("最小化ボタン"), WS_GROUP},
	{TEXT("サイズ変更枠"), WS_THICKFRAME},
	{TEXT("システムメニュー"), WS_SYSMENU},
	{TEXT("水平スクロールバー"), WS_HSCROLL},
	{TEXT("垂直スクロールバー"), WS_VSCROLL},
	{TEXT("ダイアログ枠"), WS_DLGFRAME},
	{TEXT("境界線"), WS_BORDER},
	{TEXT("最大化状態"), WS_MAXIMIZE},
	{TEXT("再描画領域から子ウィンドウを除外"), WS_CLIPCHILDREN},
	{TEXT("再描画領域から他ウィンドウを除外"), WS_CLIPSIBLINGS},
	{TEXT("無効状態"), WS_DISABLED},
	{TEXT("可視状態"), WS_VISIBLE},
	{TEXT("最小化状態"), WS_MINIMIZE},
	{TEXT("子ウィンドウ"), WS_CHILD},
	{TEXT("ポップアップ"), WS_POPUP}};

// 拡張スタイル
const STYLEBITMAP StylesDialog::extend_window_styles[16] = {
	{TEXT("モーダルフレーム"), WS_EX_DLGMODALFRAME},
	{TEXT("親ウィンドウへの通知無し"), WS_EX_NOPARENTNOTIFY},
	{TEXT("最前面表示"), WS_EX_TOPMOST},
	{TEXT("ファイルのドラッグ&ドロップ"), WS_EX_ACCEPTFILES},
	{TEXT("透明ウィンドウ"), WS_EX_TRANSPARENT},
	{TEXT("MDI子ウィンドウ"), WS_EX_MDICHILD},
	{TEXT("ツールウィンドウ"), WS_EX_TOOLWINDOW},
	{TEXT("ウィンドウエッジ"), WS_EX_WINDOWEDGE},
	{TEXT("クライアントエッジ"), WS_EX_CLIENTEDGE},
	{TEXT("ヘルプボタン"), WS_EX_CONTEXTHELP},
	{TEXT("WS_EX_RIGHT"), WS_EX_RIGHT},
	{TEXT("右から左へ"), WS_EX_RTLREADING},
	{TEXT("垂直スクロールバーを左側にする"), WS_EX_LEFTSCROLLBAR},
	{TEXT("WS_EX_CONTROLPARENT"), WS_EX_CONTROLPARENT},
	{TEXT("スタティックエッジ"), WS_EX_STATICEDGE},
	{TEXT("WS_EX_APPWINDOW"), WS_EX_APPWINDOW}};

// 拡張スタイル(Windows 2000)
const STYLEBITMAP StylesDialog::extend_window_styles_nt5[4] = {
	{TEXT("レイヤードウィンドウ"), WS_EX_LAYERED},
	{TEXT("レイアウトの継承無し"), WS_EX_NOINHERITLAYOUT},
	{TEXT("配置を左右反転"), WS_EX_LAYOUTRTL},
	{TEXT("アクティブ不可"), WS_EX_NOACTIVATE}};

// 拡張スタイル(Windows XP)
const STYLEBITMAP StylesDialog::extend_window_styles_nt51[1] = {
	{TEXT("WS_EX_COMPOSITED"), WS_EX_COMPOSITED}};

// ダミー列(非表示)
const LVCOLUMN StylesDialog::dummycolumn = {0, 0, 0, NULL, 0, 0, 0, 0};

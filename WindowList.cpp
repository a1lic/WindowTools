#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
//#include <uxtheme.h>
#include "WindowList.h"
#include "WindowConfig.h"
#include "Window.h"
#include "misc.h"
#include "resource.h"

#if _WIN32_WINNT < 0x600
#define LVCFMT_SPLITBUTTON 0
#endif

const LVCOLUMN WindowList::columns[7] = {
	/* mask,fmt,cx,pszText,cchTextMax,iSubItem,iImage,iOrder */
	{LVCF_WIDTH,0,0,NULL,0,0,0,0},
	{LVCF_TEXT | LVCF_WIDTH | LVCF_FMT,LVCFMT_SPLITBUTTON,128,TEXT("キャプション"),0,0,0,0},
	{LVCF_TEXT | LVCF_WIDTH | LVCF_FMT,LVCFMT_SPLITBUTTON,96,TEXT("ウィンドウハンドル"),0,0,0,0},
	{LVCF_TEXT | LVCF_WIDTH | LVCF_FMT,LVCFMT_SPLITBUTTON,80,TEXT("クラス名"),0,0,0,0},
	{LVCF_TEXT | LVCF_WIDTH | LVCF_FMT,LVCFMT_SPLITBUTTON,64,TEXT("状態"),0,0,0,0},
	{LVCF_TEXT | LVCF_WIDTH | LVCF_FMT,LVCFMT_SPLITBUTTON,128,TEXT("プロセス"),0,0,0,0},
	{LVCF_TEXT | LVCF_WIDTH | LVCF_FMT,LVCFMT_SPLITBUTTON,50,TEXT("Unicode"),0,0,0,0}};

#if _WIN32_WINNT >= 0x501
const LVTILEVIEWINFO WindowList::tile_info = {
	/* cbSize        */ sizeof(LVTILEVIEWINFO),
	/* dwMask        */ LVTVIM_COLUMNS,
	/* dwFlags       */ LVTVIF_AUTOSIZE,
	/* sizeTile      */ {0,0},
	/* cLines        */ 4,
	/* rcLabelMargin */ {0,0,0,0}};

const LVGROUP WindowList::grp_unicode = {
	/* cbSize    */ sizeof(LVGROUP),
	/* mask      */ LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN,
	/* pszHeader */ L"Unicode対応",
	/* cchHeader */ 0,
	/* pszFooter */ NULL,
	/* cchFooter */ 0,
	/* iGroupId  */ 1,
	/* stateMask */ LVGS_NORMAL,
	/* state     */ LVGS_NORMAL,
	/* uAlign    */ LVGA_HEADER_LEFT};
const LVGROUP WindowList::grp_ansi = {
	/* cbSize    */ sizeof(LVGROUP),
	/* mask      */ LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN,
	/* pszHeader */ L"Unicode非対応",
	/* cchHeader */ 0,
	/* pszFooter */ NULL,
	/* cchFooter */ 0,
	/* iGroupId  */ 2,
	/* stateMask */ LVGS_NORMAL,
	/* state     */ LVGS_NORMAL,
	/* uAlign    */ LVGA_HEADER_LEFT};

const UINT WindowList::show_columns[4] = {2,3,1,4};
#if _WIN32_WINNT >= 0x600
const int WindowList::show_columns_formats[4] = {LVCFMT_LEFT,LVCFMT_RIGHT,LVCFMT_LEFT,LVCFMT_LEFT};
#endif

const LVTILEINFO WindowList::tile_info2 = {
#if _WIN32_WINNT >= 0x600
	/* cbSize    */ sizeof(LVTILEINFO),
	/* iItem     */ 0,
	/* cColumns  */ 4,
	/* puColumns */ (PUINT)WindowList::show_columns,
	/* piColFmt  */ (int*)WindowList::show_columns_formats};
#elif _WIN32_WINNT >= 0x501
	/* cbSize    */ sizeof(LVTILEINFO),
	/* iItem     */ 0,
	/* cColumns  */ 4,
	/* puColumns */ (PUINT)WindowList::show_columns};
#endif
#endif

WindowList::WindowList(HWND parent,UINT id,HWND target)
{
	HINSTANCE instance;
	char i;
	HICON icons[2];
	int x[2],y[2];
	UINT imagelist_flags;
#if !defined(UNICODE)
	LVCOLUMN columns[7];
#endif
#if !defined(_UXTHEME_H_)
	HMODULE uxtheme;
	SETWINDOWTHEME *_SetWindowTheme;
#endif

	process_id = GetCurrentProcessId();
	instance = (HINSTANCE)GetWindowLongPtr(parent,GWLP_HINSTANCE);

	listview = CreateWindowEx(WS_EX_CLIENTEDGE,WC_LISTVIEW,NULL,WS_CHILD | WS_TABSTOP | WS_VISIBLE | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS | LVS_SINGLESEL,0,0,0,0,parent,(HMENU)id,instance,NULL);
	if(!listview)
	{
		return;
	}

	this->parent = parent;

	if(OSFeatureTest(OSF_LISTVIEW_NT51))
	{
#if defined(_UXTHEME_H_)
		SetWindowTheme(listview,L"Explorer",NULL);
#else
		uxtheme = LoadLibrary(TEXT("UXTHEME.DLL"));
		if(uxtheme)
		{
			_SetWindowTheme = (SETWINDOWTHEME*)GetProcAddress(uxtheme,"SetWindowTheme");
			if(_SetWindowTheme)
			{
				(*_SetWindowTheme)(listview,L"Explorer",NULL);
			}
			FreeLibrary(uxtheme);
		}
#endif
	}

#if !defined(UNICODE)
	// Unicodeの場合はLVCOLUMNがconstでもよいが、Unicodeでない時はクラッシュする
	memcpy(columns,this->columns,sizeof(LVCOLUMN) * 7);
#endif

#if _WIN32_WINNT >= 0x600
	if(OSFeatureTest(OSF_LISTVIEW_NT60))
	{
		ListView_SetExtendedListViewStyle(listview,LVS_EX_FULLROWSELECT | LVS_EX_HEADERINALLVIEWS | LVS_EX_DOUBLEBUFFER);
	}
	else if(OSFeatureTest(OSF_LISTVIEW_NT51))
	{
		ListView_SetExtendedListViewStyle(listview,LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	}
#elif _WIN32_WINNT >= 0x501
	if(OSFeatureTest(OSF_LISTVIEW_NT51))
	{
		ListView_SetExtendedListViewStyle(listview,LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	}
#endif

	for(i = 0; i < 7; i++)
	{
		ListView_InsertColumn(listview,i,&columns[i]);
	}
	ListView_DeleteColumn(listview,0);

#if _WIN32_WINNT >= 0x501
	if(OSFeatureTest(OSF_LISTVIEW_NT51))
	{
		ListView_EnableGroupView(listview,TRUE);
		ListView_InsertGroup(listview,-1,&grp_unicode);
		ListView_InsertGroup(listview,-1,&grp_ansi);
	}
#endif

	view_mode = LVS_REPORT;
	sorted_column = 0;

	x[0] = GetSystemMetrics(SM_CXICON);
	x[1] = GetSystemMetrics(SM_CXSMICON);
	y[0] = GetSystemMetrics(SM_CYICON);
	y[1] = GetSystemMetrics(SM_CYSMICON);

	icons[0] = (HICON)CopyImage((HANDLE)LoadImage(NULL,MAKEINTRESOURCE(OIC_SAMPLE),IMAGE_ICON,x[0],y[0],LR_SHARED),IMAGE_ICON,0,0,0);
	icons[1] = (HICON)CopyImage((HANDLE)LoadImage(NULL,MAKEINTRESOURCE(OIC_SAMPLE),IMAGE_ICON,x[1],y[1],LR_SHARED),IMAGE_ICON,0,0,0);

	if(OSFeatureTest(OSF_LISTVIEW_NT51))
	{
		imagelist_flags = ILC_MASK | ILC_COLOR32;
	}
	else
	{
		imagelist_flags = ILC_MASK | ILC_COLOR8;
	}

	normal_icons = ImageList_Create(x[0],y[0],imagelist_flags,1,256);
	if(normal_icons)
	{
		ImageList_ReplaceIcon(normal_icons,-1,icons[0]);
		ImageList_ReplaceIcon(normal_icons,-1,icons[0]);
		ListView_SetImageList(listview,normal_icons,LVSIL_NORMAL);
	}
	caption_icons = ImageList_Create(x[1],y[1],imagelist_flags,1,256);
	if(caption_icons)
	{
		ImageList_ReplaceIcon(caption_icons,-1,icons[1]);
		ImageList_ReplaceIcon(caption_icons,-1,icons[1]);
		ListView_SetImageList(listview,caption_icons,LVSIL_SMALL);
	}

	DestroyIcon(icons[1]);
	DestroyIcon(icons[0]);

	styles_mask = WS_MINIMIZE | WS_POPUP | WS_VISIBLE;
	extend_styles_mask = WS_EX_TOOLWINDOW;
	show_no_title_window = false;

	sort_menu = LoadMenu(instance,MAKEINTRESOURCE(IDM_SORT));

	lock = CreateMutex(NULL,FALSE,NULL);

	// Not support yet...
	target_parent = NULL;

	stop_update = false;
}

WindowList::~WindowList()
{
	if(lock)
	{
		WaitForSingleObject(lock,INFINITE);
		delete_all_items();
		CloseHandle(lock);
	}
	if(sort_menu)
	{
		DestroyMenu(sort_menu);
	}
	SendMessage(listview,WM_CLOSE,0,0);
	if(caption_icons)
	{
		ImageList_Destroy(caption_icons);
	}
	if(normal_icons)
	{
		ImageList_Destroy(normal_icons);
	}
}

HWND WindowList::GetHandle()
{
	return listview;
}

void WindowList::GiveFocus()
{
	::SetFocus(listview);
}

char WindowList::GetViewMode()
{
	return view_mode;
}

void WindowList::SetViewMode(char mode)
{
	DWORD style;

#if _WIN32_WINNT >= 0x501
	if(OSFeatureTest(OSF_LISTVIEW_NT51))
	{
		ListView_SetView(listview,mode);
		if(mode == LV_VIEW_TILE)
		{
			ListView_SetTileViewInfo(listview,&tile_info);
		}
		if(mode == LV_VIEW_DETAILS)
		{
			ListView_SetExtendedListViewStyle(listview,LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
		}
		else
		{
			ListView_SetExtendedListViewStyleEx(listview,LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER,LVS_EX_DOUBLEBUFFER);
		}
	}
	else
	{
		style = (DWORD)GetWindowLongPtr(listview,GWL_STYLE);
		style &= ~LVS_TYPEMASK;
		style |= (mode & LVS_TYPEMASK);
		SetWindowLongPtr(listview,GWL_STYLE,style);
	}
#else
	style = (DWORD)GetWindowLongPtr(listview,GWL_STYLE);
	style &= ~LVS_TYPEMASK;
	style |= (mode & LVS_TYPEMASK);
	SetWindowLongPtr(listview,GWL_STYLE,style);
#endif

	view_mode = mode;
}

void WindowList::Update()
{
	//WINDOWLIST_ENUM_CHILDREN enum_children;

	if(stop_update)
	{
		return;
	}
	// ロックを獲得できなかったらそのまま終了
	if(WaitForSingleObject(lock,0) != WAIT_OBJECT_0)
	{
		return;
	}

#if 0
	if(target_parent)
	{
		enum_children._this = this;
		enum_children.depth = 0;
		enum_children.insert_index = 0;
		EnumChildWindows(target_parent,update_child_list,(LPARAM)&enum_children);
	}
	else
	{
		EnumWindows(update_window_list,(LPARAM)this);
	}
#else
	EnumWindows(update_window_list,(LPARAM)this);
#endif
	delete_invalid_items();
	ReleaseMutex(lock);
}

void WindowList::BringToTop()
{
	int i;
	Window *item;

	i = find_next_selected_item(-1,&item);
	if(i > -1)
	{
		item->BringToTop();
	}
}

void WindowList::SinkToBottom()
{
	int i;
	Window *item;

	i = find_next_selected_item(-1,&item);
	if(i > -1)
	{
		item->SinkToBottom();
	}
}

void WindowList::ShowProperty()
{
	int i;
	Window *item;
	WindowConfig *c;

	i = find_next_selected_item(-1,&item);
	if(i > -1)
	{
		stop_update = true;
		c = new WindowConfig(item);
		c->Config(parent);
		stop_update = false;
		delete c;
	}
}

HIMAGELIST WindowList::GetImageList(bool small_image)
{
	return small_image ? caption_icons : normal_icons;
}

LRESULT WindowList::Notify(const NMHDR *nm)
{
	LRESULT rt;
#if _WIN32_WINNT >= 0x600
	RECT irc;
	POINT p;
	int id;
	HWND head;
	Window *item;
#endif

	rt = 0;

	switch(nm->code)
	{
	case NM_DBLCLK:
		if(((const NMITEMACTIVATE*)nm)->iItem >= 0)
		{
			ShowProperty();
		}
		break;
	case LVN_GETDISPINFO:
		if(((const NMLVDISPINFO*)nm)->item.lParam)
		{
			((Window*)((const NMLVDISPINFO*)nm)->item.lParam)->GetDispInfo((NMLVDISPINFO*)nm,this);
		}
		break;
	case LVN_COLUMNCLICK:
		Sort(((const NMLISTVIEW*)nm)->iSubItem);
		break;
#if _WIN32_WINNT >= 0x600
	case LVN_COLUMNDROPDOWN:
		// 未対応だとそもそもこの処理が行われないのでバージョンチェックは無し
		if(head = ListView_GetHeader(listview))
		{
			if(Header_GetItemRect(head,((const NMLISTVIEW*)nm)->iSubItem,&irc))
			{
				p.x = irc.left;
				p.y = irc.bottom;
				ClientToScreen(head,&p);
				if(id = TrackPopupMenu(GetSubMenu(sort_menu,0),TPM_NONOTIFY | TPM_RETURNCMD,p.x,p.y,0,parent,NULL))
				{
					sort_column = ((const NMLISTVIEW*)nm)->iSubItem;

					switch(id)
					{
					case IDC_SORT_NONE:
						Sort(sort_column,WLSM_NOSORT);
						break;

					case IDC_SORT_ASC:
						Sort(sort_column,WLSM_ASC);
						break;

					case IDC_SORT_DSC:
						Sort(sort_column,WLSM_DSC);
						break;
					}
				}
			}
		}
		break;
#endif
	case LVN_BEGINDRAG:
	case LVN_BEGINRDRAG:
		SetCapture(parent);
		cursor_no = (HCURSOR)LoadImage(NULL,MAKEINTRESOURCE(OCR_NO),IMAGE_CURSOR,0,0,LR_SHARED);
		if(cursor_no)
		{
			SetCursor(cursor_no);
		}
		break;

#if _WIN32_WINNT >= 0x600
	case LVN_HOTTRACK:
		item = (Window*)ListViewGetLPARAM(listview,((const NMLISTVIEW*)nm)->iItem);
		if(item)
		{
			item->PopupThumbnail();
		}
		break;
#endif

	}
	return rt;
}

void WindowList::Sort(int col)
{
	if(col != sort_column)
	{
		sort_column = col;
		sort_mode = WLSM_ASC;
	}
	else
	{
		switch(sort_mode)
		{
		case WLSM_NOSORT:
			sort_mode = WLSM_ASC;
			break;
		case WLSM_ASC:
			sort_mode = WLSM_DSC;
			break;
		default:
			sort_mode = WLSM_NOSORT;
			break;
		}
	}
	Sort(col,sort_mode);
}

void WindowList::Sort(int col,WINDOWLIST_SORT_MODE mode)
{
	HWND head;
	HDITEM head_item;

	if(head = ListView_GetHeader(listview))
	{
		head_item.mask = HDI_FORMAT;
		if(sorted_column != col)
		{
			Header_GetItem(head,sorted_column,&head_item);
			head_item.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
			Header_SetItem(head,sorted_column,&head_item);
			sorted_column = col;
		}
		Header_GetItem(head,col,&head_item);
		head_item.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
		if(mode == WLSM_ASC)
		{
			head_item.fmt |= HDF_SORTUP;
		}
		else if(mode == WLSM_DSC)
		{
			head_item.fmt |= HDF_SORTDOWN;
		}
		Header_SetItem(head,col,&head_item);
	}
	sort_mode = mode;
	ListView_SortItemsEx(listview,sort_items,this);
}

void WindowList::EndDragMode()
{
	ReleaseCapture();
	if(cursor_no)
	{
		SetCursor(NULL);
		//DestroyCursor(cursor_no);
		cursor_no = NULL;
	}
}

unsigned int WindowList::GetItemCount()
{
	return (unsigned int)ListView_GetItemCount(listview);
}

void WindowList::HandleContextMenu(HMENU menu)
{
	int id;
	POINT p;

	id = find_next_selected_item(-1,NULL);
	if(id > -1)
	{
		GetCursorPos(&p);
		id = TrackPopupMenu(menu,TPM_NONOTIFY | TPM_RETURNCMD,p.x,p.y,0,parent,NULL);

		switch(id)
		{
		case IDC_BRINGTOFRONT:
			BringToTop();
			break;

		case IDC_SINKTOBOTTOM:
			SinkToBottom();
			break;

		case IDC_WINDOWCONFIG:
			ShowProperty();
			break;
		}
	}
}

// =======================================
// ========== PRIVATE FUNCTIONS ==========
// =======================================

int CALLBACK WindowList::sort_items(LPARAM l1,LPARAM l2,LPARAM l3)
{
	int r;
	Window *item;
	INT64 time_stamp[2];

	switch(((WindowList*)l3)->sort_mode)
	{
	case WLSM_NOSORT:
		item = (Window*)((WindowList*)l3)->item_param((int)l1);
		if(item->GetDepth())
		{
			return 1;
		}
		time_stamp[0] = item ? item->GetTimeStamp() : 0;

		item = (Window*)((WindowList*)l3)->item_param((int)l2);
		if(item->GetDepth())
		{
			return 1;
		}
		time_stamp[1] = item ? item->GetTimeStamp() : 0;

		r = (int)(time_stamp[0] - time_stamp[1]);
		if(r > 0)
		{
			r = 1;
		}
		else if(r < 0)
		{
			r = -1;
		}
		break;
	case WLSM_ASC:
		item = (Window*)((WindowList*)l3)->item_param((int)l1);
		if(item->GetDepth())
		{
			return 1;
		}
		item = (Window*)((WindowList*)l3)->item_param((int)l2);
		if(item->GetDepth())
		{
			return 1;
		}
		ListView_GetItemText(((WindowList*)l3)->listview,(int)l1,((WindowList*)l3)->sort_column,((WindowList*)l3)->tmp[0],1024);
		ListView_GetItemText(((WindowList*)l3)->listview,(int)l2,((WindowList*)l3)->sort_column,((WindowList*)l3)->tmp[1],1024);
		r = _tcscmp(((WindowList*)l3)->tmp[0],((WindowList*)l3)->tmp[1]);
		break;
	case WLSM_DSC:
		item = (Window*)((WindowList*)l3)->item_param((int)l1);
		if(item->GetDepth())
		{
			return 1;
		}
		item = (Window*)((WindowList*)l3)->item_param((int)l2);
		if(item->GetDepth())
		{
			return 1;
		}
		ListView_GetItemText(((WindowList*)l3)->listview,(int)l1,((WindowList*)l3)->sort_column,((WindowList*)l3)->tmp[0],1024);
		ListView_GetItemText(((WindowList*)l3)->listview,(int)l2,((WindowList*)l3)->sort_column,((WindowList*)l3)->tmp[1],1024);
		r = _tcscmp(((WindowList*)l3)->tmp[1],((WindowList*)l3)->tmp[0]);
		break;
	}
	return r;
}

void WindowList::delete_all_items()
{
	int i,count;
	Window *item;

	count = ListView_GetItemCount(listview);
	for(i = 0; i < count; i++)
	{
		if(item = (Window*)item_param(i))
		{
			delete item;
		}
	}
	ListView_DeleteAllItems(listview);
}

void WindowList::delete_invalid_items()
{
	int i,count;
	Window *item;
	unsigned int n;

	count = ListView_GetItemCount(listview);
	for(i = count - 1; i >= 0; i--)
	{
		if(item = (Window*)item_param(i))
		{
			if(check_style(item->GetHandle(),NULL,NULL))
			{
				if(show_no_title_window)
				{
					continue;
				}
				else
				{
					n = item->GetCaptionLength();
					if(n > 0)
					{
						continue;
					}
				}
			}
		}

		if(item)
		{
			delete item;
		}
		ListView_DeleteItem(listview,i);
	}
}

int WindowList::find_next_selected_item(int index,Window **witem)
{
	int i;
	Window *item;

	i = ListView_GetNextItem(listview,index,LVNI_SELECTED);
	if(i >= 0)
	{
		if(item = (Window*)item_param(i))
		{
			if(witem)
			{
				*witem = item;
			}
		}
	}
	return i;
}

int WindowList::find_window_item(HWND handle,Window **window)
{
	int i,count;
	Window *item;

	count = ListView_GetItemCount(listview);
	for(i = 0; i < count; i++)
	{
		if(item = (Window*)item_param(i))
		{
			if(item->GetHandle() == handle)
			{
				break;
			}
		}
	}
	if(i < count)
	{
		if(window)
		{
			*window = item;
		}
	}
	else
	{
		i = -1;
	}

	return i;
}

int WindowList::add(Window *witem)
{
#if _WIN32_WINNT >= 0x501
	LVTILEINFO tile_info_l;
#endif
	LVITEM item;
	PTSTR title,class_name;
	int i;
	unsigned char depth;
	TCHAR str[256];

	memset(&item,0,sizeof(LVITEM));

	title = witem->GetCaption();
	class_name = witem->GetClassName2();

	depth = witem->GetDepth();

	item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
#if _WIN32_WINNT >= 0x501
	if(OSFeatureTest(OSF_LISTVIEW_NT51))
	{
		item.mask |= LVIF_GROUPID;
	}
#endif
	if(depth > 0)
	{
		item.mask |= LVIF_INDENT;
		item.mask &= ~LVIF_IMAGE;
		item.iIndent = depth;
	}

	item.iItem = ListView_GetItemCount(listview);
	item.pszText = title;
	if(depth == 0)
	{
		item.iImage = -1;
	}
	item.lParam = (LPARAM)witem;

#if _WIN32_WINNT >= 0x501
	if(OSFeatureTest(OSF_LISTVIEW_NT51))
	{
		item.iGroupId = witem->IsUnicodeWindow() ? 1 : 2;
	}
#endif
	i = ListView_InsertItem(listview,&item);
	if(i == -1)
	{
		LocalHeapFree(class_name);
		LocalHeapFree(title);
		return -1;
	}

	_sntprintf_s(str,32,_TRUNCATE,TEXT("0x%p"),witem->GetHandle());
	item.mask = LVIF_TEXT;
	item.iItem = i;
	item.iSubItem = 1;
	item.pszText = str;
	ListView_SetItem(listview,&item);

	item.iSubItem = 2;
	item.pszText = class_name;
	ListView_SetItem(listview,&item);

	if(!GetProcessNameFromId(witem->GetProcess(),str,256))
	{
		_sntprintf_s(str,256,_TRUNCATE,_T("ID:%u"),witem->GetProcess());
	}
	item.iSubItem = 4;
	item.pszText = _tcsrchr(str,_T('\\'));
	if(!item.pszText)
	{
		item.pszText = str;
	}
	else
	{
		item.pszText++;
	}
	ListView_SetItem(listview,&item);

	item.iSubItem = 5;
	item.pszText = witem->IsUnicodeWindow() ? TEXT("対応") : TEXT("非対応");
	ListView_SetItem(listview,&item);

#if _WIN32_WINNT >= 0x501
	if(OSFeatureTest(OSF_LISTVIEW_NT51))
	{
		tile_info_l = tile_info2;
#if _WIN32_WINNT >= 0x600
		// サイズが正しくないと、Vista未満の環境ではLVM_SETTILEINFOで失敗する
		if(!OSFeatureTest(OSF_LISTVIEW_NT60))
		{
			tile_info_l.cbSize = LVTILEINFO_V5_SIZE;
		}
#endif
		tile_info_l.iItem = item.iItem;
		if(!ListView_SetTileInfo(listview,&tile_info_l))
		{
			Debug(TEXT("ListView_SetTileInfo failed. iItem=%u"),item.iItem);
		}
	}
#endif

	LocalHeapFree(class_name);
	LocalHeapFree(title);

	return i;
}

bool WindowList::check_style(HWND window,DWORD *r_style,DWORD *r_extend_style)
{
	DWORD style,extend_style;

	style = (DWORD)GetWindowLongPtr(window,GWL_STYLE);
	extend_style = (DWORD)GetWindowLongPtr(window,GWL_EXSTYLE);

	if(r_style)
	{
		*r_style = style;
	}
	if(r_extend_style)
	{
		*r_extend_style = extend_style;
	}

	style ^= WS_VISIBLE;

	return !((style & styles_mask) || (extend_style & extend_styles_mask));
}

BOOL CALLBACK WindowList::update_child_list(HWND hwnd,LPARAM lParam)
{
#if 0
	int i;
	Window *item;
	PTSTR caption;
	DWORD style;
	bool f;

	if(((const WINDOWLIST_ENUM_CHILDREN*)lParam)->_this->check_style(hwnd,&style,NULL))
	{
		i = ((const WINDOWLIST_ENUM_CHILDREN*)lParam)->_this->find_window_item(hwnd,&item);
		if(i == -1)
		{
			i = ((const WINDOWLIST_ENUM_CHILDREN*)lParam)->_this->add(new Window(hwnd));
		}
		else
		{
			if(item)
			{
				f = ((style & WS_VISIBLE) != 0);

				caption = item->GetCaption();
				ListViewUpdateItemString(((const WINDOWLIST_ENUM_CHILDREN*)lParam)->_this->listview,i,0,caption ? caption : TEXT(""));
				item->Free(caption);

				ListViewUpdateItemString(((const WINDOWLIST_ENUM_CHILDREN*)lParam)->_this->listview,i,3,f ? TEXT("表示") : TEXT("非表示"));
			}
		}
	}

	// さらに潜る。
	((WINDOWLIST_ENUM_CHILDREN*)lParam)->depth++;
	EnumChildWindows(hwnd,update_child_list,lParam);
	((WINDOWLIST_ENUM_CHILDREN*)lParam)->depth--;
	return TRUE;
#else
	return FALSE;
#endif
}

BOOL CALLBACK WindowList::update_window_list(HWND window,LPARAM cthis)
{
	int i;
	Window *item;
	PTSTR caption;
	DWORD pid,style;
	//WINDOWLIST_ENUM_CHILDREN enum_c;
	bool f;

	GetWindowThreadProcessId(window,&pid);
	if(pid == ((WindowList*)cthis)->process_id)
	{
		return TRUE;
	}

	if(!((WindowList*)cthis)->show_no_title_window)
	{
		if(GetWindowTextLength(window) <= 0)
		{
			return TRUE;
		}
	}

	if(((WindowList*)cthis)->check_style(window,&style,NULL))
	{
		i = ((WindowList*)cthis)->find_window_item(window,&item);
		if(i == -1)
		{
			item = new Window(window);
			if(!item)
			{
				return TRUE;
			}
			i = ((WindowList*)cthis)->add(item);
		}
		if(i >= 0)
		{
			if(item)
			{
				item->UpdateIcon();

				f = ((style & WS_VISIBLE) == WS_VISIBLE);

				caption = item->GetCaption();
				ListViewUpdateItemString(((WindowList*)cthis)->listview,i,0,caption ? caption : TEXT(""));
				LocalHeapFree(caption);

				ListViewUpdateItemString(((WindowList*)cthis)->listview,i,3,f ? TEXT("表示") : TEXT("非表示"));
			}
		}
		else
		{
			delete item;
		}

#if 0
		enum_c._this = (WindowList*)cthis;
		enum_c.depth = 0;
		enum_c.insert_index = i;
		EnumChildWindows(window,update_child_list,(LPARAM)&enum_c);
#endif
	}

	return TRUE;
}

LPARAM WindowList::item_param(int index)
{
	LVITEM item;

	item.iItem = index;
	item.mask = LVIF_PARAM;
	item.iSubItem = 0;
	if(ListView_GetItem(listview,&item))
	{
		return item.lParam;
	}
	return 0;
}

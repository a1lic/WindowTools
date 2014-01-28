#include <windows.h>
#include "WinListDropSource.h"

WinListDropSource::WinListDropSource()
{
	reference_count = 1;
}

WinListDropSource::~WinListDropSource()
{
}

STDMETHODIMP WinListDropSource::QueryInterface(REFIID id,void **obj)
{
	*obj = NULL;
	if(IsEqualIID(id,IID_IUnknown) || IsEqualIID(id,IID_IDropSource))
	{
		*obj = (void*)this;
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) WinListDropSource::AddRef()
{
	return ++reference_count;
}

STDMETHODIMP_(ULONG) WinListDropSource::Release()
{
	if(!--reference_count)
	{
		delete this;
		return 0;
	}
	return reference_count;
}

STDMETHODIMP WinListDropSource::QueryContinueDrag(BOOL escape,DWORD key_state)
{
	if(escape)
	{
		return DRAGDROP_S_CANCEL;
	}
	if(!(key_state & MK_LBUTTON))
	{
		return DRAGDROP_S_DROP;
	}
	return S_OK;
}

STDMETHODIMP WinListDropSource::GiveFeedBack(DWORD effect)
{
	return S_OK;
}

#ifndef WINLISTDROPSOURCE_H
#define WINLISTDROPSOURCE_H

#include <windows.h>

class WinListDropSource: public IDropSource
{
//private:
	ULONG reference_count;
public:
	WinListDropSource();
	~WinListDropSource();
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID,void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	// IDropSource
	STDMETHODIMP QueryContinueDrag(BOOL,DWORD);
	STDMETHODIMP GiveFeedback(DWORD);
};
#endif

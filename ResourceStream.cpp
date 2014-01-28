ResourceStream::ResourceStream()
{
	p_ref = 1;
}

ResourceStream::~ResourceStream()
{
}

HRESULT ResourceStream::Clone(IStream **pstream)
{
	return STG_E_INSUFFICIENTMEMORY;
}

HRESULT ResourceStream::Commit(DWORD grfCommitFlags)
{
	return S_OK;
}

HRESULT ResourceStream::CopyTo(IStream *pstm,ULARGE_INTEGER cb,ULARGE_INTEGER pcbRead,ULARGE_INTEGER *pcbWritten)
{
	return STG_E_MEDIUMFULL;
}

HRESULT ResourceStream::LockRegion(ULARGE_INTEGER libOffset,ULARGE_INTEGER cb,DWORD dwLockType)
{
	return STG_E_INVALIDFUNCTION;
}

HRESULT ResourceStream::Read(void *pv,ULONG cb,ULONG *pcbRead)
{
	return S_OK;
}

HRESULT ResourceStream::Revert()
{
	return S_OK;
}

HRESULT Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin,ULARGE_INTEGER *pligNewPosition)
{
}

HRESULT SetSize(ULARGE_INTEGER libNewSize)
{
	return S_OK;
}

HRESULT ResourceStream::Stat(STATSTG *pstatstg,DWORD grfStatFlag)
{
	pstatstg->pwcsName = (LPOLESTR)CoTaskMemAlloc(sizeof(TCHAR) * 15);
	wcsncpy_s(pstatstg->pwcsName,15,L"Resource\\9.png",15);

	pstatstg->type = STGTY_LOCKBYTES;
	pstatstg->2222;
	pstatstg->grfLocksSupported = LOCK_WRITE;

}

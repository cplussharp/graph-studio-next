//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

const CFactoryTemplate CMonoDump::g_Template = {
		L"Dump Filter",
        &__uuidof(DumpFilter),
		CMonoDump::CreateInstance,
		NULL,
		NULL
	};

CUnknown* CMonoDump::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CMonoDump* pNewObject = new CMonoDump(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//-----------------------------------------------------------------------------
//
//	CMonoDump class
//
//-----------------------------------------------------------------------------

CMonoDump::CMonoDump(LPUNKNOWN pUnk, HRESULT *phr) :
	CBaseRenderer(__uuidof(DumpFilter), TEXT("Dump"), pUnk, phr)
{
	filename = _T("");
}

CMonoDump::~CMonoDump()
{
	DoCloseFile();
}

STDMETHODIMP CMonoDump::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == IID_IFileSinkFilter) {
		return GetInterface((IFileSinkFilter*)this, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CMonoDump::DoOpenFile()
{
#ifndef __AFX_H__
    if (S_OK == file.Create(filename, GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN))
    {
        ULONGLONG fileSize;
        if (S_OK == file.GetSize(fileSize) && fileSize > 0)
            file.Seek(fileSize, FILE_BEGIN);
    }
    else
        return E_FAIL;
#else
    if (file.Open(filename, CFile::modeNoTruncate|CFile::modeWrite|CFile::osSequentialScan|CFile::shareDenyNone)) {
		file.SeekToEnd();
	} else if (!file.Open(filename, CFile::modeCreate|CFile::osSequentialScan|CFile::shareDenyNone)) {
		return E_FAIL;
	}
#endif

	return NOERROR;
}

HRESULT CMonoDump::DoCloseFile()
{
    if (file != INVALID_HANDLE_VALUE) {
        file.Close();
	}
	return NOERROR;
}

STDMETHODIMP CMonoDump::SetFileName(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt)
{
	CString	new_fn = CString(pszFileName);
	CString old_fn = filename;

	// try to open the file
	filename = new_fn;
	HRESULT hr = DoOpenFile();
	DoCloseFile();

	if (FAILED(hr)) {
		filename = old_fn;
	}

	return hr;		
}

STDMETHODIMP CMonoDump::GetCurFile(LPOLESTR * ppszFileName,AM_MEDIA_TYPE *pmt)
{
	int	len = filename.GetLength() + 1;
	*ppszFileName = (LPOLESTR)QzTaskMemAlloc(len * sizeof(WCHAR));
	if (*ppszFileName != NULL) {
		lstrcpy(*ppszFileName, filename.GetBuffer());
	}

	if (pmt) {
        ZeroMemory(pmt, sizeof(*pmt));
        pmt->majortype = MEDIATYPE_NULL;
        pmt->subtype = MEDIASUBTYPE_NULL;
    }
    return NOERROR;
}

HRESULT CMonoDump::CheckMediaType(const CMediaType *pmt)
{
	// accept any type
	return NOERROR;
}

HRESULT CMonoDump::ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime)
{
	// ignore timestamps
	return NOERROR;
}

HRESULT CMonoDump::OnStartStreaming()
{
	return DoOpenFile();
}

HRESULT CMonoDump::OnStopStreaming()
{
	return DoCloseFile();
}

HRESULT CMonoDump::DoRenderSample(IMediaSample *pMediaSample)
{
	long	size = pMediaSample->GetActualDataLength();
	BYTE	*buf;

	pMediaSample->GetPointer(&buf);

	if (size > 0 && file) {
        file.Write(buf, size);
	}

	return NOERROR;
}


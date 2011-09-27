//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

//-----------------------------------------------------------------------------
//
//	CMonoDump class
//
//-----------------------------------------------------------------------------

CMonoDump::CMonoDump(LPUNKNOWN pUnk, HRESULT *phr) :
	CBaseRenderer(CLSID_MonoDump, _T("Dump"), pUnk, phr),
	file(NULL)
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
	if (_tfopen_s(&file, filename, _T("ab")) == NOERROR) {
		fseek(file, 0, SEEK_END);
	} else {
        if (_tfopen_s(&file, filename, _T("wb")) != NOERROR)
		    return E_FAIL;
	}
	return NOERROR;
}

HRESULT CMonoDump::DoCloseFile()
{
	if (file) {
		fclose(file);
		file = NULL;
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
		fwrite(buf, 1, size, file);
	}

	return NOERROR;
}


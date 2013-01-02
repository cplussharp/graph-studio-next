//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

const CFactoryTemplate CAnalyzerWriterFilter::g_Template = {
		L"Analyzer Writer Filter",
        &__uuidof(AnalyzerWriterFilter),
		CAnalyzerWriterFilter::CreateInstance,
		NULL,
		NULL
	};

CUnknown* CAnalyzerWriterFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CAnalyzerWriterFilter* pNewObject = new CAnalyzerWriterFilter(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

#pragma region CAnalyzerWriterInput

/*********************************************************************************************
* CAnalyzerWriterInput class
*********************************************************************************************/
CAnalyzerWriterInput::CAnalyzerWriterInput(CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr, LPCWSTR pName, HANDLE file, CAnalyzer* pAnalyzer)
    : CRenderedInputPin(NAME("Input"), pFilter, pLock, phr, pName), m_file(file), m_analyzer(pAnalyzer)
{
    ResetData();
}


CAnalyzerWriterInput::~CAnalyzerWriterInput(void)
{
}


STDMETHODIMP CAnalyzerWriterInput::NonDelegatingQueryInterface(REFIID iid, void** ppv)
{
    if (IID_IStream == iid)
		return GetInterface((IStream*) this, ppv);
    if (__uuidof(IStreamBufferDataCounters) == iid)
		return GetInterface((IStreamBufferDataCounters*) this, ppv);
	return CBaseInputPin::NonDelegatingQueryInterface(iid, ppv);
}


HRESULT CAnalyzerWriterInput::CheckMediaType(const CMediaType* pmt)
{
    CheckPointer(pmt,E_POINTER);
    return S_OK;
}

HRESULT CAnalyzerWriterInput::GetMediaType(int iPosition, CMediaType* pmt)
{
    return VFW_S_NO_MORE_ITEMS;
}

STDMETHODIMP CAnalyzerWriterInput::Receive(IMediaSample *pSample)
{
    m_analyzer->AddSample(pSample);

    BYTE* pBuf;
    HRESULT hr = pSample->GetPointer(&pBuf);
    if(FAILED(hr)) return hr;

    DWORD length = pSample->GetActualDataLength();
    DWORD written = 0;

    // Statistic
    //CAutoLock lock(m_pLock);
    m_pinData.cSamplesProcessed++;
    if(pSample->IsDiscontinuity())
        m_pinData.cDiscontinuities++;
    if(pSample->IsSyncPoint())
        m_pinData.cSyncPoints++;
    
    // write
    if(m_mt.majortype == MEDIATYPE_Stream)
    {
        REFERENCE_TIME rtStartTime = 0;
        REFERENCE_TIME rtStopTime = 0;

        pSample->GetTime(&rtStartTime, &rtStopTime);

        if(rtStopTime > 1)
        {
            // Maybe a AviMux -> FilePos as Time
            length = DWORD(rtStopTime - rtStartTime);
            LARGE_INTEGER lint;
            lint.QuadPart = rtStartTime;
            OVERLAPPED overlpd = {0};
            overlpd.Offset = lint.LowPart;
            overlpd.OffsetHigh = lint.HighPart;

            if(!WriteFile(m_file, pBuf, length, &written, &overlpd))
                return HRESULT_FROM_WIN32(GetLastError());

            m_pinData.cDataBytes += written;

            return S_OK;
        }
    }

    if(!WriteFile(m_file, pBuf, length, &written, NULL))
        return HRESULT_FROM_WIN32(GetLastError());

    m_pinData.cDataBytes += written;

    return S_OK;
}

STDMETHODIMP CAnalyzerWriterInput::EndOfStream()
{
    m_pFilter->NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)(IBaseFilter *)m_pFilter);
    return S_OK;
}


#pragma region IStreamBufferDataCounters

/*********************************************************************************************
* Implementation of IStreamBufferDataCounters
*********************************************************************************************/
STDMETHODIMP CAnalyzerWriterInput::GetData(SBE_PIN_DATA *pPinData)
{
    CheckPointer(pPinData,E_POINTER);
    //CAutoLock lock(m_pLock);
    *pPinData = m_pinData;
    return S_OK;
}

STDMETHODIMP CAnalyzerWriterInput::ResetData()
{
    //CAutoLock lock(m_pLock);
    ZeroMemory(&m_pinData, sizeof(SBE_PIN_DATA));
    return S_OK;
}

#pragma endregion

#pragma region IStream

/*********************************************************************************************
* Implmentation of IStream
* see: http://msdn2.microsoft.com/en-us/library/ms752876.aspx
*********************************************************************************************/
STDMETHODIMP CAnalyzerWriterInput::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
	// ReadFile throws error if pcbRead=0
	ULONG* read=pcbRead;
	DWORD tmp;
	if(!read) read=&tmp;

    BOOL rc = ReadFile(m_file, pv, cb, read, NULL);

    m_analyzer->AddIStreamRead(pv, cb, *read);

    return (rc) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

STDMETHODIMP CAnalyzerWriterInput::Write(void const* pv, ULONG cb, ULONG* pcbWritten)
{
	// WriteFile throws error if pcbWritten=0
	ULONG* wr=pcbWritten;
	DWORD tmp;
	if(!wr) wr=&tmp;

    m_analyzer->AddIStreamWrite(pv, cb);

    if(!WriteFile(m_file, pv, cb, wr, NULL))
        return HRESULT_FROM_WIN32(GetLastError());

    //CAutoLock lock(m_pLock);
    m_pinData.cDataBytes += *wr;

    return S_OK;
}


STDMETHODIMP CAnalyzerWriterInput::SetSize(ULARGE_INTEGER)
{ 
    return E_NOTIMPL;   
}


STDMETHODIMP CAnalyzerWriterInput::CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) 
{ 
    return E_NOTIMPL;   
}


STDMETHODIMP CAnalyzerWriterInput::Commit(DWORD)                                      
{ 
    return E_NOTIMPL;   
}


STDMETHODIMP CAnalyzerWriterInput::Revert(void)                                       
{ 
    return E_NOTIMPL;   
}


STDMETHODIMP CAnalyzerWriterInput::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)              
{ 
    return E_NOTIMPL;   
}


STDMETHODIMP CAnalyzerWriterInput::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)            
{ 
    return E_NOTIMPL;   
}


STDMETHODIMP CAnalyzerWriterInput::Clone(IStream **)                                  
{ 
    return E_NOTIMPL;   
}


STDMETHODIMP CAnalyzerWriterInput::Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
{ 
    DWORD dwMoveMethod;

    switch(dwOrigin)
    {
    case STREAM_SEEK_SET:
        dwMoveMethod = FILE_BEGIN;
        break;
    case STREAM_SEEK_CUR:
        dwMoveMethod = FILE_CURRENT;
        break;
    case STREAM_SEEK_END:
        dwMoveMethod = FILE_END;
        break;
    default:
        return STG_E_INVALIDFUNCTION;
        break;
    }

    LARGE_INTEGER newPos;
    if (SetFilePointerEx(m_file, liDistanceToMove, &newPos, dwMoveMethod) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    m_analyzer->AddIStreamSeek(dwOrigin, liDistanceToMove, newPos);

    return S_OK;
}


STDMETHODIMP CAnalyzerWriterInput::Stat(STATSTG* pStatstg, DWORD grfStatFlag) 
{
    if (GetFileSizeEx(m_file, (PLARGE_INTEGER) &pStatstg->cbSize) == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    return S_OK;
}

#pragma endregion

#pragma endregion

#pragma region CAnalyzerWriterFilter

CAnalyzerWriterFilter::CAnalyzerWriterFilter(LPUNKNOWN pUnk, HRESULT *phr)
    : CBaseFilter(_T("Analyzer Writer"), pUnk, &m_csFilter, __uuidof(AnalyzerWriterFilter), phr),
    m_dwFlags(AM_FILE_OVERWRITE), m_file(INVALID_HANDLE_VALUE), m_pPin(NULL)
{	
    ZeroMemory(m_szFileName, sizeof(WCHAR) * MAX_PATH);

    m_analyzer = new CAnalyzer(pUnk);
    if (m_analyzer == NULL)
        *phr = E_OUTOFMEMORY;
    else
        m_analyzer->AddRef();
}


CAnalyzerWriterFilter::~CAnalyzerWriterFilter(void)
{
    if (m_pPin)
		delete m_pPin;

    if (m_analyzer)
        m_analyzer->Release();

	CloseFile();
}

STDMETHODIMP CAnalyzerWriterFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
    CAutoLock lock(&m_csFilter);

    // Do we have this interface
    if (IID_IFileSinkFilter == riid)
        return GetInterface((IFileSinkFilter*) this, ppv);
    else if (IID_IFileSinkFilter2 == riid)
        return GetInterface((IFileSinkFilter2*) this, ppv);
    else if (IID_IMediaSeeking == riid)
        return GetInterface((IMediaSeeking*) this, ppv);
    else if (IID_IAMFilterMiscFlags == riid)
        return GetInterface((IAMFilterMiscFlags*) this, ppv);
    else if (__uuidof(IAnalyzerFilter) == riid)
		return m_analyzer->NonDelegatingQueryInterface(riid, ppv);

    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

int CAnalyzerWriterFilter::GetPinCount()
{
    if(m_file == INVALID_HANDLE_VALUE || m_pPin == NULL)
        return 0;

    return 1;
}

CBasePin* CAnalyzerWriterFilter::GetPin(int n)
{
    if(n == 0 && m_pPin != NULL)
        return m_pPin;

    return NULL;
}

STDMETHODIMP CAnalyzerWriterFilter::GetClassID(CLSID *pClsID)
{
    CheckPointer(pClsID,E_POINTER);
    ValidateReadWritePtr(pClsID,sizeof(CLSID));
    *pClsID = CLSID_FileWriter;
    return NOERROR;
}

#pragma region IFileSinkFilter

/*********************************************************************************************
* IFileSinkFilter implementation
*********************************************************************************************/
STDMETHODIMP CAnalyzerWriterFilter::SetFileName(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt)
{
	// close old file
	if (m_pPin != NULL)
	{
		delete m_pPin;
		m_pPin = NULL;
	}

	CloseFile();

	if (pszFileName == NULL || *pszFileName == L'\0')
	{
		m_szFileName[0] = L'\0';
		return S_OK;
	}

	// create file
	CW2T pszFileNameA(pszFileName); 
	DWORD accessRights = GENERIC_READ | GENERIC_WRITE;
	m_file = CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, m_dwFlags ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD error = GetLastError();
	if(error != 0 && error != ERROR_ALREADY_EXISTS)
		return HRESULT_FROM_WIN32(error);

	if(m_file == INVALID_HANDLE_VALUE)
		return E_FAIL;

    // create Pin
	HRESULT hr;
	m_pPin = new CAnalyzerWriterInput(this, &m_csFilter, &hr, L"Input", m_file, m_analyzer);
	if(m_pPin == NULL)
	{
		CloseFile();
		return E_OUTOFMEMORY;
	}

    // remember filename
    DWORD n = 1 + lstrlenW(pszFileName);
    ZeroMemory(m_szFileName, MAX_PATH * sizeof(WCHAR));
    CopyMemory(m_szFileName, pszFileName, sizeof(WCHAR) * n);

	return S_OK;
}

void CAnalyzerWriterFilter::CloseFile()
{
	if (m_file != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_file);
		m_file = INVALID_HANDLE_VALUE;
	}
}

STDMETHODIMP CAnalyzerWriterFilter::GetCurFile(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt)
{
    *ppszFileName = NULL;

    if (m_szFileName[0]!=NULL)
    {
    	DWORD n = sizeof(WCHAR)*(1+lstrlenW(m_szFileName));

        *ppszFileName = (LPOLESTR) CoTaskMemAlloc( n );
        if (*ppszFileName!=NULL)
            CopyMemory(*ppszFileName, m_szFileName, n);
    }

    if (pmt!=NULL && m_pPin != NULL) {
        m_pPin->ConnectionMediaType(pmt);
    }

    return S_OK;
}

#pragma endregion

#pragma region IFileSinkFilter2

/*********************************************************************************************
* Implementation of IFileSinkFilter2
*********************************************************************************************/
STDMETHODIMP CAnalyzerWriterFilter::SetMode(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
    return S_OK;
}

STDMETHODIMP CAnalyzerWriterFilter::GetMode(DWORD *dwFlags)
{
    CheckPointer(dwFlags, E_POINTER);
    *dwFlags = m_dwFlags;
    return S_OK;
}

#pragma endregion

#pragma region IMediaSeeking

/*********************************************************************************************
* IMediaSeeking implementation
*********************************************************************************************/
STDMETHODIMP CAnalyzerWriterFilter::GetCurrentPosition(LONGLONG *pCurrent)
{
    *pCurrent = 0;
    return S_OK;
}

CComQIPtr<IMediaSeeking> CAnalyzerWriterFilter::GetInputSeeking()
{
    CComQIPtr<IMediaSeeking> ptr;

	if (m_pPin != NULL)
		ptr = m_pPin->GetConnected();

	return ptr;
}

STDMETHODIMP CAnalyzerWriterFilter::GetCapabilities(DWORD * pCapabilities )
{
    // OR together all the pins' capabilities, together with our own
    DWORD caps = AM_SEEKING_CanGetCurrentPos;

	CComQIPtr<IMediaSeeking> pSeek = GetInputSeeking();
    if (pSeek)
    {
		DWORD inputCaps;
        HRESULT hr = pSeek->GetCapabilities(&inputCaps);
        if (SUCCEEDED(hr))
        {
            caps |= inputCaps;
        }
    }
    *pCapabilities = caps;
    return S_OK;
}

STDMETHODIMP CAnalyzerWriterFilter::CheckCapabilities(DWORD * pCapabilities )
{
    DWORD dwActual;
    GetCapabilities(&dwActual);
    if (*pCapabilities & (~dwActual)) {
        return S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP CAnalyzerWriterFilter::IsFormatSupported(const GUID * pFormat)
{
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
        return S_OK;
    return S_FALSE;
}

STDMETHODIMP CAnalyzerWriterFilter::QueryPreferredFormat(GUID * pFormat)
{
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

STDMETHODIMP CAnalyzerWriterFilter::GetTimeFormat(GUID *pFormat)
{
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

STDMETHODIMP CAnalyzerWriterFilter::IsUsingTimeFormat(const GUID * pFormat)
{
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
        return S_OK;
    return S_FALSE;
}

STDMETHODIMP CAnalyzerWriterFilter::SetTimeFormat(const GUID * pFormat)
{
    if ((*pFormat == TIME_FORMAT_MEDIA_TIME) ||
        (*pFormat == TIME_FORMAT_NONE))
        return S_OK;
    return VFW_E_NO_TIME_FORMAT;
}

STDMETHODIMP CAnalyzerWriterFilter::GetDuration(LONGLONG *pDuration)
{
    // length of input duration
	REFERENCE_TIME t = 0;
	CComQIPtr<IMediaSeeking> pSeek = GetInputSeeking();
	if (pSeek)
        pSeek->GetDuration(&t);

    *pDuration = t;
    return S_OK;
}

STDMETHODIMP CAnalyzerWriterFilter::GetStopPosition(LONGLONG *pStop)
{
	CComQIPtr<IMediaSeeking> pSeek = GetInputSeeking();
    if (!pSeek)
        return E_NOINTERFACE;
    return pSeek->GetStopPosition(pStop);
}

STDMETHODIMP CAnalyzerWriterFilter::ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat, LONGLONG Source, const GUID * pSourceFormat )
{
    if (((pTargetFormat == 0) || (*pTargetFormat == TIME_FORMAT_MEDIA_TIME)) &&
        ((pSourceFormat == 0) || (*pSourceFormat == TIME_FORMAT_MEDIA_TIME)))
    {
        *pTarget = Source;
        return S_OK;
    }
    return VFW_E_NO_TIME_FORMAT;
}

STDMETHODIMP CAnalyzerWriterFilter::SetPositions(LONGLONG * pCurrent, DWORD dwCurrentFlags, LONGLONG * pStop, DWORD dwStopFlags )
{
    // must be passed to input
	HRESULT hr = S_OK;
	CComQIPtr<IMediaSeeking> pSeek = GetInputSeeking();
	if (pSeek)
	{
		hr = pSeek->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
		if (SUCCEEDED(hr))
		{
			hr = pSeek->SetPositions(pCurrent, dwCurrentFlags, pStop, dwStopFlags);
			pSeek->SetTimeFormat(&TIME_FORMAT_NONE); // undo
		}
	}
    return hr;
}

STDMETHODIMP CAnalyzerWriterFilter::GetPositions(LONGLONG * pCurrent, LONGLONG * pStop )
{
    // return first valid input
	CComQIPtr<IMediaSeeking> pSeek = GetInputSeeking();
    if (!pSeek)
        return E_NOINTERFACE;
    return pSeek->GetPositions(pCurrent, pStop);
}

STDMETHODIMP CAnalyzerWriterFilter::GetAvailable(LONGLONG * pEarliest, LONGLONG * pLatest)
{
    // the available section is the area for which any
    // data is available -- and here it is not very important whether
    // it is actually available
    *pEarliest = 0;
    return GetDuration(pLatest);
}

STDMETHODIMP CAnalyzerWriterFilter::SetRate(double dRate)
{
    // must be passed to input
	HRESULT hr = S_OK;
	CComQIPtr<IMediaSeeking> pSeek = GetInputSeeking();
	if (pSeek)
	{
		hr = pSeek->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
		if (SUCCEEDED(hr))
		{
			hr = pSeek->SetRate(dRate);
			pSeek->SetTimeFormat(&TIME_FORMAT_NONE); // undo
		}
	}
    return hr;
}

STDMETHODIMP CAnalyzerWriterFilter::GetRate(double * pdRate)
{
    // return first valid input
	CComQIPtr<IMediaSeeking> pSeek = GetInputSeeking();
    if (!pSeek)
        return E_NOINTERFACE;
    return pSeek->GetRate(pdRate);
}

STDMETHODIMP CAnalyzerWriterFilter::GetPreroll(LONGLONG * pllPreroll)
{
    // preroll time needed from input
    REFERENCE_TIME t = 0;
	CComQIPtr<IMediaSeeking> pSeek = GetInputSeeking();
    if (pSeek)
        pSeek->GetPreroll(&t);
    *pllPreroll = t;
    return S_OK;
}

#pragma endregion

#pragma endregion
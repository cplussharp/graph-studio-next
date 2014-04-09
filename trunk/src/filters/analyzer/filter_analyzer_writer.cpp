//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "analyzer_pospassthru.h"

static const AMOVIESETUP_FILTER s_FilterInfo = 
{
	&__uuidof(AnalyzerWriterFilter),				// Filter CLSID
	L"GraphStudioNext Analyzer Writer Filter",		// String name
	MERIT_DO_NOT_USE,								// Filter merit
	0,												// Number pins
	NULL											// Pin details
};

const CFactoryTemplate CAnalyzerWriterFilter::g_Template = 
{
	L"GraphStudioNext Analyzer Writer Filter",
    &__uuidof(AnalyzerWriterFilter),
	CAnalyzerWriterFilter::CreateInstance,
	NULL,
	&s_FilterInfo
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
CAnalyzerWriterInput::CAnalyzerWriterInput(CBaseRenderer *pRenderer, HRESULT *phr, LPCWSTR pPinName, HANDLE* pFile, CAnalyzer* pAnalyzer)
    : CRendererInputPin(pRenderer, phr, pPinName), 
	m_pFile(pFile), 
	m_analyzer(pAnalyzer)
{
    DbgLog((LOG_MEMORY,1,TEXT("AnalyzerWriterInput created")));
}


CAnalyzerWriterInput::~CAnalyzerWriterInput(void)
{
    DbgLog((LOG_MEMORY,1,TEXT("AnalyzerWriterInput destroyed")));
}


STDMETHODIMP CAnalyzerWriterInput::NonDelegatingQueryInterface(REFIID iid, void** ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (IID_IStream == iid)
		return GetInterface((IStream*) this, ppv);
    else if (__uuidof(IStreamBufferDataCounters) == iid)
		return GetInterface((IStreamBufferDataCounters*) this, ppv);
    else if (iid == IID_ISpecifyPropertyPages)
        return GetInterface((ISpecifyPropertyPages*) this, ppv);

	return __super::NonDelegatingQueryInterface(iid, ppv);
}

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

    if (m_pFile == NULL || *m_pFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    BOOL rc = ReadFile(*m_pFile, pv, cb, read, NULL);

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

    if (m_pFile == NULL || *m_pFile == INVALID_HANDLE_VALUE)
        return S_OK;

    if(!WriteFile(*m_pFile, pv, cb, wr, NULL))
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}


STDMETHODIMP CAnalyzerWriterInput::SetSize(ULARGE_INTEGER libNewSize)
{ 
    HRESULT hr = S_OK;

    m_analyzer->AddIStreamSetSize(libNewSize);

    if (m_pFile == NULL || *m_pFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    // get current Pointer
    LARGE_INTEGER movePos = {0};
    LARGE_INTEGER oldPos = {0};
    if (!SetFilePointerEx(*m_pFile, movePos, &oldPos, FILE_CURRENT) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    // move to new file end
    movePos.QuadPart = libNewSize.QuadPart;
    if (!SetFilePointerEx(*m_pFile, movePos, NULL, FILE_BEGIN) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    // neues ende setzen
    if (!SetEndOfFile(*m_pFile))
        hr = HRESULT_FROM_WIN32(GetLastError());

    // filepointer zurücksetzen
    if (!SetFilePointerEx(*m_pFile, oldPos, NULL, FILE_BEGIN))
    {
        if (hr == S_OK)
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;  
}


STDMETHODIMP CAnalyzerWriterInput::CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) 
{ 
    return E_NOTIMPL;   
}


STDMETHODIMP CAnalyzerWriterInput::Commit(DWORD grfCommitFlags)                                      
{ 
    m_analyzer->AddIStreamCommit(grfCommitFlags);

    if (m_pFile == NULL || *m_pFile == INVALID_HANDLE_VALUE)
        return S_OK;

    if (!FlushFileBuffers(*m_pFile))
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
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

    LARGE_INTEGER newPos = { 0 };

    if (m_pFile == NULL || *m_pFile == INVALID_HANDLE_VALUE)
    {
        m_analyzer->AddIStreamSeek(dwOrigin, liDistanceToMove, newPos);
        return S_OK;
    }

    if (SetFilePointerEx(*m_pFile, liDistanceToMove, &newPos, dwMoveMethod) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    m_analyzer->AddIStreamSeek(dwOrigin, liDistanceToMove, newPos);

    return S_OK;
}


STDMETHODIMP CAnalyzerWriterInput::Stat(STATSTG* pStatstg, DWORD grfStatFlag) 
{
    if (m_pFile == NULL || *m_pFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    if (GetFileSizeEx(*m_pFile, (PLARGE_INTEGER) &pStatstg->cbSize) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}

#pragma endregion

#pragma endregion

#pragma region CAnalyzerWriterFilter

CAnalyzerWriterFilter::CAnalyzerWriterFilter(LPUNKNOWN pUnk, HRESULT *phr)
    : CBaseRenderer(__uuidof(AnalyzerWriterFilter), NAME("Analyzer Writer"), pUnk, phr),
    m_dwFlags(AM_FILE_OVERWRITE), 
	m_file(INVALID_HANDLE_VALUE)
{	
    DbgLog((LOG_MEMORY,1,TEXT("AnalyzerWriterFilter created")));

    ZeroMemory(m_szFileName, sizeof(WCHAR) * MAX_PATH);

    m_analyzer = new CAnalyzer(pUnk);
    if (m_analyzer == NULL)
        *phr = E_OUTOFMEMORY;
    else
        m_analyzer->AddRef();
}


CAnalyzerWriterFilter::~CAnalyzerWriterFilter(void)
{
    if (m_analyzer)
        m_analyzer->Release();

	CloseFile();

    DbgLog((LOG_MEMORY,1,TEXT("AnalyzerWriterFilter destroyed")));
}

STDMETHODIMP CAnalyzerWriterFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

	if (IID_IFileSinkFilter == riid)
        return GetInterface((IFileSinkFilter*) this, ppv);
    else if (IID_IFileSinkFilter2 == riid)
        return GetInterface((IFileSinkFilter2*) this, ppv);
	else if (IID_IAMFilterMiscFlags == riid)
        return GetInterface((IAMFilterMiscFlags*) this, ppv);
    else if (__uuidof(IAnalyzerCommon) == riid)
		return m_analyzer->NonDelegatingQueryInterface(riid, ppv);

    return __super::NonDelegatingQueryInterface(riid, ppv);
}

CBasePin* CAnalyzerWriterFilter::GetPin(int n)
{
    CAutoLock cObjectCreationLock(&m_ObjectCreationLock);

    // Should only ever be called with zero
    ASSERT(n == 0);

    if (n != 0) {
        return NULL;
    }

    // Create the input pin if not already done so

    if (m_pInputPin == NULL) {

        // hr must be initialized to NOERROR because
        // CRendererInputPin's constructor only changes
        // hr's value if an error occurs.
        HRESULT hr = NOERROR;

        m_pInputPin = new CAnalyzerWriterInput(this, &hr, L"In", &m_file, m_analyzer);
        if (NULL == m_pInputPin) {
            return NULL;
        }

        if (FAILED(hr)) {
            delete m_pInputPin;
            m_pInputPin = NULL;
            return NULL;
        }
    }
    return m_pInputPin;
}

HRESULT CAnalyzerWriterFilter::CheckMediaType(const CMediaType *pmt)
{
	// accept any type
	return NOERROR;
}

HRESULT CAnalyzerWriterFilter::ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime)
{
	// ignore timestamps
	return NOERROR;
}

HRESULT CAnalyzerWriterFilter::DoRenderSample(IMediaSample* pSample)
{
    m_analyzer->AnalyzeSample(pSample);

    BYTE* pBuf;
    HRESULT hr = pSample->GetPointer(&pBuf);
    if(FAILED(hr)) return hr;

    DWORD length = pSample->GetActualDataLength();
    DWORD written = 0;
    
    // write
    CMediaType mt;
    if (m_pInputPin != NULL && m_pInputPin->ConnectionMediaType(&mt) == S_OK && mt.majortype == MEDIATYPE_Stream)
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

            if (m_file != INVALID_HANDLE_VALUE)
            {
                if(!WriteFile(m_file, pBuf, length, &written, &overlpd))
                    return HRESULT_FROM_WIN32(GetLastError());
            }
            else
                written = length;

            return S_OK;
        }
    }

    if (m_file != INVALID_HANDLE_VALUE)
    {
        if(!WriteFile(m_file, pBuf, length, &written, NULL))
            return HRESULT_FROM_WIN32(GetLastError());
    }
    else
        written = length;

    return S_OK;
}

STDMETHODIMP CAnalyzerWriterFilter::GetClassID(CLSID *pClsID)
{
    CheckPointer(pClsID,E_POINTER);
    ValidateReadWritePtr(pClsID,sizeof(CLSID));
    *pClsID = CLSID_FileWriter;
    return NOERROR;
}

HRESULT CAnalyzerWriterFilter::GetMediaPositionInterface(REFIID riid, __deref_out void **ppv)
{
    CAutoLock cObjectCreationLock(&m_ObjectCreationLock);
    if (m_pPosition) {
        return m_pPosition->NonDelegatingQueryInterface(riid,ppv);
    }

    CBasePin *pPin = GetPin(0);
    if (NULL == pPin) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = NOERROR;

    // Create implementation of this dynamically since sometimes we may
    // never try and do a seek. The helper object implements a position
    // control interface (IMediaPosition) which in fact simply takes the
    // calls normally from the filter graph and passes them upstream

    m_pPosition = new CAnalyzerPosPassThru(NAME("Renderer CPosPassThru"),
                                           CBaseFilter::GetOwner(),
                                           (HRESULT *) &hr,
                                           pPin,
                                           m_analyzer);
    if (m_pPosition == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete m_pPosition;
        m_pPosition = NULL;
        return E_NOINTERFACE;
    }
    return GetMediaPositionInterface(riid,ppv);
}

#pragma region IFileSinkFilter

/*********************************************************************************************
* IFileSinkFilter implementation
*********************************************************************************************/
STDMETHODIMP CAnalyzerWriterFilter::SetFileName(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt)
{
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

    if (pmt!=NULL && m_pInputPin != NULL) {
        m_pInputPin->ConnectionMediaType(pmt);
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

#pragma region ISpecifyPropertyPages

/*********************************************************************************************
* Implementation of ISpecifyPropertyPages
*********************************************************************************************/
STDMETHODIMP CAnalyzerWriterFilter::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 2;
    pPages->pElems = (GUID *) CoTaskMemAlloc(pPages->cElems * sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    pPages->pElems[0] = __uuidof(AnalyzerPropPageLog);
    pPages->pElems[1] = __uuidof(AnalyzerPropPageConfig);
    return NOERROR;
}

#pragma endregion

#pragma endregion
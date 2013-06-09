//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

class CAnalyzerPosPassThru;

//-----------------------------------------------------------------------------
//
//	CAnalyzerWriterInput class
//
//-----------------------------------------------------------------------------

class CAnalyzerWriterInput : public CRenderedInputPin, public IStream, public IStreamBufferDataCounters
{
public:
    CAnalyzerWriterInput(CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr, LPCWSTR pName, HANDLE file, CAnalyzer* pAnalyzer);
    ~CAnalyzerWriterInput(void);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID iid, void** ppv);

    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);

    STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP ReceiveCanBlock() { return S_OK; }

    STDMETHODIMP EndOfStream(void);

    // IStreamBufferDataCounters
    STDMETHODIMP GetData(SBE_PIN_DATA *pPinData);
    STDMETHODIMP ResetData();

    // IStream Methods
    STDMETHOD(Read) (void* pv, ULONG cb, ULONG* pcbRead);
	STDMETHOD(Write) (void const* pv, ULONG cb, ULONG* pcbWritten);
	STDMETHOD(SetSize) (ULARGE_INTEGER);
	STDMETHOD(CopyTo) (IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*); 
	STDMETHOD(Commit) (DWORD);
	STDMETHOD(Revert) (void);
	STDMETHOD(LockRegion) (ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD(UnlockRegion) (ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD(Clone) (IStream **);
	STDMETHOD(Seek) (LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer);
	STDMETHOD(Stat) (STATSTG* pStatstg, DWORD grfStatFlag);

protected:
    CCritSec m_lock;
    HANDLE m_file;
    SBE_PIN_DATA m_pinData;
    CAnalyzer* m_analyzer;
};


//-----------------------------------------------------------------------------
//
//	CAnalyzerWriterFilter class
//
//-----------------------------------------------------------------------------

class CAnalyzerWriterFilter : public CBaseFilter, public IFileSinkFilter2, public IAMFilterMiscFlags, public ISpecifyPropertyPages
{
private:
    CAnalyzer*  m_analyzer;

public:
	CAnalyzerWriterFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CAnalyzerWriterFilter();

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// CBaseFilter methods
    int GetPinCount();
    CBasePin *GetPin(int n);

    STDMETHODIMP GetClassID(CLSID *pClsID); // Fake ClassID; show as FileWriter because some filters only work correct with the default FileWriter (like the MainConcept Mpeg2Demux! => shame on you!)

    // IFileSinkFilter
    STDMETHODIMP SetFileName(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetCurFile( LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt);

    // Implements the IFileSinkFilter2 interface
    STDMETHODIMP SetMode(DWORD dwFlags);
    STDMETHODIMP GetMode(DWORD *dwFlags);

    // Implements the IAMFilterMiscFlags interface
    STDMETHODIMP_(ULONG) GetMiscFlags(void) { return AM_FILTER_MISC_FLAGS_IS_RENDERER; }

    // ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID *pPages);

protected:

    // filter-wide lock
    CCritSec m_csFilter;
    CCritSec m_csTracks;

    // Pins
    CAnalyzerWriterInput* m_pPin;

    WCHAR m_szFileName[MAX_PATH];

    // for IFileSinkFilter2
    DWORD m_dwFlags;

    // File
    HANDLE m_file;

	// IMediaSeeking and IMediaPosition logging
	CAnalyzerPosPassThru*	m_PassThru;

	void CloseFile();
};


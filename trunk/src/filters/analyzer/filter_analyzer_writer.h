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

class CAnalyzerWriterInput : public CRendererInputPin, public IStream
{
public:
    CAnalyzerWriterInput(CBaseRenderer *pRenderer, HRESULT *phr, LPCWSTR pPinName, HANDLE* pFile, CAnalyzer* pAnalyzer);
    ~CAnalyzerWriterInput(void);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID iid, void** ppv);

    // IStream Methods
    STDMETHOD(Read) (void* pv, ULONG cb, ULONG* pcbRead);
	STDMETHOD(Write) (void const* pv, ULONG cb, ULONG* pcbWritten);
	STDMETHOD(SetSize) (ULARGE_INTEGER libNewSize);
	STDMETHOD(CopyTo) (IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*); 
	STDMETHOD(Commit) (DWORD grfCommitFlags);
	STDMETHOD(Revert) (void);
	STDMETHOD(LockRegion) (ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD(UnlockRegion) (ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD(Clone) (IStream **);
	STDMETHOD(Seek) (LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer);
	STDMETHOD(Stat) (STATSTG* pStatstg, DWORD grfStatFlag);

protected:
    CCritSec m_lock;
    HANDLE* m_pFile;
    CAnalyzer* m_analyzer;
};


//-----------------------------------------------------------------------------
//
//	CAnalyzerWriterFilter class
//
//-----------------------------------------------------------------------------

class CAnalyzerWriterFilter : public CBaseRenderer, public IFileSinkFilter2, public IAMFilterMiscFlags, public ISpecifyPropertyPages
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
    CBasePin *GetPin(int n);

    // overriden
	virtual HRESULT CheckMediaType(const CMediaType *pmt);
	virtual HRESULT DoRenderSample(IMediaSample* pSample);

	// ignore time stamps...
	virtual HRESULT ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime);

    HRESULT GetMediaPositionInterface(REFIID riid, __deref_out void **ppv);

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
    WCHAR m_szFileName[MAX_PATH];

    // for IFileSinkFilter2
    DWORD m_dwFlags;

    // File
    HANDLE m_file;

	// IMediaSeeking and IMediaPosition logging
	//CAnalyzerPosPassThru*	m_PassThru;

	void CloseFile();
};


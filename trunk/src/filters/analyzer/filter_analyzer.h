//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once


//-----------------------------------------------------------------------------
//
//	CAnalyzerFilter class
//
//-----------------------------------------------------------------------------

class CAnalyzerFilter : public CTransInPlaceFilter, public ISpecifyPropertyPages
{
private:
    CAnalyzer*			m_analyzer;

	// IBaseFilter overrides ////////////////////////////////////////////////////////////////

	STDMETHODIMP JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName);

	// IMediaFilter overrides ////////////////////////////////////////////////////////////////

	STDMETHODIMP Pause();
	STDMETHODIMP Run(REFERENCE_TIME tStart);
	STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
	STDMETHODIMP Stop();

	// keep track of samples
	virtual HRESULT Transform(IMediaSample *pSample);
	virtual HRESULT StartStreaming();
	virtual HRESULT StopStreaming();
    virtual HRESULT CheckInputType(const CMediaType* mtIn);

    // ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID *pPages);

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

public:
    static const CFactoryTemplate g_Template;

	CAnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CAnalyzerFilter();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    virtual CBasePin *GetPin(int n);

	CAnalyzer* Analyzer() { return m_analyzer; }
};


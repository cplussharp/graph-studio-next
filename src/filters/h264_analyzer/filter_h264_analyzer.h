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
//	CAudioAnalyzerFilter class
//
//-----------------------------------------------------------------------------

class CH264AnalyzerFilter : public CTransInPlaceFilter
{
private:
    CH264Analyzer*  m_analyzer;

public:
	CH264AnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CH264AnalyzerFilter();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// keep track of samples
	virtual HRESULT Transform(IMediaSample *pSample);
    virtual HRESULT CheckInputType(const CMediaType* mtIn);

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
};


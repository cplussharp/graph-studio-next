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

class CAudioAnalyzerFilter : public CTransInPlaceFilter
{
private:
    CAudioAnalyzer*  m_analyzer;

public:
	CAudioAnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CAudioAnalyzerFilter();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// keep track of samples
	virtual HRESULT Transform(IMediaSample *pSample);
    virtual HRESULT CheckInputType(const CMediaType* mtIn);

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
};


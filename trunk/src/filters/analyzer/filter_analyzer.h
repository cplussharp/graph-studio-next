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

class CAnalyzerFilter : public CTransInPlaceFilter
{
private:
    CAnalyzer*  m_analyzer;

public:
	CAnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CAnalyzerFilter();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// keep track of samples
	virtual HRESULT Transform(IMediaSample *pSample);
	virtual HRESULT StartStreaming();
	virtual HRESULT StopStreaming();
    virtual HRESULT CheckInputType(const CMediaType* mtIn);

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
};


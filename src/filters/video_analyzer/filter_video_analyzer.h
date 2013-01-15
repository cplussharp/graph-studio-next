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
//	CVideoAnalyzerFilter class
//
//-----------------------------------------------------------------------------

class CVideoAnalyzerFilter : public CTransInPlaceFilter
{
private:
    CVideoAnalyzer*  m_analyzer;

public:
	CVideoAnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CVideoAnalyzerFilter();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// keep track of samples
	virtual HRESULT Transform(IMediaSample *pSample);
    virtual HRESULT CheckInputType(const CMediaType* mtIn);

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
};


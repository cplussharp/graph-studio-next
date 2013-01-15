//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

#include <vector>

//-----------------------------------------------------------------------------
//
//	CVideoAnalyzer class
//
//-----------------------------------------------------------------------------

class CVideoAnalyzer : public CUnknown, public IVideoAnalyzerFilter
{
private:
    VARIANT_BOOL    m_enabled;
    IVideoAnalyzerFilterCallback* m_callback;

public:
	CVideoAnalyzer(LPUNKNOWN pUnk);
	virtual ~CVideoAnalyzer();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// keep track of samples
	virtual HRESULT AnalyzeSample(IMediaSample *pSample);

	// IVideoAnalyzerFilter
	STDMETHODIMP get_Enabled(VARIANT_BOOL *pVal);
    STDMETHODIMP put_Enabled(VARIANT_BOOL val);
    STDMETHODIMP SetCallback(IVideoAnalyzerFilterCallback* pCallback);
};
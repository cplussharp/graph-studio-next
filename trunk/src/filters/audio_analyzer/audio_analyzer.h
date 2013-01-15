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
//	CAudioAnalyzer class
//
//-----------------------------------------------------------------------------

class CAudioAnalyzer : public CUnknown, public IAudioAnalyzerFilter
{
private:
    VARIANT_BOOL    m_enabled;
    IAudioAnalyzerFilterCallback* m_callback;

public:
	CAudioAnalyzer(LPUNKNOWN pUnk);
	virtual ~CAudioAnalyzer();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// keep track of samples
	virtual HRESULT AnalyzeSample(IMediaSample *pSample);

	// IAudioAnalyzerFilter
	STDMETHODIMP get_Enabled(VARIANT_BOOL *pVal);
    STDMETHODIMP put_Enabled(VARIANT_BOOL val);
    STDMETHODIMP SetCallback(IAudioAnalyzerFilterCallback* pCallback);
};


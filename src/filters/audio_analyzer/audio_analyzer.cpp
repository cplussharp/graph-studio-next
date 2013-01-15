//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CAudioAnalyzer class
//
//-----------------------------------------------------------------------------

CAudioAnalyzer::CAudioAnalyzer(LPUNKNOWN pUnk) :
	CUnknown(NAME("Audio Analyzer"), pUnk),
    m_enabled(VARIANT_TRUE), m_callback(NULL)
{
}

CAudioAnalyzer::~CAudioAnalyzer()
{
    if (m_callback != NULL)
    {
        m_callback->Release();
        m_callback = NULL;
    }
}

STDMETHODIMP CAudioAnalyzer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IAudioAnalyzerFilter)) {
		return GetInterface((IAudioAnalyzerFilter*)this, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CAudioAnalyzer::AnalyzeSample(IMediaSample *pSample)
{
    if (!m_enabled) return S_OK;

	return S_OK;
}


#pragma region IAudioAnalyzerFilter Members

STDMETHODIMP CAudioAnalyzer::get_Enabled(VARIANT_BOOL *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_enabled;
    return S_OK;
}

STDMETHODIMP CAudioAnalyzer::put_Enabled(VARIANT_BOOL val)
{
    m_enabled = val;
    return S_OK;
}

STDMETHODIMP CAudioAnalyzer::SetCallback(IAudioAnalyzerFilterCallback* pCallback)
{
    if (pCallback != NULL)
        pCallback->AddRef();

    if (m_callback != NULL)
        m_callback->Release();

    m_callback = pCallback;

    return S_OK;
}

#pragma endregion
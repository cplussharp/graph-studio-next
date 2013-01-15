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
//	CVideoAnalyzer class
//
//-----------------------------------------------------------------------------

CVideoAnalyzer::CVideoAnalyzer(LPUNKNOWN pUnk) :
	CUnknown(NAME("Video Analyzer"), pUnk),
    m_enabled(VARIANT_TRUE), m_callback(NULL)
{
}

CVideoAnalyzer::~CVideoAnalyzer()
{
    if (m_callback != NULL)
    {
        m_callback->Release();
        m_callback = NULL;
    }
}

STDMETHODIMP CVideoAnalyzer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IVideoAnalyzerFilter)) {
		return GetInterface((IVideoAnalyzerFilter*)this, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CVideoAnalyzer::AnalyzeSample(IMediaSample *pSample)
{
    if (!m_enabled) return S_OK;

	return S_OK;
}


#pragma region IVideoAnalyzerFilter Members

STDMETHODIMP CVideoAnalyzer::get_Enabled(VARIANT_BOOL *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_enabled;
    return S_OK;
}

STDMETHODIMP CVideoAnalyzer::put_Enabled(VARIANT_BOOL val)
{
    m_enabled = val;
    return S_OK;
}

STDMETHODIMP CVideoAnalyzer::SetCallback(IVideoAnalyzerFilterCallback* pCallback)
{
    if (pCallback != NULL)
        pCallback->AddRef();

    if (m_callback != NULL)
        m_callback->Release();

    m_callback = pCallback;

    return S_OK;
}

#pragma endregion
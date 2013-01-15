//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#pragma region CAudioAnalyzerFilter

const CFactoryTemplate CAudioAnalyzerFilter::g_Template = {
		L"Audio Analyzer Filter",
        &__uuidof(AudioAnalyzerFilter),
		CAudioAnalyzerFilter::CreateInstance,
		NULL,
		NULL
	};

CUnknown* CAudioAnalyzerFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CAudioAnalyzerFilter* pNewObject = new CAudioAnalyzerFilter(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//-----------------------------------------------------------------------------
//
//	CAudioAnalyzerFilter class
//
//-----------------------------------------------------------------------------

CAudioAnalyzerFilter::CAudioAnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr) :
	CTransInPlaceFilter(NAME("Audio Analyzer"), pUnk, __uuidof(AudioAnalyzerFilter), phr, false),
        m_analyzer(NULL)
{
    m_analyzer = new CAudioAnalyzer(pUnk);
    if (m_analyzer == NULL)
        *phr = E_OUTOFMEMORY;
    else
        m_analyzer->AddRef();
}

CAudioAnalyzerFilter::~CAudioAnalyzerFilter()
{
    if (m_analyzer)
        m_analyzer->Release();
}

STDMETHODIMP CAudioAnalyzerFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IAudioAnalyzerFilter)) {
		return m_analyzer->NonDelegatingQueryInterface(riid, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CAudioAnalyzerFilter::CheckInputType(const CMediaType* mtIn)
{
	//if (mtIn->majortype != MEDIATYPE_Video && mtIn->majortype != MEDIATYPE_Audio) return E_FAIL;
	return NOERROR;
}

HRESULT CAudioAnalyzerFilter::Transform(IMediaSample *pSample)
{
    return m_analyzer->AnalyzeSample(pSample);
}

#pragma endregion
//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#pragma region CH264AnalyzerFilter

const CFactoryTemplate CH264AnalyzerFilter::g_Template = {
		L"H264 Analyzer Filter",
        &__uuidof(H264AnalyzerFilter),
		CH264AnalyzerFilter::CreateInstance,
		NULL,
		NULL
	};

CUnknown* CH264AnalyzerFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CH264AnalyzerFilter* pNewObject = new CH264AnalyzerFilter(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//-----------------------------------------------------------------------------
//
//	CH264AnalyzerFilter class
//
//-----------------------------------------------------------------------------

CH264AnalyzerFilter::CH264AnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr) :
	CTransInPlaceFilter(NAME("H264 Analyzer Filter"), pUnk, __uuidof(H264AnalyzerFilter), phr, false),
        m_analyzer(NULL)
{
    m_analyzer = new CH264Analyzer(pUnk);
    if (m_analyzer == NULL)
        *phr = E_OUTOFMEMORY;
    else
        m_analyzer->AddRef();
}

CH264AnalyzerFilter::~CH264AnalyzerFilter()
{
    if (m_analyzer)
        m_analyzer->Release();
}

STDMETHODIMP CH264AnalyzerFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IAnalyzerH264)) {
		return m_analyzer->NonDelegatingQueryInterface(riid, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CH264AnalyzerFilter::CheckInputType(const CMediaType* mtIn)
{
    return m_analyzer->CheckInputType(mtIn);
}

HRESULT CH264AnalyzerFilter::Transform(IMediaSample *pSample)
{
    return m_analyzer->AnalyzeSample(pSample);
}

#pragma endregion
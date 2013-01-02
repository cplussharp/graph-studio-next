//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#pragma region CAnalyzerFilter

const CFactoryTemplate CAnalyzerFilter::g_Template = {
		L"Analyzer Filter",
        &__uuidof(AnalyzerFilter),
		CAnalyzerFilter::CreateInstance,
		NULL,
		NULL
	};

CUnknown* CAnalyzerFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CAnalyzerFilter* pNewObject = new CAnalyzerFilter(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//-----------------------------------------------------------------------------
//
//	CAnalyzerFilter class
//
//-----------------------------------------------------------------------------

CAnalyzerFilter::CAnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr) :
	CTransInPlaceFilter(_T("Analyzer"), pUnk, __uuidof(AnalyzerFilter), phr, false),
        m_analyzer(NULL)
{
    m_analyzer = new CAnalyzer(pUnk);
    if (m_analyzer == NULL)
        *phr = E_OUTOFMEMORY;
    else
        m_analyzer->AddRef();
}

CAnalyzerFilter::~CAnalyzerFilter()
{
    if (m_analyzer)
        m_analyzer->Release();
}

STDMETHODIMP CAnalyzerFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IAnalyzerFilter)) {
		return m_analyzer->NonDelegatingQueryInterface(riid, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CAnalyzerFilter::CheckInputType(const CMediaType* mtIn)
{
	//if (mtIn->majortype != MEDIATYPE_Video && mtIn->majortype != MEDIATYPE_Audio) return E_FAIL;
	return NOERROR;
}

HRESULT CAnalyzerFilter::Transform(IMediaSample *pSample)
{
    return m_analyzer->AddSample(pSample);
}

HRESULT CAnalyzerFilter::StartStreaming()
{
    return m_analyzer->StartStreaming();
}

HRESULT CAnalyzerFilter::StopStreaming()
{
    return m_analyzer->StopStreaming();
}

#pragma endregion
//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#pragma region CVideoAnalyzerFilter

const CFactoryTemplate CVideoAnalyzerFilter::g_Template = {
		L"Video Analyzer Filter",
        &__uuidof(VideoAnalyzerFilter),
		CVideoAnalyzerFilter::CreateInstance,
		NULL,
		NULL
	};

CUnknown* CVideoAnalyzerFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CVideoAnalyzerFilter* pNewObject = new CVideoAnalyzerFilter(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//-----------------------------------------------------------------------------
//
//	CVideoAnalyzerFilter class
//
//-----------------------------------------------------------------------------

CVideoAnalyzerFilter::CVideoAnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr) :
	CTransInPlaceFilter(_T("Video Analyzer"), pUnk, __uuidof(AnalyzerFilter), phr, false),
        m_analyzer(NULL)
{
    m_analyzer = new CVideoAnalyzer(pUnk);
    if (m_analyzer == NULL)
        *phr = E_OUTOFMEMORY;
    else
        m_analyzer->AddRef();
}

CVideoAnalyzerFilter::~CVideoAnalyzerFilter()
{
    if (m_analyzer)
        m_analyzer->Release();
}

STDMETHODIMP CVideoAnalyzerFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IVideoAnalyzerFilter)) {
		return m_analyzer->NonDelegatingQueryInterface(riid, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CVideoAnalyzerFilter::CheckInputType(const CMediaType* mtIn)
{
	//if (mtIn->majortype != MEDIATYPE_Video && mtIn->majortype != MEDIATYPE_Audio) return E_FAIL;
	return NOERROR;
}

HRESULT CVideoAnalyzerFilter::Transform(IMediaSample *pSample)
{
    return m_analyzer->AnalyzeSample(pSample);
}

#pragma endregion
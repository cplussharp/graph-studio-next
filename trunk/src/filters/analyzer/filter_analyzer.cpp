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
	CTransInPlaceFilter(NAME("Analyzer"), pUnk, __uuidof(AnalyzerFilter), phr, false),
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

// Modified CTransInPlaceFilter::GetPin override to create our own pin classes
CBasePin * CAnalyzerFilter::GetPin(int n)
{
    HRESULT hr = S_OK;

    // Create an input pin if not already done
    if (m_pInput == NULL) {
        m_pInput = new CAnalyzerInputPin( NAME("CAnalyzer input pin")
                                            , this        // Owner filter
                                            , &hr         // Result code
                                            , L"Input"    // Pin name
                                            );

        // Constructor for CTransInPlaceInputPin can't fail
        ASSERT(SUCCEEDED(hr));
    }

    // Create an output pin if not already done
    if (m_pInput!=NULL && m_pOutput == NULL) {
        m_pOutput = new CAnalyzerOutputPin( NAME("CAnalyzer output pin")
                                              , this       // Owner filter
                                              , &hr        // Result code
                                              , L"Output"  // Pin name
                                              );

        // a failed return code should delete the object
        ASSERT(SUCCEEDED(hr));
        if (m_pOutput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

    // Return the appropriate pin

    ASSERT (n>=0 && n<=1);
    if (n == 0) {
        return m_pInput;
    } else if (n==1) {
        return m_pOutput;
    } else {
        return NULL;
    }

} // GetPin

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


#pragma region CAnalyzerInputPin

CAnalyzerInputPin::CAnalyzerInputPin( 
		__in_opt LPCTSTR					pObjectName
		, __inout CTransInPlaceFilter *		pFilter
		, __inout HRESULT *					phr
		, __in_opt LPCWSTR					pName)
	: CTransInPlaceInputPin(pObjectName, pFilter, phr, pName)
{
}

#pragma endregion


#pragma region CAnalyzerOutputPin

CAnalyzerOutputPin::CAnalyzerOutputPin( 
		__in_opt LPCTSTR				pObjectName
		, __inout CTransInPlaceFilter*	pFilter
		, __inout HRESULT*				phr
		, __in_opt LPCWSTR				pName)
	: CTransInPlaceOutputPin(pObjectName, pFilter, phr, pName)
{
	CAnalyzerFilter * const filter = dynamic_cast<CAnalyzerFilter*>(pFilter);
	m_Analyzer = filter ? filter->Analyzer() : NULL;

}

CAnalyzerOutputPin::~CAnalyzerOutputPin()
{
}

STDMETHODIMP CAnalyzerOutputPin::NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    *ppv = NULL;

    if (riid == __uuidof(IMediaSeeking)) {
		return GetInterface(static_cast<IMediaSeeking*>(this), ppv);
    } else {
        return __super::NonDelegatingQueryInterface(riid, ppv);
    }
}

HRESULT CAnalyzerOutputPin::GetPeerSeeking(__deref_out IMediaSeeking **ppMS)
{
	CheckPointer(ppMS, E_POINTER);

	// Get IMediaSeeking implementation from our base class to delegate to
	return __super::NonDelegatingQueryInterface(__uuidof(IMediaSeeking), (void**)ppMS);
}


STDMETHODIMP
CAnalyzerOutputPin::GetCapabilities(__out DWORD * pCaps)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->GetCapabilities(pCaps);
    pMS->Release();
    return hr;
}

STDMETHODIMP
CAnalyzerOutputPin::CheckCapabilities(__inout DWORD * pCaps)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->CheckCapabilities(pCaps);
    pMS->Release();
    return hr;
}

STDMETHODIMP
CAnalyzerOutputPin::IsFormatSupported(const GUID * pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->IsFormatSupported(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CAnalyzerOutputPin::QueryPreferredFormat(__out GUID *pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->QueryPreferredFormat(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CAnalyzerOutputPin::SetTimeFormat(const GUID * pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

	if (m_Analyzer)
		m_Analyzer->AddMSSetTimeFormat(pFormat);

    hr = pMS->SetTimeFormat(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CAnalyzerOutputPin::GetTimeFormat(__out GUID *pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->GetTimeFormat(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CAnalyzerOutputPin::IsUsingTimeFormat(const GUID * pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->IsUsingTimeFormat(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CAnalyzerOutputPin::ConvertTimeFormat(__out LONGLONG * pTarget, 
                                __in_opt const GUID * pTargetFormat,
				LONGLONG Source, 
                                __in_opt const GUID * pSourceFormat )
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat );
    pMS->Release();
    return hr;
}


STDMETHODIMP
CAnalyzerOutputPin::SetPositions( __inout_opt LONGLONG * pCurrent, 
                            DWORD CurrentFlags, 
                            __inout_opt LONGLONG * pStop, 
                            DWORD StopFlags )
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
		return hr;
    }

	if (m_Analyzer)
		m_Analyzer->AddMSSetPositions(pCurrent, CurrentFlags, pStop, StopFlags);

    hr = pMS->SetPositions(pCurrent, CurrentFlags, pStop, StopFlags );
    pMS->Release();
    return hr;
}

STDMETHODIMP
CAnalyzerOutputPin::GetPositions(__out_opt LONGLONG *pCurrent, __out_opt LONGLONG * pStop)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->GetPositions(pCurrent,pStop);
    pMS->Release();
    return hr;
}

HRESULT
CAnalyzerOutputPin::GetSeekingLongLong
( HRESULT (__stdcall IMediaSeeking::*pMethod)( __out LONGLONG * )
, LONGLONG * pll
)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (SUCCEEDED(hr))
    {
	hr = (pMS->*pMethod)(pll);
	pMS->Release();
    }
    return hr;
}

// If we don't have a current position then ask upstream

STDMETHODIMP
CAnalyzerOutputPin::GetCurrentPosition(__out LONGLONG *pCurrent)
{
	HRESULT hr = GetSeekingLongLong( &IMediaSeeking::GetCurrentPosition, pCurrent );
    return hr;
}


STDMETHODIMP
CAnalyzerOutputPin::GetStopPosition(__out LONGLONG *pStop)
{
    return GetSeekingLongLong( &IMediaSeeking::GetStopPosition, pStop );;
}

STDMETHODIMP
CAnalyzerOutputPin::GetDuration(__out LONGLONG *pDuration)
{
    return GetSeekingLongLong( &IMediaSeeking::GetDuration, pDuration );;
}


STDMETHODIMP
CAnalyzerOutputPin::GetPreroll(__out LONGLONG *pllPreroll)
{
    return GetSeekingLongLong( &IMediaSeeking::GetPreroll, pllPreroll );;
}


STDMETHODIMP
CAnalyzerOutputPin::GetAvailable( __out_opt LONGLONG *pEarliest, __out_opt LONGLONG *pLatest )
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->GetAvailable( pEarliest, pLatest );
    pMS->Release();
    return hr;
}


STDMETHODIMP
CAnalyzerOutputPin::GetRate(__out double * pdRate)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMS->GetRate(pdRate);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CAnalyzerOutputPin::SetRate(double dRate)
{
    if (0.0 == dRate) {
		return E_INVALIDARG;
    }

    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

	if (m_Analyzer)
		m_Analyzer->AddMSSetRate(dRate);

    hr = pMS->SetRate(dRate);
    pMS->Release();
    return hr;
}



#pragma endregion



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
//	CAnalyzerPosPassThru class
//
//-----------------------------------------------------------------------------

class CAnalyzerPosPassThru : public CPosPassThru
{
public:
	CAnalyzerPosPassThru(const TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr, IPin *pPin, CAnalyzer* analyzer)
		: CPosPassThru(pName, pUnk, phr, pPin)
		, m_Analyzer(analyzer)
	{
	}

    STDMETHODIMP SetPositions( __inout_opt LONGLONG * pCurrent, DWORD CurrentFlags
                             , __inout_opt LONGLONG * pStop, DWORD StopFlags )
	{
		if (m_Analyzer)
			m_Analyzer->AddMSSetPositions(pCurrent, CurrentFlags, pStop, StopFlags);
		return __super::SetPositions(pCurrent, CurrentFlags, pStop, StopFlags);
	}

	STDMETHODIMP SetRate(double dRate)
	{
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MS_SetRate, dRate);
		return __super::SetRate(dRate);
	}

	STDMETHODIMP SetTimeFormat(const GUID * pFormat)
	{
		if (m_Analyzer)
			m_Analyzer->AddMSSetTimeFormat(pFormat);
		return __super::SetTimeFormat(pFormat);
	}

    STDMETHODIMP put_CurrentPosition(REFTIME llTime)
	{
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MP_SetCurrentPosition, llTime);
		return __super::put_CurrentPosition(llTime);
	}

    STDMETHODIMP put_StopTime(REFTIME llTime)
	{
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MP_SetStopTime, llTime);
		return __super::put_StopTime(llTime);
	}

    STDMETHODIMP put_PrerollTime(REFTIME llTime)
	{
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MP_SetPrerollTime, llTime);
		return __super::put_PrerollTime(llTime);
	}

    STDMETHODIMP put_Rate(double dRate)
	{
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MP_SetRate, dRate);
		return __super::put_Rate(dRate);
	}

private:
	CAnalyzer*		m_Analyzer;
};


//-----------------------------------------------------------------------------
//
//	CAnalyzerInputPin class
//
//-----------------------------------------------------------------------------

class CAnalyzerInputPin : public CTransInPlaceInputPin
{
public:
	CAnalyzerInputPin( __in_opt LPCTSTR     pObjectName
			, __inout CTransInPlaceFilter	*pFilter
			, __inout HRESULT				*phr
			, __in_opt LPCWSTR				 pName);

protected:
	CAnalyzer* Analyzer()
	{
		CAnalyzerFilter * const filter = dynamic_cast<CAnalyzerFilter*>(m_pFilter);
		return filter ? filter->Analyzer() : NULL;
	}
};

//-----------------------------------------------------------------------------
//
//	CAnalyzerOutputPin class
//
//-----------------------------------------------------------------------------

class CAnalyzerOutputPin : public CTransInPlaceOutputPin
{
public:
    CAnalyzerOutputPin(
        __in_opt LPCTSTR				pObjectName,
        __inout CTransInPlaceFilter	*	pFilter,
        __inout HRESULT	*				phr,
        __in_opt LPCWSTR				pName);
    
	~CAnalyzerOutputPin();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv);

protected:
	CAnalyzer*					m_Analyzer;
	CAnalyzerPosPassThru*		m_PassThru;
};


//-----------------------------------------------------------------------------
//
//	CAnalyzerFilter class
//
//-----------------------------------------------------------------------------

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
	, m_PassThru(NULL)
{
	CAnalyzerFilter * const filter = dynamic_cast<CAnalyzerFilter*>(pFilter);
	m_Analyzer = filter ? filter->Analyzer() : NULL;
}

CAnalyzerOutputPin::~CAnalyzerOutputPin()
{
	delete m_PassThru;
}

STDMETHODIMP CAnalyzerOutputPin::NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    *ppv = NULL;

    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {

		if (!m_PassThru) {
			HRESULT hr = S_OK;
			// we should have an input pin by now
			IPin* const inputPin = m_pFilter->GetPin(0);
			ASSERT(inputPin);
			m_PassThru = new CAnalyzerPosPassThru(_T("Analzyer seeking pass thur"), GetOwner(), &hr, inputPin, m_Analyzer);

			if (FAILED(hr)) {
                return hr;
            }
        }
		return m_PassThru->NonDelegatingQueryInterface(riid, ppv);
    }
	return __super::NonDelegatingQueryInterface(riid, ppv);

}

#pragma endregion



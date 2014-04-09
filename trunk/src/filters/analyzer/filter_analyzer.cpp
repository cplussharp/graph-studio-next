//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "analyzer_pospassthru.h"

//-----------------------------------------------------------------------------
//
//	CAnalyzerInputPin class
//
//-----------------------------------------------------------------------------

class CAnalyzerInputPin : public CTransInPlaceInputPin
{
public:
	CAnalyzerInputPin( __in_opt LPCTSTR			pObjectName
			, __inout CTransInPlaceFilter*		pFilter
			, __inout HRESULT*					phr
			, __in_opt LPCWSTR					pName
			, CAnalyzer*						pAnalyzer)
		: CTransInPlaceInputPin(pObjectName, pFilter, phr, pName)
		, m_Analyzer(pAnalyzer)
	{
	}

protected:
	CAnalyzer*		m_Analyzer;

	// IPin overrides ////////////////////////////////////////////////////////////////

	STDMETHODIMP BeginFlush()
	{
		const HRESULT hr = __super::BeginFlush();
		if (m_Analyzer)
			m_Analyzer->AddHRESULT(SRK_IP_BeginFlush, hr);
		return hr;
	}

	STDMETHODIMP Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
	{
		const HRESULT hr = __super::Connect(pReceivePin, pmt);
		if (m_Analyzer)
			m_Analyzer->AddHRESULT(SRK_IP_Connect, hr);
		return hr;
	}

	STDMETHODIMP Disconnect()
	{
		const HRESULT hr = __super::Disconnect();
		if (m_Analyzer)
			m_Analyzer->AddHRESULT(SRK_IP_Disconnect, hr);
		return hr;
	}

	STDMETHODIMP EndFlush()
	{
		const HRESULT hr = __super::EndFlush();
		if (m_Analyzer)
			m_Analyzer->AddHRESULT(SRK_IP_EndFlush, hr);
		return hr;
	}

	STDMETHODIMP EndOfStream()
	{
		const HRESULT hr = __super::EndOfStream();
		if (m_Analyzer)
			m_Analyzer->AddHRESULT(SRK_IP_EndOfStream, hr);
		return hr;
	}

	STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
	{
		const HRESULT hr = __super::NewSegment(tStart, tStop, dRate);
		if (m_Analyzer)
			m_Analyzer->AddIPNewSegment(tStart, tStop, dRate, hr);
		return hr;
	}

	STDMETHODIMP ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE *pmt)
	{
		const HRESULT hr = __super::ReceiveConnection(pConnector, pmt);
		if (m_Analyzer)
			m_Analyzer->AddHRESULT(SRK_IP_ReceiveConnection, hr);
		return hr;
	}

	// IMemInputPin overrides ////////////////////////////////////////////////////////////////

	STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
	{
		const HRESULT hr = __super::NotifyAllocator(pAllocator, bReadOnly);
		if (m_Analyzer)
			m_Analyzer->AddHRESULT(SRK_MIP_NotifyAllocator, hr);
		return hr;
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

	// IQualityControl overrides ////////////////////////////////////////////////////////////////

	STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
	{
		const HRESULT hr = __super::Notify(pSelf, q);
		if (m_Analyzer)
			m_Analyzer->AddQCNotify(q, hr);
		return hr;
	}

	STDMETHODIMP SetSink(IQualityControl *piqc)
	{
		const HRESULT hr = __super::SetSink(piqc);
		if (m_Analyzer)
			m_Analyzer->AddHRESULT(SRK_QC_SetSink, hr);
		return hr;
	}
};


//-----------------------------------------------------------------------------
//
//	CAnalyzerFilter class
//
//-----------------------------------------------------------------------------

#pragma region CAnalyzerFilter

static const AMOVIESETUP_FILTER s_FilterInfo = 
{
	&__uuidof(AnalyzerFilter),				// Filter CLSID
	L"GraphStudioNext Analyzer Filter",		// String name
	MERIT_DO_NOT_USE,						// Filter merit
	0,										// Number pins
	NULL									// Pin details
};

const CFactoryTemplate CAnalyzerFilter::g_Template = {
		L"GraphStudioNext Analyzer Filter",
        &__uuidof(AnalyzerFilter),
		CAnalyzerFilter::CreateInstance,
		NULL,
		&s_FilterInfo
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

    DbgLog((LOG_MEMORY,1,TEXT("AnalyzerFilter created")));
}

CAnalyzerFilter::~CAnalyzerFilter()
{
    if (m_analyzer)
        m_analyzer->Release();

    DbgLog((LOG_MEMORY,1,TEXT("AnalyzerFilter destroyed")));
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
                                            , m_analyzer);

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
	if (riid == __uuidof(IAnalyzerCommon)) {
		return m_analyzer->NonDelegatingQueryInterface(riid, ppv);
	}
    else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages*) this, ppv);
    }
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CAnalyzerFilter::CheckInputType(const CMediaType* mtIn)
{
	return NOERROR;
}

HRESULT CAnalyzerFilter::Transform(IMediaSample *pSample)
{
    return m_analyzer->AnalyzeSample(pSample);
}

HRESULT CAnalyzerFilter::StartStreaming()
{
    return m_analyzer->StartStreaming();
}

HRESULT CAnalyzerFilter::StopStreaming()
{
    return m_analyzer->StopStreaming();
}

	// IBaseFilter overrides ////////////////////////////////////////////////////////////////

STDMETHODIMP CAnalyzerFilter::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
	const HRESULT hr = __super::JoinFilterGraph(pGraph, pName);
	if (m_analyzer)
		m_analyzer->AddHRESULT(SRK_BF_JoinFilterGraph, hr);
	return hr;
}

// IMediaFilter overrides ////////////////////////////////////////////////////////////////

STDMETHODIMP CAnalyzerFilter::Pause()
{
	const HRESULT hr = __super::Pause();
	if (m_analyzer)
		m_analyzer->AddHRESULT(SRK_BF_Pause, hr);
	return hr;
}

STDMETHODIMP CAnalyzerFilter::Run(REFERENCE_TIME tStart)
{
	const HRESULT hr = __super::Run(tStart);
	if (m_analyzer)
		m_analyzer->AddRefTime(SRK_BF_Run, tStart, hr);
	return hr;
}

STDMETHODIMP CAnalyzerFilter::SetSyncSource(IReferenceClock *pClock)
{
	const HRESULT hr = __super::SetSyncSource(pClock);
	if (m_analyzer) {
		REFERENCE_TIME time = 0LL;
		if (pClock)
			pClock->GetTime(&time);
		m_analyzer->AddRefTime(SRK_BF_SetSyncSource, time, hr);
	}
	return hr;
}

STDMETHODIMP CAnalyzerFilter::Stop()
{
	const HRESULT hr = __super::Stop();
	if (m_analyzer)
		m_analyzer->AddHRESULT(SRK_BF_Stop, hr);
	return hr;
}

STDMETHODIMP CAnalyzerFilter::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 2;
    pPages->pElems = (GUID *) CoTaskMemAlloc(pPages->cElems * sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    pPages->pElems[0] = __uuidof(AnalyzerPropPageLog);
    pPages->pElems[1] = __uuidof(AnalyzerPropPageConfig);
    return NOERROR;
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
			IPin* const inputPin = m_pFilter->GetPin(0);		// should always have an input pin
			ASSERT(inputPin);
			m_PassThru = new CAnalyzerPosPassThru(_T("Analzyer seeking pass thur"), GetOwner(), &hr, inputPin, m_Analyzer);

			if (FAILED(hr)) {
				delete m_PassThru;
				m_PassThru = NULL;
                return hr;
            }
        }
		return m_PassThru->NonDelegatingQueryInterface(riid, ppv);
    }
	return __super::NonDelegatingQueryInterface(riid, ppv);

}

#pragma endregion



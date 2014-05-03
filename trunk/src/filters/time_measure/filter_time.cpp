//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

static const AMOVIESETUP_FILTER s_FilterInfo = 
{
	&__uuidof(TimeMeasureFilter),				// Filter CLSID
	L"GraphStudioNext Time Measure Filter",		// String name
	MERIT_DO_NOT_USE,							// Filter merit
	0,											// Number pins
	NULL										// Pin details
};

const CFactoryTemplate CMonoTimeMeasure::g_Template = {
		L"Time Measure Filter",
        &__uuidof(TimeMeasureFilter),
		CMonoTimeMeasure::CreateInstance,
		NULL,
		&s_FilterInfo
	};

CUnknown* CMonoTimeMeasure::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CMonoTimeMeasure* pNewObject = new CMonoTimeMeasure(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//-----------------------------------------------------------------------------
//
//	CMonoTimeMeasure class
//
//-----------------------------------------------------------------------------

CMonoTimeMeasure::CMonoTimeMeasure(LPUNKNOWN pUnk, HRESULT *phr) :
	CTransInPlaceFilter(TEXT("Time Measure"), pUnk, __uuidof(TimeMeasureFilter), phr, false)
	, start_time(0)
	, stop_time(0)
	, frames_done(0)
    , real_time(0)
{
    DbgLog((LOG_MEMORY,1,TEXT("TimeMeasureFilter created")));
}

CMonoTimeMeasure::~CMonoTimeMeasure()
{
	DbgLog((LOG_MEMORY,1,TEXT("TimeMeasureFilter destroyed")));
}

STDMETHODIMP CMonoTimeMeasure::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(ITimeMeasureFilter)) {
		return GetInterface(static_cast<ITimeMeasureFilter*>(this), ppv);
	}
    else if (riid == __uuidof(IDispatch)) {
		return GetInterface(static_cast<IDispatch*>(this), ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CMonoTimeMeasure::CheckInputType(const CMediaType* mtIn)
{
	if (mtIn->majortype != MEDIATYPE_Video && mtIn->majortype != MEDIATYPE_Audio) return E_FAIL;
	return NOERROR;
}

HRESULT CMonoTimeMeasure::Transform(IMediaSample *pSample)
{
	stop_time = timer.GetTimeNS();
	frames_done ++;

    REFERENCE_TIME timeStart, timeEnd;
    if(SUCCEEDED(pSample->GetTime(&timeStart, &timeEnd)))
        real_time += timeEnd - timeStart;

	return NOERROR;
}

HRESULT CMonoTimeMeasure::StartStreaming()
{
	frames_done = 0;
	stop_time = start_time = timer.GetTimeNS();
    real_time = 0;
	return NOERROR;
}

HRESULT CMonoTimeMeasure::StopStreaming()
{
	stop_time = timer.GetTimeNS();
	return NOERROR;
}

STDMETHODIMP CMonoTimeMeasure::GetStats(__int64 *runtime_ns, __int64 *frames, __int64* realtime_ns)
{
	// return the statistics
	__int64		time = stop_time - start_time;
	if (time < 0) time = 0;

	if (runtime_ns) *runtime_ns = time;
	if (frames) *frames = frames_done;
    if (realtime_ns) *realtime_ns = real_time * 100;

	return NOERROR;
}





/*********************************************************************************************
* IDispatch
*********************************************************************************************/
STDMETHODIMP CMonoTimeMeasure::GetTypeInfoCount(__out UINT * pctinfo)
{
    return m_basedisp.GetTypeInfoCount(pctinfo);
}

STDMETHODIMP CMonoTimeMeasure::GetTypeInfo(UINT itinfo, LCID lcid, __deref_out ITypeInfo ** pptinfo)
{
    return m_basedisp.GetTypeInfo(__uuidof(ITimeMeasureFilter), itinfo, lcid, pptinfo);
}

STDMETHODIMP CMonoTimeMeasure::GetIDsOfNames(REFIID riid, __in_ecount(cNames) LPOLESTR * rgszNames, UINT cNames, LCID lcid, __out_ecount(cNames) DISPID * rgdispid)
{
    return m_basedisp.GetIDsOfNames(__uuidof(ITimeMeasureFilter), rgszNames,	cNames, lcid, rgdispid);
}

STDMETHODIMP CMonoTimeMeasure::Invoke(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    __in DISPPARAMS * pdispparams,
    __out_opt VARIANT * pvarResult,
    __out_opt EXCEPINFO * pexcepinfo,
    __out_opt UINT * puArgErr)
{
    // this parameter is a dead leftover from an earlier interface
    if (IID_NULL != riid) {
	    return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
	    return hr;
    }

    hr = pti->Invoke(
	    (ITimeMeasureFilter*)this,
	    dispidMember,
	    wFlags,
	    pdispparams,
	    pvarResult,
	    pexcepinfo,
	    puArgErr);

    pti->Release();
    return hr;
}



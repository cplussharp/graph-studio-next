//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

const CFactoryTemplate CMonoTimeMeasure::g_Template = {
		L"Time Measure Filter",
        &__uuidof(TimeMeasureFilter),
		CMonoTimeMeasure::CreateInstance,
		NULL,
		NULL
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
	CTransInPlaceFilter(_T("Time Measure"), pUnk, __uuidof(TimeMeasureFilter), phr, false)
	, start_time(0)
	, stop_time(0)
	, frames_done(0)
    , real_time(0)
{
}

CMonoTimeMeasure::~CMonoTimeMeasure()
{
	// nothing yet
}

STDMETHODIMP CMonoTimeMeasure::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(ITimeMeasureFilter)) {
		return GetInterface((ITimeMeasureFilter*)this, ppv);
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









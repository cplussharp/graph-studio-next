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
        &CLSID_MonoTimeMeasure,
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
	CTransInPlaceFilter(_T("Time Measure"), pUnk, CLSID_MonoTimeMeasure, phr, false)
{
	QueryPerformanceFrequency(&frequency);
}

CMonoTimeMeasure::~CMonoTimeMeasure()
{
	// nothing yet
}

STDMETHODIMP CMonoTimeMeasure::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == IID_IMonoTimeMeasure) {
		return GetInterface((IMonoTimeMeasure*)this, ppv);
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
	stop_time = GetTimeNS();
	frames_done ++;

    REFERENCE_TIME timeStart, timeEnd;
    if(SUCCEEDED(pSample->GetTime(&timeStart, &timeEnd)))
        real_time += timeEnd - timeStart;

	return NOERROR;
}

HRESULT CMonoTimeMeasure::StartStreaming()
{
	frames_done = 0;
	start_time = GetTimeNS();
	stop_time = GetTimeNS();
    real_time = 0;
	return NOERROR;
}

HRESULT CMonoTimeMeasure::StopStreaming()
{
	stop_time = GetTimeNS();
	return NOERROR;
}

__int64 CMonoTimeMeasure::GetTimeNS()
{
	// we use high resolution counter to get time with
	// nanosecond precision
	LARGE_INTEGER		time;
	QueryPerformanceCounter(&time);

	// convert to nanoseconds
	return llMulDiv(time.QuadPart, 1000*1000*1000, frequency.QuadPart, 0);
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









//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

// Utility class for using high res performance counter
class HighResTimer
{
private:
	LARGE_INTEGER	frequency;

public:
	HighResTimer()
	{
		QueryPerformanceFrequency(&frequency);
	}

	__int64 GetTimeNS()
	{
		// we use high resolution counter to get time with
		// nanosecond precision
		LARGE_INTEGER		time;
		QueryPerformanceCounter(&time);

		// convert to nanoseconds
		return llMulDiv(time.QuadPart, 1000*1000*1000, frequency.QuadPart, 0);
	}
};


// {9D17EE0E-E2BC-4df0-8BFF-9E32774D5681}
static const GUID CLSID_MonoTimeMeasure = 
{ 0x9d17ee0e, 0xe2bc, 0x4df0, { 0x8b, 0xff, 0x9e, 0x32, 0x77, 0x4d, 0x56, 0x81 } };

// {B278651D-1678-4add-941A-0EFDAD41F930}
static const GUID IID_IMonoTimeMeasure = 
{ 0xb278651d, 0x1678, 0x4add, { 0x94, 0x1a, 0xe, 0xfd, 0xad, 0x41, 0xf9, 0x30 } };

struct __declspec(uuid("B278651D-1678-4add-941A-0EFDAD41F930")) UUID_IMonoTimeMeasure;

DECLARE_INTERFACE_(IMonoTimeMeasure, IUnknown)
{
	STDMETHOD(GetStats)(__int64 *runtime_ns, __int64 *frames, __int64* realtime_ns) PURE;
};


//-----------------------------------------------------------------------------
//
//	CMonoTimeMeasure class
//
//-----------------------------------------------------------------------------

class CMonoTimeMeasure : public CTransInPlaceFilter, public IMonoTimeMeasure
{
public:
	HighResTimer	timer;

	// statistics
	__int64			start_time;
	__int64			stop_time;
	__int64			frames_done;
    __int64         real_time;

public:
	CMonoTimeMeasure(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CMonoTimeMeasure();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// keep track of samples
	virtual HRESULT Transform(IMediaSample *pSample);
	virtual HRESULT StartStreaming();
	virtual HRESULT StopStreaming();
    virtual HRESULT CheckInputType(const CMediaType* mtIn);


	// IMonoTimeMeasure
	STDMETHODIMP GetStats(__int64 *runtime_ns, __int64 *frames, __int64* realtime_ns);

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
};


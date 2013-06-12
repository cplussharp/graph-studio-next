//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
//
//	CMonoTimeMeasure class
//
//-----------------------------------------------------------------------------

class CMonoTimeMeasure : public CTransInPlaceFilter, public ITimeMeasureFilter
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


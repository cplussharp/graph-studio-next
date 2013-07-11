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

    // IDispatch
    STDMETHODIMP GetTypeInfoCount(__out UINT * pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, __deref_out ITypeInfo ** pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, __in_ecount(cNames) LPOLESTR * rgszNames, UINT cNames, LCID lcid, __out_ecount(cNames) DISPID * rgdispid);

    STDMETHODIMP Invoke(
        DISPID dispidMember,
        REFIID riid,
        LCID lcid,
        WORD wFlags,
        __in DISPPARAMS * pdispparams,
        __out_opt VARIANT * pvarResult,
        __out_opt EXCEPINFO * pexcepinfo,
        __out_opt UINT * puArgErr);

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

protected:
    CBaseDispatch m_basedisp;
};


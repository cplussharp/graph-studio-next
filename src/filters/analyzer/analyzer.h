//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

#include <vector>

//-----------------------------------------------------------------------------
//
//	CAnalyzer class
//
//-----------------------------------------------------------------------------

class CAnalyzer : public CUnknown, public IAnalyzerFilter
{
private:
	HighResTimer	timer;
    VARIANT_BOOL    m_enabled;
    int             m_config;
    unsigned short  m_previewSampleByteCount;
    CComBSTR        m_logFile;
    std::vector<StatisticRecordEntry> m_entries;
    IAnalyzerFilterCallback* m_callback;
    const CCrc32          m_crc;

	// Helpers
	void InitEntry(StatisticRecordEntry& entry);
    void AddEntry(const StatisticRecordEntry& entry);

public:
	CAnalyzer(LPUNKNOWN pUnk);
	virtual ~CAnalyzer();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// keep track of samples
	virtual HRESULT AddSample(IMediaSample *pSample);
	virtual HRESULT StartStreaming();
	virtual HRESULT StopStreaming();

	// IStream logging
    virtual HRESULT AddIStreamRead(const void* vp, ULONG cb, ULONG cbReaded);
    virtual HRESULT AddIStreamWrite(const void* vp, ULONG cb);
    virtual HRESULT AddIStreamSeek(DWORD dwOrigin, const LARGE_INTEGER &liDistanceToMove, const LARGE_INTEGER newPos);

	// IMediaSeeking logging
	virtual HRESULT AddMSSetPositions(HRESULT hr, __inout_opt LONGLONG * pCurrent, DWORD CurrentFlags, __inout_opt LONGLONG * pStop, DWORD StopFlags);
    virtual HRESULT AddMSSetTimeFormat(HRESULT hr, const GUID * pFormat);

	// IPin logging
	virtual HRESULT AddIPNewSegment(LONGLONG start, LONGLONG stop, double rate, HRESULT hr);

	// IQualityControl logging
	virtual HRESULT AddQCNotify(Quality q, HRESULT hr);

	// Generic logging
	virtual HRESULT AddDouble(StatisticRecordKind kind, HRESULT hr, double data);
	virtual HRESULT AddHRESULT(StatisticRecordKind kind, HRESULT hr);
	virtual HRESULT AddRefTime(StatisticRecordKind kind, REFERENCE_TIME tStart, HRESULT hr);

	// IAnalyzerFilter
	STDMETHODIMP get_Enabled(VARIANT_BOOL *pVal);
    STDMETHODIMP put_Enabled(VARIANT_BOOL val);
    STDMETHODIMP get_CaptureConfiguration(int *pVal);
    STDMETHODIMP put_CaptureConfiguration(int val);
    STDMETHODIMP get_LogFile(BSTR *pVal);
    STDMETHODIMP put_LogFile(BSTR val);
    STDMETHODIMP get_PreviewSampleByteCount(unsigned short *pVal);
    STDMETHODIMP put_PreviewSampleByteCount(unsigned short val);
    STDMETHODIMP ResetStatistic(void);
    STDMETHODIMP get_EntryCount(__int64 *pVal);
    STDMETHODIMP GetEntry(__int64 nr, StatisticRecordEntry *pVal);
    STDMETHODIMP SetCallback(IAnalyzerFilterCallback* pCallback);


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

protected:
    CBaseDispatch m_basedisp;
};


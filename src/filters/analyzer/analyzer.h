//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

#include <vector>

// Utility class for using high res performance counter
#ifndef HIGHRESTIMER
#define HIGHRESTIMER
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
#endif


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
    unsigned short  m_previewSampleByteCount;
    CComBSTR        m_logFile;
    std::vector<StatisticRecordEntry> m_entries;
    IAnalyzerFilterCallback* m_callback;

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

    virtual HRESULT AddIStreamRead(const void* vp, ULONG cb, ULONG cbReaded);
    virtual HRESULT AddIStreamWrite(const void* vp, ULONG cb);
    virtual HRESULT AddIStreamSeek(DWORD dwOrigin, const LARGE_INTEGER &liDistanceToMove, const LARGE_INTEGER newPos);

	// IAnalyzerFilter
	STDMETHODIMP get_Enabled(VARIANT_BOOL *pVal);
    STDMETHODIMP put_Enabled(VARIANT_BOOL val);
    STDMETHODIMP get_LogFile(BSTR *pVal);
    STDMETHODIMP put_LogFile(BSTR val);
    STDMETHODIMP get_PreviewSampleByteCount(unsigned short *pVal);
    STDMETHODIMP put_PreviewSampleByteCount(unsigned short val);
    STDMETHODIMP ResetStatistic(void);
    STDMETHODIMP get_EntryCount(__int64 *pVal);
    STDMETHODIMP GetEntry(__int64 nr, StatisticRecordEntry *pVal);
    STDMETHODIMP SetCallback(IAnalyzerFilterCallback* pCallback);
};


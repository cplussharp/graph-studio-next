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
//	CVideoAnalyzer class
//
//-----------------------------------------------------------------------------

class CVideoAnalyzer : public CUnknown, public IAnalyzerVideo
{
public:
	CVideoAnalyzer(LPUNKNOWN pUnk);
	virtual ~CVideoAnalyzer();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // Possible Input
    virtual HRESULT CheckInputType(const CMediaType* mtIn);

	// keep track of samples
	virtual HRESULT AnalyzeSample(IMediaSample *pSample);

	// IVideoAnalyzerFilter
    STDMETHODIMP get_Enabled(VARIANT_BOOL *pVal);
    STDMETHODIMP put_Enabled(VARIANT_BOOL val);
    STDMETHODIMP get_CaptureConfiguration(int *pVal);
    STDMETHODIMP put_CaptureConfiguration(int val);
    STDMETHODIMP ResetStatistic(void);
    STDMETHODIMP get_EntryCount(__int64 *pVal);
    STDMETHODIMP SetCallback(IAnalyzerCallback* pCallback);

private:
    VARIANT_BOOL    m_enabled;
    int             m_config;
    IAnalyzerCallback* m_callback;

    sqlite3* m_db;

    // prepared statements
    sqlite3_stmt* m_sqlHistogram;
    sqlite3_stmt* m_sqlEntropy;
};
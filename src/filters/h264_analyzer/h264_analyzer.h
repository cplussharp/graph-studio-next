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
//	CH264Analyzer class
//
//-----------------------------------------------------------------------------

class CH264Analyzer : public CUnknown, public IAnalyzerH264
{
public:
	CH264Analyzer(LPUNKNOWN pUnk);
	virtual ~CH264Analyzer();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // Possible Input
    virtual HRESULT CheckInputType(const CMediaType* mtIn);

	// keep track of samples
	virtual HRESULT AnalyzeSample(IMediaSample *pSample);

    void AddEntry(H264RecordEntry& entry);

	// IH264AnalyzerFilter
    STDMETHODIMP get_Enabled(VARIANT_BOOL *pVal);
    STDMETHODIMP put_Enabled(VARIANT_BOOL val);
    STDMETHODIMP get_CaptureConfiguration(int *pVal);
    STDMETHODIMP put_CaptureConfiguration(int val);
    STDMETHODIMP ResetStatistic(void);
    STDMETHODIMP get_EntryCount(__int64 *pVal);
    STDMETHODIMP SetCallback(IAnalyzerCallback* pCallback);
    STDMETHODIMP GetEntry(__int64 nr, H264RecordEntry *pVal);

private:
    VARIANT_BOOL    m_enabled;
    int             m_config;
    IAnalyzerCallback* m_callback;

    std::vector<H264RecordEntry> m_entries;

    GraphStudio::sps_t m_last_sps;
};


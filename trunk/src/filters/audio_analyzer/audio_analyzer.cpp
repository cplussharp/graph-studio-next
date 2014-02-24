//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CAudioAnalyzer class
//
//-----------------------------------------------------------------------------

CAudioAnalyzer::CAudioAnalyzer(LPUNKNOWN pUnk) :
    CUnknown(NAME("Audio Analyzer"), pUnk), m_enabled(VARIANT_TRUE), m_config(-1), m_callback(NULL)
{
    DbgLog((LOG_MEMORY,1,TEXT("AudioAnalyzer created")));
}

CAudioAnalyzer::~CAudioAnalyzer()
{
    if (m_callback != NULL)
    {
        m_callback->Release();
        m_callback = NULL;
    }

	ResetStatistic();

    DbgLog((LOG_MEMORY,1,TEXT("AudioAnalyzer destroyed")));
}

STDMETHODIMP CAudioAnalyzer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IAnalyzerAudio)) {
		return GetInterface((IAnalyzerAudio*)this, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CAudioAnalyzer::CheckInputType(const CMediaType* mtIn)
{
	CheckPointer(mtIn, E_POINTER);

    if (mtIn->majortype != MEDIATYPE_Audio)
        return VFW_E_TYPE_NOT_ACCEPTED;

    if (mtIn->subtype != MEDIASUBTYPE_PCM)
        return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

HRESULT CAudioAnalyzer::AnalyzeSample(IMediaSample *pSample)
{
    if (!m_enabled) return S_OK;

	return S_OK;
}


#pragma region IAnalyzerAudio Members

STDMETHODIMP CAudioAnalyzer::get_Enabled(VARIANT_BOOL *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_enabled;
    return S_OK;
}

STDMETHODIMP CAudioAnalyzer::put_Enabled(VARIANT_BOOL val)
{
    m_enabled = val;
    return S_OK;
}

STDMETHODIMP CAudioAnalyzer::get_CaptureConfiguration(int *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_config;
    return S_OK;
}

STDMETHODIMP CAudioAnalyzer::put_CaptureConfiguration(int val)
{
    m_config = val;
    return S_OK;
}

STDMETHODIMP CAudioAnalyzer::SetCallback(IAnalyzerCallback* pCallback)
{
    if (pCallback != NULL)
        pCallback->AddRef();

    if (m_callback != NULL)
        m_callback->Release();

    m_callback = pCallback;

    return S_OK;
}

// TODO check memory management of it->aData as we've had memory corruption assertions freeing it
STDMETHODIMP CAudioAnalyzer::ResetStatistic(void)
{
    /*for (std::vector<StatisticRecordEntry>::iterator it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if (it->aData != NULL)
        {
            delete [] it->aData;
            it->aData = NULL;
        }
    }*/

    //m_entries.clear();

    if (m_callback != NULL)
        m_callback->OnResetted();

    return S_OK;
}

STDMETHODIMP CAudioAnalyzer::get_EntryCount(__int64 *pVal)
{
    CheckPointer(pVal, E_POINTER);
    //*pVal = m_entries.size();
    return S_OK;
}

#pragma endregion
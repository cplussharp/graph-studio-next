//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

const CFactoryTemplate CAnalyzerFilter::g_Template = {
		L"Analyzer Filter",
        &__uuidof(AnalyzerFilter),
		CAnalyzerFilter::CreateInstance,
		NULL,
		NULL
	};

CUnknown* CAnalyzerFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CAnalyzerFilter* pNewObject = new CAnalyzerFilter(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//-----------------------------------------------------------------------------
//
//	CAnalyzerFilter class
//
//-----------------------------------------------------------------------------

CAnalyzerFilter::CAnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr) :
	CTransInPlaceFilter(_T("Analyzer"), pUnk, __uuidof(AnalyzerFilter), phr, false),
        m_enabled(VARIANT_TRUE), m_previewSampleByteCount(16), m_callback(NULL)
{
}

CAnalyzerFilter::~CAnalyzerFilter()
{
    if (m_callback != NULL)
    {
        m_callback->Release();
        m_callback = NULL;
    }

	ResetStatistic();
}

STDMETHODIMP CAnalyzerFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IAnalyzerFilter)) {
		return GetInterface((IAnalyzerFilter*)this, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CAnalyzerFilter::CheckInputType(const CMediaType* mtIn)
{
	//if (mtIn->majortype != MEDIATYPE_Video && mtIn->majortype != MEDIATYPE_Audio) return E_FAIL;
	return NOERROR;
}

HRESULT CAnalyzerFilter::Transform(IMediaSample *pSample)
{
    if (!m_enabled) return S_OK;

    // entry
    StatisticRecordEntry entry = { 0 };
    entry.EntryNr = m_entries.size();
    entry.EntryKind = SRK_MediaSample;
    entry.EntryTimeStamp = timer.GetTimeNS();

    // IMediaSample
    entry.IsDiscontinuity = pSample->IsDiscontinuity() == S_OK ? VARIANT_TRUE : VARIANT_FALSE;
    entry.IsPreroll = pSample->IsPreroll() == S_OK ? VARIANT_TRUE : VARIANT_FALSE;
    entry.IsSyncPoint = pSample->IsSyncPoint() == S_OK ? VARIANT_TRUE : VARIANT_FALSE;
    entry.BufferSize = pSample->GetSize();
    entry.ActualDataLength = pSample->GetActualDataLength();

    HRESULT hr;
    hr = pSample->GetTime(&entry.StreamTimeStart, &entry.StreamTimeStop);
    if (FAILED(hr))
        entry.StreamTimeStart = entry.StreamTimeStop = hr;

    hr = pSample->GetMediaTime(&entry.MediaTimeStart, &entry.MediaTimeStop);
    if (FAILED(hr))
        entry.MediaTimeStart = entry.MediaTimeStop = hr;

    AM_MEDIA_TYPE* pMediaType;
    hr = pSample->GetMediaType(&pMediaType);
    if (SUCCEEDED(hr))
    {
        entry.IsMediaTypeChange = VARIANT_TRUE;
        DeleteMediaType(pMediaType);
    }
    else
        entry.IsMediaTypeChange = VARIANT_FALSE;

    // IMediaSample2
    IMediaSample2* pSample2 = NULL;
    if (SUCCEEDED(pSample->QueryInterface(IID_IMediaSample2, (void**)&pSample2)))
    {
        entry.HadIMediaSample2 = VARIANT_TRUE;

        AM_SAMPLE2_PROPERTIES props;
        hr = pSample2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)&props);
        if(SUCCEEDED(hr))
        {
            entry.StreamId = props.dwStreamId;
            entry.SampleFlags = props.dwSampleFlags;
            entry.TypeSpecificFlags = props.dwTypeSpecificFlags;
        }

        pSample2->Release();
    }
    else
        entry.HadIMediaSample2 = VARIANT_FALSE;

    // PreviewBytes
    entry.nDataCount = m_previewSampleByteCount < entry.ActualDataLength ? m_previewSampleByteCount : entry.ActualDataLength;
    if (entry.nDataCount > 0)
    {
        BYTE* pData = NULL;
        hr = pSample->GetPointer(&pData);
        if(SUCCEEDED(hr))
        {
            entry.aData = new BYTE[entry.nDataCount];
            CopyMemory(entry.aData, pData, entry.nDataCount);
        }
        else
            entry.nDataCount = hr;
    }

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

	return S_OK;
}

HRESULT CAnalyzerFilter::StartStreaming()
{
    if (!m_enabled) return S_OK;

    StatisticRecordEntry entry = { 0 };
    entry.EntryNr = m_entries.size();
    entry.EntryKind = SRK_StartStreaming;
    entry.EntryTimeStamp = timer.GetTimeNS();

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

    return S_OK;
}

HRESULT CAnalyzerFilter::StopStreaming()
{
    if (!m_enabled) return S_OK;

	StatisticRecordEntry entry = { 0 };
    entry.EntryNr = m_entries.size();
    entry.EntryKind = SRK_StopStreaming;
    entry.EntryTimeStamp = timer.GetTimeNS();

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

    return S_OK;
}


STDMETHODIMP CAnalyzerFilter::get_Enabled(VARIANT_BOOL *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_enabled;
    return S_OK;
}

STDMETHODIMP CAnalyzerFilter::put_Enabled(VARIANT_BOOL val)
{
    m_enabled = val;
    return S_OK;
}

STDMETHODIMP CAnalyzerFilter::get_LogFile(BSTR *pVal)
{
    CheckPointer(pVal, E_POINTER);
    return E_NOTIMPL;
}

STDMETHODIMP CAnalyzerFilter::put_LogFile(BSTR val)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAnalyzerFilter::get_PreviewSampleByteCount(unsigned short *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_previewSampleByteCount;
    return S_OK;
}

STDMETHODIMP CAnalyzerFilter::put_PreviewSampleByteCount(unsigned short val)
{
    m_previewSampleByteCount = val;
    return S_OK;
}

STDMETHODIMP CAnalyzerFilter::ResetStatistic(void)
{
    for (vector<StatisticRecordEntry>::iterator it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if (it->aData != NULL)
        {
            delete [] it->aData;
            it->aData = NULL;
        }
    }

    m_entries.clear();

    if (m_callback != NULL)
        m_callback->OnStatisticResetted();

    return S_OK;
}

STDMETHODIMP CAnalyzerFilter::get_EntryCount(__int64 *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_entries.size();
    return S_OK;
}

STDMETHODIMP CAnalyzerFilter::GetEntry(__int64 nr, StatisticRecordEntry *pVal)
{
    CheckPointer(pVal, E_POINTER);
    if (nr < 0 || nr >= m_entries.size())
    {
        ZeroMemory(pVal, sizeof(StatisticRecordEntry));
        return E_INVALIDARG;
    }

    const StatisticRecordEntry entry = m_entries.at(nr);
    CopyMemory(pVal, &entry, sizeof(StatisticRecordEntry));
    if (entry.nDataCount > 0)
    {
        pVal->aData = (BYTE*)CoTaskMemAlloc(pVal->nDataCount);
        CopyMemory(pVal->aData, entry.aData, entry.nDataCount);
    }

    return S_OK;
}

STDMETHODIMP CAnalyzerFilter::SetCallback(IAnalyzerFilterCallback* pCallback)
{
    if (pCallback != NULL)
        pCallback->AddRef();

    if (m_callback != NULL)
        m_callback->Release();

    m_callback = pCallback;

    return S_OK;
}

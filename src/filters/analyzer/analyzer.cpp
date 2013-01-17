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
//	CAnalyzer class
//
//-----------------------------------------------------------------------------

CAnalyzer::CAnalyzer(LPUNKNOWN pUnk) :
	CUnknown(NAME("Analyzer"), pUnk),
    m_enabled(VARIANT_TRUE), m_previewSampleByteCount(16), m_callback(NULL)
{
}

CAnalyzer::~CAnalyzer()
{
    if (m_callback != NULL)
    {
        m_callback->Release();
        m_callback = NULL;
    }

	ResetStatistic();
}

STDMETHODIMP CAnalyzer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IAnalyzerFilter)) {
		return GetInterface((IAnalyzerFilter*)this, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

void CAnalyzer::InitEntry(StatisticRecordEntry& entry)
{
    entry.EntryNr = m_entries.size();
    entry.EntryTimeStamp = timer.GetTimeNS();
	entry.StreamTimeStart = entry.StreamTimeStop = VFW_E_SAMPLE_TIME_NOT_SET;
	entry.MediaTimeStart = entry.MediaTimeStop = VFW_E_MEDIA_TIME_NOT_SET;
}

HRESULT CAnalyzer::AddSample(IMediaSample *pSample)
{
    if (!m_enabled) return S_OK;

    // entry
    StatisticRecordEntry entry = { 0 };
	InitEntry(entry);
    entry.EntryKind = SRK_MediaSample;

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

HRESULT CAnalyzer::StartStreaming()
{
    if (!m_enabled) return S_OK;

    StatisticRecordEntry entry = { 0 };
	InitEntry(entry);
    entry.EntryKind = SRK_StartStreaming;

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

    return S_OK;
}

HRESULT CAnalyzer::StopStreaming()
{
    if (!m_enabled) return S_OK;

	StatisticRecordEntry entry = { 0 };
	InitEntry(entry);
    entry.EntryKind = SRK_StopStreaming;

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

    return S_OK;
}


HRESULT CAnalyzer::AddIStreamRead(const void* vp, ULONG cb, ULONG cbReaded)
{
    if (!m_enabled) return S_OK;

	StatisticRecordEntry entry = { 0 };
	InitEntry(entry);
    entry.EntryKind = SRK_IS_Read;

    entry.StreamTimeStart = cb;
    entry.ActualDataLength = cbReaded;
    entry.nDataCount = m_previewSampleByteCount < entry.ActualDataLength ? m_previewSampleByteCount : entry.ActualDataLength;
    if (entry.nDataCount > 0)
    {
        entry.aData = new BYTE[entry.nDataCount];
        CopyMemory(entry.aData, vp, entry.nDataCount);
    }

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

    return S_OK;
}

HRESULT CAnalyzer::AddIStreamWrite(const void* vp, ULONG cb)
{
    if (!m_enabled) return S_OK;

	StatisticRecordEntry entry = { 0 };
	InitEntry(entry);
    entry.EntryKind = SRK_IS_Write;

    entry.ActualDataLength = cb;
    entry.nDataCount = m_previewSampleByteCount < entry.ActualDataLength ? m_previewSampleByteCount : entry.ActualDataLength;
    if (entry.nDataCount > 0)
    {
        entry.aData = new BYTE[entry.nDataCount];
        CopyMemory(entry.aData, vp, entry.nDataCount);
    }

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

    return S_OK;
}

HRESULT CAnalyzer::AddIStreamSeek(DWORD dwOrigin, const LARGE_INTEGER &liDistanceToMove, const LARGE_INTEGER newPos)
{
    if (!m_enabled) return S_OK;

	StatisticRecordEntry entry = { 0 };
	InitEntry(entry);
    entry.EntryKind = SRK_IS_Seek;

    entry.StreamTimeStart = newPos.QuadPart;
    entry.SampleFlags = dwOrigin;

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

    return S_OK;
}

HRESULT CAnalyzer::AddMSSetPositions(__inout_opt LONGLONG * pCurrent, DWORD CurrentFlags, __inout_opt LONGLONG * pStop, DWORD StopFlags)
{
    if (!m_enabled) return S_OK;

	StatisticRecordEntry entry = { 0 };
	InitEntry(entry);
	entry.EntryKind = SRK_MS_SetPositions;

	if (pCurrent && (CurrentFlags & AM_SEEKING_PositioningBitsMask)) {
		entry.MediaTimeStart = *pCurrent;
		entry.TypeSpecificFlags = CurrentFlags;
	}

	if (pStop && (StopFlags & AM_SEEKING_PositioningBitsMask)) {
		entry.MediaTimeStop  = *pStop;
		entry.SampleFlags = StopFlags;
	}

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

    return S_OK;
}

HRESULT CAnalyzer::AddMSSetRate(double dRate)
{
    if (!m_enabled) return S_OK;

	StatisticRecordEntry entry = { 0 };
	InitEntry(entry);
	entry.EntryKind = SRK_MS_SetRate;

	entry.MediaTimeStart = 100.0 * dRate;

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

	return S_OK;
}

HRESULT CAnalyzer::AddMSSetTimeFormat(const GUID * pFormat)
{
    if (!m_enabled) return S_OK;

	StatisticRecordEntry entry = { 0 };
	InitEntry(entry);
	entry.EntryKind = SRK_MS_SetTimeFormat;

	if (pFormat) {
		entry.nDataCount = sizeof(*pFormat);
        entry.aData = new BYTE[entry.nDataCount];
        CopyMemory(entry.aData, pFormat, entry.nDataCount);
	}

    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnStatisticNewEntry(entry.EntryNr);

	return S_OK;
}


#pragma region IAnalyzerFilter Members

STDMETHODIMP CAnalyzer::get_Enabled(VARIANT_BOOL *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_enabled;
    return S_OK;
}

STDMETHODIMP CAnalyzer::put_Enabled(VARIANT_BOOL val)
{
    m_enabled = val;
    return S_OK;
}

STDMETHODIMP CAnalyzer::get_LogFile(BSTR *pVal)
{
    CheckPointer(pVal, E_POINTER);
    return E_NOTIMPL;
}

STDMETHODIMP CAnalyzer::put_LogFile(BSTR val)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAnalyzer::get_PreviewSampleByteCount(unsigned short *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_previewSampleByteCount;
    return S_OK;
}

STDMETHODIMP CAnalyzer::put_PreviewSampleByteCount(unsigned short val)
{
    m_previewSampleByteCount = val;
    return S_OK;
}

// TODO check memory management of it->aData as we've had memory corruption assertions freeing it
STDMETHODIMP CAnalyzer::ResetStatistic(void)
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

STDMETHODIMP CAnalyzer::get_EntryCount(__int64 *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_entries.size();
    return S_OK;
}

STDMETHODIMP CAnalyzer::GetEntry(__int64 nr, StatisticRecordEntry *pVal)
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

STDMETHODIMP CAnalyzer::SetCallback(IAnalyzerFilterCallback* pCallback)
{
    if (pCallback != NULL)
        pCallback->AddRef();

    if (m_callback != NULL)
        m_callback->Release();

    m_callback = pCallback;

    return S_OK;
}

#pragma endregion
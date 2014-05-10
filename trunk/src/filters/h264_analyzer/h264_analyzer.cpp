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
//	CH264Analyzer class
//
//-----------------------------------------------------------------------------

CH264Analyzer::CH264Analyzer(LPUNKNOWN pUnk) :
    CUnknown(NAME("H264 Analyzer"), pUnk), m_enabled(VARIANT_TRUE), m_config(-1), m_callback(NULL)
{
    ZeroMemory(&m_last_sps, sizeof(GraphStudio::sps_t));

    DbgLog((LOG_MEMORY,1,TEXT("H264Analyzer created")));
}

CH264Analyzer::~CH264Analyzer()
{
    if (m_callback != NULL)
    {
        m_callback->Release();
        m_callback = NULL;
    }

	ResetStatistic();

    DbgLog((LOG_MEMORY,1,TEXT("H264Analyzer destroyed")));
}

STDMETHODIMP CH264Analyzer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IAnalyzerH264)) {
		return GetInterface(static_cast<IAnalyzerH264*>(this), ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CH264Analyzer::CheckInputType(const CMediaType* mtIn)
{
	CheckPointer(mtIn, E_POINTER);

    if (mtIn->majortype != MEDIATYPE_Video)
        return VFW_E_TYPE_NOT_ACCEPTED;

    if (mtIn->subtype != MEDIASUBTYPE_H264 && mtIn->subtype != MEDIASUBTYPE_h264 && mtIn->subtype != MEDIASUBTYPE_H264_bis)
        return VFW_E_TYPE_NOT_ACCEPTED;

	return S_OK;
}

HRESULT CH264Analyzer::AnalyzeSample(IMediaSample *pSample)
{
    if (!m_enabled) return S_OK;

    long sampleDataLength = pSample->GetActualDataLength();

    BYTE* pData = NULL;
    if (FAILED(pSample->GetPointer(&pData)))
        return S_OK;

    __int64 sampleTimeStart, sampleTimeStop;
    HRESULT hr = pSample->GetTime(&sampleTimeStart, &sampleTimeStop);
    if (FAILED(hr))
        sampleTimeStart = hr;

    GraphStudio::CBitStreamReader br(pData, sampleDataLength);

    int lastNullBytes = 0;
    // NALU Startcode suchen
    while (!br.IsEnd())
    {
        BYTE val = br.ReadU8();

        if (val == 0) lastNullBytes++;
        else if (val == 1 && lastNullBytes >= 3)
        {
            UINT8 nalType = br.ReadU8() & 0x1f;
            switch (nalType)
            {
            case 1: // SLICE_IDR
            case 5: // NON_IDR 
            case 16: //CODED_SLICE_AUX
                {
                    GraphStudio::slice_header_t sh = {0};
                    GraphStudio::CH264StructReader::ReadSliceHeader(br, sh, m_last_sps, nalType == 5);

                    H264RecordEntry entry = { 0, HRK_Slice, sampleTimeStart, sizeof(GraphStudio::slice_header_t)};
                    entry.aData = new BYTE[entry.nDataCount];
                    CopyMemory(entry.aData, &sh, entry.nDataCount);
                    AddEntry(entry);

                    DbgLog((LOG_TRACE,0,TEXT("H264Analyzer::Slice Type %d => FrameNr %d"), sh.slice_type, sh.frame_num));
                }
                break;

            case 6: // parse SEI Message Header
                {
                    GraphStudio::sei_t sei = {0};
                    GraphStudio::CH264StructReader::ReadSEI(br, sei);

                    H264RecordEntry entry = { 0, (H264RecordKind)(HCF_SEI + sei.payloadType), sampleTimeStart};
                    entry.nDataCount = 0;
                    entry.aData = NULL;
                    AddEntry(entry);

                    if (sei.payload != NULL)
                        delete [] sei.payload;
                }
                break;

            case 7:
                {
                    GraphStudio::sps_t sps = {0};
                    GraphStudio::CH264StructReader::ReadSPS(br, sps);

                    H264RecordEntry entry = { 0, HRK_SPS, sampleTimeStart, sizeof(GraphStudio::sps_t)};
                    entry.aData = new BYTE[entry.nDataCount];
                    CopyMemory(entry.aData, &sps, entry.nDataCount);
                    AddEntry(entry);

                    CopyMemory(&m_last_sps, &sps, sizeof(GraphStudio::sps_t));
                }
                break;

            case 8:
                {
                    GraphStudio::pps_t pps = {0};
                    GraphStudio::CH264StructReader::ReadPPS(br, pps);

                    H264RecordEntry entry = { 0, HRK_PPS, sampleTimeStart, sizeof(GraphStudio::pps_t)};
                    entry.aData = new BYTE[entry.nDataCount];
                    CopyMemory(entry.aData, &pps, entry.nDataCount);
                    AddEntry(entry);
                }
                break;
            }

            // zum vollen byte springen
            br.SetPos(br.GetPos());
            lastNullBytes = 0;
        }
        else
            lastNullBytes = 0;
    }

	return S_OK;
}

void CH264Analyzer::AddEntry(H264RecordEntry& entry)
{
    entry.EntryNr = m_entries.size();
    m_entries.push_back(entry);

    if (m_callback != NULL)
        m_callback->OnNewEntry((unsigned long) entry.EntryNr);

    DbgLog((LOG_TRACE,0,TEXT("H264Analyzer::Add %I64d => 0x%x"), entry.StreamTimeStart, entry.EntryKind));
}


#pragma region IAnalyzerH264 Members

STDMETHODIMP CH264Analyzer::get_Enabled(VARIANT_BOOL *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_enabled;
    return S_OK;
}

STDMETHODIMP CH264Analyzer::put_Enabled(VARIANT_BOOL val)
{
    m_enabled = val;
    return S_OK;
}

STDMETHODIMP CH264Analyzer::get_CaptureConfiguration(int *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_config;
    return S_OK;
}

STDMETHODIMP CH264Analyzer::put_CaptureConfiguration(int val)
{
    m_config = val;
    return S_OK;
}

STDMETHODIMP CH264Analyzer::SetCallback(IAnalyzerCallback* pCallback)
{
    if (pCallback != NULL)
        pCallback->AddRef();

    if (m_callback != NULL)
        m_callback->Release();

    m_callback = pCallback;

    return S_OK;
}

// TODO check memory management of it->aData as we've had memory corruption assertions freeing it
STDMETHODIMP CH264Analyzer::ResetStatistic(void)
{
    for (std::vector<H264RecordEntry>::iterator it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if (it->aData != NULL)
        {
            delete [] it->aData;
            it->aData = NULL;
        }
    }

    m_entries.clear();

    if (m_callback != NULL)
        m_callback->OnResetted();

    return S_OK;
}

STDMETHODIMP CH264Analyzer::get_EntryCount(__int64 *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_entries.size();
    return S_OK;
}

STDMETHODIMP CH264Analyzer::GetEntry(__int64 nr, H264RecordEntry *pVal)
{
    CheckPointer(pVal, E_POINTER);
    if (nr < 0 || (size_t) nr >= m_entries.size())
    {
        ZeroMemory(pVal, sizeof(H264RecordEntry));
        return E_INVALIDARG;
    }

    const H264RecordEntry entry = m_entries.at((size_t) nr);
    CopyMemory(pVal, &entry, sizeof(H264RecordEntry));
    if (entry.nDataCount > 0)
    {
        pVal->aData = (BYTE*)CoTaskMemAlloc(pVal->nDataCount);
		if (!pVal->aData) return E_OUTOFMEMORY;
        CopyMemory(pVal->aData, entry.aData, entry.nDataCount);
    }

    return S_OK;
}

#pragma endregion
//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

// Setup data
const AMOVIESETUP_FILTER sudPsiConfig =
{
    &__uuidof(PsiConfigFilter),     // Filter CLSID
    L"PSI Config",                  // String name
    MERIT_DO_NOT_USE,               // Filter merit
    0,
    NULL
};

// class ID and creator function for class factory
const CFactoryTemplate CPsiConfigFilter::g_Template = {
    L"PSI Config",
    &__uuidof(PsiConfigFilter),
    CPsiConfigFilter::CreateInstance,
    NULL,
    &sudPsiConfig
};

CUnknown * WINAPI CPsiConfigFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CPsiConfigFilter(pUnk, phr);
}

//  This table was generated with the polynomial 0x04c11db7.  It is used to
//  check the CRC on MPEG-2 PSI, as defined in H.222.0, Systems specification
static ULONG g_MPEG2_PSI_CRC_32_Lookup [] = {
0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f,
0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75, 0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d, 0xd4326d90, 0xd0f37027,
0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c,
0x2e003dc5, 0x2ac12072, 0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba, 0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044,
0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59,
0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53, 0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b, 0xd727bbb6, 0xd3e6a601,
0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad,
0x81b02d74, 0x857130c3, 0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec, 0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12,
0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06,
0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4 } ;


//-----------------------------------------------------------------------------
//
//	CPsiConfigFilter class
//
//-----------------------------------------------------------------------------

CPsiConfigFilter::CPsiConfigFilter(LPUNKNOWN pUnk, HRESULT *phr)
    : CBaseFilter(TEXT("CPSIConfig"), pUnk, &m_Lock, __uuidof(PsiConfigFilter)),
    m_pDemux(NULL), m_pMediaCtrl(NULL), m_countPayloadPins(0), m_payloadPins(NULL), m_bRenderPayloadParserOnNextStop(false)
{
    // Create the single input pin
    m_pInputPin = new CPsiParserInputPin(this, GetOwner(), &m_Lock, &m_ReceiveLock, phr);
    if(m_pInputPin == NULL)
    {
        if (phr) *phr = E_OUTOFMEMORY;
    }
}

CPsiConfigFilter::~CPsiConfigFilter()
{
    if(m_pDemux)
    {
        m_pDemux->Release();
        m_pDemux = NULL;
    }

    if(m_pInputPin)
    {
        delete m_pInputPin;
        m_pInputPin = NULL;
    }

    if(m_countPayloadPins > 0)
    {
        for(int i=0; i<m_countPayloadPins; i++)
            delete m_payloadPins[i];

        delete [] m_payloadPins;
    }

    if(m_pMediaCtrl)
    {
        m_pMediaCtrl->Release();
        m_pMediaCtrl = NULL;
    }
}

STDMETHODIMP CPsiConfigFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (IID_IAMFilterMiscFlags == riid)
        return GetInterface((IAMFilterMiscFlags*) this, ppv);

    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

void CPsiConfigFilter::ConfigurePmtSectionsOnDemux(MPEG2_PMT_SECTION* pmtsec)
{
    // from MPEG2StreamType http://msdn.microsoft.com/en-us/library/windows/desktop/dd695092(v=vs.85).aspx
    static const DWORD ST_MPEG1_VIDEO = ISO_IEC_11172_2_VIDEO;
    static const DWORD ST_MPEG2_VIDEO = ISO_IEC_13818_2_VIDEO;
    static const DWORD ST_MPEG1_AUDIO = ISO_IEC_11172_3_AUDIO;
    static const DWORD ST_MPEG2_AUDIO = ISO_IEC_13818_3_AUDIO;
    static const DWORD ST_AC3_AUDIO = DOLBY_AC3_AUDIO;

    if(!m_pDemux || !pmtsec) return;
    if(pmtsec->number_of_elementary_streams == 0) return;

    bool hasEsToConfig = false;
    for(int i = 0;i<(int)pmtsec->number_of_elementary_streams; i++)
    {
        DWORD streamType = pmtsec->elementary_stream_info[i].stream_type;
        if(streamType == ST_MPEG1_VIDEO ||
           streamType == ST_MPEG2_VIDEO ||
           streamType == ST_MPEG1_AUDIO ||
           streamType == ST_MPEG2_AUDIO ||
           streamType == ST_AC3_AUDIO)
        {
            hasEsToConfig = true;

            AM_MEDIA_TYPE mt = {0};
            switch(streamType)
            {
            case ST_MPEG1_VIDEO:
                mt.majortype = MEDIATYPE_Video;
                mt.subtype = MEDIASUBTYPE_MPEG1Video;
                break;
            case ST_MPEG2_VIDEO:
                mt.majortype = MEDIATYPE_Video;
                mt.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
                //mt.formattype = FORMAT_MPEG2Video;
                break;
            case ST_MPEG1_AUDIO:
                mt.majortype = MEDIATYPE_Audio;
                mt.subtype = MEDIASUBTYPE_MPEG1AudioPayload;
                //mt.formattype = FORMAT_WaveFormatEx;
                break;
            case ST_MPEG2_AUDIO:
                mt.majortype = MEDIATYPE_Audio;
                mt.subtype = MEDIASUBTYPE_MPEG1AudioPayload;
                //mt.formattype = FORMAT_WaveFormatEx;
                break;
            case ST_AC3_AUDIO:
                mt.majortype = MEDIATYPE_Audio;
                mt.subtype = MEDIASUBTYPE_DOLBY_AC3;
                //mt.formattype = FORMAT_WaveFormatEx;
                break;
            }

            mt.bFixedSizeSamples = mt.majortype == MEDIATYPE_Audio ? TRUE : FALSE;
            mt.bTemporalCompression = mt.majortype == MEDIATYPE_Audio ? FALSE : TRUE;

            TCHAR tszPinName[50];
            StringCchPrintf(tszPinName, 50, TEXT("PID %d @ P# %d"), pmtsec->elementary_stream_info[i].elementary_PID, pmtsec->program_number); 

            IPin* pPin = NULL;
            if(SUCCEEDED(m_pDemux->CreateOutputPin(&mt, tszPinName, &pPin)))
            {
                IMPEG2PIDMap* pidMap = NULL;
                pPin->QueryInterface(__uuidof(IMPEG2PIDMap), (void**)&pidMap);
                if(pidMap)
                {
                    ULONG Pid[] = { pmtsec->elementary_stream_info[i].elementary_PID }; // Map any desired PIDs. 
                    ULONG cPid = 1;
                    pidMap->MapPID(cPid, Pid, MEDIA_ELEMENTARY_STREAM);
                    pidMap->Release();
                }

                AddPayloadParserPin(pPin, IsEqualGUID(mt.majortype, MEDIATYPE_Video) != FALSE);
            }
        }
    }

    if(hasEsToConfig)
    {
        if(m_pMediaCtrl == NULL)
            m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pMediaCtrl);

        m_threadPinConnector.m_pFilter = this;
        m_threadPinConnector.Create();
    }
}

void CPsiConfigFilter::AddPayloadParserPin(IPin* pConnectToPin, bool forVideo)
{
    CPayloadParserInputPin** pins = new CPayloadParserInputPin*[m_countPayloadPins + 1];

    for(int i = 0; i<m_countPayloadPins; i++)
        pins[i] = m_payloadPins[i];

    HRESULT hr;
    pins[m_countPayloadPins] = new CPayloadParserInputPin(this, GetOwner(), &m_Lock, &m_ReceiveLock, pConnectToPin, forVideo, &hr);

    if(m_payloadPins) delete [] m_payloadPins;
    m_payloadPins = pins;
    m_countPayloadPins++;
}

void CPsiConfigFilter::PayloadParserPinReady()
{
    for(int i = 0; i<m_countPayloadPins; i++)
        if(!m_payloadPins[i]->readyToReconfigDemuxer)
            return;

    m_threadPinReconfigurer.m_pFilter = this;
    m_threadPinReconfigurer.Create();
}


//-----------------------------------------------------------------------------
//
//	CPsiParserInputPin class
//
//-----------------------------------------------------------------------------

CPsiParserInputPin::CPsiParserInputPin( CPsiConfigFilter *pFilter, LPUNKNOWN pUnk, CCritSec *pLock, CCritSec *pReceiveLock, HRESULT *phr)
: CRenderedInputPin(NAME("CPSIParserInputPin"), (CBaseFilter *) pFilter, pLock, phr, L"Input"),
    m_pFilter(pFilter), m_pReceiveLock(pReceiveLock), m_pPatProcessor(NULL), m_pPmtProcessor(NULL), m_bCreated(false)
{
}

CPsiParserInputPin::~CPsiParserInputPin()
{
	if(m_pPmtProcessor != NULL)
		delete m_pPmtProcessor;

	if(m_pPatProcessor != NULL)
		delete m_pPatProcessor;
}

HRESULT CPsiParserInputPin::GetPatProcessor(CPATProcessor ** ppPatProcessor)
{
    if(ppPatProcessor == NULL)
        return E_INVALIDARG;

    CAutoLock lock(m_pReceiveLock);

    if(m_pPatProcessor != NULL)
        *ppPatProcessor = m_pPatProcessor;
    else
        return E_INVALIDARG;

    return NOERROR;
}

HRESULT CPsiParserInputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    if(iPosition < 0) return E_INVALIDARG;
    if(iPosition > 0) return VFW_S_NO_MORE_ITEMS;

    pmt->majortype = MEDIATYPE_MPEG2_SECTIONS;

    return NOERROR;
}

HRESULT CPsiParserInputPin::CheckMediaType(const CMediaType *pMediaType)
{
    if( pMediaType->majortype == MEDIATYPE_MPEG2_SECTIONS)
        return S_OK;

    return VFW_E_INVALIDMEDIATYPE;
}

HRESULT CPsiParserInputPin::Receive(IMediaSample * pSample)
{
    CAutoLock lock(m_pReceiveLock);

    HRESULT hr = CBaseInputPin::Receive(pSample);
    if(hr != S_OK) return hr;

    CHECK_BADPTR( TEXT("invalid sample"), pSample);

    BYTE* pData = NULL;
    hr = pSample->GetPointer(&pData);
    CHECK_ERROR( TEXT("pSample->GetPointer() failed"), hr);
    CHECK_BADPTR( TEXT("invalid sample"), pData);

    long lDataLen;
    lDataLen =  pSample->GetActualDataLength();

    unsigned char tableID;
    tableID = *pData;

    // the two processors are created here instead of in the constructor,
    // because the demux and the psi filter should be connected first before
    // the demux is retrieved from psi filter in the processor's constructor
    if (m_bCreated == false)
    {
        // search Demux
        IPin* pPinCon = GetConnected();
        PIN_INFO pinInfo;
        hr = pPinCon->QueryPinInfo(&pinInfo);
        if(FAILED(hr) || !pinInfo.pFilter) return hr;

        IBaseFilter* pDemux = pinInfo.pFilter;
        IMpeg2Demultiplexer* piDemux = NULL;
        hr = pinInfo.pFilter->QueryInterface(IID_IMpeg2Demultiplexer, (void**)&piDemux);
        if(FAILED(hr))
        {
            // Just test previus filter now
            // TODO look for the demux in previous filters of this
            pDemux->Release();
            return hr;
        }

        // initialize data shared by two processors:
        m_Programs.init_programs();

        // create two processors once
        m_pPatProcessor = new CPATProcessor(m_pFilter, &m_Programs, pDemux, &hr);
        if(m_pPatProcessor == NULL)
        {
            piDemux->Release();
            pDemux->Release();
            return E_OUTOFMEMORY;
        }

        m_pPmtProcessor = new CPMTProcessor(m_pFilter, &m_Programs, &hr);
        if(m_pPmtProcessor == NULL)
        {
            piDemux->Release();
            pDemux->Release();
            delete m_pPatProcessor;
            m_pPatProcessor = NULL;
            return E_OUTOFMEMORY;
        }

        m_pFilter->m_pDemux = piDemux;  // store on filter to create Audio/Video Pins
        
        pDemux->Release();
        m_bCreated = true;
    }

    // PAT section
    if(*pData == PAT_TABLE_ID && VALID_PSI_HEADER(pData))
        m_pPatProcessor->process(pData, lDataLen);
    //PMT section
    else if(*pData == PMT_TABLE_ID && VALID_PSI_HEADER(pData))
        m_pPmtProcessor->process(pData, lDataLen);

    return hr;
}

STDMETHODIMP CPsiParserInputPin::EndOfStream(void)
{
    CAutoLock lock(m_pReceiveLock);
    return CRenderedInputPin::EndOfStream();
}


//-----------------------------------------------------------------------------
//
//	CPATProcessor class
//
//-----------------------------------------------------------------------------

CPATProcessor::CPATProcessor(CPsiConfigFilter *pParser, CPrograms * pPrograms, IBaseFilter* pDemuxFilter, HRESULT *phr)
: m_pParser(pParser), m_pPrograms(pPrograms), m_current_transport_stream_id( 0xff), m_pat_section_count(0),
  m_mapped_pmt_pid_count(0), m_pDemuxPsiOutputPin(NULL)
{
    // get demux psi output pin
    *phr = GetDemuxPsiOutputPin(pDemuxFilter, &m_pDemuxPsiOutputPin);
}

CPATProcessor::~CPATProcessor()
{
    if(m_pDemuxPsiOutputPin)
    {
		// problem with the Release: memory corruption
        //m_pDemuxPsiOutputPin->Release();
        m_pDemuxPsiOutputPin = NULL;
    }
}


BOOL CPATProcessor::IsNewPATSection(DWORD dwSectionNumber)
{
    BOOL bIsNewSection = TRUE;

    for(int i = 0; i<(int)m_pat_section_count; i++) {
        if(m_pat_section_number_array[i] == dwSectionNumber) {
            return FALSE;
        }
    }

    return bIsNewSection;
}

BOOL CPATProcessor::MapPmtPid(DWORD dwPmtPid)
{
    BOOL bResult = TRUE;

    if(m_pDemuxPsiOutputPin == NULL)
        return FALSE;

    IMPEG2PIDMap * pIPmtPIDMap;
    HRESULT hr = m_pDemuxPsiOutputPin->QueryInterface(__uuidof(IMPEG2PIDMap), (void **) &pIPmtPIDMap);
    RETURN_FALSE_IF_FAILED(TEXT("CPATProcessor::MapPmtPid():: QI the IMPEG2PIDMap inf on PSI out pin failed %X"), hr);
    RETURN_FALSE_IF_BADPTR(TEXT("CPATProcessor::MapPmtPid()::pIPmtPIDMap is null "), pIPmtPIDMap) ;

    ULONG ulPmtPID[1] = {(ULONG) dwPmtPid};

    hr = pIPmtPIDMap->MapPID(1, ulPmtPID, MEDIA_MPEG2_PSI);
    if(FAILED(hr)){
        pIPmtPIDMap->Release();
        return FALSE;
    }

    pIPmtPIDMap->Release();
    return bResult;
}

BOOL CPATProcessor::UnmapPmtPid()
{
    BOOL bResult = TRUE;

    if(m_pDemuxPsiOutputPin == NULL)
        return FALSE;

    IMPEG2PIDMap * pIPmtPIDMap;
    HRESULT hr = m_pDemuxPsiOutputPin->QueryInterface(__uuidof(IMPEG2PIDMap), (void **) &pIPmtPIDMap);
    RETURN_FALSE_IF_FAILED(TEXT(" CPATProcessor::SetupMPEGDeMux():: QI the IMPEG2PIDMap inf on PSI out pin failed %X"), hr);
    RETURN_FALSE_IF_BADPTR(TEXT("CPATProcessor::SetupMPEGDeMux()::pIPmtPIDMap is null "), pIPmtPIDMap) ;

    for(int i = 0; i<(int)m_mapped_pmt_pid_count;i++){

        ULONG ulPmtPID[1] = {(ULONG) m_mapped_pmt_pid_array[i]};

        hr = pIPmtPIDMap->UnmapPID(1, ulPmtPID);
        if(FAILED(hr)){
            pIPmtPIDMap->Release();
            return FALSE;
        }
    }
    pIPmtPIDMap->Release();
    return bResult;
}

// process the pat section received (see data flow chart in SDK document)
BOOL CPATProcessor::process(BYTE * pbBuffer, long lDataLen)
{
    BOOL bResult = TRUE;

    //Is this section currently applicable ?
    // "0" indicates that the table sent is not yet applicable
    // and shall be the next table to become valid
    BOOL current_next_indicator = PAT_CURRENT_NEXT_INDICATOR_BIT(pbBuffer);
    if( current_next_indicator == 0 ){
        return FALSE; // discard and do nothing
    }

    // new transport stream
    DWORD transport_stream_id = PAT_TRANSPORT_STREAM_ID_VALUE(pbBuffer);
    if( transport_stream_id != m_current_transport_stream_id ) {
        // flush all programs
        if(!flush())
            return FALSE;
        bResult = store(pbBuffer, lDataLen);
    }
    else {

        // new section
        DWORD section_number = PAT_SECTION_NUMBER_VALUE(pbBuffer);
        if( IsNewPATSection(section_number) ){
            bResult = store(pbBuffer, lDataLen);
        }
        else {
            // new PAT version, i.e. transport stream changed (such as adding or deleting programs)
            DWORD version_number = PAT_VERSION_NUMBER_VALUE(pbBuffer);
            if( m_current_pat_version_number != version_number ){
                if(!flush())
                    return FALSE;
                bResult = store(pbBuffer, lDataLen);
            }
            // else discard the section and do nothing;
        }
    }
    return bResult;
} //process


// add new program or update existing program
BOOL CPATProcessor::store(BYTE * pbBuffer, long lDataLen)
{
    BOOL bResult =TRUE;
    //if it is not valid pat section, ignore it
    if(!VALID_PAT_SECTION(pbBuffer))
        return FALSE;

    if(!ConfirmMpeg2PSICRC_32 (pbBuffer,lDataLen) )
        return FALSE;

    MPEG2_PAT_SECTION mpeg2_pat_section;
    // if we can't parse it, then ignore it
    if(!mpeg2_pat_section.Parse(pbBuffer))
        return FALSE;


    // update m_current_transport_stream_id
    m_current_transport_stream_id = mpeg2_pat_section.transport_stream_id;

    // update m_current_pat_version_number
    m_current_pat_version_number = mpeg2_pat_section.version_number;

    // update m_pat_section_number_list
    m_pat_section_number_array[m_pat_section_count] = mpeg2_pat_section.section_number;
    m_pat_section_count ++;

    // update s_mpeg2_programs, for each program:
    for( int i=0;i<(int) (mpeg2_pat_section.number_of_programs); i++)// For each program contained in this pat section
    {
        //  add new program or update existing program
        DumpTrace(TEXT("add new program\n"));
        m_pPrograms->add_program_from_pat(&mpeg2_pat_section, i);

        // if the pmt PID has not been mapped to demux output pin, map it
        if( !HasPmtPidMappedToPin(mpeg2_pat_section.program_descriptor[i].program_number)) {
            DumpTrace1(TEXT("map new pmt pid %d to demux output pin\n"), mpeg2_pat_section.program_descriptor[i].network_or_program_map_PID);
            if(!MapPmtPid(mpeg2_pat_section.program_descriptor[i].network_or_program_map_PID))
                return FALSE;
        }
    }

    m_pParser->NotifyEvent(EC_PROGRAM_CHANGED,0,(LONG_PTR)(IBaseFilter*)m_pParser);

    return bResult;
}

// if the pmt pid has been mapped to demux's psi output pin, return TRUE;
BOOL CPATProcessor::HasPmtPidMappedToPin(DWORD dwPid)
{
    BOOL bMapped = FALSE;
    for(int i = 0; i<(int)m_mapped_pmt_pid_count;i++){
        if(m_mapped_pmt_pid_array[i] == dwPid)
            return TRUE;
    }
    return bMapped;
}

// flush an array of struct: m_mpeg2_program[];
// and unmap all PMT_PIDs pids, except one: PAT
BOOL CPATProcessor::flush()
{
    BOOL bResult = TRUE;
    bResult = m_pPrograms->free_programs();
    if(bResult == FALSE)
        return bResult;
    bResult = UnmapPmtPid();
    return bResult;
}

//-----------------------------------------------------------------------------
//
//	CPMTProcessor class
//
//-----------------------------------------------------------------------------

CPMTProcessor::CPMTProcessor(CPsiConfigFilter *pParser,CPrograms * pPrograms, HRESULT *phr)
: m_pParser(pParser), m_pPrograms(pPrograms), m_pmt_section_count(0)
{
}

// for a pmt section of a given program_number, if the version is the same as recorded before,
// return TRUE; otherwise, return false;
BOOL CPMTProcessor::HasPMTVersionOfThisProgramChanged(DWORD dwProgramNumber, DWORD dwSectionVersion)
{
    if(m_pmt_section_count != 0){
        for(int i = 0; i<(int)m_pmt_section_count; i++)
            if( dwProgramNumber == m_pmt_program_number_version_array[i].pmt_program_number &&
                dwSectionVersion == m_pmt_program_number_version_array[i].pmt_section_version )
                return FALSE;
    }
    return TRUE;
}

// if the section number has been received before, return TRUE; else, return FALSE
BOOL CPMTProcessor::IsNewPMTSection(DWORD dwProgramNumber)
{
    if(m_pmt_section_count != 0){
        // in each pmt section, the section number field shall be set to zero.
        // Sections are identified by the program_number field (ISO/IEC 13818-1:1996(E))
        for(int i = 0; i<(int)m_pmt_section_count; i++)
            if(dwProgramNumber == m_pmt_program_number_version_array[i].pmt_program_number)
                return FALSE;

    }
    return TRUE;
}

// process a pmt section received(see data flow chart in SDK document)
BOOL CPMTProcessor::process(BYTE * pbBuffer, long lDataLen)
{
    BOOL bResult = TRUE;

    //Is this section currently applicable ?
    // "0" indicates that the table sent is not yet applicable
    // and shall be the next table to become valid
    BOOL current_next_indicator = PMT_CURRENT_NEXT_INDICATOR_BIT(pbBuffer);
    if( current_next_indicator == 0 ){
        return FALSE; // discard and do nothing
    }

    // new section
    DWORD program_number = PMT_PROGRAM_NUMBER_VALUE(pbBuffer);
    if( IsNewPMTSection(program_number)) {
        bResult = store(pbBuffer, lDataLen);
    }
    else{
        // new pmt version, just for this program
        DWORD version_number = PMT_VERSION_NUMBER_VALUE(pbBuffer);
        if( HasPMTVersionOfThisProgramChanged(program_number, version_number )){
            bResult = store(pbBuffer, lDataLen);
        }
        //else discard and do nothing
    }
    return bResult;
}

// store new program info in the CProgram object
BOOL CPMTProcessor::store(BYTE * pbBuffer, long lDataLen)
{
    BOOL bResult = TRUE;

    //if it is not valid pmt, ignore it
    if(!VALID_PMT_SECTION(pbBuffer))
        return FALSE;

    if(!ConfirmMpeg2PSICRC_32 (pbBuffer,lDataLen) )
        return FALSE;

    MPEG2_PMT_SECTION mpeg2_pmt_section;

    // if we can't parse it, then ignore it
    if(!mpeg2_pmt_section.Parse(pbBuffer))
        return FALSE;

    // keep track of received pmt sections with their program_number and version_number
    m_pmt_program_number_version_array[m_pmt_section_count].pmt_program_number =
        mpeg2_pmt_section.program_number;
    m_pmt_program_number_version_array[m_pmt_section_count].pmt_section_version =
        mpeg2_pmt_section.version_number;

    // keep track of number of received pmt sections
    m_pmt_section_count++;
    ASSERT(m_pmt_section_count <= MAX_PROGRAM_NUMBER_IN_TRANSPORT_STREAM);

    //  add new program or update existing program
    DumpTrace(TEXT("add/ update pmt section to programs\n"));
    m_pPrograms->update_program_from_pmt(&mpeg2_pmt_section);

    m_pParser->NotifyEvent(EC_PROGRAM_CHANGED,0,(LONG_PTR)(IBaseFilter*)m_pParser);

    // create pins on demux
    m_pParser->ConfigurePmtSectionsOnDemux(&mpeg2_pmt_section);

    return bResult;
}


//-----------------------------------------------------------------------------
//
//	CPayloadParserInputPin class
//
//-----------------------------------------------------------------------------

CPayloadParserInputPin::CPayloadParserInputPin( CPsiConfigFilter *pFilter, LPUNKNOWN pUnk, CCritSec *pLock, CCritSec *pReceiveLock, IPin* pConnectToPin, bool forVideo, HRESULT *phr)
: CRenderedInputPin(NAME("CPayloadParserInputPin"), (CBaseFilter *) pFilter, pLock, phr, L"Input"),
  m_pConnecToPin(pConnectToPin), m_forVideo(forVideo), m_pFilter(pFilter), m_pReceiveLock(pReceiveLock), readyToReconfigDemuxer(false)
{
}

CPayloadParserInputPin::~CPayloadParserInputPin()
{
    if(m_pConnecToPin)
    {
        m_pConnecToPin->Release();
        m_pConnecToPin = NULL;
    }
}

HRESULT CPayloadParserInputPin::DoConnect()
{
    HRESULT hr = m_pConnecToPin->Connect(this, NULL);
    if(SUCCEEDED(hr))
    {
        // not needed anymore
        m_pConnecToPin->Release();
        m_pConnecToPin = NULL;

        m_parsedMediaType = m_mt;
    }

    return hr;
}

HRESULT CPayloadParserInputPin::DoReconfigDemuxPin()
{
    IPin* pPin = GetConnected();

    PIN_INFO info;
    HRESULT hr = pPin->QueryPinInfo(&info);
    if(info.pFilter)
        info.pFilter->Release();
    
    hr = Disconnect();
    hr = m_pFilter->m_pDemux->SetOutputPinMediaType(info.achName, &m_parsedMediaType);
    return hr;
}

HRESULT CPayloadParserInputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    if(iPosition < 0) return E_INVALIDARG;
    if(iPosition > 1) return VFW_S_NO_MORE_ITEMS;

    pmt->majortype = m_forVideo ? MEDIATYPE_Video : MEDIATYPE_Audio;

    if(m_forVideo)
    {
        if(!iPosition) pmt->subtype = MEDIASUBTYPE_MPEG1Video;
        else pmt->subtype = MEDIASUBTYPE_MPEG2_VIDEO;
    }
    else
    {
        if(!iPosition) pmt->subtype = MEDIASUBTYPE_MPEG1Audio;
        else pmt->subtype = MEDIASUBTYPE_MPEG2_AUDIO;
    }

    return NOERROR;
}

HRESULT CPayloadParserInputPin::CheckMediaType(const CMediaType *pMediaType)
{
    if(m_forVideo)
    {
        if(pMediaType->majortype != MEDIATYPE_Video)
            return VFW_E_INVALIDMEDIATYPE;

        if(pMediaType->subtype != MEDIASUBTYPE_NULL &&
            pMediaType->subtype != MEDIASUBTYPE_MPEG1Video && 
            pMediaType->subtype != MEDIASUBTYPE_MPEG2_VIDEO)
            return VFW_E_INVALIDMEDIATYPE;
    }
    else
    {
        if(pMediaType->majortype != MEDIATYPE_Audio)
            return VFW_E_INVALIDMEDIATYPE;

        if(pMediaType->subtype != MEDIASUBTYPE_NULL &&
            pMediaType->subtype != MEDIASUBTYPE_MPEG1Audio && 
            pMediaType->subtype != MEDIASUBTYPE_MPEG1AudioPayload && 
            pMediaType->subtype != MEDIASUBTYPE_MPEG2_AUDIO)
            return VFW_E_INVALIDMEDIATYPE;
    }

    return S_OK;
}

const WORD bitrates[16][6] =
// V1,L1	V1,L2   V1,L3	V2,L1	V2,L2	V2,L3
{
    {0	    ,0	    ,0	    ,0	    ,0	    ,0    },
    {32	    ,32	    ,32	    ,32	    ,32	    ,8    },
    {64	    ,48	    ,40	    ,64	    ,48	    ,16   },
    {96	    ,56	    ,48	    ,96	    ,56	    ,24   },
    {128    ,64	    ,56	    ,128    ,64	    ,32   },
    {160    ,80	    ,64	    ,160    ,80	    ,64   },
    {192    ,96	    ,80	    ,192    ,96	    ,80   },
    {224    ,112    ,96	    ,224    ,112    ,56   },
    {256    ,128    ,112    ,256    ,128    ,64   },
    {288    ,160    ,128    ,288    ,160    ,128  },
    {320    ,192    ,160    ,320    ,192    ,160  },
    {352    ,224    ,192    ,352    ,224    ,112  },
    {384    ,256    ,224    ,384    ,256    ,128  },
    {416    ,320    ,256    ,416    ,320    ,256  },
    {448    ,384    ,320    ,448    ,384    ,320  },
    {0	    ,0	    ,0	    ,0	    ,0	    ,0    }
};

const DWORD samplerates[4][3] = 
    // MPEG1    MPEG2   MPEG2.5
{
    {44100,	    22050,	11025},
    {48000,     24000,	12000},
    {32000,     16000,	 8000},
    {    0,         0,      0}
};

HRESULT CPayloadParserInputPin::Receive(IMediaSample * pSample)
{
    CAutoLock lock(m_pReceiveLock);

    HRESULT hr = CBaseInputPin::Receive(pSample);
    if(hr != S_OK) return hr;

    // allready parsed
    if(readyToReconfigDemuxer) return S_OK;

    CHECK_BADPTR( TEXT("invalid sample"), pSample);

    BYTE* pData = NULL;
    hr = pSample->GetPointer(&pData);
    CHECK_ERROR( TEXT("pSample->GetPointer() failed"), hr);
    CHECK_BADPTR( TEXT("invalid sample"), pData);

    long lDataLen;
    lDataLen = pSample->GetActualDataLength();
    lDataLen -= 4;

    if(m_forVideo)
    {
        // Search for MPEG Sequence Header and Sequence Extension
        int iSeqHeadStart = -1;
        int iSeqHeadStop = -1;
        int iSeqExtStart = -1;
        int iSeqExtStop = -1;

        for(int i=0; i<lDataLen; i++)
        {
            // Syncbyte
            if(pData[i] == 0 && pData[i+1] == 0 && pData[i+2] == 1)
            {
                // set StopPos
                if(iSeqHeadStart != -1 && iSeqHeadStop == -1)
                    iSeqHeadStop = i-1;

                if(iSeqExtStart != -1 && iSeqExtStop == -1)
                    iSeqExtStop = i-1;
                
                if(iSeqHeadStart == -1 && pData[i+3] == 0xb3)
                {
                    iSeqHeadStart = i;
                    i += 10;
                }
                else if(iSeqExtStart == -1 && pData[i+3] == 0xb5)
                {
                    iSeqExtStart = i;
                    i += 8;
                }
            }

            if(iSeqHeadStop != -1 && iSeqExtStop != -1)
                break;
        }

        if(iSeqHeadStart != -1 && iSeqHeadStop != -1)
        {
            if(iSeqExtStart != -1 && iSeqExtStop != -1)
            {
                // Mpeg2
                m_parsedMediaType.majortype = MEDIATYPE_Video;
                m_parsedMediaType.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
                m_parsedMediaType.formattype = FORMAT_MPEG2_VIDEO;
                m_parsedMediaType.bFixedSizeSamples = FALSE;
                m_parsedMediaType.bTemporalCompression = TRUE;
                m_parsedMediaType.lSampleSize = 0;

                int cbSequenceHeader = (iSeqHeadStop-iSeqHeadStart+1) + (iSeqExtStop-iSeqExtStart+1);
                if(cbSequenceHeader % 4 != 0) cbSequenceHeader += 4 - cbSequenceHeader % 4; // DWORD align seqheader
                m_parsedMediaType.cbFormat = sizeof(MPEG2VIDEOINFO) + cbSequenceHeader;
                m_parsedMediaType.pbFormat = (BYTE*)CoTaskMemAlloc(m_parsedMediaType.cbFormat);

                ZeroMemory(m_parsedMediaType.pbFormat, m_parsedMediaType.cbFormat);

                MPEG2VIDEOINFO* pInfo = (MPEG2VIDEOINFO*)m_parsedMediaType.pbFormat;

                pInfo->cbSequenceHeader = cbSequenceHeader;
                BYTE* dst = (BYTE*)pInfo->dwSequenceHeader;
                CopyMemory(dst, pData + iSeqHeadStart, (iSeqHeadStop-iSeqHeadStart+1));
                dst += (iSeqHeadStop-iSeqHeadStart+1);
                CopyMemory(dst, pData + iSeqExtStart, (iSeqExtStop-iSeqExtStart+1));
            }
            else
            {
                // TODO Mpeg1
            }

            readyToReconfigDemuxer = true;
        }
        else
        {
            // can't parse, no b3 found
        }
     }
    else
    {
        // Search for MPEG Sequence Header and Sequence Extension
        int iFrameStart = -1;

        for(int i=0; i<lDataLen; i++)
            if(pData[i] == 0xFF && ((pData[i+1] & 0xE0) == 0xE0))
            {
                iFrameStart = i;
                break;
            }

        if(iFrameStart != -1)
        {
            m_parsedMediaType.majortype = MEDIATYPE_Audio;

            BYTE version = (pData[iFrameStart+1] & 0x18) >> 3;
            BYTE layer = (pData[iFrameStart+1] & 0x06) >> 1;
            BYTE protect = (pData[iFrameStart+1] & 0x01);

            if(version == 2 || version == 0)
                m_parsedMediaType.subtype = MEDIASUBTYPE_MPEG1AudioPayload;//MEDIASUBTYPE_MPEG2_AUDIO;
            else if(version == 3)
                m_parsedMediaType.subtype = MEDIASUBTYPE_MPEG1AudioPayload;
            else
            {
                // i don't know
                m_parsedMediaType.subtype = MEDIASUBTYPE_NULL;
                readyToReconfigDemuxer = true;
                return hr;
            }

            m_parsedMediaType.formattype = FORMAT_WaveFormatEx;
            m_parsedMediaType.bFixedSizeSamples = TRUE;
            m_parsedMediaType.bTemporalCompression = FALSE;
            m_parsedMediaType.cbFormat = sizeof(MPEG1WAVEFORMAT);
            m_parsedMediaType.pbFormat = (BYTE*)CoTaskMemAlloc(m_parsedMediaType.cbFormat);
            ZeroMemory(m_parsedMediaType.pbFormat, m_parsedMediaType.cbFormat);

            MPEG1WAVEFORMAT* pInfo = (MPEG1WAVEFORMAT*)m_parsedMediaType.pbFormat;
            pInfo->wfx.wFormatTag = WAVE_FORMAT_MPEG;
            pInfo->wfx.cbSize = sizeof(MPEG1WAVEFORMAT) - sizeof(WAVEFORMATEX);

            switch(layer)
            {
            case 1: pInfo->fwHeadLayer = ACM_MPEG_LAYER3; break;
            case 2: pInfo->fwHeadLayer = ACM_MPEG_LAYER2; break;
            case 3: pInfo->fwHeadLayer = ACM_MPEG_LAYER1; break;
            }

            BYTE bitrateIndex = (pData[iFrameStart+2] & 0xF0) >> 4;
            BYTE sampleRate = (pData[iFrameStart+2] & 0x0C) >> 2;
            BYTE padding = (pData[iFrameStart+2] & 0x02) >> 1;
            BYTE privat = pData[iFrameStart+2] & 0x01;

            // Bitrate
            int bitrateType = 0;
            if(version == 3)
            {
                if(pInfo->fwHeadLayer == ACM_MPEG_LAYER1) bitrateType = 0;
                else if(pInfo->fwHeadLayer == ACM_MPEG_LAYER2) bitrateType = 1;
                else if(pInfo->fwHeadLayer == ACM_MPEG_LAYER3) bitrateType = 2;
            }
            else
            {
                if(pInfo->fwHeadLayer == ACM_MPEG_LAYER1) bitrateType = 3;
                else if(pInfo->fwHeadLayer == ACM_MPEG_LAYER2) bitrateType = 4;
                else if(pInfo->fwHeadLayer == ACM_MPEG_LAYER3) bitrateType = 5;
            }
            pInfo->dwHeadBitrate = bitrates[bitrateIndex][bitrateType] * 1000;

            // samplerate
            int samplerateType = 0;
            if(version == 0) samplerateType = 2;
            else if(version == 2) samplerateType = 1;
            pInfo->wfx.nSamplesPerSec = samplerates[sampleRate][samplerateType];

            pInfo->wfx.nBlockAlign = 1;

            // Channels
            BYTE channel = (pData[iFrameStart+3] & 0xC0) >> 6;
            pInfo->wfx.nChannels = channel == 3 ? 1 : 2;
            switch(channel)
            {
            case 0: pInfo->fwHeadMode = ACM_MPEG_STEREO; break;
            case 1: pInfo->fwHeadMode = ACM_MPEG_JOINTSTEREO; break;
            case 2: pInfo->fwHeadMode = ACM_MPEG_DUALCHANNEL; break;
            case 3: pInfo->fwHeadMode = ACM_MPEG_SINGLECHANNEL; break;
            }

            if(channel == 1)    // only for JointStereo
                pInfo->fwHeadModeExt = (pData[iFrameStart+3] & 0x30) >> 4;
                
            BYTE copyright = (pData[iFrameStart+3] & 0x08) >> 3;
            BYTE original = (pData[iFrameStart+3] & 0x04) >> 2;

            pInfo->wHeadEmphasis = pData[iFrameStart+3] & 0x03;

            if(privat) pInfo->fwHeadFlags |= ACM_MPEG_PRIVATEBIT;
            if(copyright) pInfo->fwHeadFlags |= ACM_MPEG_COPYRIGHT;
            if(original) pInfo->fwHeadFlags |= ACM_MPEG_ORIGINALHOME;
            if(!protect) pInfo->fwHeadFlags |= ACM_MPEG_PROTECTIONBIT;
            if(version == 3) pInfo->fwHeadFlags |= ACM_MPEG_ID_MPEG1;

            readyToReconfigDemuxer = true;
        }
        else
        {
            // can't parse
            //readyToReconfigDemuxer = true;
        }
    }

    if(readyToReconfigDemuxer)
        m_pFilter->PayloadParserPinReady();

    return hr;
}

STDMETHODIMP CPayloadParserInputPin::EndOfStream(void)
{
    CAutoLock lock(m_pReceiveLock);
    return CRenderedInputPin::EndOfStream();
}


//-----------------------------------------------------------------------------
//
//	CPayloadParserConnector class
//
//-----------------------------------------------------------------------------
DWORD CPayloadParserConnector::ThreadProc()
{
    if(!m_pFilter) return 0;

    // short wait
    Sleep(300);

    // stop graph
    HRESULT hr = m_pFilter->m_pMediaCtrl->StopWhenReady();

    // connect parser pins
    for(int i=0; i<m_pFilter->m_countPayloadPins; i++)
        m_pFilter->m_payloadPins[i]->DoConnect();

    // restart graph
    hr = m_pFilter->m_pMediaCtrl->Run();

    // Notify application
    m_pFilter->NotifyEvent(EC_GRAPH_CHANGED,0,0);
    m_pFilter->NotifyEvent(EC_PIDS_PARSED,0,0);

    return 0;
}

//-----------------------------------------------------------------------------
//
//	CPayloadParserReconfigurer class
//
//-----------------------------------------------------------------------------
DWORD CPayloadParserReconfigurer::ThreadProc()
{
    if(!m_pFilter) return 0;

    // short wait
    Sleep(200);

    // stop graph
    HRESULT hr = m_pFilter->m_pMediaCtrl->StopWhenReady();

    // reconfigur pins
    for(int i=0; i<m_pFilter->m_countPayloadPins; i++)
        m_pFilter->m_payloadPins[i]->DoReconfigDemuxPin();

    // Notify application
    m_pFilter->m_pMediaCtrl->Stop();
    m_pFilter->NotifyEvent(EC_GRAPH_CHANGED,0,0);
    m_pFilter->NotifyEvent(EC_PINS_CONFIGURED,0,0);

    return 0;
}

//----------------------------------------------------------------------------
//
// Given a filter, find the output pin which output pecific media type
//
//----------------------------------------------------------------------------
HRESULT GetDemuxPsiOutputPin(IBaseFilter * pDemux, IPin ** ppOutPin)
{
    if (!pDemux) return E_POINTER;

    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    BOOL bFound = FALSE;
    ULONG            ul ;

    HRESULT hr = pDemux->EnumPins(&pEnum);
    if (FAILED(hr)) return hr;
    while (S_OK == pEnum->Next(1, &pPin, 0))
    {
        // See if this pin matches the specified direction.
        PIN_DIRECTION ThisPinDir;
        hr = pPin->QueryDirection(&ThisPinDir);
        if (FAILED(hr))
        {
            // Something strange happened.
            hr = E_UNEXPECTED;
            pPin->Release();
            break;
        }

        if (ThisPinDir == PINDIR_OUTPUT)
        {
            // Get the filter that owns that pin.
            IEnumMediaTypes* pTypeEnum;
            hr = pPin->EnumMediaTypes (&pTypeEnum);

            AM_MEDIA_TYPE* pMediaTypes;
            // Loop thru' preferred media type list for a match
            do {
                hr = pTypeEnum->Next(1, &pMediaTypes, &ul) ;
                if (FAILED(hr) || 0 == ul) {
                    pPin->Release();
                    DumpTrace1(TEXT("IEnumMediaTypes::Next() failed (Error 0x%lx)"),hr) ;  // should be out
                    break ;
                }

                if (pMediaTypes->majortype == MEDIATYPE_MPEG2_SECTIONS)
                {
                    *ppOutPin = pPin;
                    bFound = TRUE ;
                    pPin->Release();
                    DeleteMediaType( pMediaTypes );

                    DumpTrace(TEXT("AM_MEDIA_TYPE.majortype matches")) ;

                    pTypeEnum->Release();
                    pEnum->Release();
                    return S_OK;
                }
            } while (!bFound) ;  // until the reqd one is found

            pTypeEnum->Release();
        }
        pPin->Release();
    }
    pEnum->Release();

    // Did not find a matching filter.
    return E_FAIL;
}

static BOOL ConfirmMpeg2PSICRC_32 (IN  BYTE *  pb, IN  ULONG   ulLen)
{
    ULONG   ulCRCAccum ;
    ULONG   i, j ;

    ulCRCAccum = 0xffffffff ;
    for (j = 0; j < ulLen; j++, pb++) {
        i = (( ulCRCAccum >> 24) ^ (* pb)) & 0xff ;
        ulCRCAccum = ((ulCRCAccum << 8 ) ^ g_MPEG2_PSI_CRC_32_Lookup [i]) ;
    }

    return ulCRCAccum == 0x00000000 ;
}

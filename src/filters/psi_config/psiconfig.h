//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

#include "psiobj.h"

#define EC_PROGRAM_CHANGED EC_USER + 100
#define EC_PIDS_PARSED EC_USER + 101
#define EC_PINS_CONFIGURED EC_USER + 102

class CPsiConfigFilter;
class CPATProcessor;
class CPMTProcessor;

// Helper functions
static BOOL ConfirmMpeg2PSICRC_32 ( BYTE *  pb, ULONG   ulLen );
static HRESULT GetDemuxPsiOutputPin(IBaseFilter * pDemux, IPin ** ppOutPin);

//-----------------------------------------------------------------------------
//
//	CPsiParserInputPin class
//
//-----------------------------------------------------------------------------
class CPsiParserInputPin : public CRenderedInputPin
{
    friend class CPsiConfigFilter;
    friend class CPATProcessor;
    friend class CPMTProcessor;

private:
    CPsiConfigFilter *m_pFilter;        // The filter that owns us
    CCritSec * const m_pReceiveLock;    // Sample critical section

    CPATProcessor*   m_pPatProcessor;   // Handles PAT sections
    CPMTProcessor*   m_pPmtProcessor;   // Handles PMT sections

    CPrograms m_Programs;               // Updated by the above two objects
    bool m_bCreated;

public:
    CPsiParserInputPin(CPsiConfigFilter *pTextOutFilter, LPUNKNOWN pUnk, CCritSec *pLock, CCritSec *pReceiveLock, HRESULT *phr);
    ~CPsiParserInputPin();

    virtual HRESULT GetMediaType(int iPosition, CMediaType *pmt);
    HRESULT CheckMediaType(const CMediaType *pmt);
    STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP EndOfStream(void);

    HRESULT GetPatProcessor(CPATProcessor ** ppPatProcessor);
};

//-----------------------------------------------------------------------------
//
//	CPATProcessor class
//
//-----------------------------------------------------------------------------
class CPATProcessor
{
public:
    CPATProcessor(CPsiConfigFilter *pParser,CPrograms * pPrograms, IBaseFilter* pDemuxFilter, HRESULT *phr);
    ~CPATProcessor();

    BOOL process(BYTE * pbBuffer, long lDataLen);
    BOOL store(BYTE * pbBuffer, long lDataLen);
    BOOL flush(); // delete all programs, PAT and PMT section list

    // from pat: keep track of stream id and pat version # change
    DWORD           m_current_transport_stream_id;  // one pat table per transport stream
    DWORD           m_current_pat_version_number;   // one version# per PAT table

private:
    //parser filter
    CPsiConfigFilter    * m_pParser;
    CPrograms           * m_pPrograms;
    IPin                * m_pDemuxPsiOutputPin;

    HRESULT             *m_phr;
    CCritSec m_Lock;

    // keep track of section number
    DWORD           m_pat_section_count;
    DWORD           m_pat_section_number_array[MAX_PROGRAM_NUMBER_IN_TRANSPORT_STREAM];

    // keep track of pmt pids
    DWORD           m_mapped_pmt_pid_array[MAX_PROGRAM_NUMBER_IN_TRANSPORT_STREAM];
    DWORD           m_mapped_pmt_pid_count;

    BOOL IsNewPATSection(DWORD dwSectionNumber);
    BOOL HasPmtPidMappedToPin(DWORD dwPid);
    BOOL MapPmtPid(DWORD dwPid);
    BOOL UnmapPmtPid();
};

// keep track of section version_number for each pmt section identified by program_number
typedef struct Pmt_Version_Of_A_Program
{
    // in each pmt section, the section number field shall be set to zero. 
    // Sections are identified by the program_number field (ISO/IEC 13818-1:1996(E))
    DWORD pmt_program_number;
    DWORD pmt_section_version;


} PMT_VERSION_OF_A_PROGRAM;

//-----------------------------------------------------------------------------
//
//	CPMTProcessor class
//
//-----------------------------------------------------------------------------
class CPMTProcessor
{
public:

    CPMTProcessor(CPsiConfigFilter *pParser,CPrograms * pPrograms, HRESULT *phr);
    ~CPMTProcessor() {}

    BOOL process(BYTE * pbBuffer, long lDataLen);
    BOOL store(BYTE * pbBuffer, long lDataLen);


private:
    CPsiConfigFilter    * m_pParser;
    CPrograms           * m_pPrograms;

    // from pmt
    // keep track of section # of PMT sections read 
    // one pmt section per program, so we keep an array of section #, 
    // along with their corresponding current version#
    DWORD                       m_pmt_section_count; 

    // note: the number of pmt sections should be >= MAX_PROGRAM_NUMBER_IN_TRANSPORT_STREAM

    PMT_VERSION_OF_A_PROGRAM  m_pmt_program_number_version_array[MAX_PROGRAM_NUMBER_IN_TRANSPORT_STREAM];

    BOOL HasPMTVersionOfThisProgramChanged(DWORD dwProgramNumber,DWORD dwVersionNumber);
    BOOL IsNewPMTSection(DWORD dwProgramNumber);
};

//-----------------------------------------------------------------------------
//
//	CPayloadParserInputPin class
//
//-----------------------------------------------------------------------------
class CPayloadParserInputPin : public CRenderedInputPin
{
    friend class CPsiConfigFilter;

private:
    CPsiConfigFilter *m_pFilter;        // The filter that owns us
    CCritSec * const m_pReceiveLock;    // Sample critical section
    IPin* m_pConnecToPin;
    bool m_forVideo;
    CMediaType m_parsedMediaType;

public:
    CPayloadParserInputPin(CPsiConfigFilter *pTextOutFilter, LPUNKNOWN pUnk,
                            CCritSec *pLock, CCritSec *pReceiveLock, IPin* pConnectToPin,
                            bool forVideo, HRESULT *phr);
    ~CPayloadParserInputPin();

    HRESULT DoConnect();
    HRESULT DoReconfigDemuxPin();
    bool readyToReconfigDemuxer;

    virtual HRESULT GetMediaType(int iPosition, CMediaType *pmt);
    HRESULT CheckMediaType(const CMediaType *pmt);
    STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP EndOfStream(void);
};


//-----------------------------------------------------------------------------
//
//	CPayloadParserConnector class
//
//-----------------------------------------------------------------------------
class CPayloadParserConnector : public CAMThread
{
public:
    CPsiConfigFilter* m_pFilter;

protected:
    DWORD ThreadProc(void);
};

//-----------------------------------------------------------------------------
//
//	CPayloadParserReconfigurer class
//
//-----------------------------------------------------------------------------
class CPayloadParserReconfigurer : public CAMThread
{
public:
    CPsiConfigFilter* m_pFilter;

protected:
    DWORD ThreadProc(void);
};

//-----------------------------------------------------------------------------
//
//	CPsiConfigFilter class
//
//-----------------------------------------------------------------------------
class CPsiConfigFilter : public CBaseFilter, public IAMFilterMiscFlags
{
public:
    CPsiConfigFilter(LPUNKNOWN pUnk,HRESULT *phr);
    ~CPsiConfigFilter();

    DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
    static const CFactoryTemplate g_Template;

    // IAMFilterMiscFlags
    STDMETHODIMP_(ULONG) GetMiscFlags(void) { return AM_FILTER_MISC_FLAGS_IS_RENDERER; }

    // Return the pins that we support
    int GetPinCount() { return 1 + m_countPayloadPins; }
    CBasePin* GetPin(int n) { return (n==0 ? (CBasePin*)m_pInputPin : (n>0 && n<=m_countPayloadPins ? (CBasePin*)m_payloadPins[n-1] : NULL)); }

    CCritSec        m_Lock;                // Main renderer critical section
    CCritSec        m_ReceiveLock;         // Sublock for received samples

    // Non interface locking critical section
    CCritSec        m_ParserLock;          // To serialise access.

    void ConfigurePmtSectionsOnDemux(MPEG2_PMT_SECTION* pmtsec);

private:
    // The nested classes may access our private state
    friend class CPsiParserInputPin;
    friend class CPayloadParserInputPin;
    friend class CPayloadParserConnector;
    friend class CPayloadParserReconfigurer;

    CPsiParserInputPin  *m_pInputPin;      // Handles pin interfaces
    IMpeg2Demultiplexer* m_pDemux;
    IMediaControl* m_pMediaCtrl;

    CPayloadParserConnector m_threadPinConnector;
    CPayloadParserReconfigurer m_threadPinReconfigurer;

    void AddPayloadParserPin(IPin* pConnectToPin, bool forVideo);
    void PayloadParserPinReady();

    int m_countPayloadPins;
    CPayloadParserInputPin **m_payloadPins;   // the pins on this filter.
    bool m_bRenderPayloadParserOnNextStop;
};

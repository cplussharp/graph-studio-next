//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CDetailsPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CDetailsPage, CDSPropertyPage)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CDetailsPage class
//
//-----------------------------------------------------------------------------
CDetailsPage::CDetailsPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("FilterDetails"), pUnk, IDD, strTitle),
	info(_T("root"))
{
	// retval
	if (phr) *phr = NOERROR;

}

CDetailsPage::~CDetailsPage()
{
	// todo
}


// overriden
BOOL CDetailsPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// create the tree
	CRect	rc;
	GetClientRect(&rc);

	ok = tree.Create(NULL, WS_CHILD | WS_VISIBLE, rc, this, IDC_TREE);
	if (!ok) return FALSE;

	info.Clear();
	OnBuildTree();

	tree.Initialize();
	tree.BuildPropertyTree(&info);

	return TRUE;
}

void CDetailsPage::OnBuildTree()
{
}

void CDetailsPage::OnSize(UINT nType, int cx, int cy)
{
	if (IsWindow(tree)) tree.MoveWindow(0, 0, cx, cy);
}



//-----------------------------------------------------------------------------
//
//	CFilterDetailsPage class
//
//-----------------------------------------------------------------------------

CFilterDetailsPage::CFilterDetailsPage(LPUNKNOWN pUnk, HRESULT *phr) :
	CDetailsPage(pUnk, phr, _T("Filter")),
	filter(NULL)
{
	// retval
	if (phr) *phr = NOERROR;

}

CFilterDetailsPage::~CFilterDetailsPage()
{
	// todo
}

HRESULT CFilterDetailsPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IBaseFilter, (void**)&filter);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CFilterDetailsPage::OnDisconnect()
{
	filter = NULL;
	return NOERROR;
}

void CFilterDetailsPage::OnBuildTree()
{
	GraphStudio::PropItem	*group;
	GraphStudio::Filter		gfilter(NULL);

	gfilter.LoadFromFilter(filter);

	group = info.AddItem(new GraphStudio::PropItem(_T("Filter Details")));
		CString	type;
		switch (gfilter.filter_type) {
		case GraphStudio::Filter::FILTER_DMO:		type = _T("DMO"); break;
		case GraphStudio::Filter::FILTER_WDM:		type = _T("WDM"); break;
		case GraphStudio::Filter::FILTER_STANDARD:	type = _T("Standard"); break;
		case GraphStudio::Filter::FILTER_UNKNOWN:	type = _T("Unknown"); break;
		}	
		group->AddItem(new GraphStudio::PropItem(_T("Type"), type));
		GraphStudio::GetFilterDetails(gfilter.clsid, group);
}

//-----------------------------------------------------------------------------
//
//	CPinDetailsPage class
//
//-----------------------------------------------------------------------------

CPinDetailsPage::CPinDetailsPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDetailsPage(pUnk, phr, strTitle),
	pin(NULL)
{
	// retval
	if (phr) *phr = NOERROR;

}

CPinDetailsPage::~CPinDetailsPage()
{
	// todo
}

HRESULT CPinDetailsPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IPin, (void**)&pin);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CPinDetailsPage::OnDisconnect()
{
	pin = NULL;
	return NOERROR;
}

void CPinDetailsPage::OnBuildTree()
{
	//GraphStudio::PropItem	*group;
	GraphStudio::Pin		gpin(NULL);
	int						ret;

	ret = gpin.Load(pin);
	if (ret == 0) {
		GetPinDetails(gpin.pin, &info);
	}
}



//-----------------------------------------------------------------------------
//
//	CInterfaceDetailsPage class
//
//-----------------------------------------------------------------------------

CInterfaceDetailsPage::CInterfaceDetailsPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDetailsPage(pUnk, phr, strTitle),
	pInterfaces(NULL)
{
	// retval
	if (phr) *phr = NOERROR;

}

CInterfaceDetailsPage::~CInterfaceDetailsPage()
{
	if(pInterfaces) delete pInterfaces;
}

HRESULT CInterfaceDetailsPage::OnConnect(IUnknown *pUnknown)
{
    pInterfaces = new CInterfaceScanner(pUnknown);
	return NOERROR;
}

HRESULT CInterfaceDetailsPage::OnDisconnect()
{
    if(pInterfaces)
    {
        delete pInterfaces;
        pInterfaces = NULL;
    }
	return NOERROR;
}

void CInterfaceDetailsPage::OnBuildTree()
{
	pInterfaces->GetDetails(&info);
}

//-----------------------------------------------------------------------------
//
//	CMediaInfoPage class
//
//-----------------------------------------------------------------------------

CMediaInfoPage* CMediaInfoPage::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR pszFile)
{
    CMediaInfo* info = CMediaInfo::GetInfoForFile(pszFile);
    if(info == NULL) return NULL;
    return new CMediaInfoPage(pUnk, phr, info);
}

CMediaInfoPage::CMediaInfoPage(LPUNKNOWN pUnk, HRESULT *phr, CMediaInfo* pInfo) :
	CDetailsPage(pUnk, phr, _T("MediaInfo")),
	m_pInfo(pInfo)
{
	// retval
	if (phr) *phr = NOERROR;

}

CMediaInfoPage::~CMediaInfoPage()
{
}

HRESULT CMediaInfoPage::OnConnect(IUnknown *pUnknown)
{
	return NOERROR;
}

HRESULT CMediaInfoPage::OnDisconnect()
{
	return NOERROR;
}

void CMediaInfoPage::OnBuildTree()
{
	m_pInfo->GetDetails(&info);
}


//-----------------------------------------------------------------------------
//
//	CAMExtendedSeekingPage class
//
//-----------------------------------------------------------------------------

CAMExtendedSeekingPage::CAMExtendedSeekingPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDetailsPage(pUnk, phr, strTitle),
	filter(NULL)
{
	// retval
	if (phr) *phr = NOERROR;
}

CAMExtendedSeekingPage::~CAMExtendedSeekingPage()
{
    filter = NULL;
}

HRESULT CAMExtendedSeekingPage::OnConnect(IUnknown *pUnknown)
{
    if(!pUnknown) return E_POINTER;
    return pUnknown->QueryInterface(IID_IAMExtendedSeeking, (void**)&filter);
}

HRESULT CAMExtendedSeekingPage::OnDisconnect()
{
    filter = NULL;
	return NOERROR;
}

void CAMExtendedSeekingPage::OnBuildTree()
{
	if(!filter) return;

    GraphStudio::PropItem *group = info.AddItem(new GraphStudio::PropItem(TEXT("IAMExtendedSeeking")));

    long cap=0;
    filter->get_ExSeekCapabilities(&cap);
    if(cap == 0)
        group->AddItem(new GraphStudio::PropItem(TEXT("ExSeekCapabilities"), TEXT("None")));
    else
    {
        CStringArray strCap;
        if((cap & AM_EXSEEK_BUFFERING) == AM_EXSEEK_BUFFERING)
            strCap.Add(_T("AM_EXSEEK_BUFFERING"));
        if((cap & AM_EXSEEK_NOSTANDARDREPAINT) == AM_EXSEEK_NOSTANDARDREPAINT)
            strCap.Add(_T("AM_EXSEEK_NOSTANDARDREPAINT"));
        if((cap & AM_EXSEEK_SENDS_VIDEOFRAMEREADY) == AM_EXSEEK_SENDS_VIDEOFRAMEREADY)
            strCap.Add(_T("AM_EXSEEK_SENDS_VIDEOFRAMEREADY"));
        if((cap & AM_EXSEEK_CANSCAN) == AM_EXSEEK_CANSCAN)
            strCap.Add(_T("AM_EXSEEK_CANSCAN"));
        if((cap & AM_EXSEEK_SCANWITHOUTCLOCK) == AM_EXSEEK_SCANWITHOUTCLOCK)
            strCap.Add(_T("AM_EXSEEK_SCANWITHOUTCLOCK"));
        if((cap & AM_EXSEEK_CANSEEK) == AM_EXSEEK_CANSEEK)
            strCap.Add(_T("AM_EXSEEK_CANSEEK"));
        if((cap & AM_EXSEEK_MARKERSEEK) == AM_EXSEEK_MARKERSEEK)
            strCap.Add(_T("AM_EXSEEK_MARKERSEEK"));

        CString strCapText;
        for(int i=0;i<strCap.GetCount(); i++)
        {
            if(i > 0) strCapText += _T(", ");
            strCapText += strCap[i];
        }

        group->AddItem(new GraphStudio::PropItem(_T("ExSeekCapabilities"), strCapText));
    }

    double speed=1.0;
    filter->get_PlaybackSpeed(&speed);
    group->AddItem(new GraphStudio::PropItem(_T("PlaybackSpeed"), speed));

    long cur=0;
    filter->get_CurrentMarker(&cur);
    group->AddItem(new GraphStudio::PropItem(_T("CurrentMarker"), cur));

    long count=0;
    filter->get_MarkerCount(&count);
    group->AddItem(new GraphStudio::PropItem(_T("MarkerCount"), count));

    for(long i=0; i<=count+1; i++)
    {
        double time = 0.0;
        HRESULT hr = filter->GetMarkerTime(i, &time);
        if(FAILED(hr)) continue;

        CString strMark;
        strMark.Format(_T("Marker %d"), i);
        GraphStudio::PropItem *mark = group->AddItem(new GraphStudio::PropItem(strMark));

        mark->AddItem(new GraphStudio::PropItem(_T("Time"), time));

        BSTR name = NULL;
        filter->GetMarkerName(i, &name);
        mark->AddItem(new GraphStudio::PropItem(_T("Name"), CString(name)));
        if(name) SysFreeString(name);
    }
}

//-----------------------------------------------------------------------------
//
//	CTunerInfoPage class
//
//-----------------------------------------------------------------------------

CTunerInfoPage::CTunerInfoPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDetailsPage(pUnk, phr, strTitle)
{
	// retval
	if (phr) *phr = NOERROR;
}

CTunerInfoPage::~CTunerInfoPage()
{
}

HRESULT CTunerInfoPage::OnConnect(IUnknown *pUnknown)
{
    if(!pUnknown) return E_POINTER;
    tuner = pUnknown;
    return !tuner ? E_NOINTERFACE : S_OK;
}

HRESULT CTunerInfoPage::OnDisconnect()
{
	return NOERROR;
}

void CTunerInfoPage::OnBuildTree()
{
	if(!tuner) return;

    GraphStudio::PropItem *group = info.AddItem(new GraphStudio::PropItem(TEXT("ITuner")));

    long sigstrength = -1;
    tuner->get_SignalStrength(&sigstrength);
    group->AddItem(new GraphStudio::PropItem(_T("SignalStrength"), sigstrength));

    CComPtr<ITuningSpace> ts;
    tuner->get_TuningSpace(&ts);
    if(ts)
    {
        GraphStudio::PropItem *space = group->AddItem(new GraphStudio::PropItem(_T("TuningSpace")));

        CComBSTR str;
        ts->get_CLSID(&str);
        space->AddItem(new GraphStudio::PropItem(_T("CLSID"), (LPCTSTR)str, false));
        ts->get_FriendlyName(&str);
        space->AddItem(new GraphStudio::PropItem(_T("FriendlyName"), (LPCTSTR)str, false));
        ts->get_UniqueName(&str);
        space->AddItem(new GraphStudio::PropItem(_T("UniqueName"), (LPCTSTR)str, false));
        ts->get_NetworkType(&str);
        space->AddItem(new GraphStudio::PropItem(_T("NetworkType"), (LPCTSTR)str, false));
        GUID networkType;
        ts->get__NetworkType(&networkType);
        space->AddItem(new GraphStudio::PropItem(_T("NetworkType CLSID"), networkType));
        ts->get_FrequencyMapping(&str);
        space->AddItem(new GraphStudio::PropItem(_T("FrequencyMapping"), (LPCTSTR)str, false));

        /* mostly empty
        CComPtr<ILocator> tsloc;
        ts->get_DefaultLocator(&tsloc);
        if(tsloc)
        {
            GraphStudio::PropItem *tsdefloc = group->AddItem(new GraphStudio::PropItem(_T("TuningSpace DefaultLocator")));
            GetLocatorInfo(tsloc, tsdefloc);
        }*/
    }
    else
    {
        group->AddItem(new GraphStudio::PropItem(_T("TuningSpace"), _T("(empty)"), false));
    }

    CComPtr<ITuneRequest> tr;
    tuner->get_TuneRequest(&tr);
    if(tr)
    {
        GraphStudio::PropItem *req = group->AddItem(new GraphStudio::PropItem(_T("TuneRequest")));

        CComQIPtr<IDVBTuneRequest> trDVB = tr;
        if(trDVB)
        {
            req->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IDVBTuneRequest"), false));
            long val;
            trDVB->get_ONID(&val);
            req->AddItem(new GraphStudio::PropItem(_T("ONID"), val));
            trDVB->get_SID(&val);
            req->AddItem(new GraphStudio::PropItem(_T("SID"), val));
            trDVB->get_TSID(&val);
            req->AddItem(new GraphStudio::PropItem(_T("TSID"), val));
        }

        CComQIPtr<IMPEG2TuneRequest> trMP2 = tr;
        if(trMP2)
        {
            req->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IMPEG2TuneRequest"), false));
            long val;
            trMP2->get_ProgNo(&val);
            req->AddItem(new GraphStudio::PropItem(_T("ProgNo"), val));
            trMP2->get_TSID(&val);
            req->AddItem(new GraphStudio::PropItem(_T("TSID"), val));
        }

        CComQIPtr<IDigitalCableTuneRequest> trDC = tr;
        if(trDC)
        {
            req->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IDigitalCableTuneRequest"), false));
            long val;
            trDC->get_SourceID(&val);
            req->AddItem(new GraphStudio::PropItem(_T("SourceID"), val));
            trDC->get_MajorChannel(&val);
            req->AddItem(new GraphStudio::PropItem(_T("MajorChannel"), val));
            trDC->get_Channel(&val);
            req->AddItem(new GraphStudio::PropItem(_T("Channel"), val));
            trDC->get_MinorChannel(&val);
            req->AddItem(new GraphStudio::PropItem(_T("MinorChannel"), val));
        }
        else
        {
            CComQIPtr<IATSCChannelTuneRequest> trATSC = tr;
            if(trATSC)
            {
                req->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IATSCChannelTuneRequest"), false));
                long val;
                trATSC->get_Channel(&val);
                req->AddItem(new GraphStudio::PropItem(_T("Channel"), val));
                trATSC->get_MinorChannel(&val);
                req->AddItem(new GraphStudio::PropItem(_T("MinorChannel"), val));
            }
            else
            {
                CComQIPtr<IChannelTuneRequest> trCh = tr;
                if(trCh)
                {
                    req->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IChannelTuneRequest"), false));
                    long val;
                    trCh->get_Channel(&val);
                    req->AddItem(new GraphStudio::PropItem(_T("Channel"), val));
                }
            }
        }

        CComPtr<ILocator> trloc;
        tr->get_Locator(&trloc);
        if(trloc)
        {
            GraphStudio::PropItem *trlocInfo = group->AddItem(new GraphStudio::PropItem(_T("TuneRequest Locator")));
            GetLocatorInfo(trloc, trlocInfo);
        }
    }
    else
    {
        group->AddItem(new GraphStudio::PropItem(_T("TuneRequest"), _T("(empty)"), false));
    }
}

CString FECMethod2String(FECMethod method)
{
    switch(method)
    {
        case BDA_FEC_METHOD_NOT_SET: return _T("method not set");
        case BDA_FEC_METHOD_NOT_DEFINED: return _T("method not defined");
        case BDA_FEC_VITERBI: return _T("Viterbi Binary Convolution");
        case BDA_FEC_RS_204_188: return _T("Reed-Solomon 204/188");
        case BDA_FEC_LDPC: return _T("Low Density Parity Check (LDPC)");
        case BDA_FEC_BCH: return _T("Bose-Chaudhuri-Hocquenghem (BCH)");
        case BDA_FEC_RS_147_130: return _T("Reed-Solomon 147/130");
        case BDA_FEC_MAX: return _T("MAX");
    }

    return _T("Unknown");
}

CString FECRate2String(BinaryConvolutionCodeRate rate)
{
    switch(rate)
    {
        case BDA_BCC_RATE_NOT_SET: return _T("not set");
        case BDA_BCC_RATE_NOT_DEFINED: return _T("not defined");
        case BDA_BCC_RATE_1_2: return _T("1/2");
        case BDA_BCC_RATE_2_3: return _T("2/3");
        case BDA_BCC_RATE_3_4: return _T("3/4");
        case BDA_BCC_RATE_3_5: return _T("3/5");
        case BDA_BCC_RATE_4_5: return _T("4/5");
        case BDA_BCC_RATE_5_6: return _T("5/6");
        case BDA_BCC_RATE_5_11: return _T("5/11");
        case BDA_BCC_RATE_7_8: return _T("7/8");
        case BDA_BCC_RATE_1_4: return _T("1/4");
        case BDA_BCC_RATE_1_3: return _T("1/3");
        case BDA_BCC_RATE_2_5: return _T("2/5");
        case BDA_BCC_RATE_6_7: return _T("6/7");
        case BDA_BCC_RATE_8_9: return _T("8/9");
        case BDA_BCC_RATE_9_10: return _T("9/10");
        case BDA_BCC_RATE_MAX: return _T("MAX");
    }

    return _T("Unknown");
}

CString Modulation2String(ModulationType mod)
{
    switch(mod)
    {
        case BDA_MOD_NOT_SET: return _T("not set");
        case BDA_MOD_NOT_DEFINED: return _T("not defined");
        case BDA_MOD_16QAM: return _T("32 symbol QAM");
        case BDA_MOD_32QAM: return _T("32 symbol QAM");
        case BDA_MOD_64QAM: return _T("64 symbol QAM");
        case BDA_MOD_80QAM: return _T("80 symbol QAM");
        case BDA_MOD_96QAM: return _T("96 symbol QAM");
        case BDA_MOD_112QAM: return _T("112 symbol QAM");
        case BDA_MOD_128QAM: return _T("128 symbol QAM");
        case BDA_MOD_160QAM: return _T("160 symbol QAM");
        case BDA_MOD_192QAM: return _T("192 symbol QAM");
        case BDA_MOD_224QAM: return _T("224 symbol QAM");
        case BDA_MOD_256QAM: return _T("256 symbol QAM");
        case BDA_MOD_320QAM: return _T("320 symbol QAM");
        case BDA_MOD_384QAM: return _T("384 symbol QAM");
        case BDA_MOD_448QAM: return _T("448 symbol QAM");
        case BDA_MOD_512QAM: return _T("512 symbol QAM");
        case BDA_MOD_640QAM: return _T("640 symbol QAM");
        case BDA_MOD_768QAM: return _T("768 symbol QAM");
        case BDA_MOD_896QAM: return _T("896 symbol QAM");
        case BDA_MOD_1024QAM: return _T("1024 symbol QAM");
        case BDA_MOD_QPSK: return _T("Quadrature Phase-Shift Keying (QPSK)");
        case BDA_MOD_BPSK: return _T("Binary Phase Shift Keying (BPSK)");
        case BDA_MOD_OQPSK: return _T("Offset QPSK");
        case BDA_MOD_8VSB: return _T("8 Vestigial Side-Band");
        case BDA_MOD_16VSB: return _T("16 Vestigial Side-Band");
        case BDA_MOD_ANALOG_AMPLITUDE: return _T("Analog amplitude modulation (AM)");
        case BDA_MOD_ANALOG_FREQUENCY: return _T("Analog frequency modulation (FM)");
        case BDA_MOD_8PSK: return _T("8-Phase shift keying");
        case BDA_MOD_RF: return _T("Analog TV (RF)");
        case BDA_MOD_16APSK: return _T("DVB-S2 modulation 16-level APSK");
        case BDA_MOD_32APSK: return _T("DVB-S2 modulation 32-level APSK");
        case BDA_MOD_NBC_QPSK: return _T("Non-backwards compatible QPSK");
        case BDA_MOD_NBC_8PSK: return _T("Non-backwards compatible 8-phase shift keying");
        case BDA_MOD_DIRECTV: return _T("DIRECTV DSS");
        case BDA_MOD_ISDB_T_TMCC: return _T("TMCC for ISDB-T");
        case BDA_MOD_ISDB_S_TMCC: return _T("TMCC for ISDB-S");
        case BDA_MOD_MAX: return _T("MAX");
    }

    return _T("Unknown");
}

CString AnalogVideoStandard2String(AnalogVideoStandard avs)
{
    switch(avs)
    {
        case AnalogVideo_None       : return _T("None");
        case AnalogVideo_NTSC_M     : return _T("NTSC (M) standard, 7.5 IRE black");
        case AnalogVideo_NTSC_M_J   : return _T("NTSC (M) standard, 0 IRE black (Japan)");
        case AnalogVideo_NTSC_433   : return _T("NTSC-433");
        case AnalogVideo_PAL_B      : return _T("PAL-B standard");
        case AnalogVideo_PAL_D      : return _T("PAL (D) standard");
        case AnalogVideo_PAL_H      : return _T("PAL (H) standard");
        case AnalogVideo_PAL_I      : return _T("PAL (I) standard");
        case AnalogVideo_PAL_M      : return _T("PAL (M) standard");
        case AnalogVideo_PAL_N      : return _T("PAL (N) standard");
        case AnalogVideo_PAL_60     : return _T("PAL-60 standard");
        case AnalogVideo_SECAM_B    : return _T("SECAM (B) standard");
        case AnalogVideo_SECAM_D    : return _T("SECAM (D) standard");
        case AnalogVideo_SECAM_G    : return _T("SECAM (G) standard");
        case AnalogVideo_SECAM_H    : return _T("SECAM (H) standard");
        case AnalogVideo_SECAM_K    : return _T("SECAM (K) standard");
        case AnalogVideo_SECAM_K1   : return _T("SECAM (K1) standard");
        case AnalogVideo_SECAM_L    : return _T("SECAM (L) standard");
        case AnalogVideo_SECAM_L1   : return _T("SECAM (L1) standard");
        case AnalogVideo_PAL_N_COMBO: return _T("Combination (N) PAL standard (Argentina)");
    }

    return _T("Unknown");
}

void CTunerInfoPage::GetLocatorInfo(CComPtr<ILocator> loc, GraphStudio::PropItem* info)
{
    long val;
    loc->get_CarrierFrequency(&val);
    info->AddItem(new GraphStudio::PropItem(_T("CarrierFrequency (kHz)"), val));

    FECMethod fecm;
    loc->get_InnerFEC(&fecm);
    info->AddItem(new GraphStudio::PropItem(_T("InnerFEC"), FECMethod2String(fecm)));

    BinaryConvolutionCodeRate bccr;
    loc->get_InnerFECRate(&bccr);
    info->AddItem(new GraphStudio::PropItem(_T("InnerFECRate"), FECRate2String(bccr)));

    loc->get_OuterFEC(&fecm);
    info->AddItem(new GraphStudio::PropItem(_T("OuterFEC"), FECMethod2String(fecm)));
    loc->get_OuterFECRate(&bccr);
    info->AddItem(new GraphStudio::PropItem(_T("OuterFECRate"), FECRate2String(bccr)));

    loc->get_SymbolRate(&val);
    info->AddItem(new GraphStudio::PropItem(_T("SymbolRate"), val));

    ModulationType mod;
    loc->get_Modulation(&mod);
    info->AddItem(new GraphStudio::PropItem(_T("Modulation"), Modulation2String(mod)));

    CComQIPtr<IAnalogLocator> locAna = loc;
    if(locAna)
    {
        info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IAnalogLocator"), false));
        AnalogVideoStandard vidst;
        locAna->get_VideoStandard(&vidst);
        info->AddItem(new GraphStudio::PropItem(_T("VideoStandard"),  AnalogVideoStandard2String(vidst)));
        return;
    }

    CComQIPtr<IDigitalLocator> locDig = loc;
    if(!locDig) return;
    
    CComQIPtr<IATSCLocator> locATSC = loc;
    if(locATSC)
    {
        CComQIPtr<IATSCLocator2> locATSC2 = loc;
        if(locATSC2)
        {
            CComQIPtr<IDigitalCableLocator> locDigC = loc;
            if(locDigC)
                info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IDigitalCableLocator"), false));
            else
                info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IATSCLocator2"), false));

            locATSC2->get_ProgramNumber(&val);
            info->AddItem(new GraphStudio::PropItem(_T("ProgramNumber"), val));
        }
        else
        {
            info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IATSCLocator"), false));
        }

        locATSC->get_PhysicalChannel(&val);
        info->AddItem(new GraphStudio::PropItem(_T("PhysicalChannel"), val));
        locATSC->get_TSID(&val);
        info->AddItem(new GraphStudio::PropItem(_T("TSID"), val));

        return;
    }

    CComQIPtr<IDVBTLocator> locT = loc;
    if(locT)
    {
        CComQIPtr<IDVBTLocator2> locT2 = loc;
        if(locT2)
        {
            info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IDVBTLocator2"), false));
            locT2->get_PhysicalLayerPipeId(&val);
            info->AddItem(new GraphStudio::PropItem(_T("PhysicalLayerPipeId"), val));
        }
        else
            info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IDVBTLocator"), false));

        locT->get_Bandwidth(&val);
        info->AddItem(new GraphStudio::PropItem(_T("Bandwidth"), val));

        GuardInterval guard;
        locT->get_Guard(&guard);
        CString guardStr = _T("Unknown");
        switch(guard)
        {
            case BDA_GUARD_NOT_SET     : guardStr = _T("not set"); break;
            case BDA_GUARD_NOT_DEFINED : guardStr = _T("not defined"); break;
            case BDA_GUARD_1_32        : guardStr = _T("1/32"); break;
            case BDA_GUARD_1_16        : guardStr = _T("1/16"); break;
            case BDA_GUARD_1_8         : guardStr = _T("1/8"); break;
            case BDA_GUARD_1_4         : guardStr = _T("1/4"); break;
            case BDA_GUARD_MAX         : guardStr = _T("MAX"); break;
        }
        info->AddItem(new GraphStudio::PropItem(_T("Guard"), guardStr));

        HierarchyAlpha halpha;
        locT->get_HAlpha(&halpha);
        CString halphaStr = _T("Unknown");
        switch(halpha)
        {
            case BDA_HALPHA_NOT_SET     : halphaStr = _T("not set"); break;
            case BDA_HALPHA_NOT_DEFINED : halphaStr = _T("not defined"); break;
            case BDA_HALPHA_1           : halphaStr = _T("1"); break;
            case BDA_HALPHA_2           : halphaStr = _T("2"); break;
            case BDA_HALPHA_4           : halphaStr = _T("4"); break;
            case BDA_HALPHA_MAX         : halphaStr = _T("MAX"); break;
        }
        info->AddItem(new GraphStudio::PropItem(_T("HAlpha"), halphaStr));

        locT->get_LPInnerFEC(&fecm);
        info->AddItem(new GraphStudio::PropItem(_T("LPInnerFEC"), FECMethod2String(fecm)));
        locT->get_LPInnerFECRate(&bccr);
        info->AddItem(new GraphStudio::PropItem(_T("LPInnerFECRate"), FECRate2String(bccr)));

        TransmissionMode tmode;
        locT->get_Mode(&tmode);
        CString tmodeStr = _T("Unknown");
        switch(tmode)
        {
            case BDA_XMIT_MODE_NOT_SET         : tmodeStr = _T("not set"); break;
            case BDA_XMIT_MODE_NOT_DEFINED     : tmodeStr = _T("not defined"); break;
            case BDA_XMIT_MODE_2K              : tmodeStr = _T("2k"); break;
            case BDA_XMIT_MODE_8K              : tmodeStr = _T("8k"); break;
            case BDA_XMIT_MODE_4K              : tmodeStr = _T("4k"); break;
            case BDA_XMIT_MODE_2K_INTERLEAVED  : tmodeStr = _T("interleaved 2k"); break;
            case BDA_XMIT_MODE_4K_INTERLEAVED  : tmodeStr = _T("interleaved 4k"); break;
            case BDA_XMIT_MODE_MAX             : tmodeStr = _T("MAX"); break;
        }
        info->AddItem(new GraphStudio::PropItem(_T("Mode"), tmodeStr));

        VARIANT_BOOL varb;
        locT->get_OtherFrequencyInUse(&varb);
        info->AddItem(new GraphStudio::PropItem(_T("OtherFrequencyInUse"), varb == VARIANT_TRUE));

        return;
    }

    CComQIPtr<IDVBSLocator> locS = loc;
    if(locS)
    {
        CComQIPtr<IDVBSLocator2> locS2 = loc;
        if(locS2)
            info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IDVBSLocator2"), false));
        else
            info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IDVBSLocator"), false));

        // TODO IDVBSLocator

        if(locS2)
        {
            // TODO IDVBSLocator2
        }

        return;
    }

    CComQIPtr<IDVBCLocator> locC = loc;
    if(locC)
    {
        info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("IDVBCLocator"), false));
    }
    else
    {
        info->AddItem(new GraphStudio::PropItem(_T("Type"), _T("Unknown"), false));
    }
}

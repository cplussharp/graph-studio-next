#include "stdafx.h"

CInterfaceInfo::CInterfaceInfo()
{
}

CInterfaceInfo::CInterfaceInfo(const CString& guid, const CString& name, const CString& header, const CString& url)
    : m_name(name), m_header(header), m_url(url), m_getDetailsFunc(NULL)
{
    CLSIDFromString(guid, &m_guid);
}

CInterfaceInfo::CInterfaceInfo(const CString& guid, const CString& name, const CString& header, const CString& url, ptGetInterfaceDetails detailsFunc)
        : m_name(name), m_header(header), m_url(url), m_getDetailsFunc(detailsFunc)
{
    CLSIDFromString(guid, &m_guid);
}

CInterfaceInfo::CInterfaceInfo(const CInterfaceInfo& info)
    : m_guid(info.m_guid), m_name(info.m_name), m_header(info.m_header), m_url(info.m_url)
{
}

const CString CInterfaceInfo::GetGuidString() const
{
    CString str;
    LPOLESTR ostr = NULL;
	StringFromCLSID(m_guid, &ostr);
	if (ostr)
    {
        str = CString(ostr);
		CoTaskMemFree(ostr);
	}
    else
    {
        str = _T("");
    }

    return str;
}

void CInterfaceInfo::GetInfo(GraphStudio::PropItem* group, IUnknown* pUnk) const
{
    group->AddItem(new GraphStudio::PropItem(_T("IID"), m_guid));
    group->AddItem(new GraphStudio::PropItem(_T("Header"), m_header));
    if(m_url.GetLength() > 0)
        group->AddItem(new GraphStudio::PropItem(_T("Info"), m_url, true));

    if(m_getDetailsFunc)
        m_getDetailsFunc(group, pUnk);
}

/*****************************************************************************************
* some Interface Infos
******************************************************************************************/
void GetInterfaceInfo_IAMFilterMiscFlags(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IAMFilterMiscFlags> pI = pUnk;
    if(pI)
    {
        ULONG flags = pI->GetMiscFlags();
        if((flags & AM_FILTER_MISC_FLAGS_IS_RENDERER) == AM_FILTER_MISC_FLAGS_IS_RENDERER)
            group->AddItem(new GraphStudio::PropItem(_T("MiscFlags"), _T("AM_FILTER_MISC_FLAGS_IS_RENDERER"), false));
        if((flags & AM_FILTER_MISC_FLAGS_IS_SOURCE) == AM_FILTER_MISC_FLAGS_IS_SOURCE)
            group->AddItem(new GraphStudio::PropItem(_T("MiscFlags"), _T("AM_FILTER_MISC_FLAGS_IS_SOURCE"), false));
    }
}

void GetInterfaceInfo_IAMDroppedFrames(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IAMDroppedFrames> pI = pUnk;
    if(pI)
    {
        long val;
        if(SUCCEEDED(pI->GetAverageFrameSize(&val)))
            group->AddItem(new GraphStudio::PropItem(_T("AverageFrameSize"), (__int64)val));

        if(SUCCEEDED(pI->GetNumDropped(&val)))
            group->AddItem(new GraphStudio::PropItem(_T("NumDropped"), (__int64)val));

        if(SUCCEEDED(pI->GetNumNotDropped(&val)))
            group->AddItem(new GraphStudio::PropItem(_T("NumNotDropped"), (__int64)val));
    }
}

void GetInterfaceInfo_IAsyncReader(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IAsyncReader> pI = pUnk;
    if(pI)
    {
        CString strTotal, strAvailable;
        LONGLONG total, available;
        HRESULT hr = pI->Length(&total, &available);
        if(hr == VFW_S_ESTIMATED)
        {
            strTotal.Format(_T("%I64d (estimated)"),total);
            strAvailable.Format(_T("%I64d (estimated)"),total);
        }
        else if(hr == S_OK)
        {
            strTotal.Format(_T("%I64d"),total);
            strAvailable.Format(_T("%I64d"),total);
        }

        group->AddItem(new GraphStudio::PropItem(_T("Length (Total)"), strTotal));
        group->AddItem(new GraphStudio::PropItem(_T("Length (Available)"), strTotal));
    }
}

void GetInterfaceInfo_IMediaFilter(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IMediaFilter> pI = pUnk;
    if(pI)
    {
        FILTER_STATE state;
        HRESULT hr = pI->GetState(100, &state);
        if(hr == VFW_S_STATE_INTERMEDIATE)
        {
            group->AddItem(new GraphStudio::PropItem(_T("State"), _T("VFW_S_STATE_INTERMEDIATE"), false));
        }
        else if(hr == VFW_S_CANT_CUE)
        {
            group->AddItem(new GraphStudio::PropItem(_T("State"), _T("VFW_S_CANT_CUE"), false));
        }
        else if(hr == S_OK)
        {
            if(state == State_Stopped)
                group->AddItem(new GraphStudio::PropItem(_T("State"), _T("Stopped"), false));
            else if(state == State_Running)
                group->AddItem(new GraphStudio::PropItem(_T("State"), _T("Running"), false));
            else if(state == State_Paused)
                group->AddItem(new GraphStudio::PropItem(_T("State"), _T("Paused"), false));
        }
    }
}

void GetInterfaceInfo_IFileSinkFilter(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IFileSinkFilter> pI = pUnk;
    if(pI)
    {
        LPOLESTR strFile;
        HRESULT hr = pI->GetCurFile(&strFile, NULL);
        if(hr == S_OK && strFile != NULL)
        {
            group->AddItem(new GraphStudio::PropItem(_T("CurFile"), CString(strFile), false));
        }
        else if(hr == E_FAIL || strFile != NULL)
        {
            group->AddItem(new GraphStudio::PropItem(_T("CurFile"), _T("No file is opened"), false));
        }

        if(strFile)
            CoTaskMemFree(strFile);
    }
}

void GetInterfaceInfo_IFileSinkFilter2(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IFileSinkFilter2> pI = pUnk;
    if(pI)
    {
        LPOLESTR strFile;
        HRESULT hr = pI->GetCurFile(&strFile, NULL);
        if(hr == S_OK && strFile != NULL)
        {
            group->AddItem(new GraphStudio::PropItem(_T("CurFile"), CString(strFile), false));
        }
        else if(hr == E_FAIL || strFile != NULL)
        {
            group->AddItem(new GraphStudio::PropItem(_T("CurFile"), _T("No file is opened"), false));
        }

        if(strFile)
            CoTaskMemFree(strFile);
        
        DWORD flags = 0;
        hr = pI->GetMode(&flags);
        if(SUCCEEDED(hr))
        {
            if((flags & AM_FILE_OVERWRITE) == AM_FILE_OVERWRITE)
            {
                group->AddItem(new GraphStudio::PropItem(_T("Mode"), _T("AM_FILE_OVERWRITE"), false));
            }
            else if(flags == 0)
            {
                group->AddItem(new GraphStudio::PropItem(_T("Mode"), (int)0));
            }
            else
            {
                group->AddItem(new GraphStudio::PropItem(_T("Mode"), _T("Unknown Flag"), false));
            }
        }
    }
}

void GetInterfaceInfo_IFileSourceFilter(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IFileSourceFilter> pI = pUnk;
    if(pI)
    {
        LPOLESTR strFile;
        HRESULT hr = pI->GetCurFile(&strFile, NULL);
        if(hr == S_OK && strFile != NULL)
        {
            group->AddItem(new GraphStudio::PropItem(_T("CurFile"), CString(strFile), false));
        }
        else if(hr == E_FAIL || strFile != NULL)
        {
            group->AddItem(new GraphStudio::PropItem(_T("CurFile"), _T("No file is opened"), false));
        }

        if(strFile)
            CoTaskMemFree(strFile);
    }
}

const CString TimeFormat2String(const GUID& timeFormatGuid)
{
    if(timeFormatGuid == GUID_TIME_MUSIC)
        return CString(_T("Music"));
    else if(timeFormatGuid == GUID_TIME_SAMPLES)
        return CString(_T("Sample"));
    else if(timeFormatGuid == GUID_TIME_REFERENCE)
        return CString(_T("Reference"));

    if(timeFormatGuid == TIME_FORMAT_MEDIA_TIME)
        return CString(_T("MediaTime"));
    else if(timeFormatGuid == TIME_FORMAT_NONE)
        return CString(_T("None"));
    else if(timeFormatGuid == TIME_FORMAT_FRAME)
        return CString(_T("Frame"));
    else if(timeFormatGuid == TIME_FORMAT_SAMPLE)
        return CString(_T("Sample"));
    else if(timeFormatGuid == TIME_FORMAT_FIELD)
        return CString(_T("Field"));
    else if(timeFormatGuid == TIME_FORMAT_BYTE)
        return CString(_T("Byte"));

    return CString(_T(""));
}

void GetInterfaceInfo_IMediaSeeking(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IMediaSeeking> pI = pUnk;
    if(pI)
    {
        LONGLONG valEarliest, valLatest;
        HRESULT hr = pI->GetAvailable(&valEarliest,&valLatest);
        if(SUCCEEDED(hr))
        {
            group->AddItem(new GraphStudio::PropItem(_T("Available Earliest"), valEarliest));
            group->AddItem(new GraphStudio::PropItem(_T("Available Latest"), valLatest));
        }
        else
        {
            group->AddItem(new GraphStudio::PropItem(_T("Available"), _T("Not implementet"), false));
        }

        DWORD cap;
        pI->GetCapabilities(&cap);
        CStringArray strCap;
        if((cap & AM_SEEKING_CanSeekAbsolute) == AM_SEEKING_CanSeekAbsolute)
            strCap.Add(_T("SeekAbsolute"));
        if((cap & AM_SEEKING_CanSeekForwards) == AM_SEEKING_CanSeekForwards)
            strCap.Add(_T("SeekForwards"));
        if((cap & AM_SEEKING_CanSeekBackwards) == AM_SEEKING_CanSeekBackwards)
            strCap.Add(_T("SeekBackwards"));
        if((cap & AM_SEEKING_CanGetCurrentPos) == AM_SEEKING_CanGetCurrentPos)
            strCap.Add(_T("GetCurrentPos"));
        if((cap & AM_SEEKING_CanGetStopPos) == AM_SEEKING_CanGetStopPos)
            strCap.Add(_T("GetStopPos"));
        if((cap & AM_SEEKING_CanGetDuration) == AM_SEEKING_CanGetDuration)
            strCap.Add(_T("GetDuration"));
        if((cap & AM_SEEKING_CanPlayBackwards) == AM_SEEKING_CanPlayBackwards)
            strCap.Add(_T("PlayBackwards"));
        if((cap & AM_SEEKING_CanDoSegments) == AM_SEEKING_CanDoSegments)
            strCap.Add(_T("DoSegments"));
        if((cap & AM_SEEKING_Source) == AM_SEEKING_Source)
            strCap.Add(_T("Source"));

        CString strCapText;
        for(int i=0;i<strCap.GetCount(); i++)
        {
            if(i > 0) strCapText += _T(", ");
            strCapText += strCap[i];
        }
        group->AddItem(new GraphStudio::PropItem(_T("Capabilities"), strCapText));

        if(SUCCEEDED(pI->GetCurrentPosition(&valEarliest)))
            group->AddItem(new GraphStudio::PropItem(_T("CurrentPosition"), valEarliest));
        else
            group->AddItem(new GraphStudio::PropItem(_T("CurrentPosition"), _T(""), false));

        if(SUCCEEDED(pI->GetDuration(&valEarliest)))
            group->AddItem(new GraphStudio::PropItem(_T("Duration"), valEarliest));
        else
            group->AddItem(new GraphStudio::PropItem(_T("Duration"), _T(""), false));

        if(SUCCEEDED(pI->GetStopPosition(&valEarliest)))
            group->AddItem(new GraphStudio::PropItem(_T("StopPosition"), valEarliest));
        else
            group->AddItem(new GraphStudio::PropItem(_T("StopPosition"), _T(""), false));

        if(SUCCEEDED(pI->GetPreroll(&valEarliest)))
            group->AddItem(new GraphStudio::PropItem(_T("Preroll"), valEarliest));
        else
            group->AddItem(new GraphStudio::PropItem(_T("Preroll"), _T(""), false));

        double rate;
        if(SUCCEEDED(pI->GetRate(&rate)))
            group->AddItem(new GraphStudio::PropItem(_T("Rate"), (int)(rate * 100.0)));
        else
            group->AddItem(new GraphStudio::PropItem(_T("Rate"), _T(""), false));

        GUID timeFormat;
        if(SUCCEEDED(pI->GetTimeFormat(&timeFormat)))
            group->AddItem(new GraphStudio::PropItem(_T("TimeFormat"), TimeFormat2String(timeFormat)));
        else
            group->AddItem(new GraphStudio::PropItem(_T("TimeFormat"), _T(""), false));

        if(SUCCEEDED(pI->QueryPreferredFormat(&timeFormat)))
            group->AddItem(new GraphStudio::PropItem(_T("PreferredFormat"), TimeFormat2String(timeFormat)));
        else
            group->AddItem(new GraphStudio::PropItem(_T("PreferredFormat"), _T(""), false));

        CStringArray strFormats;
        if(S_OK == pI->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME))
            strFormats.Add(TimeFormat2String(TIME_FORMAT_MEDIA_TIME));
        else if(S_OK == pI->IsFormatSupported(&TIME_FORMAT_NONE))
            strFormats.Add(TimeFormat2String(TIME_FORMAT_NONE));
        else if(S_OK == pI->IsFormatSupported(&TIME_FORMAT_FRAME))
            strFormats.Add(TimeFormat2String(TIME_FORMAT_FRAME));
        else if(S_OK == pI->IsFormatSupported(&TIME_FORMAT_SAMPLE))
            strFormats.Add(TimeFormat2String(TIME_FORMAT_SAMPLE));
        else if(S_OK == pI->IsFormatSupported(&TIME_FORMAT_FIELD))
            strFormats.Add(TimeFormat2String(TIME_FORMAT_FIELD));
        else if(S_OK == pI->IsFormatSupported(&TIME_FORMAT_BYTE))
            strFormats.Add(TimeFormat2String(TIME_FORMAT_BYTE));

        CString strFormatText;
        for(int i=0;i<strFormats.GetCount(); i++)
        {
            if(i > 0) strFormatText += _T(", ");
            strFormatText += strFormats[i];
        }
        group->AddItem(new GraphStudio::PropItem(_T("SupportedFormats"), strFormatText));
    }
}

void GetInterfaceInfo_IMediaParamInfo(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IMediaParamInfo> pI = pUnk;
    if(pI)
    {
        GUID timeFormat;
        MP_TIMEDATA timeData;
        HRESULT hr = pI->GetCurrentTimeFormat(&timeFormat, &timeData);
        if(SUCCEEDED(hr))
            group->AddItem(new GraphStudio::PropItem(_T("CurrentFormat"), TimeFormat2String(timeFormat)));

        DWORD numFormats;
        if(SUCCEEDED(pI->GetNumTimeFormats(&numFormats)))
        {
            CString strFormatText;
            for(DWORD i=0;i<numFormats; i++)
            {
                if(SUCCEEDED(pI->GetSupportedTimeFormat(i, &timeFormat)))
                {
                    if(strFormatText.GetLength() > 0) strFormatText += _T(", ");
                    strFormatText += TimeFormat2String(timeFormat);
                }
            }

            group->AddItem(new GraphStudio::PropItem(_T("SupportedFormat"), strFormatText));
        }
    }
}

void GetInterfaceInfo_IMediaParams(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    
}

void GetInterfaceInfo_IUnknown(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    pUnk->AddRef();
    group->AddItem(new GraphStudio::PropItem(_T("RefCount"), (int)pUnk->Release() - 1));
}

void GetInterfaceInfo_IDMOVideoOutputOptimizations(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IDMOVideoOutputOptimizations> pI = pUnk;
    if(pI)
    {
        CString str = TEXT("DMO_VOSF_NEEDS_PREVIOUS_SAMPLE");
        CString strNone = TEXT("");
        DWORD val;
        HRESULT hr = pI->GetCurrentOperationMode(0, &val);
        if(SUCCEEDED(pI->GetCurrentOperationMode(0, &val)))
            group->AddItem(new GraphStudio::PropItem(_T("CurrentOperationMode"), val ? str : strNone));
        if(SUCCEEDED(pI->GetCurrentSampleRequirements(0, &val)))
            group->AddItem(new GraphStudio::PropItem(_T("CurrentSampleRequirements"), val ? str : strNone));
        if(SUCCEEDED(pI->QueryOperationModePreferences(0, &val)))
            group->AddItem(new GraphStudio::PropItem(_T("QueryOperationModePreferences"), val ? str : strNone));
    }
}

void GetInterfaceInfo_IAMMediaContent(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComPtr<IAMMediaContent> pI;
    pUnk->QueryInterface(IID_IAMMediaContent, (void**)&pI);
    if(pI)
    {
        BSTR val;
        if(SUCCEEDED(pI->get_AuthorName(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("AuthorName"), CString(val)));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_Title(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("Title"), CString(val)));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_Rating(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("Rating"), CString(val)));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_Description(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("Description"), CString(val)));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_Copyright(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("Copyright"), CString(val)));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_BaseURL(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("BaseURL"), CString(val), true));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_LogoURL(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("LogoURL"), CString(val), true));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_LogoIconURL(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("LogoIconURL"), CString(val), true));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_WatermarkURL(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("WatermarkURL"), CString(val), true));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_MoreInfoBannerImage(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("MoreInfoBannerImage"), CString(val)));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_MoreInfoBannerURL(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("MoreInfoBannerURL"), CString(val), true));
            SysFreeString(val);
        }
        if(SUCCEEDED(pI->get_MoreInfoText(&val))) {
            group->AddItem(new GraphStudio::PropItem(_T("MoreInfoText"), CString(val)));
            SysFreeString(val);
        }
    }
}

void GetInterfaceInfo_IAMOpenProgress(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComQIPtr<IAMOpenProgress> pI = pUnk;
    if(pI)
    {
        LONGLONG total, cur;
        HRESULT hr = pI->QueryProgress(&total, &cur);
        if(hr == S_OK || hr == VFW_S_ESTIMATED)
        {
            CString propName;
            propName.Format(_T("Progress Total%s"), hr == VFW_S_ESTIMATED ? _T(" (estimated)") : _T(""));
            group->AddItem(new GraphStudio::PropItem(propName, total));
            propName.Format(_T("Progress Current%s"), hr == VFW_S_ESTIMATED ? _T(" (estimated)") : _T(""));
            group->AddItem(new GraphStudio::PropItem(propName, cur));
        }
    }
}

void GetInterfaceInfo_IAMExtendedErrorInfo(GraphStudio::PropItem* group, IUnknown* pUnk)
{
    CComPtr<IAMExtendedErrorInfo> pI;
    pUnk->QueryInterface(IID_IAMExtendedErrorInfo, (void**)&pI);
    if(pI)
    {
        VARIANT_BOOL hasError;
        pI->get_HasError(&hasError);
        CString strHasError = hasError != VARIANT_FALSE ? _T("TRUE") : _T("FALSE");
        group->AddItem(new GraphStudio::PropItem(_T("HasError"), strHasError));
        if(hasError)
        {
            long errCode = 0;
            pI->get_ErrorCode(&errCode);
            group->AddItem(new GraphStudio::PropItem(_T("ErrorCode"), errCode));

            BSTR errDesc = NULL;
            pI->get_ErrorDescription(&errDesc);
            group->AddItem(new GraphStudio::PropItem(_T("ErrorCode"), CString(errDesc)));
            if(errDesc) SysFreeString(errDesc);
        }
    }
}

/*****************************************************************************************
* known Interfaces to test
******************************************************************************************/
const CInterfaceInfo CInterfaceScanner::m_knownInterfaces[] = 
{
    CInterfaceInfo(TEXT("{54C39221-8380-11d0-B3F0-00AA003761C5}"), TEXT("IAMAudioInputMixer"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389123.aspx")),
    CInterfaceInfo(TEXT("{22320CB2-D41A-11D2-BF7C-D7CB9DF0BF93}"), TEXT("IAMAudioRendererStats"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389140.aspx")),
    CInterfaceInfo(TEXT("{56ED71A0-AF5F-11D0-B3F0-00AA003761C5}"), TEXT("IAMBufferNegotiation"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389142.aspx")),
    CInterfaceInfo(TEXT("{C6E13370-30AC-11D0-A18C-00A0C9118956}"), TEXT("IAMCameraControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389145.aspx")),
    CInterfaceInfo(TEXT("{6feded3e-0ff1-4901-a2f1-43f7012c8515}"), TEXT("IAMCertifiedOutputProtection"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389149.aspx")),
    CInterfaceInfo(TEXT("{FA2AA8F2-8B62-11D0-A520-000000000000}"), TEXT("IAMChannelInfo"), TEXT("qnetwork.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389154.aspx")),
    CInterfaceInfo(TEXT("{4D5466B0-A49C-11D1-ABE8-00A0C905F375}"), TEXT("IAMClockAdjust"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389161.aspx")),
    CInterfaceInfo(TEXT("{9FD52741-176D-4B36-8F51-CA8F933223BE}"), TEXT("IAMClockSlave"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389163.aspx")),
    CInterfaceInfo(TEXT("{670D1D20-A068-11D0-B3F0-00AA003761C5}"), TEXT("IAMCopyCaptureFileProgress"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389169.aspx")),
    CInterfaceInfo(TEXT("{C6E13380-30AC-11D0-A18C-00A0C9118956}"), TEXT("IAMCrossbar"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389171.aspx")),
    CInterfaceInfo(TEXT("{C0DFF467-D499-4986-972B-E1D9090FA941}"), TEXT("IAMDecoderCaps"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389177.aspx")),
    CInterfaceInfo(TEXT("{546F4260-D53E-11CF-B3F0-00AA003761C5}"), TEXT("IAMDirectSound"), TEXT("uuids.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389193.aspx")),
    CInterfaceInfo(TEXT("{C6E13344-30AC-11D0-A18C-00A0C9118956}"), TEXT("IAMDroppedFrames"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389311.aspx"), GetInterfaceInfo_IAMDroppedFrames),
    CInterfaceInfo(TEXT("{E43E73A2-0EFA-11D3-9601-00A0C9441E20}"), TEXT("IAMErrorLog"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389316.aspx")),
    CInterfaceInfo(TEXT("{B5730A90-1A2C-11CF-8C23-00AA006B6814}"), TEXT("IAMExtDevice"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389318.aspx")),
    CInterfaceInfo(TEXT("{FA2AA8F6-8B62-11D0-A520-000000000000}"), TEXT("IAMExtendedErrorInfo"), TEXT("qnetwork.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389327.aspx"), GetInterfaceInfo_IAMExtendedErrorInfo),
    CInterfaceInfo(TEXT("{FA2AA8F9-8B62-11D0-A520-000000000000}"), TEXT("IAMExtendedSeeking"), TEXT("qnetwork.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389331.aspx")),
    CInterfaceInfo(TEXT("{A03CD5F0-3045-11CF-8C44-00AA006B6814}"), TEXT("IAMExtTransport"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389340.aspx")),
    CInterfaceInfo(TEXT("{2DD74950-A890-11D1-ABE8-00A0C905F375}"), TEXT("IAMFilterMiscFlags"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389374.aspx"), GetInterfaceInfo_IAMFilterMiscFlags),
    CInterfaceInfo(TEXT("{4995F511-9DDB-4F12-BD3B-F04611807B79}"), TEXT("IAMGraphBuilderCallback"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389376.aspx")),
    CInterfaceInfo(TEXT("{632105FA-072E-11D3-8AF9-00C04FB6BD3D}"), TEXT("IAMGraphStreams"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389379.aspx")),
    CInterfaceInfo(TEXT("{6E8D4A21-310C-11D0-B79A-00AA003767A7}"), TEXT("IAMLine21Decode"), TEXT("uuids.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389385.aspx")),
    CInterfaceInfo(TEXT("{FA2AA8F4-8B62-11D0-A520-000000000000}"), TEXT("IAMMediaContent"), TEXT("qnetwork.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319638.aspx"), GetInterfaceInfo_IAMMediaContent),
    CInterfaceInfo(TEXT("{CE8F78C1-74D9-11D2-B09D-00A0C9A81117}"), TEXT("IAMMediaContent2"), TEXT("qnetwork.h"), TEXT("")),
    CInterfaceInfo(TEXT("{BEBE595D-9A6F-11D0-8FDE-00C04FD9189D}"), TEXT("IAMMediaStream"), TEXT("amstream.h"), TEXT("")),
    CInterfaceInfo(TEXT("{AB6B4AFB-F6E4-11D0-900D-00C04FD9189D}"), TEXT("IAMMediaTypeSample"), TEXT("amstream.h"), TEXT("")),
    CInterfaceInfo(TEXT("{AB6B4AFA-F6E4-11D0-900D-00C04FD9189D}"), TEXT("IAMMediaTypeStream"), TEXT("amstream.h"), TEXT("")),
    CInterfaceInfo(TEXT("{BEBE595C-9A6F-11D0-8FDE-00C04FD9189D}"), TEXT("IAMMultiMediaStream"), TEXT("amstream.h"), TEXT("")),
    CInterfaceInfo(TEXT("{FA2AA8F1-8B62-11D0-A520-000000000000}"), TEXT("IAMNetShowConfig"), TEXT("qnetwork.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319696.aspx")),
    CInterfaceInfo(TEXT("{FA2AA8F5-8B62-11D0-A520-000000000000}"), TEXT("IAMNetShowExProps"), TEXT("qnetwork.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319719.aspx")),
    CInterfaceInfo(TEXT("{AAE7E4E2-6388-11D1-8D93-006097C9A2B2}"), TEXT("IAMNetShowPreroll"), TEXT("qnetwork.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319729.aspx")),
    CInterfaceInfo(TEXT("{FA2AA8F3-8B62-11D0-A520-000000000000}"), TEXT("IAMNetworkStatus"), TEXT("qnetwork.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319732.aspx")),
    CInterfaceInfo(TEXT("{8E1C39A1-DE53-11CF-AA63-0080C744528D}"), TEXT("IAMOpenProgress"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319740.aspx"), GetInterfaceInfo_IAMOpenProgress),
    CInterfaceInfo(TEXT("{62FAE250-7E65-4460-BFC9-6398B322073C}"), TEXT("IAMOverlayFX"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319743.aspx")),
    CInterfaceInfo(TEXT("{F185FE76-E64E-11d2-B76E-00C04FB6BD3D}"), TEXT("IAMPushSource"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319764.aspx")),
    CInterfaceInfo(TEXT("{8389D2D0-77D7-11D1-ABE6-00A0C905F375}"), TEXT("IAMResourceControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319771.aspx")),
    CInterfaceInfo(TEXT("{963566DA-BE21-4EAF-88E9-35704F8F52A1}"), TEXT("IAMSetErrorLog"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319773.aspx")),
    CInterfaceInfo(TEXT("{BC9BCF80-DCD2-11D2-ABF6-00A0C905F375}"), TEXT("IAMStats"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319776.aspx")),
    CInterfaceInfo(TEXT("{C6E13340-30AC-11D0-A18C-00A0C9118956}"), TEXT("IAMStreamConfig"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319784.aspx")),
    CInterfaceInfo(TEXT("{36b73881-c2c8-11cf-8b46-00805f6cef60}"), TEXT("IAMStreamControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319789.aspx")),
    CInterfaceInfo(TEXT("{C1960960-17F5-11D1-ABE1-00A0C905F375}"), TEXT("IAMStreamSelect"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319793.aspx")),
    CInterfaceInfo(TEXT("{9B496CE1-811B-11cf-8C77-00AA006B6814}"), TEXT("IAMTimecodeReader"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319809.aspx")),
    CInterfaceInfo(TEXT("{78530B74-61F9-11D2-8CAD-00A024580902}"), TEXT("IAMTimeline"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319815.aspx")),
    CInterfaceInfo(TEXT("{EAE58536-622E-11D2-8CAD-00A024580902}"), TEXT("IAMTimelineComp"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319816.aspx")),
    CInterfaceInfo(TEXT("{BCE0C264-622D-11D2-8CAD-00A024580902}"), TEXT("IAMTimelineEffect"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319825.aspx")),
    CInterfaceInfo(TEXT("{EAE58537-622E-11D2-8CAD-00A024580902}"), TEXT("IAMTimelineEffectable"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319826.aspx")),
    CInterfaceInfo(TEXT("{9EED4F00-B8A6-11D2-8023-00C0DF10D434}"), TEXT("IAMTimelineGroup"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319832.aspx")),
    CInterfaceInfo(TEXT("{78530B77-61F9-11D2-8CAD-00A024580902}"), TEXT("IAMTimelineObj"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd319854.aspx")),
    CInterfaceInfo(TEXT("{A0F840A0-D590-11D2-8D55-00A0C9441E20}"), TEXT("IAMTimelineSplittable"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375853.aspx")),
    CInterfaceInfo(TEXT("{78530B79-61F9-11D2-8CAD-00A024580902}"), TEXT("IAMTimelineSrc"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375856.aspx")),
    CInterfaceInfo(TEXT("{EAE58538-622E-11D2-8CAD-00A024580902}"), TEXT("IAMTimelineTrack"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375879.aspx")),
    CInterfaceInfo(TEXT("{BCE0C265-622D-11D2-8CAD-00A024580902}"), TEXT("IAMTimelineTrans"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375895.aspx")),
    CInterfaceInfo(TEXT("{378FA386-622E-11D2-8CAD-00A024580902}"), TEXT("IAMTimelineTransable"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375896.aspx")),
    CInterfaceInfo(TEXT("{A8ED5F80-C2C7-11D2-8D39-00A0C9441E20}"), TEXT("IAMTimelineVirtualTrack"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375912.aspx")),
    CInterfaceInfo(TEXT("{211A8761-03AC-11D1-8D13-00AA00BD8339}"), TEXT("IAMTuner"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375944.aspx")),
    CInterfaceInfo(TEXT("{211A8760-03AC-11D1-8D13-00AA00BD8339}"), TEXT("IAMTunerNotification"), TEXT("strmif.h"), TEXT("")),
    CInterfaceInfo(TEXT("{83EC1C30-23D1-11D1-99E6-00A0C9560266}"), TEXT("IAMTVAudio"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375962.aspx")),
    CInterfaceInfo(TEXT("{211A8766-03AC-11d1-8D13-00AA00BD8339}"), TEXT("IAMTVTuner"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375971.aspx")),
    CInterfaceInfo(TEXT("{D8D715A0-6E5E-11D0-B3F0-00AA003761C5}"), TEXT("IAMVfwCaptureDialogs"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375983.aspx")),
    CInterfaceInfo(TEXT("{D8D715A3-6E5E-11D0-B3F0-00AA003761C5}"), TEXT("IAMVfwCompressDialogs"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375987.aspx")),
    CInterfaceInfo(TEXT("{256A6A22-FBAD-11d1-82BF-00A0C9696C8F}"), TEXT("IAMVideoAccelerator"), TEXT("Videoacc.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd375992.aspx")),
    CInterfaceInfo(TEXT("{C6E13343-30AC-11D0-A18C-00A0C9118956}"), TEXT("IAMVideoCompression"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376011.aspx")),
    CInterfaceInfo(TEXT("{6A2E0670-28E4-11D0-A18C-00A0C9118956}"), TEXT("IAMVideoControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376023.aspx")),
    CInterfaceInfo(TEXT("{60D32930-13DA-11D3-9EC6-C4FCAEF5C7BE}"), TEXT("IAMVideoDecimationProperties"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376030.aspx")),
    CInterfaceInfo(TEXT("{C6E13360-30AC-11D0-A18C-00A0C9118956}"), TEXT("IAMVideoProcAmp"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376033.aspx")),
    CInterfaceInfo(TEXT("{6DD816D7-E740-4123-9E24-2444412644D8}"), TEXT("IAMWMBufferPass"), TEXT("Dshowasf.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376037.aspx")),
    CInterfaceInfo(TEXT("{C056DE21-75C2-11D3-A184-00105AEF9F33}"), TEXT("IAMWstDecoder"), TEXT("uuids.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376041.aspx")),
    CInterfaceInfo(TEXT("{2A6E293B-2595-11D3-B64C-00C04F79498E}"), TEXT("IAnalogRadioTuningSpace"), TEXT("tuner.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693098.aspx")),
    CInterfaceInfo(TEXT("{39DD45DA-2DA8-46BA-8A8A-87E2B73D983A}"), TEXT("IAnalogRadioTuningSpace2"), TEXT("tuner.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693099.aspx")),
    CInterfaceInfo(TEXT("{56A868AA-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IAsyncReader"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376085.aspx"),GetInterfaceInfo_IAsyncReader),
    CInterfaceInfo(TEXT("{FC189E4D-7BD4-4125-B3B3-3A76A332CC96}"), TEXT("IATSCComponentType"), TEXT("tuner.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693121.aspx")),
    CInterfaceInfo(TEXT("{BF8D986F-8C2B-4131-94D7-4D3D9FCC21EF}"), TEXT("IATSCLocator"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{612AA885-66CF-4090-BA0A-566F5312E4CA}"), TEXT("IATSCLocator2"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{F7537560-A3BE-11D0-8212-00C04FC32C45}"), TEXT("IAudioMediaStream"), TEXT("austream.h"), TEXT("")),
    CInterfaceInfo(TEXT("{E48244B8-7E17-4F76-A763-5090FF1E2F30}"), TEXT("IAuxInTuningSpace"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{B10931ED-8BFE-4AB0-9DCE-E469C29A9729}"), TEXT("IAuxInTuningSpace2"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{56A86895-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IBaseFilter"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389526.aspx")),
    CInterfaceInfo(TEXT("{56A868B3-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IBasicAudio"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389532.aspx")),
    CInterfaceInfo(TEXT("{56A868B5-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IBasicVideo"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389540.aspx")),
    CInterfaceInfo(TEXT("{329BB360-F6EA-11D1-9038-00A0C9697298}"), TEXT("IBasicVideo2"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389541.aspx")),
    CInterfaceInfo(TEXT("{DDF15B12-BD25-11D2-9CA0-00C04F7971E0}"), TEXT("IBDA_AutoDemodulate"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693252.aspx")),
    CInterfaceInfo(TEXT("{34518D13-1182-48e6-B28F-B24987787326}"), TEXT("IBDA_AutoDemodulateEx"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693253.aspx")),
    CInterfaceInfo(TEXT("{FD0A5AF3-B41D-11D2-9C95-00C04F7971E0}"), TEXT("IBDA_DeviceControl"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693278.aspx")),
    CInterfaceInfo(TEXT("{20e80cb5-c543-4c1b-8eb3-49e719eee7d4}"), TEXT("IBDA_DiagnosticProperties"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693283.aspx")),
    CInterfaceInfo(TEXT("{EF30F379-985B-4D10-B640-A79D5E04E1E0}"), TEXT("IBDA_DigitalDemodulator"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693284.aspx")),
    CInterfaceInfo(TEXT("{525ED3EE-5CF3-4e1e-9A06-5368A84F9A6E}"), TEXT("IBDA_DigitalDemodulator2"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693285.aspx")),
    CInterfaceInfo(TEXT("{13F19604-7D32-4359-93A2-A05205D90AC9}"), TEXT("IBDA_DigitalDemodulator3"), TEXT("bdaiface.h"), TEXT("")),
    CInterfaceInfo(TEXT("{71985F43-1CA1-11D3-9CC8-00C04F7971E0}"), TEXT("IBDA_EthernetFilter"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693329.aspx")),
    CInterfaceInfo(TEXT("{71985F47-1CA1-11D3-9CC8-00C04F7971E0}"), TEXT("IBDA_FrequencyFilter"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693355.aspx")),
    CInterfaceInfo(TEXT("{3F4DC8E2-4050-11D3-8F4B-00C04F7971E2}"), TEXT("IBDA_IPSinkControl"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693375.aspx")),
    CInterfaceInfo(TEXT("{A750108F-492E-4D51-95F7-649B23FF7AD7}"), TEXT("IBDA_IPSinkInfo"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693378.aspx")),
    CInterfaceInfo(TEXT("{71985F44-1CA1-11D3-9CC8-00C04F7971E0}"), TEXT("IBDA_IPV4Filter"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693382.aspx")),
    CInterfaceInfo(TEXT("{E1785A74-2A23-4FB3-9245-A8F88017EF33}"), TEXT("IBDA_IPV6Filter"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693388.aspx")),
    CInterfaceInfo(TEXT("{992CF102-49F9-4719-A664-C4F23E2408F4}"), TEXT("IBDA_LNBInfo"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693396.aspx")),
    CInterfaceInfo(TEXT("{DDF15B0D-BD25-11d2-9CA0-00C04F7971E0}"), TEXT("IBDA_NullTransform"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693418.aspx")),
    CInterfaceInfo(TEXT("{fd501041-8ebe-11ce-8183-00aa00577da2}"), TEXT("IBDA_NetworkProvider"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693410.aspx")),
    CInterfaceInfo(TEXT("{0DED49D5-A8B7-4d5d-97A1-12B0C195874D}"), TEXT("IBDA_PinControl"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693421.aspx")),
    CInterfaceInfo(TEXT("{D2F1644B-B409-11D2-BC69-00A0C9EE9E16}"), TEXT("IBDA_SignalProperties"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693425.aspx")),
    CInterfaceInfo(TEXT("{1347D106-CF3A-428A-A5CB-AC0D9A2A4338}"), TEXT("IBDA_SignalStatistics"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693432.aspx")),
    CInterfaceInfo(TEXT("{79B56888-7FEA-4690-B45D-38FD3C7849BE}"), TEXT("IBDA_Topology"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693447.aspx")),
    CInterfaceInfo(TEXT("{8E882535-5F86-47AB-86CF-C281A72A0549}"), TEXT("IBDA_TransportStreamInfo"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693459.aspx")),
    CInterfaceInfo(TEXT("{71985F46-1CA1-11d3-9CC8-00C04F7971E0}"), TEXT("IBDA_VoidTransform"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd693461.aspx")),
    CInterfaceInfo(TEXT("{B34505E0-2F0E-497B-80BC-D43F3B24ED7F}"), TEXT("IBDAComparable"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{3B21263F-26E8-489D-AAC4-924F7EFD9511}"), TEXT("IBroadcastEvent"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{3D9E3887-1929-423F-8021-43682DE95448}"), TEXT("IBroadcastEventEx"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{93E5A4E0-2D50-11D2-ABFA-00A0C9C6E38D}"), TEXT("ICaptureGraphBuilder2"), TEXT("strmif.h"), TEXT("")),
    CInterfaceInfo(TEXT("{7C6995FB-2A31-4BD7-953E-B1AD7FB7D31C}"), TEXT("ICAT"), TEXT("mpeg2psiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{4B2BD7EA-8347-467B-8DBF-62F784929CC3}"), TEXT("ICCSubStreamFiltering"), TEXT("bdaiface.h"), TEXT("")),
    CInterfaceInfo(TEXT("{0369B4E0-45B6-11D3-B650-00C04F79498E}"), TEXT("IChannelTuneRequest"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{901db4c7-31ce-41a2-85dc-8fa0bf41b8da}"), TEXT("ICodecAPI"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd311953.aspx")),
    CInterfaceInfo(TEXT("{1A5576FC-0E19-11D3-9D8E-00C04F72D980}"), TEXT("IComponent"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{39A48091-FFFE-4182-A161-3FF802640E26}"), TEXT("IComponents"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{FCD01846-0E19-11D3-9D8E-00C04F72D980}"), TEXT("IComponentsOld"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{6A340DC0-0311-11D3-9D8E-00C04F72D980}"), TEXT("IComponentType"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{0DC13D4A-0313-11D3-9D8E-00C04F72D980}"), TEXT("IComponentTypes"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{45086030-F7E4-486A-B504-826BB5792A3B}"), TEXT("IConfigAsfWriter"), TEXT("dshowasf.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd312017.aspx")),
    CInterfaceInfo(TEXT("{7989CCAA-53F0-44F0-884A-F3B03F6AE066}"), TEXT("IConfigAsfWriter2"), TEXT("dshowasf.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd312018.aspx")),
    CInterfaceInfo(TEXT("{5ACD6AA0-F482-11CE-8B67-00AA00A3F1A6}"), TEXT("IConfigAviMux"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd312018.aspx")),
    CInterfaceInfo(TEXT("{BEE3D220-157B-11D0-BD23-00A0C911CE86}"), TEXT("IConfigInterleaving"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406738.aspx")),
    CInterfaceInfo(TEXT("{29840822-5B84-11D0-BD3B-00A0C911CE86}"), TEXT("ICreateDevEnum"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406743.aspx")),
    CInterfaceInfo(TEXT("{8A674B48-1F63-11D3-B64C-00C04F79498E}"), TEXT("ICreatePropBagOnRegKey"), TEXT("regbag.h"), TEXT("")),
    CInterfaceInfo(TEXT("{153ACC21-D83B-11d1-82BF-00A0C9696C8F}"), TEXT("IDDrawExclModeVideo"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406747.aspx")),
    CInterfaceInfo(TEXT("{2e5ea3e0-e924-11d2-b6da-00a0c995e8df}"), TEXT("IDecimateVideoImage"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406759.aspx")),
    CInterfaceInfo(TEXT("{56A868B8-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IDeferredCommand"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406762.aspx")),
    CInterfaceInfo(TEXT("{19B595D8-839A-47F0-96DF-4F194F3C768C}"), TEXT("IDigitalLocator"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{65abea96-cf36-453f-af8a-705e98f16260}"), TEXT("IDMOQualityControl"), TEXT("mediaobj.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406839.aspx")),
    CInterfaceInfo(TEXT("{BE8F4F4E-5B16-4D29-B350-7F6B5D9298AC}"), TEXT("IDMOVideoOutputOptimizations"), TEXT("mediaobj.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406843.aspx"), GetInterfaceInfo_IDMOVideoOutputOptimizations),
    CInterfaceInfo(TEXT("{52D6F586-9F0F-4824-8FC8-E32CA04930C2}"), TEXT("IDMOWrapperFilter"), TEXT("dmoshow.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406848.aspx")),
    CInterfaceInfo(TEXT("{C4C4C4B2-0049-4E2B-98FB-9537F6CE516D}"), TEXT("IDTFilter"), TEXT("encdec.h"), TEXT("")),
    CInterfaceInfo(TEXT("{C4C4C4B4-0049-4E2B-98FB-9537F6CE516D}"), TEXT("IDTFilter2"), TEXT("encdec.h"), TEXT("")),
    CInterfaceInfo(TEXT("{513998CC-E929-4CDF-9FBD-BAD1E0314866}"), TEXT("IDTFilter3"), TEXT("encdec.h"), TEXT("")),
    CInterfaceInfo(TEXT("{C4C4C4D2-0049-4E2B-98FB-9537F6CE516D}"), TEXT("IDTFilterConfig"), TEXT("encdec.h"), TEXT("")),
    CInterfaceInfo(TEXT("{ECE9BB0C-43B6-4558-A0EC-1812C34CD6CA}"), TEXT("IDVB_BAT"), TEXT("dvbsiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{442DB029-02CB-4495-8B92-1C13375BCE99}"), TEXT("IDVB_EIT"), TEXT("dvbsiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{C64935F4-29E4-4E22-911A-63F7F55CB097}"), TEXT("IDVB_NIT"), TEXT("dvbsiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{02CAD8D3-FE43-48E2-90BD-450ED9A8A5FD}"), TEXT("IDVB_SDT"), TEXT("dvbsiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{83295D6A-FABA-4EE1-9B15-8067696910AE}"), TEXT("IDVB_TOT"), TEXT("dvbsiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{6E42F36E-1DD2-43C4-9F78-69D25AE39034}"), TEXT("IDVBCLocator"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{CF1EDAFF-3FFD-4CF7-8201-35756ACBF85F}"), TEXT("IDvbLogicalChannelDescriptor"), TEXT("dvbsiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{02F2225A-805B-4EC5-A9A6-F9B5913CD470}"), TEXT("IDvbSatelliteDeliverySystemDescriptor"), TEXT("dvbsiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{3D7C353C-0D04-45F1-A742-F97CC1188DC8}"), TEXT("IDVBSLocator"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{CDF7BE60-D954-42FD-A972-78971958E470}"), TEXT("IDVBSTuningSpace"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{ED7E1B91-D12E-420C-B41D-A49D84FE1823}"), TEXT("IDvbTerrestrialDeliverySystemDescriptor"), TEXT("dvbsiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{8664DA16-DDA2-42AC-926A-C18F9127C302}"), TEXT("IDVBTLocator"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{0D6F567E-A636-42BB-83BA-CE4C1704AFA2}"), TEXT("IDVBTuneRequest"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{ADA0B268-3B19-4E5B-ACC4-49F852BE13BA}"), TEXT("IDVBTuningSpace"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{843188B4-CE62-43DB-966B-8145A094E040}"), TEXT("IDVBTuningSpace2"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{5A4A97E4-94EE-4A55-9751-74B5643AA27D}"), TEXT("IDvdCmd"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389891.aspx")),
    CInterfaceInfo(TEXT("{33BC7430-EEC0-11D2-8201-00A0C9D74842}"), TEXT("IDvdControl2"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389895.aspx")),
    CInterfaceInfo(TEXT("{FCC152B6-F372-11D0-8E00-00C04FD7C08B}"), TEXT("IDvdGraphBuilder"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376427.aspx")),
    CInterfaceInfo(TEXT("{34151510-EEC0-11D2-8201-00A0C9D74842}"), TEXT("IDvdInfo2"), TEXT("strmif.h"), TEXT("")),
    CInterfaceInfo(TEXT("{86303D6D-1C4A-4087-AB42-F711167048EF}"), TEXT("IDvdState"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376496.aspx")),
    CInterfaceInfo(TEXT("{D18E17A0-AACB-11D0-AFB0-00AA00B67A42}"), TEXT("IDVEnc"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376499.aspx")),
    CInterfaceInfo(TEXT("{58473A19-2BC8-4663-8012-25F81BABDDD1}"), TEXT("IDVRGB219"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376502.aspx")),
    CInterfaceInfo(TEXT("{92A3A302-DA7C-4A1F-BA7E-1802BB5D2D02}"), TEXT("IDVSplitter"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376504.aspx")),
    CInterfaceInfo(TEXT("{2A6E2939-2595-11D3-B64C-00C04F79498E}"), TEXT("IEnumComponents"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{8A674B4A-1F63-11D3-B64C-00C04F79498E}"), TEXT("IEnumComponentTypes"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{2C3CD98A-2BFA-4A53-9C27-5249BA64BA0F}"), TEXT("IEnumDMO"), TEXT("mediaobj.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376585.aspx")),
    CInterfaceInfo(TEXT("{56A86893-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IEnumFilters"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376590.aspx")),
    CInterfaceInfo(TEXT("{89C31040-846B-11CE-97D3-00AA0055595A}"), TEXT("IEnumMediaTypes"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376600.aspx")),
    CInterfaceInfo(TEXT("{56A86892-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IEnumPins"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376610.aspx")),
    CInterfaceInfo(TEXT("{C18A9162-1E82-4142-8C73-5690FA62FE33}"), TEXT("IEnumStreamBufferRecordingAttrib"), TEXT("sbe.h"), TEXT("")),
    CInterfaceInfo(TEXT("{8B8EB248-FC2B-11D2-9D8C-00C04F72D980}"), TEXT("IEnumTuningSpaces"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{3127CA40-446E-11CE-8135-00AA004BB851}"), TEXT("IErrorLog"), TEXT("oaidl.h"), TEXT("")),
    CInterfaceInfo(TEXT("{C4C4C4D1-0049-4E2B-98FB-9537F6CE516D}"), TEXT("IETFilterConfig"), TEXT("encdec.h"), TEXT("")),
    CInterfaceInfo(TEXT("{C5C5C5B1-3ABC-11D6-B25B-00C04FA0C026}"), TEXT("IEvalRat"), TEXT("tvratings.h"), TEXT("")),
    CInterfaceInfo(TEXT("{A2104830-7C70-11CF-8BCE-00AA00A3F1A6}"), TEXT("IFileSinkFilter"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389975.aspx"), GetInterfaceInfo_IFileSinkFilter),
    CInterfaceInfo(TEXT("{00855B90-CE1B-11D0-BD4F-00A0C911CE86}"), TEXT("IFileSinkFilter2"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389976.aspx"), GetInterfaceInfo_IFileSinkFilter2),
    CInterfaceInfo(TEXT("{56A868A6-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IFileSourceFilter"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389981.aspx"), GetInterfaceInfo_IFileSourceFilter),
    CInterfaceInfo(TEXT("{DCFBDCF6-0DC2-45F5-9AB2-7C330EA09C29}"), TEXT("IFilterChain"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389984.aspx")),
    CInterfaceInfo(TEXT("{56A8689F-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IFilterGraph"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd389989.aspx")),
    CInterfaceInfo(TEXT("{AAF38154-B80B-422F-91E6-B66467509A07}"), TEXT("IFilterGraph3"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390014.aspx")),
    CInterfaceInfo(TEXT("{B79BB0B0-33C1-11D1-ABE1-00A0C905F375}"), TEXT("IFilterMapper2"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390033.aspx")),
    CInterfaceInfo(TEXT("{B79BB0B1-33C1-11D1-ABE1-00A0C905F375}"), TEXT("IFilterMapper3"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390038.aspx")),
    CInterfaceInfo(TEXT("{06FB45C1-693C-4EA7-B79F-7A6A54D8DEF2}"), TEXT("IFrequencyMap"), TEXT("bdaiface.h"), TEXT("")),
    CInterfaceInfo(TEXT("{6A5918F8-A77A-4F61-AED0-5702BDCDA3E6}"), TEXT("IGenericDescriptor"), TEXT("mpeg2psiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{56A868A9-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IGraphBuilder"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390085.aspx")),
    CInterfaceInfo(TEXT("{03A1EB8E-32BF-4245-8502-114D08A9CB88}"), TEXT("IGraphConfig"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390093.aspx")),
    CInterfaceInfo(TEXT("{ADE0FD60-D19D-11D2-ABF6-00A0C905F375}"), TEXT("IGraphConfigCallback"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390094.aspx")),
    CInterfaceInfo(TEXT("{56A868AB-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IGraphVersion"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390106.aspx")),
    CInterfaceInfo(TEXT("{B8E8BD60-0BFE-11D0-AF91-00AA00B67A42}"), TEXT("IIPDVDec"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390136.aspx")),
    CInterfaceInfo(TEXT("{B61178D1-A2D9-11CF-9E53-00AA00A216A1}"), TEXT("IKsPin"), TEXT("KsProxy.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390142.aspx")),
    CInterfaceInfo(TEXT("{31EFAC30-515C-11D0-A9AA-00AA0061BE93}"), TEXT("IKsPropertySet"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390144.aspx")),
    CInterfaceInfo(TEXT("{720D4AC0-7533-11D0-A5D6-28DB04C10000}"), TEXT("IKsTopologyInfo"), TEXT("ks.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390148.aspx")),
    CInterfaceInfo(TEXT("{B874C8BA-0FA2-11D3-9D8E-00C04F72D980}"), TEXT("ILanguageComponentType"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{286D7F89-760C-4F89-80C4-66841D2507AA}"), TEXT("ILocator"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{59EFF8B9-938C-4A26-82F2-95CB84CDC837}"), TEXT("IMediaBuffer"), TEXT("mediaobj.h"), TEXT("")),
    CInterfaceInfo(TEXT("{56A868B1-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IMediaControl"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390170.aspx")),
    CInterfaceInfo(TEXT("{65BD0710-24D2-4FF7-9324-ED2E5D3ABAFA}"), TEXT("IMediaDet"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390180.aspx")),
    CInterfaceInfo(TEXT("{56A868B6-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IMediaEvent"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406896.aspx")),
    CInterfaceInfo(TEXT("{56A868C0-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IMediaEventEx"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406897.aspx")),
    CInterfaceInfo(TEXT("{56A868A2-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IMediaEventSink"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406901.aspx")),
    CInterfaceInfo(TEXT("{56A86899-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IMediaFilter"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406916.aspx"), GetInterfaceInfo_IMediaFilter),
    CInterfaceInfo(TEXT("{288581E0-66CE-11D2-918F-00C0DF10D434}"), TEXT("IMediaLocator"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406923.aspx")),
    CInterfaceInfo(TEXT("{D8AD0F58-5494-4102-97C5-EC798E59BCF4}"), TEXT("IMediaObject"), TEXT("mediaobj.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406926.aspx")),
    CInterfaceInfo(TEXT("{651B9AD0-0FC7-4AA9-9538-D89931010741}"), TEXT("IMediaObjectInPlace"), TEXT("mediaobj.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406939.aspx")),
    CInterfaceInfo(TEXT("{6D6CBB60-A223-44AA-842F-A2F06750BE6D}"), TEXT("IMediaParamInfo"), TEXT("medparam.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406964.aspx"), GetInterfaceInfo_IMediaParamInfo),
    CInterfaceInfo(TEXT("{6D6CBB61-A223-44AA-842F-A2F06750BE6E}"), TEXT("IMediaParams"), TEXT("medparam.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406971.aspx"), GetInterfaceInfo_IMediaParams),
    CInterfaceInfo(TEXT("{56A868B2-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IMediaPosition"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406977.aspx")),
    CInterfaceInfo(TEXT("{6025A880-C0D5-11D0-BD4E-00A0C911CE86}"), TEXT("IMediaPropertyBag"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd406997.aspx")),
    CInterfaceInfo(TEXT("{56A8689A-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IMediaSample"), TEXT("strmif.h"), TEXT("")),
    CInterfaceInfo(TEXT("{36B73880-C2C8-11CF-8B46-00805F6CEF60}"), TEXT("IMediaSeeking"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407023.aspx"), GetInterfaceInfo_IMediaSeeking),
    CInterfaceInfo(TEXT("{B502D1BD-9A57-11D0-8FDE-00C04FD9189D}"), TEXT("IMediaStream"), TEXT("mmstream.h"), TEXT("")),
    CInterfaceInfo(TEXT("{BEBE595E-9A6F-11D0-8FDE-00C04FD9189D}"), TEXT("IMediaStreamFilter"), TEXT("amstream.h"), TEXT("")),
    CInterfaceInfo(TEXT("{56A8689C-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IMemAllocator"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407061.aspx")),
    CInterfaceInfo(TEXT("{379A0CF0-C1DE-11D2-ABF5-00A0C905F375}"), TEXT("IMemAllocatorCallbackTemp"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407062.aspx")),
    CInterfaceInfo(TEXT("{92980B30-C1DE-11D2-ABF5-00A0C905F375}"), TEXT("IMemAllocatorNotifyCallbackTemp"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407065.aspx")),
    CInterfaceInfo(TEXT("{56A8689D-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IMemInputPin"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407073.aspx")),
    CInterfaceInfo(TEXT("{81A3BD32-DEE1-11D1-8508-00A0C91F9CA0}"), TEXT("IMixerOCX"), TEXT("mixerocx.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407084.aspx")),
    CInterfaceInfo(TEXT("{81A3BD31-DEE1-11D1-8508-00A0C91F9CA0}"), TEXT("IMixerOCXNotify"), TEXT("mixerocx.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407085.aspx")),
    CInterfaceInfo(TEXT("{593CDDE1-0759-11D1-9E69-00C04FD7C15B}"), TEXT("IMixerPinConfig"), TEXT("uuids.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407097.aspx")),
    CInterfaceInfo(TEXT("{EBF47182-8764-11D1-9E69-00C04FD7C15B}"), TEXT("IMixerPinConfig2"), TEXT("uuids.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407098.aspx")),
    CInterfaceInfo(TEXT("{1493E353-1EB6-473C-802D-8E6B8EC9D2A9}"), TEXT("IMPEG2Component"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{2C073D84-B51C-48C9-AA9F-68971E1F6E38}"), TEXT("IMPEG2ComponentType"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{9B396D40-F380-4E3C-A514-1A82BF6EBFE6}"), TEXT("IMpeg2Data"), TEXT("mpeg2data.h"), TEXT("")),
    CInterfaceInfo(TEXT("{436EEE9C-264F-4242-90E1-4E330C107512}"), TEXT("IMpeg2Demultiplexer"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407128.aspx")),
    CInterfaceInfo(TEXT("{afb6c2a1-2c41-11d3-8a60-0000f81e0e4a}"), TEXT("IMPEG2PIDMap"), TEXT("bdaiface.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd407132.aspx")),
    CInterfaceInfo(TEXT("{400CC286-32A0-4CE4-9041-39571125A635}"), TEXT("IMpeg2Stream"), TEXT("mpeg2data.h"), TEXT("")),
    CInterfaceInfo(TEXT("{D0E04C47-25B8-4369-925A-362A01D95444}"), TEXT("IMPEG2StreamIdMap"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376629.aspx")),
    CInterfaceInfo(TEXT("{EB7D987F-8A01-42AD-B8AE-574DEEE44D1A}"), TEXT("IMPEG2TuneRequest"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{14E11ABD-EE37-4893-9EA1-6964DE933E39}"), TEXT("IMPEG2TuneRequestFactory"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{1B9D5FC3-5BBC-4B6C-BB18-B9D10E3EEEBF}"), TEXT("IMPEG2TuneRequestSupport"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{B45DD570-3C77-11D1-ABE1-00A0C905F375}"), TEXT("IMpegAudioDecoder"), TEXT("uuids.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376656.aspx")),
    CInterfaceInfo(TEXT("{B502D1BC-9A57-11D0-8FDE-00C04FD9189D}"), TEXT("IMultiMediaStream"), TEXT("mmstream.h"), TEXT("")),
    CInterfaceInfo(TEXT("{FC4801A3-2BA9-11CF-A229-00AA003D7352}"), TEXT("IObjectWithSite"), TEXT("ocidl.h"), TEXT("")),
    CInterfaceInfo(TEXT("{56a868a1-0ad4-11ce-b03a-0020af0ba770}"), TEXT("IOverlay"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390357.aspx")),
    CInterfaceInfo(TEXT("{6623B511-4B5F-43C3-9A01-E8FF84188060}"), TEXT("IPAT"), TEXT("mpeg2psiparser.h"), TEXT("")),
    CInterfaceInfo(TEXT("{0000010C-0000-0000-C000-000000000046}"), TEXT("IPersist"), TEXT("objidl.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ms688695.aspx")),
    CInterfaceInfo(TEXT("{0000010b-0000-0000-C000-000000000046}"), TEXT("IPersistFile"), TEXT("objidl.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ms687223.aspx")),
    CInterfaceInfo(TEXT("{5738E040-B67F-11D0-BD4D-00A0C911CE86}"), TEXT("IPersistMediaPropertyBag"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390387.aspx")),
    CInterfaceInfo(TEXT("{0000010a-0000-0000-C000-000000000046}"), TEXT("IPersistStorage"), TEXT("objidl.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ms679731.aspx")),
    CInterfaceInfo(TEXT("{00000109-0000-0000-C000-000000000046}"), TEXT("IPersistStream"), TEXT("objidl.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ms690091.aspx")),
    CInterfaceInfo(TEXT("{7FD52380-4E07-101B-AE2D-08002B2EC713}"), TEXT("IPersistStreamInit"), TEXT("objidl.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ms682273.aspx")),
    CInterfaceInfo(TEXT("{56A86891-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IPin"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390397.aspx")),
    CInterfaceInfo(TEXT("{4A9A62D3-27D4-403D-91E9-89F540E55534}"), TEXT("IPinConnection"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390398.aspx")),
    CInterfaceInfo(TEXT("{C56E9858-DBF3-4F6B-8119-384AF2060DEB}"), TEXT("IPinFlowControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390403.aspx")),
    CInterfaceInfo(TEXT("{01F3B398-9527-4736-94DB-5195878E97A8}"), TEXT("IPMT"), TEXT("mpeg2psiparser.cs"), TEXT("")),
    CInterfaceInfo(TEXT("{55272A00-42CB-11CE-8135-00AA004BB851}"), TEXT("IPropertyBag"), TEXT("oaidl.h"), TEXT("")),
    CInterfaceInfo(TEXT("{AE9472BD-B0C3-11D2-8D24-00A0C9441E20}"), TEXT("IPropertySetter"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376901.aspx")),
    CInterfaceInfo(TEXT("{56A868A5-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IQualityControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376912.aspx")),
    CInterfaceInfo(TEXT("{1BD0ECB0-F8E2-11CE-AAC6-0020AF0B99A3}"), TEXT("IQualProp"), TEXT("uuids.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376915.aspx")),
    CInterfaceInfo(TEXT("{56A868B7-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IQueueCommand"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376922.aspx")),
    CInterfaceInfo(TEXT("{56A86897-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IReferenceClock"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376925.aspx")),
    CInterfaceInfo(TEXT("{EBEC459C-2ECA-4D42-A8AF-30DF557614B8}"), TEXT("IReferenceClockTimerControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376936.aspx")),
    CInterfaceInfo(TEXT("{7B3A2F01-0751-48DD-B556-004785171C54}"), TEXT("IRegisterServiceProvider"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376936.aspx")),
    CInterfaceInfo(TEXT("{6BEE3A81-66C9-11D2-918F-00C0DF10D434}"), TEXT("IRenderEngine"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376942.aspx")),
    CInterfaceInfo(TEXT("{6BEE3A82-66C9-11d2-918F-00C0DF10D434}"), TEXT("IRenderEngine2"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376943.aspx")),
    CInterfaceInfo(TEXT("{56a868ad-0ad4-11ce-b03a-0020af0ba770}"), TEXT("IResourceConsumer"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376972.aspx")),
    CInterfaceInfo(TEXT("{6B652FFF-11FE-4FCE-92AD-0266B5D7C78F}"), TEXT("ISampleGrabber"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376984.aspx")),
    CInterfaceInfo(TEXT("{0579154A-2B53-4994-B0D0-E773148EFF85}"), TEXT("ISampleGrabberCB"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd376985.aspx")),
    CInterfaceInfo(TEXT("{547b6d26-3226-487e-8253-8aa168749434}"), TEXT("ISBE2Crossbar"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694845.aspx")),
    CInterfaceInfo(TEXT("{AFEC1EB5-2A64-46C6-BF4B-AE3CCB6AFDB0}"), TEXT("ISectionList"), TEXT("mpeg2data.h"), TEXT("")),
    CInterfaceInfo(TEXT("{36b73883-c2c8-11cf-8b46-00805f6cef60}"), TEXT("ISeekingPassThru"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377073.aspx")),
    CInterfaceInfo(TEXT("{1ABDAECA-68B6-4F83-9371-B413907C7B9F}"), TEXT("ISelector"), TEXT("Vidcap.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377075.aspx")),
    CInterfaceInfo(TEXT("{6D5140C1-7436-11CE-8034-00AA006009FA}"), TEXT("IServiceProvider"), TEXT("servprov.h"), TEXT("")),
    CInterfaceInfo(TEXT("{F03FA8CE-879A-4D59-9B2C-26BB1CF83461}"), TEXT("ISmartRenderEngine"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377080.aspx")),
    CInterfaceInfo(TEXT("{B196B28B-BAB4-101A-B69C-00AA00341D07}"), TEXT("ISpecifyPropertyPages"), TEXT("ocidl.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ms695217.aspx")),
    CInterfaceInfo(TEXT("{CE14DFAE-4098-4AF7-BBF7-D6511F835414}"), TEXT("IStreamBufferConfigure"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694927.aspx")),
    CInterfaceInfo(TEXT("{53E037BF-3992-4282-AE34-2487B4DAE06B}"), TEXT("IStreamBufferConfigure2"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694928.aspx")),
    CInterfaceInfo(TEXT("{7E2D2A1E-7192-4BD7-80C1-061FD1D10402}"), TEXT("IStreamBufferConfigure3"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694933.aspx")),
    CInterfaceInfo(TEXT("{9D2A2563-31AB-402E-9A6B-ADB903489440}"), TEXT("IStreamBufferDataCounters"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694944.aspx")),
    CInterfaceInfo(TEXT("{9CE50F2D-6BA7-40FB-A034-50B1A674EC78}"), TEXT("IStreamBufferInitialize"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694947.aspx")),
    CInterfaceInfo(TEXT("{F61F5C26-863D-4AFA-B0BA-2F81DC978596}"), TEXT("IStreamBufferMediaSeeking"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694950.aspx")),
    CInterfaceInfo(TEXT("{3A439AB0-155F-470A-86A6-9EA54AFD6EAF}"), TEXT("IStreamBufferMediaSeeking2"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694951.aspx")),
    CInterfaceInfo(TEXT("{9E259A9B-8815-42AE-B09F-221970B154FD}"), TEXT("IStreamBufferRecComp"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694953.aspx")),
    CInterfaceInfo(TEXT("{BA9B6C99-F3C7-4FF2-92DB-CFDD4851BF31}"), TEXT("IStreamBufferRecordControl"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694960.aspx")),
    CInterfaceInfo(TEXT("{16CA4E03-FE69-4705-BD41-5B7DFC0C95F3}"), TEXT("IStreamBufferRecordingAttribute"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694964.aspx")),
    CInterfaceInfo(TEXT("{AFD1F242-7EFD-45EE-BA4E-407A25C9A77A}"), TEXT("IStreamBufferSink"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694970.aspx")),
    CInterfaceInfo(TEXT("{DB94A660-F4FB-4BFA-BCC6-FE159A4EEA93}"), TEXT("IStreamBufferSink2"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694971.aspx")),
    CInterfaceInfo(TEXT("{974723F2-887A-4452-9366-2CFF3057BC8F}"), TEXT("IStreamBufferSink3"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694973.aspx")),
    CInterfaceInfo(TEXT("{1C5BD776-6CED-4F44-8164-5EAB0E98DB12}"), TEXT("IStreamBufferSource"), TEXT("sbe.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694978.aspx")),
    CInterfaceInfo(TEXT("{B502D1BE-9A57-11D0-8FDE-00C04FD9189D}"), TEXT("IStreamSample"), TEXT("mmstream.h"), TEXT("")),
    CInterfaceInfo(TEXT("{28C52640-018A-11D3-9D8E-00C04F72D980}"), TEXT("ITuner"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{E60DFA45-8D56-4E65-A8AB-D6BE9412C249}"), TEXT("ITunerCap"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{07DDC146-FC3D-11D2-9D8C-00C04F72D980}"), TEXT("ITuneRequest"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{061C6E30-E622-11D2-9493-00C04F72D980}"), TEXT("ITuningSpace"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{5B692E84-E2F1-11D2-9493-00C04F72D980}"), TEXT("ITuningSpaceContainer"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{901284E4-33FE-4B69-8D63-634A596F3756}"), TEXT("ITuningSpaces"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{00000000-0000-0000-C000-000000000046}"), TEXT("IUnknown"), TEXT("Unknwn.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ms680509.aspx"), GetInterfaceInfo_IUnknown),
    CInterfaceInfo(TEXT("{E46A9787-2B71-444D-A4B5-1FAB7B708D6A}"), TEXT("IVideoFrameStep"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377232.aspx")),
    CInterfaceInfo(TEXT("{56A868B4-0AD4-11CE-B03A-0020AF0BA770}"), TEXT("IVideoWindow"), TEXT("control.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377276.aspx")),
    CInterfaceInfo(TEXT("{EDE80B5C-BAD6-4623-B537-65586C9F8DFD}"), TEXT("IVMRAspectRatioControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377342.aspx")),
    CInterfaceInfo(TEXT("{00D96C29-BBDE-4EFC-9901-BB5036392146}"), TEXT("IVMRAspectRatioControl9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377343.aspx")),
    CInterfaceInfo(TEXT("{BB057577-0DB8-4E6A-87A7-1A8C9A505A0F}"), TEXT("IVMRDeinterlaceControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377348.aspx")),
    CInterfaceInfo(TEXT("{A215FB8D-13C2-4F7F-993C-003D6271A459}"), TEXT("IVMRDeinterlaceControl9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377349.aspx")),
    CInterfaceInfo(TEXT("{9E5530C5-7034-48B4-BB46-0B8A6EFC8E36}"), TEXT("IVMRFilterConfig"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377364.aspx")),
    CInterfaceInfo(TEXT("{5A804648-4F66-4867-9C43-4F5C822CF1B8}"), TEXT("IVMRFilterConfig9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377365.aspx")),
    CInterfaceInfo(TEXT("{7A4FB5AF-479F-4074-BB40-CE6722E43C82}"), TEXT("IVMRImageCompositor"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377380.aspx")),
    CInterfaceInfo(TEXT("{4A5C89EB-DF51-4654-AC2A-E48E02BBABF6}"), TEXT("IVMRImageCompositor9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377381.aspx")),
    CInterfaceInfo(TEXT("{CE704FE7-E71E-41fb-BAA2-C4403E1182F5}"), TEXT("IVMRImagePresenter"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377390.aspx")),
    CInterfaceInfo(TEXT("{69188C61-12A3-40F0-8FFC-342E7B433FD7}"), TEXT("IVMRImagePresenter9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377391.aspx")),
    CInterfaceInfo(TEXT("{9F3A1C85-8555-49BA-935F-BE5B5B29D178}"), TEXT("IVMRImagePresenterConfig"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377395.aspx")),
    CInterfaceInfo(TEXT("{45C15CAB-6E22-420A-8043-AE1F0AC02C7D}"), TEXT("IVMRImagePresenterConfig9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd377396.aspx")),
    CInterfaceInfo(TEXT("{e6f7ce40-4673-44f1-8f77-5499d68cb4ea}"), TEXT("IVMRImagePresenterExclModeConfig"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390441.aspx")),
    CInterfaceInfo(TEXT("{1E673275-0257-40AA-AF20-7C608D4A0428}"), TEXT("IVMRMixerBitmap"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390448.aspx")),
    CInterfaceInfo(TEXT("{CED175E5-1935-4820-81BD-FF6AD00C9108}"), TEXT("IVMRMixerBitmap9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390449.aspx")),
    CInterfaceInfo(TEXT("{1C1A17B0-BED0-415D-974B-DC6696131599}"), TEXT("IVMRMixerControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390456.aspx")),
    CInterfaceInfo(TEXT("{1A777EAA-47C8-4930-B2C9-8FEE1C1B0F3B}"), TEXT("IVMRMixerControl9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390457.aspx")),
    CInterfaceInfo(TEXT("{9CF0B1B6-FBAA-4B7F-88CF-CF1F130A0DCE}"), TEXT("IVMRMonitorConfig"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390481.aspx")),
    CInterfaceInfo(TEXT("{46C2E457-8BA0-4EEF-B80B-0680F0978749}"), TEXT("IVMRMonitorConfig9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390457.aspx")),
    CInterfaceInfo(TEXT("{31ce832e-4484-458b-8cca-f4d7e3db0b52}"), TEXT("IVMRSurfaceAllocator"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390499.aspx")),
    CInterfaceInfo(TEXT("{8D5148EA-3F5D-46CF-9DF1-D1B896EEDB1F}"), TEXT("IVMRSurfaceAllocator9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390500.aspx")),
    CInterfaceInfo(TEXT("{6DE9A68A-A928-4522-BF57-655AE3866456}"), TEXT("IVMRSurfaceAllocatorEx9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390505.aspx")),
    CInterfaceInfo(TEXT("{DCA3F5DF-BB3A-4D03-BD81-84614BFBFA0C}"), TEXT("IVMRSurfaceAllocatorNotify9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390508.aspx")),
    CInterfaceInfo(TEXT("{058D1F11-2A54-4BEF-BD54-DF706626B727}"), TEXT("IVMRVideoStreamControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390528.aspx")),
    CInterfaceInfo(TEXT("{D0CFE38B-93E7-4772-8957-0400C49A4485}"), TEXT("IVMRVideoStreamControl9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390529.aspx")),
    CInterfaceInfo(TEXT("{0EB1088C-4DCD-46F0-878F-39DAE86A51B7}"), TEXT("IVMRWindowlessControl"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390536.aspx")),
    CInterfaceInfo(TEXT("{8F537D09-F85E-4414-B23B-502E54C79927}"), TEXT("IVMRWindowlessControl9"), TEXT("vmr9.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390537.aspx")),
    CInterfaceInfo(TEXT("{bc29a660-30e3-11d0-9e69-00c04fd7c15b}"), TEXT("IVPConfig"), TEXT("Vpconfig.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390583.aspx")),
    CInterfaceInfo(TEXT("{AAC18C18-E186-46D2-825D-A1F8DC8E395A}"), TEXT("IVPManager"), TEXT("strmif.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390586.aspx")),
    CInterfaceInfo(TEXT("{C76794A1-D6C5-11D0-9E69-00C04FD7C15B}"), TEXT("IVPNotify"), TEXT("uuids.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390589.aspx")),
    CInterfaceInfo(TEXT("{EBF47183-8764-11D1-9E69-00C04FD7C15B}"), TEXT("IVPNotify2"), TEXT("uuids.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390590.aspx")),
    CInterfaceInfo(TEXT("{C4C4C4D3-0049-4E2B-98FB-9537F6CE516D}"), TEXT("IXDSCodecConfig"), TEXT("encdec.h"), TEXT("")),
    CInterfaceInfo(TEXT("{18C628ED-962A-11D2-8D08-00A0C9441E20}"), TEXT("IXml2Dex"), TEXT("qedit.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd390609.aspx")),

    CInterfaceInfo(TEXT("{DF9C0DC3-1924-4bfe-8DC1-1084453A0F8F}"), TEXT("IFLACEncodeSettings"), TEXT("FlacTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/codecs/flac/filters/dsfFLACEncoder/IFLACEncodeSettings.h")),
    CInterfaceInfo(TEXT("{43F0F818-10B0-4c86-B9F1-F6B6E2D33462}"), TEXT("IOggDecoder"), TEXT("OggTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/core/directshow/dsfOggDemux2/IOggDecoder.h")),
    CInterfaceInfo(TEXT("{83D7F506-53ED-4f15-B6D8-7D8E9E72A918}"), TEXT("IOggOutputPin"), TEXT("OggTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/core/directshow/dsfOggDemux2/IOggOutputPin.h")),
    CInterfaceInfo(TEXT("{90D6513C-A665-4b16-ACA7-B3D1D4EFE58D}"), TEXT("IOggMuxProgress"), TEXT("OggTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/core/directshow/dsfOggMux/IOggMuxProgress.h")),
    CInterfaceInfo(TEXT("{30EB3AD8-B2DD-4f9a-9C25-845999B03476}"), TEXT("IOggSeekTable"), TEXT("OggTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/core/directshow/dsfOggDemux2/IOggSeekTable.h")),
    CInterfaceInfo(TEXT("{3a2cf997-0aeb-4d3f-9846-b5db2ca4c80b}"), TEXT("IOggMuxSettings"), TEXT("OggTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/core/directshow/dsfOggMux/IOggMuxSettings.h")),
    CInterfaceInfo(TEXT("{EB5AED9C-8CD0-4c4b-B5E8-F5D10AD1314D}"), TEXT("IOggBaseTime"), TEXT("OggTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/core/directshow/dsfOggDemux2/IOggBaseTime.h")),
    CInterfaceInfo(TEXT("{479038D2-57FF-41ee-B397-FB98199BF1E8}"), TEXT("ISpeexEncodeSettings"), TEXT("SpeexTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/codecs/speex/filters/dsfSpeexEncoder/ISpeexEncodeSettings.h")),
    CInterfaceInfo(TEXT("{4F063B3A-B397-4c22-AFF4-2F8DB96D292A}"), TEXT("ITheoraEncodeSettings"), TEXT("TheoraTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/codecs/theora/filters/dsfTheoraEncoder/ITheoraEncodeSettings.h")),
    CInterfaceInfo(TEXT("{A4C6A887-7BD3-4b33-9A57-A3EB10924D3A}"), TEXT("IVorbisEncodeSettings"), TEXT("VorbisTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/codecs/vorbis/filters/dsfVorbisEncoder/IVorbisEncodeSettings.h")),
    CInterfaceInfo(TEXT("{BFF86BE7-9E32-40EF-B200-7BCC7800CC72}"), TEXT("IDownmixAudio"), TEXT("VorbisTypes.h"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/codecs/vorbis/filters/dsfVorbisDecoder/IDownmixAudio.h")),
    CInterfaceInfo(TEXT("{ED3110F2-5211-11DF-94AF-0026B977EEAA}"), TEXT("IVP8PostProcessing"), TEXT("vp8decoder.idl"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/codecs/webm/webmdshow/IDL/vp8decoder.idl")),
    CInterfaceInfo(TEXT("{ED3110FE-5211-11DF-94AF-0026B977EEAA}"), TEXT("IVP8Encoder"), TEXT("vp8encoder.idl"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/codecs/webm/webmdshow/IDL/vp8encoder.idl")),
    CInterfaceInfo(TEXT("{ED311101-5211-11DF-94AF-0026B977EEAA}"), TEXT("IWebmMux"), TEXT("webmmux.idl"), TEXT("http://svn.xiph.org/trunk/oggdsf/src/lib/codecs/webm/webmdshow/IDL/webmmux.idl")),

    CInterfaceInfo(TEXT("{FA40D6E9-4D38-4761-ADD2-71A9EC5FD32F}"), TEXT("ILAVVideoSettings"), TEXT("LAVVideoSettings.h"), TEXT("http://code.google.com/p/lavfilters/source/browse/developer_info/LAVVideoSettings.h")),
    CInterfaceInfo(TEXT("{774A919D-EA95-4A87-8A1E-F48ABE8499C7}"), TEXT("ILAVFSettings"), TEXT("LAVSplitterSettings.h"), TEXT("http://code.google.com/p/lavfilters/source/browse/developer_info/LAVSplitterSettings.h")),
    CInterfaceInfo(TEXT("{4158A22B-6553-45D0-8069-24716F8FF171}"), TEXT("ILAVAudioSettings"), TEXT("LAVAudioSettings.h"), TEXT("http://code.google.com/p/lavfilters/source/browse/developer_info/LAVAudioSettings.h")),
    CInterfaceInfo(TEXT("{A668B8F2-BA87-4F63-9D41-768F7DE9C50E}"), TEXT("ILAVAudioStatus"), TEXT("LAVAudioSettings.h"), TEXT("http://code.google.com/p/lavfilters/source/browse/developer_info/LAVAudioSettings.h")),
    CInterfaceInfo(TEXT("{01A5BBD3-FE71-487C-A2EC-F585918A8724}"), TEXT("IKeyFrameInfo"), TEXT("IKeyFrameInfo.h"), TEXT("http://code.google.com/p/lavfilters/source/browse/developer_info/IKeyFrameInfo.h")),
    CInterfaceInfo(TEXT("{03E98D51-DDE7-43aa-B70C-42EF84A3A23D}"), TEXT("ITrackInfo"), TEXT("ITrackInfo.h"), TEXT("http://code.google.com/p/lavfilters/source/browse/developer_info/ITrackInfo.h")),

    CInterfaceInfo(TEXT("{352bb3bd-2d4d-4323-9e71-dcdcfbd53ca6}"), TEXT("IWMVideoDecoderHurryup"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819262.aspx")),
    CInterfaceInfo(TEXT("{9F8496BE-5B9A-41b9-A9E8-F21CD80596C2}"), TEXT("IWMVideoForceKeyFrame"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819268.aspx")),
    CInterfaceInfo(TEXT("{A7B2504B-E58A-47fb-958B-CAC7165A057D}"), TEXT("IWMCodecStrings"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819246.aspx")),
    CInterfaceInfo(TEXT("{2573e11a-f01a-4fdd-a98d-63b8e0ba9589}"), TEXT("IWMCodecProps"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819243.aspx")),
    CInterfaceInfo(TEXT("{A81BA647-6227-43b7-B231-C7B15135DD7D}"), TEXT("IWMCodecLeakyBucket"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819233.aspx")),
    CInterfaceInfo(TEXT("{B72ADF95-7ADC-4a72-BC05-577D8EA6BF68}"), TEXT("IWMCodecOutputTimestamp"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819237.aspx")),
    CInterfaceInfo(TEXT("{45BDA2AC-88E2-4923-98BA-3949080711A3}"), TEXT("IWMVideoDecoderReconBuffer"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819266.aspx")),
    CInterfaceInfo(TEXT("{73F0BE8E-57F7-4f01-AA66-9F57340CFE0E}"), TEXT("IWMCodecPrivateData"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819239.aspx")),
    CInterfaceInfo(TEXT("{9bca9884-0604-4c2a-87da-793ff4d586c3}"), TEXT("IWMSampleExtensionSupport"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819259.aspx")),
    CInterfaceInfo(TEXT("{E7E9984F-F09F-4da4-903F-6E2E0EFE56B5}"), TEXT("IWMResamplerProps"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819250.aspx")),
    CInterfaceInfo(TEXT("{57665D4C-0414-4faa-905B-10E546F81C33}"), TEXT("IWMResizerProps"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819254.aspx")),
    CInterfaceInfo(TEXT("{776C93B3-B72D-4508-B6D0-208785F553E7}"), TEXT("IWMColorLegalizerProps"), TEXT("wmcodecdsp.h"), TEXT("")),
    CInterfaceInfo(TEXT("{7B12E5D1-BD22-48ea-BC06-98E893221C89}"), TEXT("IWMInterlaceProps"), TEXT("wmcodecdsp.h"), TEXT("")),
    CInterfaceInfo(TEXT("{4C06BB9B-626C-4614-8329-CC6A21B93FA0}"), TEXT("IWMFrameInterpProps"), TEXT("wmcodecdsp.h"), TEXT("")),
    CInterfaceInfo(TEXT("{E6A49E22-C099-421d-AAD3-C061FB4AE85B}"), TEXT("IWMColorConvProps"), TEXT("wmcodecdsp.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/ff819247.aspx")),
    CInterfaceInfo(TEXT("{886d8eeb-8cf2-4446-8d02-cdba1dbdcf99}"), TEXT("IPropertyStore"), TEXT("Propsys.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/bb761474.aspx")),
    CInterfaceInfo(TEXT("{71604b0f-97b0-4764-8577-2f13e98a1422}"), TEXT("INamedPropertyStore"), TEXT("Propsys.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/bb761664.aspx")),
    CInterfaceInfo(TEXT("{c8e2d566-186e-4d49-bf41-6909ead56acc}"), TEXT("IPropertyStoreCapabilities"), TEXT("Propsys.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/bb761452.aspx")),

    CInterfaceInfo(TEXT("{1DFD0A5C-0284-11d3-9D8E-00C04F72D980}"), TEXT("IScanningTuner"), TEXT("tuner.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694868.aspx")),
    CInterfaceInfo(TEXT("{04BBD195-0E2D-4593-9BD5-4F908BC33CF5}"), TEXT("IScanningTunerEx"), TEXT("tuner.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694869.aspx")),
    CInterfaceInfo(TEXT("{ed3e0c66-18c8-4ea6-9300-f6841fdd35dc}"), TEXT("ITunerCapEx"), TEXT("tuner.h"), TEXT("")),
    CInterfaceInfo(TEXT("{919F24C5-7B14-42ac-A4B0-2AE08DAF00AC}"), TEXT("IPSITables"), TEXT("mpeg2psiparser.h"), TEXT("http://msdn.microsoft.com/en-us/library/windows/desktop/dd694840.aspx")),
};
const UINT CInterfaceScanner::m_countKnownInterfaces = sizeof(m_knownInterfaces) / sizeof(m_knownInterfaces[0]);


CInterfaceScanner::CInterfaceScanner(IUnknown* pUnk)
{
    m_pUnk = pUnk;
    m_pUnk->AddRef();

    for(int i=0; i<m_countKnownInterfaces; i++)
    {
        IUnknown* pI = NULL;
        if(S_OK == pUnk->QueryInterface(m_knownInterfaces[i].GetGuid(), (void**)&pI))
        {
            m_supportedInterfaces.Add(&m_knownInterfaces[i]);
            pI->Release();
        }
    }
}

CInterfaceScanner::~CInterfaceScanner(void)
{
    m_pUnk->Release();
    m_pUnk = NULL;
}

void CInterfaceScanner::GetDetails(GraphStudio::PropItem* info)
{
    for(int i=0;i<m_supportedInterfaces.GetCount(); i++)
    {
        GraphStudio::PropItem *group = info->AddItem(new GraphStudio::PropItem(m_supportedInterfaces[i]->GetName()));
        m_supportedInterfaces[i]->GetInfo(group, m_pUnk);
        group->expand = false;
    }
}

bool CInterfaceScanner::InsertInterfaceLookup(int i, CListCtrl* pListCtrl)
{
    if(i < 0 || i >= m_countKnownInterfaces) return false;

    int nIndex = pListCtrl->InsertItem(pListCtrl->GetItemCount(), m_knownInterfaces[i].GetName());
    if(nIndex == -1) return false;

    pListCtrl->SetItemText(nIndex, 1, m_knownInterfaces[i].GetGuidString());
    pListCtrl->SetItemText(nIndex, 2, m_knownInterfaces[i].GetHeader());
    pListCtrl->SetItemText(nIndex, 3, m_knownInterfaces[i].GetUrl());

    CStringArray* arrData = new CStringArray();
    arrData->Add(m_knownInterfaces[i].GetName());
    arrData->Add(m_knownInterfaces[i].GetGuidString());
    arrData->Add(m_knownInterfaces[i].GetHeader());
    arrData->Add(m_knownInterfaces[i].GetUrl());
    pListCtrl->SetItemData(nIndex, (DWORD_PTR)arrData);

    return true;
}

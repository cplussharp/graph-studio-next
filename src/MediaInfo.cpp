#include "stdafx.h"
using namespace MediaInfoDLL;

static bool searchRegistry = true;
static CArray<CMediaInfo*> storedInfos;

CMediaInfo* CMediaInfo::GetInfoForFile(LPCTSTR pszFile, bool useCache=true)
{
    if(!MediaInfoDLL_IsLoaded())
    {
        if(MediaInfoDLL_Load() != 1)
        {
            // search in registry for MediaInfo installation
            if(searchRegistry)
            {
                searchRegistry = false;
                TCHAR strPath[MAX_PATH];
                LSTATUS ret = SHRegGetPath(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\MediaInfo.exe"), _T(""), strPath, 0);
                if(ret != ERROR_SUCCESS)
                    ret = SHRegGetPath(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MediaInfo"), _T("DisplayIcon"), strPath, 0);

                if(ret != ERROR_SUCCESS)
                    return NULL;

                PathRemoveFileSpec(strPath);
                SetDllDirectory(strPath);

                 if(MediaInfoDLL_Load() != 1)
                    return NULL;
            }
            else
                return NULL;
        }
    }
    else if (useCache)
    {
        CString fileName = pszFile;
        fileName.MakeLower();

        // search for file in cache (backwards because newer entries are appended to the end)
        for (int i = (int) storedInfos.GetCount() - 1; i >= 0; i--)
            if (storedInfos[i]->m_file == fileName)
			    return storedInfos[i];
    }

    // MediaInfo geladen, jetzt dateipfad testen
    if(!PathFileExists(pszFile))
        return NULL;

    MediaInfo* mi = new MediaInfo();
    size_t ret = mi->Open(pszFile);
    if(ret == 0)
    {
        mi->Close();
        delete mi;
        return NULL;
    }

    CMediaInfo* mediaInfo = new CMediaInfo(mi, pszFile);
	if (useCache)
		storedInfos.Add(mediaInfo);
    return mediaInfo;
}

void CMediaInfo::FreeInfoCache()
{
    while(storedInfos.GetCount() > 0)
    {
        CMediaInfo* info = storedInfos[0];
        storedInfos.RemoveAt(0);
        delete info;
    }
}

CMediaInfo::CMediaInfo(MediaInfo* mi, LPCTSTR strFile)
{
    m_mi = mi;
    m_file = strFile;
    m_file.MakeLower();
}


CMediaInfo::~CMediaInfo(void)
{
    if(m_mi != NULL)
        delete m_mi;
}

void CMediaInfo::GetDetails(GraphStudio::PropItem* info) const
{
    size_t count = m_mi->Count_Get(Stream_General);
    for(size_t i=0; i<count;i++)
        CMediaInfoStreamGeneral(m_mi, i).GetDetails(info);

    count = m_mi->Count_Get(Stream_Video);
    for(size_t i=0; i<count;i++)
        CMediaInfoStreamVideo(m_mi, i).GetDetails(info);

    count = m_mi->Count_Get(Stream_Audio);
    for(size_t i=0; i<count;i++)
        CMediaInfoStreamAudio(m_mi, i).GetDetails(info);

    count = m_mi->Count_Get(Stream_Text);
    for(size_t i=0; i<count;i++)
        CMediaInfoStreamText(m_mi, i).GetDetails(info);

    count = m_mi->Count_Get(Stream_Image);
    for(size_t i=0; i<count;i++)
        CMediaInfoStreamImage(m_mi, i).GetDetails(info);

    count = m_mi->Count_Get(Stream_Chapters);
    for(size_t i=0; i<count;i++)
        CMediaInfoStreamChapters(m_mi, i).GetDetails(info);

    count = m_mi->Count_Get(Stream_Menu);
    for(size_t i=0; i<count;i++)
        CMediaInfoStreamMenu(m_mi, i).GetDetails(info);
}

CString CMediaInfo::GetText() const
{
    GraphStudio::PropItem info(_T("MediaInfo"));
    GetDetails(&info);

    CString str;
    for(int i=0;i<info.GetCount();i++)
    {
        GraphStudio::PropItem* group = info.GetItem(i);

        if(i>0) str.Append(_T("\r\n"));
        str.Append(_T("                       "));
        str.Append(group->name);
        str.Append(_T("\r\n"));

        for(int j=0;j<group->GetCount();j++)
        {
            GraphStudio::PropItem* item = group->GetItem(j);
            CString frmt;
			frmt.Format(_T("%20s = %s\r\n"), (LPCTSTR)item->name, (LPCTSTR)item->value);
            str.Append(frmt);
        }
    }

    return str;
}



CString CMediaInfoStream::GetValue(LPCTSTR param) const
{ 
    return CString(m_mi->Get(m_streamType, m_streamNr, param).c_str());
}

#define ADD_MEDIAINFO_DETAILS(param) \
do { \
    val = GetValue(param); \
    if(val.GetLength()>0) \
        group->AddItem(new GraphStudio::PropItem(CString(param),val)); \
} while(0)

#define ADD_MEDIAINFO_DETAILS2(param,name) \
do { \
    val = GetValue(param); \
    if(val.GetLength()>0) \
        group->AddItem(new GraphStudio::PropItem(CString(name),val)); \
} while(0)

void CMediaInfoStream::GetDetails(GraphStudio::PropItem* info) const
{
    GraphStudio::PropItem *group = info->AddItem(new GraphStudio::PropItem(GetStreamTypeText()));
    CString val;
    
    ADD_MEDIAINFO_DETAILS(TEXT("ID"));
    ADD_MEDIAINFO_DETAILS(TEXT("Title"));
    ADD_MEDIAINFO_DETAILS2(TEXT("Duration/String3"),TEXT("Duration"));
    ADD_MEDIAINFO_DETAILS(TEXT("Language"));
    ADD_MEDIAINFO_DETAILS2(TEXT("BitRate/String"),TEXT("BitRate"));
    ADD_MEDIAINFO_DETAILS(TEXT("BitRate_Mode"));
    ADD_MEDIAINFO_DETAILS(TEXT("BitDepth"));
    ADD_MEDIAINFO_DETAILS(TEXT("Format"));
    ADD_MEDIAINFO_DETAILS2(TEXT("Format_Commercial_IfAny"),TEXT("Format_Commercial"));
    ADD_MEDIAINFO_DETAILS(TEXT("Format_Profile"));
    ADD_MEDIAINFO_DETAILS(TEXT("Format_Compression"));
    ADD_MEDIAINFO_DETAILS(TEXT("Format_Settings"));
    ADD_MEDIAINFO_DETAILS(TEXT("CodecID"));
    ADD_MEDIAINFO_DETAILS2(TEXT("CodecID/Hint"), TEXT("CodecHint"));
    ADD_MEDIAINFO_DETAILS(TEXT("Encoded_Application"));
    ADD_MEDIAINFO_DETAILS(TEXT("Encoded_Library"));

    // more Details
    OnGetDetails(group);
}

CString CMediaInfoStream::GetText() const
{
    GraphStudio::PropItem* info = new GraphStudio::PropItem(_T("MediaInfo"));
    GetDetails(info);

    CString str;
    for(int i=0;i<info->GetCount();i++)
    {
        GraphStudio::PropItem* group = info->GetItem(i);

        str.Append(group->name);
        str.Append(_T("\n\r"));

        for(int j=0;j<group->GetCount();j++)
        {
            GraphStudio::PropItem* item = group->GetItem(j);
            CString frmt;
			frmt.Format(_T("%[20]s = %s\n\r"), (LPCTSTR)item->name, (LPCTSTR)item->value);
            str.Append(frmt);
        }
    }

    return str;
}

CString CMediaInfoStream::GetStreamTypeText() const
{
    switch(m_streamType)
    {
    case Stream_General:
        return _T("General");
    case Stream_Video:
        return _T("Video");
    case Stream_Audio:
        return _T("Audio");
    case Stream_Image:
        return _T("Image");
    case Stream_Text:
        return _T("Text");
    case Stream_Chapters:
        return _T("Chapters");
    case Stream_Menu:
        return _T("Menu");
    }

    return _T("");
}

void CMediaInfoStreamGeneral::OnGetDetails(GraphStudio::PropItem* group) const
{
    CString val;
    ADD_MEDIAINFO_DETAILS2(TEXT("CompleteName"),TEXT("File"));
    ADD_MEDIAINFO_DETAILS2(TEXT("FileSize/String1"),TEXT("FileSize"));
    ADD_MEDIAINFO_DETAILS2(TEXT("OverallBitRate/String"),TEXT("BitRate"));
    ADD_MEDIAINFO_DETAILS2(TEXT("OverallBitRate_Mode"),TEXT("BitRate_Mode"));
}

void CMediaInfoStreamVideo::OnGetDetails(GraphStudio::PropItem* group) const
{
    CString val;
    ADD_MEDIAINFO_DETAILS(TEXT("Width"));
    ADD_MEDIAINFO_DETAILS(TEXT("Height"));
    ADD_MEDIAINFO_DETAILS(TEXT("FrameRate"));
    ADD_MEDIAINFO_DETAILS2(TEXT("DisplayAspectRatio/String"),TEXT("DisplayAspectRatio"));
    ADD_MEDIAINFO_DETAILS(TEXT("ColorSpace"));
    ADD_MEDIAINFO_DETAILS(TEXT("ChromaSubsampling"));
    ADD_MEDIAINFO_DETAILS(TEXT("ScanType"));
    ADD_MEDIAINFO_DETAILS(TEXT("ScanOrder"));
}

void CMediaInfoStreamAudio::OnGetDetails(GraphStudio::PropItem*group) const
{
    CString val;
    ADD_MEDIAINFO_DETAILS(TEXT("Channels"));
    ADD_MEDIAINFO_DETAILS2(TEXT("SamplingRate/String"),TEXT("SamplingRate"));

}

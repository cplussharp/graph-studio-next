#pragma once

using namespace MediaInfoDLL;

class CMediaInfo
{
public:
    static CMediaInfo* GetInfoForFile(LPCTSTR pszFile);
    static void FreeInfoCache();

    ~CMediaInfo();
    void GetDetails(GraphStudio::PropItem* info) const;
    CString GetText() const;

protected:
    CMediaInfo(MediaInfo* mi, LPCTSTR strFile);

    MediaInfo* m_mi;
    CString m_file;
};


class CMediaInfoStream
{
public:
    CMediaInfoStream(MediaInfo* mi, stream_t streamType, size_t streamNr) : m_mi(mi), m_streamType(streamType), m_streamNr(streamNr) {}
    void GetDetails(GraphStudio::PropItem* info) const;
    CString GetText() const;

protected:
    CString GetValue(LPCTSTR param) const;
    virtual void OnGetDetails(GraphStudio::PropItem* info) const {}
    CString GetStreamTypeText() const;

    MediaInfo* m_mi;
    stream_t m_streamType;
    size_t m_streamNr;
};

class CMediaInfoStreamGeneral : public CMediaInfoStream
{
public:
    CMediaInfoStreamGeneral(MediaInfo* mi, size_t streamNr) : CMediaInfoStream(mi, Stream_General, streamNr) {}
protected:
    void OnGetDetails(GraphStudio::PropItem* info) const;
};

class CMediaInfoStreamVideo : public CMediaInfoStream
{
public:
    CMediaInfoStreamVideo(MediaInfo* mi, size_t streamNr) : CMediaInfoStream(mi, Stream_Video, streamNr) {}
protected:
    void OnGetDetails(GraphStudio::PropItem* info) const;
};

class CMediaInfoStreamAudio : public CMediaInfoStream
{
public:
    CMediaInfoStreamAudio(MediaInfo* mi, size_t streamNr) : CMediaInfoStream(mi, Stream_Audio, streamNr) {}
protected:
    void OnGetDetails(GraphStudio::PropItem* info) const;
};

class CMediaInfoStreamImage : public CMediaInfoStream
{
public:
    CMediaInfoStreamImage(MediaInfo* mi, size_t streamNr) : CMediaInfoStream(mi, Stream_Image, streamNr) {}
};

class CMediaInfoStreamText : public CMediaInfoStream
{
public:
    CMediaInfoStreamText(MediaInfo* mi, size_t streamNr) : CMediaInfoStream(mi, Stream_Text, streamNr) {}
};

class CMediaInfoStreamChapters : public CMediaInfoStream
{
public:
    CMediaInfoStreamChapters(MediaInfo* mi, size_t streamNr) : CMediaInfoStream(mi, Stream_Chapters, streamNr) {}
};

class CMediaInfoStreamMenu : public CMediaInfoStream
{
public:
    CMediaInfoStreamMenu(MediaInfo* mi, size_t streamNr) : CMediaInfoStream(mi, Stream_Menu, streamNr) {}
};

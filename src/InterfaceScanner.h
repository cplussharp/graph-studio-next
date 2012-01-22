#pragma once

typedef void (*ptGetInterfaceDetails)(GraphStudio::PropItem* group, IUnknown* pUnk);

class CInterfaceInfo
{
public:
    CInterfaceInfo();
    CInterfaceInfo(const CString& guid, const CString& name, const CString& header, const CString& url);
    CInterfaceInfo(const CString& guid, const CString& name, const CString& header, const CString& url, ptGetInterfaceDetails detailsFunc);
    CInterfaceInfo(const CInterfaceInfo& info);
    virtual ~CInterfaceInfo(void) {}

    const GUID& GetGuid() const { return m_guid; }
    const CString GetGuidString() const;
    const CString GetName() const { return m_name; }
    const CString GetHeader() const { return m_header; }
    const CString GetUrl() const { return m_url; }
    virtual void GetInfo(GraphStudio::PropItem* group, IUnknown* pUnk) const;

protected:
    GUID m_guid;
    CString m_name;
    CString m_header;
    CString m_url;
    ptGetInterfaceDetails m_getDetailsFunc;
};


class CInterfaceScanner
{
public:
    CInterfaceScanner(IUnknown* pUnk);
    ~CInterfaceScanner(void);

    void GetDetails(GraphStudio::PropItem* group);
    static bool InsertInterfaceLookup(int i, CListCtrl* pListCtrl);

protected:
    CArray<const CInterfaceInfo*, const CInterfaceInfo*> m_supportedInterfaces;
    IUnknown* m_pUnk;

    static const CInterfaceInfo m_knownInterfaces[];
    static const UINT m_countKnownInterfaces;
};




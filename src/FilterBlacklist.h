#pragma once

class CFilterBlacklist
{
public:
    CFilterBlacklist(void);
    ~CFilterBlacklist(void);

    void Free();

    void LoadFromFile(const CString& filepath);
    void SaveToFile(const CString& filepath) const;

    void LoadFromRegistry(CRegKey& regkey);
    void SaveToRegistry(CRegKey& regkey) const;

    bool IsEmpty() const;

	void Add(const CLSID& clsid);
    void Add(const CString& wildcard);
	void Remove(const CLSID& clsid);
    void Remove(const CString& wildcard);

	bool IsOnBlacklist(const CLSID& clsid, const CString& filtername) const;

    // Implementation
private:
    class CFilterBlacklistEntry
    {
    private:
        CLSID m_clsid;
        CString m_wildcard;

    public:
        CFilterBlacklistEntry() : m_clsid(CLSID_NULL) {}
        CFilterBlacklistEntry(const CLSID& clsid) : m_clsid(clsid) {}
        CFilterBlacklistEntry(const CString& wildcard) : m_wildcard(wildcard) {}

        const CLSID get_CLSID() const { return m_clsid; }
        void set_CLSID(const CLSID& clsid) { m_clsid = clsid; }

        const CString get_Wildcard() const { return m_wildcard; }
        void set_Wildcard(const CString& wildcard) { m_wildcard = wildcard; }

        bool Matches(const CLSID& clsid, const CString& filtername) const;
    };

    typedef CArray<CFilterBlacklistEntry> CFilterBlacklistArray;
	CFilterBlacklistArray   m_aBlacklist;

    // no implementation
private:
	CFilterBlacklist(const CFilterBlacklist&);
	CFilterBlacklist& operator =(const CFilterBlacklist&);
};


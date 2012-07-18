#include "stdafx.h"
#include "FilterBlacklist.h"

CFilterBlacklist::CFilterBlacklist(void)
{
}


CFilterBlacklist::~CFilterBlacklist(void)
{
}


void CFilterBlacklist::Free()
{
    m_aBlacklist.RemoveAll();
}

void CFilterBlacklist::LoadFromFile(const CString& filepath)
{
    Free();


}

void CFilterBlacklist::SaveToFile(const CString& filepath) const
{

}

void CFilterBlacklist::LoadFromRegistry(CRegKey& regkey)
{
    Free();

    TCHAR strValName[MAX_PATH];
    TCHAR strValue[MAX_PATH];
    DWORD strValNameLength = MAX_PATH;
    int i=0;
    while (ERROR_SUCCESS == RegEnumValue(regkey, i++, strValName, &strValNameLength, NULL, NULL, NULL, NULL))
    {
        DWORD nValueLength = MAX_PATH;
        if(ERROR_SUCCESS == regkey.QueryStringValue(strValName, strValue, &nValueLength))
        {
            CString valName(strValName);
            if (valName.Find(_T("clsid")) == 0)
            {
                CLSID clsid;
                CLSIDFromString(strValue, &clsid);
                if (!IsEqualCLSID(clsid, CLSID_NULL))
                    m_aBlacklist.Add(CFilterBlacklistEntry(clsid));
            }
            else if (valName.Find(_T("wildcard")) == 0)
            {
                m_aBlacklist.Add(CFilterBlacklistEntry(strValue));
            }
        }
    }
}

void CFilterBlacklist::SaveToRegistry(CRegKey& regkey) const
{
    // delete old values
    TCHAR strValName[MAX_PATH];
    DWORD strValNameLength = MAX_PATH;
    while(ERROR_SUCCESS == RegEnumValue(regkey, 0, strValName, &strValNameLength, NULL, NULL, NULL, NULL))
        regkey.DeleteValue(strValName);

    // write new values
    CString valName;
    int nFilterCount = 0;
    int nCount = m_aBlacklist.GetSize();
	for (int i = 0; i < nCount; ++i)
	{
        if (!IsEqualCLSID(m_aBlacklist[i].get_CLSID(), CLSID_NULL))
        {
            valName.Format(_T("clsid_%d"), nFilterCount);

            LPOLESTR strClsid = NULL;
            StringFromCLSID(m_aBlacklist[i].get_CLSID(), &strClsid);
            CString val(strClsid);
            regkey.SetStringValue(valName, val);
            if (strClsid) CoTaskMemFree(strClsid);
            nFilterCount++;
        }
        else if(!m_aBlacklist[i].get_Wildcard().IsEmpty())
        {
            valName.Format(_T("wildcard_%d"), nFilterCount);

            regkey.SetStringValue(valName, m_aBlacklist[i].get_Wildcard());
            nFilterCount++;
        }
	}
}

bool CFilterBlacklist::IsEmpty() const
{
    return m_aBlacklist.IsEmpty();
}

void CFilterBlacklist::Add(const CLSID& clsid)
{
    m_aBlacklist.Add(CFilterBlacklistEntry(clsid));
}

void CFilterBlacklist::Add(const CString& wildcard)
{
    m_aBlacklist.Add(CFilterBlacklistEntry(wildcard));
}

void CFilterBlacklist::Remove(const CLSID& clsid)
{
    if(clsid == CLSID_NULL) return;

    int nCount = m_aBlacklist.GetSize();
	for (int i = nCount; i >= 0; --i)
	{
		if (IsEqualCLSID(m_aBlacklist[i].get_CLSID(),clsid))
			m_aBlacklist.RemoveAt(i);
	}
}

void CFilterBlacklist::Remove(const CString& wildcard)
{
    if(wildcard.IsEmpty()) return;

    int nCount = m_aBlacklist.GetSize();
	for (int i = nCount; i >= 0; --i)
	{
        if (m_aBlacklist[i].get_Wildcard() == wildcard)
			m_aBlacklist.RemoveAt(i);
	}
}

bool CFilterBlacklist::IsOnBlacklist(const CLSID& clsid, const CString& filtername) const
{
    int nCount = m_aBlacklist.GetSize();
	for (int i = 0; i < nCount; ++i)
	{
		if (m_aBlacklist[i].Matches(clsid, filtername))
			return true;
	}

	return false;
}


bool CFilterBlacklist::CFilterBlacklistEntry::Matches(const CLSID& clsid, const CString& filtername) const
{
    if(!IsEqualCLSID(m_clsid,CLSID_NULL))
        return IsEqualCLSID(m_clsid, clsid) != 0;

    if(filtername.IsEmpty() || m_wildcard.IsEmpty()) return false;

	int nLength = m_wildcard.GetLength();
	if (m_wildcard[nLength - 1] == _T('*')) // starts with
	{
        CString strPrefix, strName;

        if(m_wildcard[0] == _T('*'))    // contains
        {
            strPrefix = m_wildcard.Mid(1,nLength - 2);
            strPrefix.MakeLower();
            strName = filtername;
            strName.MakeLower();
            return strName.Find(strPrefix) >= 0;
        }

		strPrefix = m_wildcard.Left(nLength - 1);
		strName = filtername.Left(nLength - 1);
		return strName.CompareNoCase(strPrefix) == 0;
	}
    else if (m_wildcard[0] == _T('*'))  // ends with
    {
        CString strPrefix = m_wildcard.Right(nLength - 1);
		CString strName = filtername.Right(nLength - 1); // Praefix-Length
		return strName.CompareNoCase(strPrefix) == 0;
    }

	return filtername.CompareNoCase(m_wildcard) == 0;
}

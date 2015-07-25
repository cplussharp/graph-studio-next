//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#pragma once

class CRegistryValue
{
public:
	CRegistryValue(LPCTSTR pValueName, LPCTSTR strVal);
	CRegistryValue(LPCTSTR pValueName, DWORD dwVal);
	CRegistryValue(LPCTSTR pValueName, DWORD type, BYTE* data, DWORD dataLen);
	virtual ~CRegistryValue();

	virtual CString ToString() const;

	CString m_name;
	DWORD m_type;

	CString m_strValue;
	DWORD m_dwValue;

	BYTE* m_valueData;
	DWORD m_valueDataLen;
};

class CRegistryKey
{
public:
	CRegistryKey(LPCTSTR pKeyName, LPCTSTR pKeyPath);
	virtual ~CRegistryKey();

	void AddKey(HKEY hKey, LPCTSTR keyName);

	bool HasData() const;
	virtual CString ToString() const;

	CString m_name;
	CString m_path;
	CSimpleArray<CRegistryValue*> m_values;
	CSimpleArray<CRegistryKey*> m_subkeys;
};

class CRegistryExporter : public CRegistryKey
{
public:
	CRegistryExporter();
	virtual ~CRegistryExporter();

	virtual CString ToString();
};
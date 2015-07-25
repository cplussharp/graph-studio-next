//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "RegistryExporter.h"

CRegistryKey::CRegistryKey(LPCTSTR pKeyName, LPCTSTR pKeyPath)
	: m_name(pKeyName), m_path(pKeyPath)
{
}

CRegistryKey::~CRegistryKey()
{
	for (int i = 0; i < m_subkeys.GetSize(); i++)
		delete m_subkeys[i];
	m_subkeys.RemoveAll();

	for (int i = 0; i < m_values.GetSize(); i++)
		delete m_values[i];
	m_values.RemoveAll();
}

void CRegistryKey::AddKey(HKEY hKey, LPCTSTR keyName)
{
	CString keyPath(m_path);
	if (!keyPath.IsEmpty()) keyPath.Append(_T("\\"));
	keyPath.Append(keyName);

	CRegistryKey* rKey = new CRegistryKey(keyName, keyPath);
	m_subkeys.Add(rKey);

	// get key content info
	DWORD    num_sub_keys = 0;
	DWORD    longest_subkey = 0;
	DWORD    num_values = 0;
	DWORD    longest_value_name = 0;
	DWORD    longest_value_data = 0;

	DWORD retCode = RegQueryInfoKey(hKey, NULL, NULL, NULL,
		&num_sub_keys, &longest_subkey, NULL, &num_values, &longest_value_name, &longest_value_data, NULL, NULL);

	// interate subkeys
	if (num_sub_keys > 0)
	{
		const int			MAX_KEY_LENGTH = 50;
		TCHAR				subkey_name[MAX_KEY_LENGTH];   // buffer for subkey name
		DWORD				subkey_name_size;                   // size of name string 

		for (DWORD i = 0; i < num_sub_keys; i++)
		{
			subkey_name_size = MAX_KEY_LENGTH;
			if (ERROR_SUCCESS == RegEnumKeyEx(hKey, i, subkey_name, &subkey_name_size, NULL, NULL, NULL, NULL))
			{
				CRegKey rkKey;
				if (ERROR_SUCCESS == rkKey.Open(hKey, subkey_name, KEY_READ))
					rKey->AddKey(rkKey, subkey_name);
			}
		}
	}

	// interate values
	if (num_values > 0)
	{
		longest_value_name++;
		longest_value_data++;

		TCHAR* value_name = new TCHAR[longest_value_name];
		BYTE* value_data = new BYTE[longest_value_data];

		DWORD value_type;
		DWORD value_data_length, value_name_length;

		for (DWORD i = 0; i < num_values; i++)
		{
			TCHAR* vn = value_name;
			BYTE* vd = value_data;
			value_type = 0;
			memset(vn, 0, longest_value_name);
			memset(vd, 0, longest_value_data);
			value_name_length = longest_value_name;
			value_data_length = longest_value_data;

			LONG lResult = RegEnumValue(hKey, i, vn, &value_name_length, NULL, &value_type, vd, &value_data_length);
			if (lResult == ERROR_NO_MORE_ITEMS)
				break;

			CRegistryValue* rVal = NULL;
			switch (value_type)
			{
				case REG_SZ:
				{
					CString str;
					str.Format(_T("%s"), vd);
					rVal = new CRegistryValue(vn, str);
					break;
				}

				case REG_DWORD:
				{
					DWORD dwValue;
					memcpy(&dwValue, vd, sizeof DWORD);
					rVal = new CRegistryValue(vn, dwValue);
					break;
				}

				default:
					rVal = new CRegistryValue(vn, value_type, vd, value_data_length);
					break;
			}

			if (rVal) rKey->m_values.Add(rVal);
		}

		delete[] value_name;
		delete[] value_data;
	}
}

bool CRegistryKey::HasData() const
{
	return m_subkeys.GetSize() > 0 || m_values.GetSize() > 0;
}

CString CRegistryKey::ToString() const
{
	CString str;

	if (m_values.GetSize() > 0)
	{
		str.Format(_T("[%s]\r\n"), m_path);

		for (int i = 0; i < m_values.GetSize(); i++)
			str.AppendFormat(_T("%s\r\n"), m_values[i]->ToString());

		str.Append(_T("\r\n"));
	}

	for (int i = 0; i < m_subkeys.GetSize(); i++)
	{
		if (!m_subkeys[i]->HasData()) continue;
		str.Append(m_subkeys[i]->ToString());
	}

	return str;
}






CRegistryValue::CRegistryValue(LPCTSTR pValueName, LPCTSTR strVal)
	: m_name(pValueName), m_type(REG_SZ), m_strValue(strVal), m_dwValue(0), m_valueData(NULL), m_valueDataLen(0)
{
}

CRegistryValue::CRegistryValue(LPCTSTR pValueName, DWORD dwVal)
	: m_name(pValueName), m_type(REG_DWORD), m_dwValue(dwVal), m_valueData(NULL), m_valueDataLen(0)
{
}

CRegistryValue::CRegistryValue(LPCTSTR pValueName, DWORD type, BYTE* data, DWORD dataLen)
	: m_name(pValueName), m_type(type), m_dwValue(0), m_valueData(NULL), m_valueDataLen(dataLen)
{
	if (dataLen > 0 && data)
	{
		m_valueData = new BYTE[dataLen];
		memcpy(m_valueData, data, dataLen);
	}
}

CRegistryValue::~CRegistryValue()
{
	if (m_valueData)
		delete[] m_valueData;
}

CString CRegistryValue::ToString() const
{
	CString str;
	if (m_name.IsEmpty())
		str.Append(_T("@="));
	else
		str.Format(_T("\"%s\"="), m_name);

	switch (m_type)
	{
		case REG_SZ:
			str.AppendFormat(_T("\"%s\""), m_strValue);
			break;

		case REG_DWORD:
			str.AppendFormat(_T("dword:%08x"), m_dwValue);
			break;

		default:
		{
			if (m_type != REG_BINARY)
				str.AppendFormat(_T("hex(%d):"), m_type);
			else
				str.Append(_T("hex:"));

			for (DWORD i = 0; i < m_valueDataLen; i++)
			{
				str.AppendFormat(_T("%02x"), m_valueData[i]);

				if (i + 1 < m_valueDataLen)
					str.Append(_T(","));

				if (i != 0 && (i % 0x15 == 0))
					str.Append(_T("\\\r\n"));
			}

			break;
		}
	}

	return str;
}







CRegistryExporter::CRegistryExporter()
	: CRegistryKey(_T("root"), _T(""))
{
}

CRegistryExporter::~CRegistryExporter()
{
}

CString CRegistryExporter::ToString()
{
	CString str = _T("Windows Registry Editor Version 5.00\r\n\r\n");
	
	for (int i = 0; i < m_subkeys.GetSize(); i++)
	{
		if (!m_subkeys[i]->HasData()) continue;
		str.Append(m_subkeys[i]->ToString());
	}

	return str;
}

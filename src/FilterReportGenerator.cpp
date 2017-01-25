//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "FilterReportGenerator.h"


CFilterReportGenerator::CFilterReportGenerator()
{
}


CFilterReportGenerator::~CFilterReportGenerator()
{
}

const CString& CFilterReportGenerator::GetReport()
{
	if (m_report.IsEmpty())
		GenerateReport();

	return m_report;
}

void CFilterReportGenerator::SaveReport(CString fn)
{
	if (m_report.IsEmpty())
		GenerateReport();

	FILE		*f = NULL;
	if (_tfopen_s(&f, fn, _T("wb")) != NOERROR || !f)
		return;

	CStringA	xa = UTF16toUTF8(m_report);
	fwrite(xa.GetBuffer(), 1, xa.GetLength(), f);
	fclose(f);
}

CStringA CFilterReportGenerator::UTF16toUTF8(const CStringW &utf16)
{
	CStringA utf8;
	int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, 0, 0);
	if (len>1) {
		char *ptr = utf8.GetBuffer(len - 1);
		if (ptr) WideCharToMultiByte(CP_UTF8, 0, utf16, -1, ptr, len, 0, 0);
		utf8.ReleaseBuffer();
	}
	return utf8;
}

bool CFilterReportGenerator::GenerateReport()
{
	m_used_mt_guids.RemoveAll();
	m_files.RemoveAll();

	CComPtr<IStream> outStream;
	HRESULT hr = ::CreateStreamOnHGlobal(NULL, FALSE, &outStream);
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error creating stream on HGlobal for xml");
		return false;
	}

	CComPtr<IXmlWriter> writer;
	hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**)&writer, NULL);
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error creating xml writer");
		return false;
	}

	hr = writer->SetOutput(outStream);
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error setting output for writer");
		return false;
	}

	writer->SetProperty(XmlWriterProperty_ByteOrderMark, FALSE);
	writer->SetProperty(XmlWriterProperty_Indent, TRUE);

	hr = writer->WriteStartDocument(XmlStandalone_Omit);
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error, Method: WriteStartDocument");
		return false;
	}

	if (!WriteStartElement(writer, _T("filterreport"))) return false;

		CString timestamp = CTime::GetCurrentTime().Format("%Y-%m-%dT%H:%M:%S%z");
		if (!WriteAttribute(writer, _T("created"), timestamp)) return false;

		if (!EnumerateCategories(writer)) return false;

	if (!WriteEndElement(writer)) return false;

	hr = writer->WriteEndDocument();
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error WriteEndDocument");
		return false;
	}

	hr = writer->Flush();
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error Flush");
		return false;
	}

	// get content size
	STATSTG ssStreamData = { 0 };
	hr = outStream->Stat(&ssStreamData, STATFLAG_NONAME);
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error getting stream information");
		return false;
	}

	// seek to the start of the stream
	LARGE_INTEGER position;
	position.QuadPart = 0;
	hr = outStream->Seek(position, STREAM_SEEK_SET, NULL);
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error seeking to the start of the stream");
		return false;
	}

	// Copies the content from the stream to the buffer.
	SIZE_T cbSize = ssStreamData.cbSize.LowPart;
	CStringW strBuffer;
	LPWSTR pwszBuffer = strBuffer.GetBuffer(cbSize / sizeof(WCHAR));
	ULONG cbRead;
	hr = outStream->Read(pwszBuffer, cbSize, &cbRead);
	if (FAILED(hr))
	{
		strBuffer.ReleaseBuffer(0);
		errorHr = hr;
		errorMsg = _T("Error reading stream content");
		return false;
	}

	strBuffer.ReleaseBuffer(cbRead / sizeof(WCHAR));
	m_report = strBuffer;

	return true;
}

bool CFilterReportGenerator::EnumerateCategories(CComPtr<IXmlWriter>& writer)
{
	CComPtr<ICreateDevEnum> sys_dev_enum;
	CComPtr<IEnumMoniker>	enum_moniker;
	CComPtr<IMoniker>		moniker;
	CComPtr<IPropertyBag>	propbag;
	ULONG					f;

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&sys_dev_enum);
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error creating CLSID_SystemDeviceEnum");
		return false;
	}

	hr = sys_dev_enum->CreateClassEnumerator(CLSID_ActiveMovieCategories, &enum_moniker, 0);
	if (FAILED(hr) || !enum_moniker)
	{
		errorHr = FAILED(hr) ? hr : E_FAIL;
		errorMsg = _T("Error create ClassEnumerator for CLSID_ActiveMovieCategories");
		return false;
	}

	enum_moniker->Reset();
	while (enum_moniker->Next(1, &moniker, &f) == NOERROR)
	{
		hr = moniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&propbag);
		if (SUCCEEDED(hr))
		{
			CString strName;
			GUID clsid;
			VARIANT var;

			VariantInit(&var);
			hr = propbag->Read(L"FriendlyName", &var, 0);
			if (SUCCEEDED(hr)) strName = CString(var.bstrVal);
			VariantClear(&var);

			VariantInit(&var);
			hr = propbag->Read(L"CLSID", &var, 0);
			if (SUCCEEDED(hr))
			{
				if (SUCCEEDED(CLSIDFromString(var.bstrVal, &clsid)))
					if (!WriteFilterCategory(writer, strName, clsid, false)) return false;
			}
			VariantClear(&var);
		}
	}

	// now add the DMO categories
	if (!WriteFilterCategory(writer, _T("DMO Video Decoder"), DMOCATEGORY_VIDEO_DECODER, true)) return false;
	if (!WriteFilterCategory(writer, _T("DMO Video Effect"), DMOCATEGORY_VIDEO_EFFECT, true)) return false;
	if (!WriteFilterCategory(writer, _T("DMO Video Encoder"), DMOCATEGORY_VIDEO_ENCODER, true)) return false;
	if (!WriteFilterCategory(writer, _T("DMO Audio Decoder"), DMOCATEGORY_AUDIO_DECODER, true)) return false;
	if (!WriteFilterCategory(writer, _T("DMO Audio Effect"), DMOCATEGORY_AUDIO_EFFECT, true)) return false;
	if (!WriteFilterCategory(writer, _T("DMO Audio Encoder"), DMOCATEGORY_AUDIO_ENCODER, true)) return false;
	if (!WriteFilterCategory(writer, _T("DMO Audio Capture Effect"), DMOCATEGORY_AUDIO_CAPTURE_EFFECT, true)) return false;

	return true;
}

bool CFilterReportGenerator::WriteFilterCategory(CComPtr<IXmlWriter> &writer, const CString& catName, const CLSID& clsid, bool isDMO)
{
	if (!WriteStartElement(writer, _T("category"))) return false;

	{
		if (!WriteAttribute(writer, _T("clsid"), clsid)) return false;
		if (!WriteAttribute(writer, _T("name"), catName)) return false;
		if (!WriteAttribute(writer, _T("dmo"), isDMO ? _T("true") : _T("false"))) return false;

		if (isDMO)
		{
			if (!EnumerateFiltersOfCategory(writer, clsid)) return false;
		}
		else
		{
			if (!EnumerateFiltersOfDMOCategory(writer, clsid)) return false;
		}
	}

	if (!WriteEndElement(writer)) return false;
}

bool CFilterReportGenerator::EnumerateFiltersOfCategory(CComPtr<IXmlWriter>& writer, const CLSID& clsid)
{
	CComPtr<ICreateDevEnum> sys_dev_enum;
	CComPtr<IEnumMoniker> enum_moniker;
	CComPtr<IMoniker> moniker;
	CComPtr<IPropertyBag> propbag;
	ULONG f;
	
	HRESULT	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&sys_dev_enum);
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error creating CLSID_SystemDeviceEnum");
		return false;
	}

	hr = sys_dev_enum->CreateClassEnumerator(clsid, &enum_moniker, 0);
	if (FAILED(hr) || !enum_moniker)
	{
		errorHr = FAILED(hr) ? hr : E_FAIL;
		errorMsg = _T("Error create ClassEnumerator for CLSID");
		return false;
	}

	enum_moniker->Reset();
	while (enum_moniker->Next(1, &moniker, &f) == NOERROR)
	{
		hr = moniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&propbag);
		if (SUCCEEDED(hr))
		{
			if (!WriteStartElement(writer, _T("filter"))) return false;

			VARIANT var;
			CLSID clsid = CLSID_NULL;

			VariantInit(&var);
			hr = propbag->Read(L"CLSID", &var, 0);
			if (SUCCEEDED(hr) && var.vt == VT_BSTR)
			{
				hr = CLSIDFromString(var.bstrVal, &clsid);
				if (SUCCEEDED(hr))
					WriteAttribute(writer, _T("clsid"), clsid);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr = propbag->Read(L"FriendlyName", &var, 0);
			if (SUCCEEDED(hr) && var.vt == VT_BSTR)
			{
				CString strVal = CString(var.bstrVal);
				WriteAttribute(writer, _T("name"), strVal);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr = propbag->Read(L"DevicePath", &var, 0);
			if (SUCCEEDED(hr) && var.vt == VT_BSTR)
			{
				CString strVal = CString(var.bstrVal);
				WriteAttribute(writer, _T("device_path"), strVal);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr = propbag->Read(L"Description", &var, 0);
			if (SUCCEEDED(hr) && var.vt == VT_BSTR)
			{
				CString strVal = CString(var.bstrVal);
				WriteAttribute(writer, _T("description"), strVal);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr = propbag->Read(L"WaveInID", &var, 0);
			if (SUCCEEDED(hr) && var.vt == VT_I4)
			{
				CString strVal;
				strVal.Format(_T("%d"), var.intVal);
				WriteAttribute(writer, _T("wave_in_id"), strVal);
			}
			VariantClear(&var);

			LPOLESTR moniker_name;
			hr = moniker->GetDisplayName(NULL, NULL, &moniker_name);
			if (SUCCEEDED(hr))
			{
				CString strVal = CString(moniker_name);

				IMalloc *alloc = NULL;
				hr = CoGetMalloc(1, &alloc);
				if (SUCCEEDED(hr)) {
					alloc->Free(moniker_name);
					alloc->Release();
				}

				if (!strVal.IsEmpty())
				{
					CString strType;
					if (strVal.Find(_T(":dmo:")) >= 0) strType = _T("DMO");
					else if (strVal.Find(_T(":cm:")) >= 0) strType = _T("ACM/ICM");
					else if (strVal.Find(_T(":pnp:")) >= 0)	strType = _T("PNP");

					if (!strType.IsEmpty())
					{
						WriteAttribute(writer, _T("type"), strType);
					}
				}
			}

			VariantInit(&var);
			hr = propbag->Read(L"FilterData", &var, 0);
			if (SUCCEEDED(hr))
			{
				SAFEARRAY	*ar = var.parray;
				int	size = ar->rgsabound[0].cElements;

				// load version, merit and pins
				WriteFilterData(writer, (char*)ar->pvData, size);
			}
			VariantClear(&var);

			// try to get the filename
			OLECHAR szCLSID[CHARS_IN_GUID];
			StringFromGUID2(clsid, szCLSID, CHARS_IN_GUID);
			CString strCLSID = szCLSID;
			
			CString		key_name;
			key_name.Format(_T("CLSID\\%s\\InprocServer32"), (LPCTSTR)strCLSID);
			CRegKey		key;
			if (key.Open(HKEY_CLASSES_ROOT, key_name, KEY_READ) == ERROR_SUCCESS)
			{
				TCHAR		temp[2 * MAX_PATH];
				ULONG		chars = 2 * MAX_PATH;
				key.QueryStringValue(_T(""), temp, &chars);
				temp[chars] = 0;
				CString strFile = temp;

				if (!strFile.IsEmpty())
				{
					if (!WriteElement(writer, _T("file"), strFile)) return false;
					AddFile(strFile);
				}
			}

			if (WriteEndElement(writer)) return false;
		}
	}

	return true;
}

bool CFilterReportGenerator::WriteFilterData(CComPtr<IXmlWriter> &writer, char *buf, int size)
{
	DWORD *b = (DWORD*)buf;

	// Version
	DWORD version = b[0];
	CString strVersion;
	strVersion.Format(_T("0x%08x"), version);
	if (WriteAttribute(writer, _T("version"), strVersion)) return false;

	// Merit
	DWORD merit = b[1];
	if (WriteStartElement(writer, _T("merit"))) return false;

	{
		CString strVal;

		strVal.Format(_T("%d"), merit);
		if (WriteAttribute(writer, _T("dec"), strVal)) return false;

		strVal.Format(_T("0x%08x"), merit);
		if (WriteAttribute(writer, _T("hex"), strVal)) return false;

		strVal.Empty();
		static const struct { DWORD nValue; LPCSTR pszName; } meritNames[] =
		{
			#define A(x) { x, #x },
			A(MERIT_PREFERRED)
			A(MERIT_NORMAL)
			A(MERIT_UNLIKELY)
			A(MERIT_DO_NOT_USE)
			A(MERIT_SW_COMPRESSOR)
			A(MERIT_HW_COMPRESSOR)
			#undef A
		};
		for (SIZE_T i = 0; i < _countof(meritNames); i++)
		{
			if (merit >= meritNames[i].nValue - 0x20 && merit <= meritNames[i].nValue + 0x20)
			{
				const INT nDelta = (INT)(merit - meritNames[i].nValue);
				strVal = CA2CT(meritNames[i].pszName);
				if (nDelta > 0)
					strVal.AppendFormat(_T(" + %d"), nDelta);
				else if (nDelta < 0)
					strVal.AppendFormat(_T(" - %d"), -nDelta);

				if (WriteAttribute(writer, _T("name"), strVal)) return false;

				break;
			}
		}
	}

	if (WriteEndElement(writer)) return false;

	// Pins
	int pinCount = b[2];
	DWORD *pinData = b + 4;
	for (int i = 0; i < pinCount; i++)
	{
		if ((char*)pinData >(buf + size - 6 * 4)) break;

		if (!WriteStartElement(writer, _T("pin"))) return false;

		{
			DWORD flags = pinData[1];
			if (!WriteAttribute(writer, _T("dir"), flags & 0x08 ? _T("out") : _T("in"))) return false;
			if (!WriteAttribute(writer, _T("many"), flags & 0x04 ? _T("true") : _T("false"))) return false;
			if (!WriteAttribute(writer, _T("rendered"), flags & 0x02 ? _T("true") : _T("false"))) return false;

			int typeCount = pinData[3];

			// skip dummy data
			pinData += 6;
			for (int j = 0; j < typeCount; j++) {
				// make sure we have at least 16 bytes available
				if ((char*)pinData >(buf + size - 16)) break;

				DWORD maj_offset = pinData[2];
				DWORD min_offset = pinData[3];

				if ((maj_offset + 16 <= (DWORD)size) && (min_offset + 16 <= (DWORD)size))
				{
					GUID	g;
					BYTE	*m = (BYTE*)(&buf[maj_offset]);
					if ((char*)m > (buf + size - 16)) break;
					g.Data1 = m[0] | (m[1] << 8) | (m[2] << 16) | (m[3] << 24);
					g.Data2 = m[4] | (m[5] << 8);
					g.Data3 = m[6] | (m[7] << 8);
					memcpy(g.Data4, m + 8, 8);
					GUID major = g;
					AddUsedMtGuid(major);

					m = (BYTE*)(&buf[min_offset]);
					if ((char*)m > (buf + size - 16)) break;
					g.Data1 = m[0] | (m[1] << 8) | (m[2] << 16) | (m[3] << 24);
					g.Data2 = m[4] | (m[5] << 8);
					g.Data3 = m[6] | (m[7] << 8);
					memcpy(g.Data4, m + 8, 8);
					GUID minor = g;
					AddUsedMtGuid(minor);

					if (!WriteStartElement(writer, _T("mt"))) return false;
						
						if (!WriteAttribute(writer, _T("major"), major)) return false;
						if (!WriteAttribute(writer, _T("minor"), minor)) return false;

					if (!WriteEndElement(writer)) return false;	// mt
				}

				pinData += 4;
			}
		}

		if (!WriteEndElement(writer)) return false;	// pin
	}
}

bool CFilterReportGenerator::EnumerateFiltersOfDMOCategory(CComPtr<IXmlWriter>& writer, const CLSID& clsid)
{
	// TODO
	return true;
}


bool CFilterReportGenerator::WriteStartElement(CComPtr<IXmlWriter> &writer, const CString& elName)
{
	HRESULT hr = writer->WriteStartElement(NULL, CT2W(elName), NULL);
	if (FAILED(hr))
	{
		errorHr = hr;
		CString err;
		err.Format(_T("Error WriteStartElement '%s'"), elName);
		errorMsg = err;
		return false;
	}

	return true;
}

bool CFilterReportGenerator::WriteEndElement(CComPtr<IXmlWriter> &writer)
{
	HRESULT hr = writer->WriteEndElement();
	if (FAILED(hr))
	{
		errorHr = hr;
		errorMsg = _T("Error WriteEndElement");
		return false;
	}

	return true;
}

bool CFilterReportGenerator::WriteElement(CComPtr<IXmlWriter> &writer, const CString& elName, const CString& elText)
{
	HRESULT hr = writer->WriteElementString(NULL, CT2W(elName), NULL, CT2W(elText));
	if (FAILED(hr))
	{
		errorHr = hr;
		CString err;
		err.Format(_T("Error WriteElementString '%s' = '%s'"), elName, elText);
		errorMsg = err;
		return false;
	}

	return true;
}

bool CFilterReportGenerator::WriteAttribute(CComPtr<IXmlWriter> &writer, const CString& attrName, const CString& attrValue)
{
	HRESULT hr = writer->WriteAttributeString(NULL, CT2W(attrName), NULL, CT2W(attrValue));
	if (FAILED(hr))
	{
		errorHr = hr;
		CString err;
		err.Format(_T("Error WriteAttributeString '%s'='%s'"), attrName, attrValue);
		errorMsg = err;
		return false;
	}

	return true;
}

bool CFilterReportGenerator::WriteAttribute(CComPtr<IXmlWriter> &writer, const CString& attrName, const GUID& guid)
{
	OLECHAR szGUID[CHARS_IN_GUID];
	StringFromGUID2(guid, szGUID, CHARS_IN_GUID);
	CStringW strGuid = szGUID;

	HRESULT hr = writer->WriteAttributeString(NULL, CT2W(attrName), NULL, strGuid);
	if (FAILED(hr))
	{
		errorHr = hr;
		CString err;
		err.Format(_T("Error WriteAttributeString '%s'='%s'"), attrName, strGuid);
		errorMsg = err;
		return false;
	}

	return true;
}



void CFilterReportGenerator::AddUsedMtGuid(const GUID& guid)
{
	for (int i = 0; i < m_used_mt_guids.GetSize(); i++)
		if (m_used_mt_guids[i] == guid) return;

	m_used_mt_guids.Add(guid);
}

void CFilterReportGenerator::AddFile(const CString& file)
{
	for (int i = 0; i < m_files.GetSize(); i++)
		if (m_files[i] == file) return;

	m_files.Add(file);
}
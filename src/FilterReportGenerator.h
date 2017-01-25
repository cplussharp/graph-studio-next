#pragma once
//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

class CFilterReportGenerator
{
public:
	CFilterReportGenerator();
	virtual ~CFilterReportGenerator();

	const CString& GetReport();
	void SaveReport(CString fn);

protected:
	bool GenerateReport();

	bool EnumerateCategories(CComPtr<IXmlWriter>& writer);
	bool EnumerateFiltersOfCategory(CComPtr<IXmlWriter>& writer, const CLSID& clsid);
	bool EnumerateFiltersOfDMOCategory(CComPtr<IXmlWriter>& writer, const CLSID& clsid);
	bool EnumerateFiles(CComPtr<IXmlWriter>& writer);

	static CStringA UTF16toUTF8(const CStringW &utf16);

	CString m_report;

	bool HasError() const { return FAILED(errorHr); }
	HRESULT errorHr;
	CString errorMsg;

	CSimpleArray<GUID> m_used_mt_guids;
	CSimpleArray<CString> m_files;

	void AddUsedMtGuid(const GUID& guid);
	void AddFile(const CString& file);

	// XML Helper
	bool WriteStartElement(CComPtr<IXmlWriter> &writer, const CString& elName);
	bool WriteEndElement(CComPtr<IXmlWriter> &writer);
	bool WriteElement(CComPtr<IXmlWriter> &writer, const CString& elName, const CString& elText);
	bool WriteAttribute(CComPtr<IXmlWriter> &writer, const CString& attrName, const CString& attrValue);
	bool WriteAttribute(CComPtr<IXmlWriter> &writer, const CString& attrName, const GUID& guid);

	bool WriteFilterCategory(CComPtr<IXmlWriter> &writer, const CString& catName, const CLSID& clsid, bool isDMO);
	bool WriteFilterData(CComPtr<IXmlWriter> &writer, char *buf, int size);
};


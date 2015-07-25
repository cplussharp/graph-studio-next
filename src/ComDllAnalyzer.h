//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#pragma once

#include "RegistryExporter.h"

class CComDllAnalyzer
{
public:
	CComDllAnalyzer(LPCTSTR pDllFile);
	~CComDllAnalyzer();

	static HRESULT GetClassFactoryEntryPoint(LPCOLESTR dll_file, HMODULE& hLib, LPFNGETCLASSOBJECT & entry_point);
	static void GetClsidsFromRegistry(HKEY keyClass, LPFNGETCLASSOBJECT entry_point, CAtlMap<CLSID, CString>& matched_clsids);

	CRegistryExporter registry;
	CAtlMap<CLSID, CString> clsids;

	bool HasError() const { return FAILED(errorHr); }
	HRESULT errorHr;
	CString errorMsg;
};


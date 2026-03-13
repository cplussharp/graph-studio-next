//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "VariantTypeHelper.h"

GRAPHSTUDIO_NAMESPACE_START

// Helper structure for VARTYPE to/from string conversion
struct VarTypeMapping {
	VARTYPE vt;
	const TCHAR* name;
};

static const VarTypeMapping g_VarTypeMappings[] = {
	{ VT_EMPTY,			_T("EMPTY") },
	{ VT_NULL,			_T("NULL") },
	{ VT_I2,			_T("I2") },
	{ VT_I4,			_T("I4") },
	{ VT_R4,			_T("R4") },
	{ VT_R8,			_T("R8") },
	{ VT_CY,			_T("CURRENCY") },
	{ VT_DATE,			_T("DATE") },
	{ VT_BSTR,			_T("BSTR") },
	{ VT_DISPATCH,		_T("DISPATCH") },
	{ VT_ERROR,			_T("ERROR") },
	{ VT_BOOL,			_T("BOOL") },
	{ VT_VARIANT,		_T("VARIANT") },
	{ VT_UNKNOWN,		_T("UNKNOWN") },
	{ VT_DECIMAL,		_T("DECIMAL") },
	{ VT_I1,			_T("I1") },
	{ VT_UI1,			_T("UI1") },
	{ VT_UI2,			_T("UI2") },
	{ VT_UI4,			_T("UI4") },
	{ VT_I8,			_T("I8") },
	{ VT_UI8,			_T("UI8") },
	{ VT_INT,			_T("INT") },
	{ VT_UINT,			_T("UINT") },
	{ VT_VOID,			_T("VOID") },
	{ VT_HRESULT,		_T("HRESULT") },
	{ VT_PTR,			_T("PTR") },
	{ VT_SAFEARRAY,		_T("SAFEARRAY") },
	{ VT_CARRAY,		_T("CARRAY") },
	{ VT_USERDEFINED,	_T("USERDEFINED") },
	{ VT_LPSTR,			_T("LPSTR") },
	{ VT_LPWSTR,		_T("LPWSTR") },
	{ VT_RECORD,		_T("RECORD") },
	{ VT_INT_PTR,		_T("INT_PTR") },
	{ VT_UINT_PTR,		_T("UINT_PTR") },
	{ VT_FILETIME,		_T("FILETIME") },
	{ VT_BLOB,			_T("BLOB") },
	{ VT_STREAM,		_T("STREAM") },
	{ VT_STORAGE,		_T("STORAGE") },
	{ VT_STREAMED_OBJECT,_T("STREAMED_OBJECT") },
	{ VT_STORED_OBJECT,	_T("STORED_OBJECT") },
	{ VT_BLOB_OBJECT,	_T("BLOB_OBJECT") },
	{ VT_CF,			_T("CLIPBOARD") },
	{ VT_CLSID,			_T("CLSID") },
	{ VT_VERSIONED_STREAM,_T("VERSIONED_STREAM") },
	{ VT_BSTR_BLOB,		_T("BSTR_BLOB") },
	{ VT_VECTOR,		_T("VECTOR") },
	{ VT_ARRAY,			_T("ARRAY") },
	{ VT_BYREF,			_T("BYREF") },
};


// Convert VARTYPE to string representation
CString VarTypeToString(VARTYPE vt)
{
	for (int i = 0; i < _countof(g_VarTypeMappings); i++) {
		if (g_VarTypeMappings[i].vt == vt) {
			return g_VarTypeMappings[i].name;
		}
	}
	return _T("");
}

// Convert string representation back to VARTYPE
VARTYPE StringToVarType(const CString& str)
{
	if (str.IsEmpty())
		return VT_EMPTY;

	for (int i = 0; i < _countof(g_VarTypeMappings); i++) {
		if (str.Compare(g_VarTypeMappings[i].name) == 0) {
			return g_VarTypeMappings[i].vt;
		}
	}

	return VT_EMPTY;  // default to EMPTY if not found
}

GRAPHSTUDIO_NAMESPACE_END

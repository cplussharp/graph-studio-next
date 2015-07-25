//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ComDllAnalyzer.h"

CComDllAnalyzer::CComDllAnalyzer(LPCTSTR pDllFile)
	: errorHr(S_OK)
{
	// get DLL entry point
	HMODULE	library;
	LPFNGETCLASSOBJECT entry_point = NULL;
	HRESULT hr = GetClassFactoryEntryPoint(T2COLE(pDllFile), library, entry_point);
	if (FAILED(hr) || !library || !entry_point) {
		errorHr = FAILED(hr) ? hr : E_FAIL;
		errorMsg = _T("Error getting DLL entry point");
		return;
	}

	// get DllRegistry Functions
	typedef HRESULT(_stdcall *DllRegisterServerProc)();
	typedef HRESULT(_stdcall *DllUnregisterServerProc)();

	DllRegisterServerProc reg = (DllUnregisterServerProc)GetProcAddress(library, "DllRegisterServer");
	if (!reg)
	{
		errorHr = HRESULT_FROM_WIN32(GetLastError());
		errorMsg = _T("Error getting DllRegisterServer");
	}

	DllUnregisterServerProc unreg = (DllUnregisterServerProc)GetProcAddress(library, "DllUnregisterServer");
	if (!unreg)
	{
		errorHr = HRESULT_FROM_WIN32(GetLastError());
		errorMsg = _T("Error getting DllUnregisterServer");
	}

	// redirect Registry Keys to keep the system registry and have a clean tree to analyse
	// override HKEY_CLASSES_ROOT
	HKEY hKeyFakeClass;
	CString strKeyClass = _T("Software\\MONOGRAM\\GraphStudioNext\\HKEY_CLASSES_ROOT");
	long lRes = RegCreateKey(HKEY_CURRENT_USER, strKeyClass, &hKeyFakeClass);
	if (lRes == ERROR_SUCCESS)
		lRes = RegOverridePredefKey(HKEY_CLASSES_ROOT, hKeyFakeClass);

	if (lRes == ERROR_SUCCESS)
	{
		// override HKEY_LOCAL_MACHINE
		HKEY hKeyFakeLocal;
		CString strKeyLocal = _T("Software\\MONOGRAM\\GraphStudioNext\\HKEY_LOCAL_MACHINE");
		long lRes = RegCreateKey(HKEY_CURRENT_USER, strKeyLocal, &hKeyFakeLocal);
		if (lRes == ERROR_SUCCESS)
			lRes = RegOverridePredefKey(HKEY_LOCAL_MACHINE, hKeyFakeLocal);

		if (lRes == ERROR_SUCCESS)
		{
			// override HKEY_CURRENT_USER
			HKEY hKeyFakeUser;
			CString strKeyUser = _T("Software\\MONOGRAM\\GraphStudioNext\\HKEY_CURRENT_USER");
			long lRes = RegCreateKey(HKEY_CURRENT_USER, strKeyUser, &hKeyFakeUser);
			if (lRes == ERROR_SUCCESS)
				lRes = RegOverridePredefKey(HKEY_CURRENT_USER, hKeyFakeUser);

			if (lRes == ERROR_SUCCESS)
			{
				// temporary register the file and then search the CLSIDs
				HRESULT hr = reg();
				if (SUCCEEDED(hr))
				{
					// search for CLSIDs
					GetClsidsFromRegistry(hKeyFakeClass, entry_point, clsids);

					// remember registry entries
					registry.AddKey(hKeyFakeClass, _T("HKEY_CLASSES_ROOT"));
					registry.AddKey(hKeyFakeLocal, _T("HKEY_LOCAL_MACHINE"));
					registry.AddKey(hKeyFakeUser, _T("HKEY_CURRENT_USER"));

					// unregister to clean up
					unreg();
				}
				else
				{
					errorHr = hr;
					errorMsg = _T("Error temporary registering the DLL");
				}

				// restore HKEY_CURRENT_USER
				RegOverridePredefKey(HKEY_CURRENT_USER, NULL);

				// delete key
				//RegDeleteTree(hKeyFakeUser, NULL); // not working on XP
				SHDeleteKey(hKeyFakeUser, NULL);
				RegCloseKey(hKeyFakeUser);
				RegDeleteKey(HKEY_CURRENT_USER, strKeyUser);

			}

			// restore HKEY_LOCAL_MACHINE
			RegOverridePredefKey(HKEY_LOCAL_MACHINE, NULL);

			// delete key
			//RegDeleteTree(hKeyFakeLocal, NULL); // not working on XP
			SHDeleteKey(hKeyFakeLocal, NULL);
			RegCloseKey(hKeyFakeLocal);
			RegDeleteKey(HKEY_CURRENT_USER, strKeyLocal);
		}

		// restore HKEY_CLASSES_ROOT
		RegOverridePredefKey(HKEY_CLASSES_ROOT, NULL);

		// delete key
		//RegDeleteTree(hKeyFakeClass, NULL); // not working on XP
		SHDeleteKey(hKeyFakeClass, NULL);
		RegCloseKey(hKeyFakeClass);
		RegDeleteKey(HKEY_CURRENT_USER, strKeyClass);
	}

	if (HasError()) return;

	// Find CLSIDs in Registry
	GetClsidsFromRegistry(HKEY_CLASSES_ROOT, entry_point, clsids);

	// Find CLSIDs of all coclasses in type library
	CComPtr<ITypeLib> typelib;
	if (SUCCEEDED(LoadTypeLibEx(T2COLE(pDllFile), REGKIND_NONE, &typelib)) && typelib)
	{
		CComPtr<ITypeInfo> typeinfo;
		for (UINT i = 0; i < typelib->GetTypeInfoCount(); ++i) {
			typeinfo.Release();

			TYPEKIND typekind;
			if (SUCCEEDED(typelib->GetTypeInfoType(i, &typekind))
				&& typekind == TKIND_COCLASS) {
				CComBSTR class_name;
				TYPEATTR *typeattr = NULL;
				typelib->GetTypeInfo(i, &typeinfo);
				typeinfo->GetDocumentation(MEMBERID_NIL, &class_name, NULL, NULL, NULL);
				typeinfo->GetTypeAttr(&typeattr);

				IClassFactory * factory = NULL;
				if (SUCCEEDED(entry_point(typeattr->guid, __uuidof(IClassFactory), (void**)&factory))) {
					CString guid_name(class_name);
					guid_name += _T(" (typelib)");
					clsids.SetAt(typeattr->guid, guid_name);
				}
				if (factory)
					factory->Release();
			}
		}
	}
}


CComDllAnalyzer::~CComDllAnalyzer()
{
}

HRESULT CComDllAnalyzer::GetClassFactoryEntryPoint(LPCOLESTR dll_file, HMODULE& hLib, LPFNGETCLASSOBJECT& entry_point)
{
	hLib = NULL;
	entry_point = NULL;
	HRESULT hr = S_OK;
	if (!dll_file)
		return E_POINTER;

	// set dll path as lib path
	CString libPath = dll_file;
	PathRemoveFileSpec(libPath.GetBuffer());
	libPath.ReleaseBuffer();
	SetDllDirectory(libPath);

	SetLastError(0);
	hLib = CoLoadLibrary(const_cast<LPOLESTR>(dll_file), TRUE);
	if (hLib != NULL)
		entry_point = (LPFNGETCLASSOBJECT)GetProcAddress(hLib, "DllGetClassObject");

	if (hLib == NULL || entry_point == NULL)
	{
		DWORD dwError = GetLastError();
		if (dwError != 0)
			hr = HRESULT_FROM_WIN32(dwError);
		else
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
	}

	return hr;
}

void CComDllAnalyzer::GetClsidsFromRegistry(HKEY keyClass, LPFNGETCLASSOBJECT entry_point, CAtlMap<CLSID, CString>& matched_clsids)
{
	HKEY hKey = NULL;
	if (RegOpenKeyEx(keyClass,
		TEXT("CLSID"),
		0,
		KEY_READ,
		&hKey) == ERROR_SUCCESS)
	{
		DWORD    num_sub_keys = 0;
		DWORD    longest_subkey = 0;

		// Get the class name and the value count. 
		DWORD retCode = RegQueryInfoKey(hKey, NULL, NULL, NULL,
			&num_sub_keys, &longest_subkey, NULL, NULL, NULL, NULL, NULL, NULL);

		if (num_sub_keys > 0)
		{
			const int			MAX_KEY_LENGTH = 50;
			TCHAR				subkey_name[MAX_KEY_LENGTH];   // buffer for subkey name
			DWORD				subkey_name_size;                   // size of name string 
			CLSID				subkey_clsid;
			IClassFactory *		factory = NULL;

			for (DWORD i = 0; i < num_sub_keys; i++)
			{
				subkey_name_size = MAX_KEY_LENGTH;
				if (ERROR_SUCCESS == RegEnumKeyEx(hKey, i, subkey_name, &subkey_name_size, NULL, NULL, NULL, NULL)
					&& SUCCEEDED(CLSIDFromString(subkey_name, &subkey_clsid))
					&& SUCCEEDED(entry_point(subkey_clsid, __uuidof(IClassFactory), (void**)&factory)))
				{
					CRegKey rkKey;
					if (ERROR_SUCCESS == rkKey.Open(hKey, subkey_name, KEY_READ))
					{
						const int MAX_NAME_LENGTH = 256;
						TCHAR szFilterName[MAX_NAME_LENGTH] = { 0 };
						DWORD szLength = MAX_NAME_LENGTH;

						// Get name of COM object
						if (ERROR_SUCCESS == rkKey.QueryStringValue(NULL, szFilterName, &szLength))
						{
							CString clsid_name = szFilterName;
							matched_clsids.SetAt(subkey_clsid, clsid_name);
						}
					}
				}
				if (factory) {
					factory->Release();
					factory = NULL;
				}
			}
		}
	}
	RegCloseKey(hKey);
}
//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : Christian Gräfe
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FilterFromFile.h"

#include "time_utils.h"

// CFilterFromFile-Dialogfeld

IMPLEMENT_DYNAMIC(CFilterFromFile, CDialog)

CFilterFromFile::CFilterFromFile(CWnd* pParent /*=NULL*/)
: CDialog(CFilterFromFile::IDD, pParent), result_clsid(CLSID_NULL), filterFactory(NULL), hr(S_OK)
{

}

CFilterFromFile::~CFilterFromFile()
{
    if(filterFactory)
        filterFactory->Release();
}

void CFilterFromFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, button_browse);
	DDX_Control(pDX, IDC_COMBO_FILE, combo_file);
	DDX_Control(pDX, IDC_COMBO_CLSID, combo_clsid);
    DDX_Control(pDX, IDC_LIST_DATA, list_clsid);
}


BEGIN_MESSAGE_MAP(CFilterFromFile, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CFilterFromFile::OnClickedButtonBrowse)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CFilterFromFile::OnClickedButtonClear)
    ON_CBN_EDITCHANGE(IDC_COMBO_FILE, &CFilterFromFile::OnChangeComboFile)
    ON_CBN_SELCHANGE(IDC_COMBO_FILE, &CFilterFromFile::OnChangeComboFile)
	ON_BN_CLICKED(IDC_BUTTON_SCAN_CLSIDS, &CFilterFromFile::OnBnClickedButtonScanClsids)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_DATA, &CFilterFromFile::OnLvnColumnclickListData)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DATA, &CFilterFromFile::OnNMDblclkListData)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_DATA, &CFilterFromFile::OnLvnItemchangedListData)
END_MESSAGE_MAP()


// CFilterFromFile-Meldungshandler


void CFilterFromFile::OnOK()
{
	combo_file.GetWindowText(result_file);
	if (result_file != _T(""))
    {
        if(PathFileExists(result_file))
        {
            file_list.UpdateList(result_file);
			file_list.SaveList(_T("FilterFileCache"));

            CString strClsid;
            combo_clsid.GetWindowText(strClsid);
		    if (strClsid != _T(""))
            {
                strClsid.Trim();
                strClsid.MakeUpper();
                if(strClsid.Left(1) != _T("{"))
                    strClsid.Insert(0,_T("{"));
                if(strClsid.Right(1) != _T("}"))
                    strClsid.Append(_T("}"));

                hr = CLSIDFromString(strClsid, &result_clsid);
                if(SUCCEEDED(hr))
                {
					hr = DSUtil::GetClassFactoryFromDll(T2COLE(result_file), result_clsid, &filterFactory);
					if (FAILED(hr))
						return;

                    CString entry = PathFindFileName(result_file);
                    entry.MakeLower();
                    entry += _T("-");
                    entry += strClsid;
                    clsid_list.UpdateList(entry);
			        clsid_list.SaveList(_T("FilterFileClsidCache"));
                }
                else
                {
                    DSUtil::ShowError(hr, _T("CLSID - Parsing Error"));
                    result_clsid = CLSID_NULL;
                    return;
                }
		    }
            else
            {
                DSUtil::ShowError(_T("No CLSID provided"));
                return;
            }
        }
        else
        {
            result_file.Empty();
            DSUtil::ShowError(_T("Can't find specified file"));
            return;
        }
	}

	EndDialog(IDOK);
}


BOOL CFilterFromFile::OnInitDialog()
{
    CDialog::OnInitDialog();

    file_list.LoadList(_T("FilterFileCache"));
	clsid_list.LoadList(_T("FilterFileClsidCache"));

    int sel = -1;

	for (int i=0; i<file_list.GetCount(); i++)
        sel = combo_file.AddString(file_list[i]);

	if (result_file.GetLength() > 0)
		combo_file.SetWindowText(result_file);
	else
		combo_file.SetCurSel(sel);
    OnChangeComboFile();

	list_clsid.InsertColumn(0, _T("CLSID"),	LVCFMT_LEFT, 180);
    list_clsid.InsertColumn(1, _T("Name"),	LVCFMT_LEFT, 120);

    list_clsid.SetExtendedStyle( list_clsid.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP );
	list_clsid.EnableWindow(FALSE);

    return TRUE;
}

void CFilterFromFile::OnClickedButtonBrowse()
{
	CString		filter;
	CString		filename;

	filter = _T("Filter-File (*.dll,*.ax)|*.dll;*.ax|All Files|*.*|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    INT_PTR ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		combo_file.SetWindowText(filename);
        OnChangeComboFile();
	}
}

void CFilterFromFile::OnClickedButtonClear()
{
    file_list.RemoveAll();
    clsid_list.RemoveAll();

    file_list.SaveList(_T("FilterFileCache"));
	clsid_list.SaveList(_T("FilterFileClsidCache"));

	combo_file.ResetContent();
	combo_clsid.ResetContent();
}


void CFilterFromFile::OnChangeComboFile()
{
    combo_clsid.ResetContent();
    CString strFile;

    int sel = combo_file.GetCurSel();
    if(sel != CB_ERR)
    {
        int bufferLen = combo_file.GetLBTextLen(sel);
        combo_file.GetLBText(sel, strFile.GetBuffer(bufferLen));
    }
    else
    {
        combo_file.GetWindowText(strFile);
    }

	if (strFile != _T(""))
    {
        if(PathFileExists(strFile))
        {
            strFile = PathFindFileName(strFile);
            strFile.MakeLower();
            strFile.Append(_T("-"));
            for(int i=0; i<clsid_list.GetCount(); i++)
            {
                if(clsid_list[i].Left(strFile.GetLength()) == strFile)
                    combo_clsid.AddString(clsid_list[i].Mid(strFile.GetLength()));
            }

            combo_clsid.SetCurSel(0);
        }
    }
}

HRESULT CFilterFromFile::GetClassFactoryEntryPoint(LPCOLESTR dll_file, LPFNGETCLASSOBJECT & entry_point)
{
	entry_point = NULL;
	HRESULT hr = S_OK;
	if (!dll_file)
		return E_POINTER;

	SetLastError(0);
	HINSTANCE hLib = CoLoadLibrary(const_cast<LPOLESTR>(dll_file), TRUE);
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

void CFilterFromFile::OnBnClickedButtonScanClsids()
{
	LPFNGETCLASSOBJECT entry_point = NULL;
	CString dll_file;
	combo_file.GetWindowText(dll_file);
	HRESULT hr = GetClassFactoryEntryPoint(T2COLE(dll_file), entry_point); 
	if (FAILED(hr)) {
		DSUtil::ShowError(hr, _T("Error getting DLL entry point"));
		return;
	}

	list_clsid.EnableWindow(FALSE);
	list_clsid.DeleteAllItems();

	CAtlMap<CLSID, CString> matched_clsids;

	HKEY hKey = NULL;
	if( RegOpenKeyEx( HKEY_CLASSES_ROOT,
		TEXT("CLSID"),
		0,
		KEY_READ,
		&hKey) == ERROR_SUCCESS)
	{
		DWORD    num_sub_keys=0;
		DWORD    longest_subkey=0;

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

			for (DWORD i=0; i<num_sub_keys; i++) 
			{ 
				subkey_name_size = MAX_KEY_LENGTH;
				if (ERROR_SUCCESS == RegEnumKeyEx(hKey, i, subkey_name, &subkey_name_size, NULL, NULL, NULL, NULL)
						&& SUCCEEDED(CLSIDFromString(subkey_name, &subkey_clsid))
						&& SUCCEEDED(entry_point(subkey_clsid, __uuidof(IClassFactory), (void**)&factory))) 
				{
					// Get name of COM object
					CString clsid_name;
					const int MAX_NAME_LENGTH = 256;
					OLECHAR * const name_buffer = clsid_name.GetBufferSetLength(MAX_NAME_LENGTH);
					DWORD name_size = MAX_NAME_LENGTH;
					LSTATUS status = RegGetValue(hKey, subkey_name, NULL, RRF_RT_REG_SZ, NULL, name_buffer, &name_size);
					clsid_name.ReleaseBuffer(name_size/sizeof(*name_buffer));

					matched_clsids.SetAt(subkey_clsid, clsid_name);
				}
				if (factory) {
					factory->Release();
					factory = NULL;
				}
			}
		} 
	}
	RegCloseKey(hKey);

	// Find CLSIDs of all coclasses in type library

	CComPtr<ITypeLib> typelib;
	if (SUCCEEDED(LoadTypeLibEx(T2COLE(dll_file), REGKIND_NONE, &typelib)) && typelib) 
	{
		CComPtr<ITypeInfo> typeinfo;
		for (UINT i = 0;i < typelib->GetTypeInfoCount();++i) {
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
					matched_clsids.SetAt(typeattr->guid, guid_name);
				}
				if (factory)
					factory->Release();
			}
		}
	}

	CLSID clsid;
	CString clsid_description;
	POSITION pos = matched_clsids.GetStartPosition();
	int index = 0;
	while (matched_clsids.GetNextAssoc(pos, clsid, clsid_description), pos) {
		CString clsid_string;
		CLSIDToString(clsid, clsid_string);
		const int item = list_clsid.InsertItem(index++, clsid_string);
		list_clsid.SetItemText(item, 1, clsid_description);
	}
	if (list_clsid.GetItemCount() > 0) {
		list_clsid.EnableWindow(TRUE);
		list_clsid.SetColumnWidth(0, -1);
		const int width = list_clsid.GetColumnWidth(0);		// add some extra to make a small gap
		list_clsid.SetColumnWidth(0, width + 20);
		list_clsid.SetColumnWidth(1, -1);
	}
}

struct SortData
{
	CListCtrl *		list;
	int				column_index;
};

int CALLBACK ListCompare(LPARAM item1, LPARAM item2, LPARAM data)
{
	SortData * const sort_data = (SortData*)data;
	if (sort_data && sort_data->list) {

		const CString str1 = sort_data->list->GetItemText(item1, sort_data->column_index);
		const CString str2 = sort_data->list->GetItemText(item2, sort_data->column_index);

		if (str1 < str2)
			return -1;
		if (str2 < str1)
			return 1;
		return 0;
	}
	return 0;
}

void CFilterFromFile::OnLvnColumnclickListData(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	const SortData sort_data = { &list_clsid, pNMLV->iSubItem };
	list_clsid.SortItemsEx(ListCompare, (DWORD_PTR)&sort_data);

	*pResult = 0;
}

void CFilterFromFile::OnNMDblclkListData(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	OnOK();
	*pResult = 0;
}

void CFilterFromFile::OnLvnItemchangedListData(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if ((pNMLV->uNewState & LVIS_SELECTED) && 
			!(pNMLV->uOldState & LVIS_SELECTED)) {
		combo_clsid.SetWindowText(list_clsid.GetItemText(pNMLV->iItem, 0));
	}

	*pResult = 0;
}

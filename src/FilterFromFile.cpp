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
#include "ComDllAnalyzer.h"""

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
                if (SUCCEEDED(hr))
                {
					CString error_msg;
					hr = DSUtil::GetClassFactoryFromDll(T2COLE(result_file), result_clsid, &filterFactory, error_msg);
					if (FAILED(hr)) {
						DSUtil::ShowError(hr, error_msg);
						return;
					}

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

void CFilterFromFile::OnBnClickedButtonScanClsids()
{
	LPFNGETCLASSOBJECT entry_point = NULL;
	CString dll_file;
	combo_file.GetWindowText(dll_file);

	CComDllAnalyzer comDllAnalyzer(dll_file);

	if (FAILED(comDllAnalyzer.errorHr))
	{
		DSUtil::ShowError(comDllAnalyzer.errorHr, comDllAnalyzer.errorMsg);
		return;
	}

	list_clsid.EnableWindow(FALSE);
	list_clsid.DeleteAllItems();

	int index = 0;
	POSITION pos = comDllAnalyzer.clsids.GetStartPosition();
	while (pos)
	{
		CLSID clsid;
		CString clsid_description;
		comDllAnalyzer.clsids.GetNextAssoc(pos, clsid, clsid_description);

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

		const CString str1 = sort_data->list->GetItemText((int) item1, sort_data->column_index);
		const CString str2 = sort_data->list->GetItemText((int) item2, sort_data->column_index);

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
		result_name = list_clsid.GetItemText(pNMLV->iItem, 1);
	}

	*pResult = 0;
}

//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : Christian Gräfe
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FilterFromFile.h"


// CFilterFromFile-Dialogfeld

IMPLEMENT_DYNAMIC(CFilterFromFile, CDialog)

CFilterFromFile::CFilterFromFile(CWnd* pParent /*=NULL*/)
: CDialog(CFilterFromFile::IDD, pParent), result_clsid(CLSID_NULL), filterFactory(NULL)
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
}


BEGIN_MESSAGE_MAP(CFilterFromFile, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CFilterFromFile::OnClickedButtonBrowse)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CFilterFromFile::OnClickedButtonClear)
    ON_CBN_EDITCHANGE(IDC_COMBO_FILE, &CFilterFromFile::OnChangeComboFile)
    ON_CBN_SELCHANGE(IDC_COMBO_FILE, &CFilterFromFile::OnChangeComboFile)
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

                HRESULT hr = CLSIDFromString(strClsid, &result_clsid);
                if(SUCCEEDED(hr))
                {
                    SetLastError(0);
			        HINSTANCE hLib = CoLoadLibrary(const_cast<LPOLESTR>(T2COLE(result_file)), TRUE);
                    LPFNGETCLASSOBJECT pfnGetClassObject = NULL;
			        if (hLib != NULL)
				        pfnGetClassObject = (LPFNGETCLASSOBJECT)GetProcAddress(hLib, "DllGetClassObject");

			        if (hLib == NULL || pfnGetClassObject == NULL)
			        {
				        DWORD dwError = GetLastError();
				        if (dwError != 0)
					        hr = HRESULT_FROM_WIN32(dwError);
				        else
					        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DLL);
			        }

                    if(FAILED(hr))
                    {
                        CString msg;
                        msg.Format(_T("Error loading library (hr = 0x%08x)"), hr);
                        MessageBox(msg, _T("Error"), MB_ICONERROR);
                        return;
                    }

                    hr = pfnGetClassObject(result_clsid, IID_IClassFactory, (void**)&filterFactory);
                    if(FAILED(hr))
                    {
                        CString msg;
                        msg.Format(_T("Error getting IClassFactory for filter (hr = 0x%08x)"), hr);
                        MessageBox(msg, _T("Error"), MB_ICONERROR);
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
                    MessageBox(_T("CLSID - Parsing Error"), _T("Error"), MB_ICONERROR);
                    result_clsid = CLSID_NULL;
                    return;
                }
		    }
            else
            {
                MessageBox(_T("No CLSID provided"), _T("Error"), MB_ICONERROR);
                return;
            }
        }
        else
        {
            result_file.Empty();
            MessageBox(_T("Can not find specified file"), _T("Error"), MB_ICONERROR);
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

    combo_file.SetCurSel(sel);
    OnChangeComboFile();

    return TRUE;
}


void CFilterFromFile::OnClickedButtonBrowse()
{
	CString		filter;
	CString		filename;

	filter = _T("Filter-File (*.dll,*.ax)|*.dll;*.ax|All Files|*.*|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    int ret = dlg.DoModal();

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

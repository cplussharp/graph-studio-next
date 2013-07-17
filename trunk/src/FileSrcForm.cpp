//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FileSrcForm.h"


//-----------------------------------------------------------------------------
//
//	CFileSrcForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CFileSrcForm, CDialog)
BEGIN_MESSAGE_MAP(CFileSrcForm, CDialog)
	ON_BN_CLICKED(IDC_RADIO_FILE, &CFileSrcForm::OnBnClickedRadioFile)
	ON_BN_CLICKED(IDC_RADIO_URL, &CFileSrcForm::OnBnClickedRadioUrl)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CFileSrcForm::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CFileSrcForm::OnBnClickedButtonClear)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFileSrcForm class
//
//-----------------------------------------------------------------------------

CFileSrcForm::CFileSrcForm(CWnd* pParent)	: 
	CDialog(CFileSrcForm::IDD, pParent)
{

}

CFileSrcForm::CFileSrcForm(const CString& filterName, CWnd* pParent)	: 
	CDialog(CFileSrcForm::IDD, pParent), filter_name(filterName)
{

}

CFileSrcForm::~CFileSrcForm()
{
}

void CFileSrcForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, button_browse);
	DDX_Control(pDX, IDC_RADIO_FILE, radio_file);
	DDX_Control(pDX, IDC_RADIO_URL, radio_url);
	DDX_Control(pDX, IDC_COMBO_FILE, combo_file);
	DDX_Control(pDX, IDC_COMBO_URL, combo_url);
}

BOOL CFileSrcForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

    if(filter_name.GetLength() > 0)
    {
        CString strTitle;
        GetWindowText(strTitle);
        strTitle += _T(" (");
        strTitle += filter_name;
        strTitle += _T(")");
        SetWindowText(strTitle);
    }

	if (result_file.GetLength() > 0) {
		combo_file.SetWindowText(result_file);
		combo_url.SetWindowText(result_file);
	}

	// load saved lists
	file_list.LoadList(_T("FileCache"));
	url_list.LoadList(_T("URLCache"));

	int i;
	for (i=0; i<file_list.GetCount(); i++) combo_file.AddString(file_list[i]);
	for (i=0; i<url_list.GetCount(); i++) combo_url.AddString(url_list[i]);

	OnBnClickedRadioFile();
	GotoDlgCtrl(&combo_file);
	return TRUE;
}

void CFileSrcForm::OnBnClickedRadioFile()
{
	radio_url.SetCheck(FALSE);
	radio_file.SetCheck(TRUE);
	combo_url.EnableWindow(FALSE);
	combo_file.EnableWindow(TRUE);
	button_browse.EnableWindow(TRUE);
	GotoDlgCtrl(&combo_file);
}

void CFileSrcForm::OnBnClickedRadioUrl()
{
	radio_url.SetCheck(TRUE);
	radio_file.SetCheck(FALSE);
	combo_url.EnableWindow(TRUE);
	combo_file.EnableWindow(FALSE);
	button_browse.EnableWindow(FALSE);
	GotoDlgCtrl(&combo_url);
}

void CFileSrcForm::OnBnClickedButtonBrowse()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter = _T("All Files|*.*|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		combo_file.SetWindowText(filename);
	}
}

void CFileSrcForm::OnOK()
{
	if (radio_url.GetCheck()) {
		combo_url.GetWindowText(result_file);

		if (result_file != _T("")) {
			url_list.UpdateList(result_file);
			url_list.SaveList(_T("URLCache"));
		}
	} else {
		combo_file.GetWindowText(result_file);
		if (result_file != _T("")) {
			file_list.UpdateList(result_file);
			file_list.SaveList(_T("FileCache"));
		}
	}

	EndDialog(IDOK);
}

void CFileSrcForm::OnBnClickedButtonClear()
{
	url_list.RemoveAll();
	file_list.RemoveAll();

	url_list.SaveList(_T("URLCache"));
	file_list.SaveList(_T("FileCache"));

	combo_file.ResetContent();
	combo_url.ResetContent();
}

// static helper to show dialog and set file source
HRESULT CFileSrcForm::ChooseSourceFile(IFileSourceFilter* fs)
{
	HRESULT hr = S_FALSE;		// return S_FALSE if user cancelled

	if (!fs)
		return E_POINTER;
	
	LPOLESTR curFile = NULL;
	CMediaType media_type;
	if (SUCCEEDED(fs->GetCurFile(&curFile, &media_type)) && curFile) {
		result_file = curFile;
	}
	if (curFile) {
		CoTaskMemFree(curFile);
		curFile = NULL;
	}

	if (DoModal() == IDOK) {
		hr = fs->Load((LPCOLESTR)result_file, NULL);
		DSUtil::ShowError(hr, _T("Can't load specified file"));
	}
	return hr;
}


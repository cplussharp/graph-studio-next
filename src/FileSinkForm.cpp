//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FileSinkForm.h"


//-----------------------------------------------------------------------------
//
//	CFileSinkForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CFileSinkForm, CDialog)
BEGIN_MESSAGE_MAP(CFileSinkForm, CDialog)
	ON_BN_CLICKED(IDC_RADIO_FILE, &CFileSinkForm::OnBnClickedRadioFile)
	ON_BN_CLICKED(IDC_RADIO_URL, &CFileSinkForm::OnBnClickedRadioUrl)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CFileSinkForm::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CFileSinkForm::OnBnClickedButtonClear)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFileSinkForm class
//
//-----------------------------------------------------------------------------

CFileSinkForm::CFileSinkForm(CWnd* pParent)	: 
	CDialog(CFileSinkForm::IDD, pParent)
{

}

CFileSinkForm::CFileSinkForm(const CString& filterName, CWnd* pParent)	: 
    CDialog(CFileSinkForm::IDD, pParent), filter_name(filterName)
{

}

CFileSinkForm::~CFileSinkForm()
{
}

void CFileSinkForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, button_browse);
	DDX_Control(pDX, IDC_RADIO_FILE, radio_file);
	DDX_Control(pDX, IDC_RADIO_URL, radio_url);
	DDX_Control(pDX, IDC_COMBO_FILE, combo_file);
	DDX_Control(pDX, IDC_COMBO_URL, combo_url);
}

BOOL CFileSinkForm::OnInitDialog()
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
	file_list.LoadList(_T("Sink-FileCache"));
	url_list.LoadList(_T("Sink-URLCache"));

	int i;
	for (i=0; i<file_list.GetCount(); i++) combo_file.AddString(file_list[i]);
	for (i=0; i<url_list.GetCount(); i++) combo_url.AddString(url_list[i]);

	OnBnClickedRadioFile();
	return TRUE;
}

void CFileSinkForm::OnBnClickedRadioFile()
{
	radio_url.SetCheck(FALSE);
	radio_file.SetCheck(TRUE);
	combo_url.EnableWindow(FALSE);
	combo_file.EnableWindow(TRUE);
	button_browse.EnableWindow(TRUE);
}

void CFileSinkForm::OnBnClickedRadioUrl()
{
	radio_url.SetCheck(TRUE);
	radio_file.SetCheck(FALSE);
	combo_url.EnableWindow(TRUE);
	combo_file.EnableWindow(FALSE);
	button_browse.EnableWindow(FALSE);
}

void CFileSinkForm::OnBnClickedButtonBrowse()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter = _T("All Files|*.*|");

	CFileDialog dlg(FALSE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		combo_file.SetWindowText(filename);
	}
}

void CFileSinkForm::OnOK()
{
	if (radio_url.GetCheck()) {
		combo_url.GetWindowText(result_file);

		if (result_file != _T("")) {
			url_list.UpdateList(result_file);
			url_list.SaveList(_T("Sink-URLCache"));
		}
	} else {
		combo_file.GetWindowText(result_file);
		if (result_file != _T("")) {
			file_list.UpdateList(result_file);
			file_list.SaveList(_T("Sink-FileCache"));
		}
	}

	EndDialog(IDOK);
}

void CFileSinkForm::OnBnClickedButtonClear()
{
	url_list.RemoveAll();
	file_list.RemoveAll();

	url_list.SaveList(_T("Sink-URLCache"));
	file_list.SaveList(_T("Sink-FileCache"));

	combo_file.ResetContent();
	combo_url.ResetContent();
}

HRESULT CFileSinkForm::ChooseSinkFile(IFileSinkFilter* fsink, const CString& filterName)
{
	HRESULT hr = S_FALSE;		// return S_FALSE if user cancelled
	if (!fsink)
		return E_POINTER;

	CFileSinkForm		sink_form(filterName);

	LPOLESTR curFile = NULL;
	CMediaType media_type;
	if (SUCCEEDED(fsink->GetCurFile(&curFile, &media_type)) && curFile) {
		sink_form.result_file = curFile;
	}
	if (curFile) {
		CoTaskMemFree(curFile);
		curFile = NULL;
	}

	if (sink_form.DoModal() == IDOK) {
		hr = fsink->SetFileName((LPCOLESTR)sink_form.result_file, NULL);
		if (FAILED(hr)) {
			DSUtil::ShowError(_T("Can't write specified file"));
		}
	}
	return hr;
}

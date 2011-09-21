//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CRenderUrlForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CRenderUrlForm, CDialog)
BEGIN_MESSAGE_MAP(CRenderUrlForm, CDialog)
	ON_BN_CLICKED(IDC_RADIO_URL, &CRenderUrlForm::OnBnClickedRadioUrl)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CRenderUrlForm::OnBnClickedButtonClear)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CRenderUrlForm class
//
//-----------------------------------------------------------------------------

CRenderUrlForm::CRenderUrlForm(CWnd* pParent)	: 
	CDialog(CRenderUrlForm::IDD, pParent)
{

}

CRenderUrlForm::~CRenderUrlForm()
{
}

void CRenderUrlForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_RADIO_URL, radio_url);
	DDX_Control(pDX, IDC_COMBO_URL, combo_url);
}

BOOL CRenderUrlForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	// load saved lists
	url_list.LoadList(_T("MRU-URLCache"));

	int i;
	for (i=0; i<url_list.GetCount(); i++) combo_url.AddString(url_list[i]);

	OnBnClickedRadioUrl();
	return TRUE;
}

void CRenderUrlForm::OnBnClickedRadioUrl()
{
	radio_url.SetCheck(TRUE);
	combo_url.EnableWindow(TRUE);
}

void CRenderUrlForm::OnOK()
{
	combo_url.GetWindowText(result_file);

	if (result_file != _T("")) {
		url_list.UpdateList(result_file);
		url_list.SaveList(_T("MRU-URLCache"));
	}

	EndDialog(IDOK);
}

void CRenderUrlForm::OnBnClickedButtonClear()
{
	url_list.RemoveAll();
	url_list.SaveList(_T("MRU-URLCache"));
	combo_url.ResetContent();
}

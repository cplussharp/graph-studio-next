//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FileSrcForm.h"


//-----------------------------------------------------------------------------
//
//	CSbeConfigForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CSbeConfigForm, CDialog)
BEGIN_MESSAGE_MAP(CSbeConfigForm, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CSbeConfigForm::OnBnClickedButtonBrowse)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CSbeConfigForm class
//
//-----------------------------------------------------------------------------

CSbeConfigForm::CSbeConfigForm(CWnd* pParent)	: 
	CDialog(CSbeConfigForm::IDD, pParent)
        , m_strDir(_T(""))
        , m_nFileDuration(0)
        , m_nFileCountMin(0)
        , m_nFileCountMax(0)
    {

}

CSbeConfigForm::~CSbeConfigForm()
{
}

void CSbeConfigForm::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
    DDX_Text(pDX, IDC_EDIT_DIR, m_strDir);
    DDX_Text(pDX, IDC_EDIT_FILEDURATION, m_nFileDuration);
    DDX_Text(pDX, IDC_EDIT_FILECOUNT_MIN, m_nFileCountMin);
    DDX_Text(pDX, IDC_EDIT_FILECOUNT_MAX, m_nFileCountMax);
    DDX_Control(pDX, IDC_SPIN_FILECOUNT_MIN, m_spinFileCountMin);
    DDX_Control(pDX, IDC_SPIN_FILECOUNT_MAX, m_spinFileCountMax);
}

BOOL CSbeConfigForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

    HRESULT hr = m_pConfig.CoCreateInstance(CLSID_StreamBufferConfig);
    if(FAILED(hr))
    {
        DSUtil::ShowError(hr, _T("Can't create StreamBufferConfig object."));
        return FALSE;
    }

    // load current config
    CComQIPtr<IStreamBufferInitialize> pInit = m_pConfig;
    DSUtil::InitSbeObject(pInit);

    LPWSTR strDir = NULL;
    m_pConfig->GetDirectory(&strDir);
    if(strDir)
    {
        m_strDir = strDir;
        CoTaskMemFree(strDir);
    }

    m_pConfig->GetBackingFileDuration(&m_nFileDuration);
    m_pConfig->GetBackingFileCount(&m_nFileCountMin, &m_nFileCountMax);

    UpdateData(0);

	return TRUE;
}

void CSbeConfigForm::OnBnClickedButtonBrowse()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter = _T("All Files|*.*|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    int ret = dlg.DoModal();


}

void CSbeConfigForm::OnOK()
{
	

	EndDialog(IDOK);
}

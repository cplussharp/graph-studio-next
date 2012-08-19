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

    m_spinFileCountMin.SetRange32(4,100);
    m_spinFileCountMax.SetRange32(6,102);

	return TRUE;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	// If the BFFM_INITIALIZED message is received
	// set the path to the start path.
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			if (NULL != lpData)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}
		}
	}

	return 0; // The function should always return 0.
}

void CSbeConfigForm::OnBnClickedButtonBrowse()
{
	BROWSEINFO   bi; 
    ZeroMemory(&bi,   sizeof(bi)); 
    TCHAR szDisplayName[MAX_PATH];
    szDisplayName[0]    =   TCHAR(' ');  
    LPCTSTR szInitDir = m_strDir; 
    bi.hwndOwner        =   m_hWnd; 
    bi.pidlRoot         =   NULL; 
    bi.pszDisplayName   =   szDisplayName; 
    bi.lpszTitle        =   _T("Please select a folder for storing the temporary files :"); 
    bi.ulFlags          =   BIF_RETURNONLYFSDIRS|BIF_USENEWUI;
    bi.lParam           =   (LPARAM)szInitDir; 
    bi.lpfn             =   BrowseCallbackProc;
    bi.iImage           =   0;  

    LPITEMIDLIST   pidl   =   SHBrowseForFolder(&bi);
    TCHAR   szPathName[MAX_PATH]; 
    if (NULL != pidl)
    {
        BOOL bRet = SHGetPathFromIDList(pidl,szPathName);
        if(FALSE == bRet)
            return;
        m_strDir = szPathName;
        UpdateData(0);
    }
}

void CSbeConfigForm::OnOK()
{
    UpdateData(TRUE);

    if(m_nFileCountMax - m_nFileCountMin < 2)
    {
        DSUtil::ShowWarning(_T("The 'max file count' must at least be 'min file count' + 2!"),_T("Wrong parameters"));
        return;
    }

    HRESULT hr = m_pConfig->SetDirectory(m_strDir);
    if(FAILED(hr))
    {
        DSUtil::ShowError(hr, _T("Error Setting Directory"));
        return;
    }

    hr = m_pConfig->SetBackingFileDuration(m_nFileDuration);
    if(FAILED(hr))
    {
        DSUtil::ShowError(hr, _T("Error Setting Duration"));
        return;
    }

    hr = m_pConfig->SetBackingFileCount(m_nFileCountMin, m_nFileCountMax);
    if(FAILED(hr))
    {
        DSUtil::ShowError(hr, _T("Error Setting File Count"));
        return;
    }

	EndDialog(IDOK);
}

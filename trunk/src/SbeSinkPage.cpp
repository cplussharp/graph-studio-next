//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "WMResizerPage.h"
#include "SbeSinkPage.h"


//-----------------------------------------------------------------------------
//
//	CSbeSinkPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CSbeSinkPage, CDSPropertyPage)
	ON_WM_SIZE()
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CSbeSinkPage::OnBnClickedButtonBrowse)
    ON_BN_CLICKED(IDC_BUTTON_LOCK, &CSbeSinkPage::OnBnClickedButtonLock)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CSbeSinkPage class
//
//-----------------------------------------------------------------------------
CSbeSinkPage::CSbeSinkPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("SbeSinkPage"), pUnk, IDD, strTitle), isActiv(false)
{
	// retval
	if (phr) *phr = NOERROR;
	filter = NULL;
}

CSbeSinkPage::~CSbeSinkPage()
{
	filter = NULL;
}


BOOL CSbeSinkPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	return TRUE;
}

void CSbeSinkPage::OnSize(UINT nType, int cx, int cy)
{
}


void CSbeSinkPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
    DDX_Control(pDX, IDC_COMBO_FILE, combo_file);
}

HRESULT CSbeSinkPage::OnConnect(IUnknown *pUnknown)
{
    HRESULT hr = pUnknown->QueryInterface(__uuidof(IStreamBufferSink), (void**)&filter);
	if (FAILED(hr)) return E_FAIL;

	return NOERROR;
}

HRESULT CSbeSinkPage::OnActivate()
{
    isActiv = true;

	HRESULT hr = filter->IsProfileLocked();
    if(hr == S_OK)
    {
        EnableWindow(FALSE);
        return NOERROR;
    }

    // load saved lists
	file_list.LoadList(_T("Sink-FileCache"));
    for (int i=0; i<file_list.GetCount(); i++) combo_file.AddString(file_list[i]);

	return NOERROR;
}

HRESULT CSbeSinkPage::OnDisconnect()
{
    isActiv = false;
	filter = NULL;
	return NOERROR;
}

HRESULT CSbeSinkPage::OnApplyChanges()
{
	return NOERROR;
}



void CSbeSinkPage::OnBnClickedButtonBrowse()
{
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


void CSbeSinkPage::OnBnClickedButtonLock()
{
    CString result_file;
    combo_file.GetWindowText(result_file);
	if (result_file != _T(""))
    {
        HRESULT hr = filter->LockProfile(result_file);
        if(FAILED(hr))
        {
            DSUtil::ShowError(hr, _T("Can't lock profile"));
            return;
        }

        file_list.UpdateList(result_file);
		file_list.SaveList(_T("Sink-FileCache"));

        EnableWindow(FALSE);
	}
}

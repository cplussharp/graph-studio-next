//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CDMOQualCtrlPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CDMOQualCtrlPage, CDSPropertyPage)
	ON_WM_SIZE()
    ON_COMMAND(IDC_CHECK_QUALCTRL, &CDMOQualCtrlPage::OnCheckOrRadioClick)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CDMOQualCtrlPage class
//
//-----------------------------------------------------------------------------
CDMOQualCtrlPage::CDMOQualCtrlPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("DMOQualCtrlPage"), pUnk, IDD, strTitle), isActiv(false)
{
	// retval
	if (phr) *phr = NOERROR;
	dmo = NULL;
}

CDMOQualCtrlPage::~CDMOQualCtrlPage()
{
	dmo = NULL;
}


BOOL CDMOQualCtrlPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	return TRUE;
}

void CDMOQualCtrlPage::OnSize(UINT nType, int cx, int cy)
{
}


void CDMOQualCtrlPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
}

HRESULT CDMOQualCtrlPage::OnConnect(IUnknown *pUnknown)
{
    HRESULT hr = pUnknown->QueryInterface(IID_IDMOQualityControl, (void**)&dmo);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CDMOQualCtrlPage::OnActivate()
{
    DWORD val;
    if(SUCCEEDED(dmo->GetStatus(&val)))
        CheckDlgButton(IDC_CHECK_QUALCTRL, val == DMO_QUALITY_STATUS_ENABLED ? BST_CHECKED : BST_UNCHECKED);

    isActiv = true;

	return NOERROR;
}

HRESULT CDMOQualCtrlPage::OnDisconnect()
{
    isActiv = false;
	dmo = NULL;
	return NOERROR;
}

HRESULT CDMOQualCtrlPage::OnApplyChanges()
{
    dmo->SetStatus(IsDlgButtonChecked(IDC_CHECK_QUALCTRL) ?  DMO_QUALITY_STATUS_ENABLED : 0);

	return NOERROR;
}

void CDMOQualCtrlPage::OnCheckOrRadioClick()
{
    if(isActiv)
	    SetDirty();
}


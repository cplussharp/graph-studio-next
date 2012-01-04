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
//	CAMStreamSelectPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAMStreamSelectPage, CDSPropertyPage)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CAMStreamSelectPage class
//
//-----------------------------------------------------------------------------
CAMStreamSelectPage::CAMStreamSelectPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("AMStreamSelectPage"), pUnk, IDD, strTitle), isActiv(false)
{
	// retval
	if (phr) *phr = NOERROR;
	filter = NULL;
}

CAMStreamSelectPage::~CAMStreamSelectPage()
{
	filter = NULL;
}


BOOL CAMStreamSelectPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	return TRUE;
}

void CAMStreamSelectPage::OnSize(UINT nType, int cx, int cy)
{
}


void CAMStreamSelectPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
}

HRESULT CAMStreamSelectPage::OnConnect(IUnknown *pUnknown)
{
    HRESULT hr = pUnknown->QueryInterface(IID_IAMStreamSelect, (void**)&filter);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CAMStreamSelectPage::OnActivate()
{
    isActiv = true;

	return NOERROR;
}

HRESULT CAMStreamSelectPage::OnDisconnect()
{
    isActiv = false;
	filter = NULL;
	return NOERROR;
}

HRESULT CAMStreamSelectPage::OnApplyChanges()
{
	return NOERROR;
}

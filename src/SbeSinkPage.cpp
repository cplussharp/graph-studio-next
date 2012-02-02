//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "WMResizerPage.h"


//-----------------------------------------------------------------------------
//
//	CSbeSinkPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CSbeSinkPage, CDSPropertyPage)
	ON_WM_SIZE()
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
}

HRESULT CSbeSinkPage::OnConnect(IUnknown *pUnknown)
{
    HRESULT hr = pUnknown->QueryInterface(__uuidof(IStreamBufferSink), (void**)&filter);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CSbeSinkPage::OnActivate()
{
	// Read values

    isActiv = true;

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


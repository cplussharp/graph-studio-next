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
//	CWMADecPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CWMADecPage, CDSPropertyPage)
	ON_WM_SIZE()
	ON_COMMAND(IDC_CHECK_HIRES, &CWMADecPage::OnHiResClick)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CWMADecPage class
//
//-----------------------------------------------------------------------------
CWMADecPage::CWMADecPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("WMADecPage"), pUnk, IDD, strTitle)
{
	// retval
	if (phr) *phr = NOERROR;
	dmo = NULL;
}

CWMADecPage::~CWMADecPage()
{
	dmo = NULL;
}


BOOL CWMADecPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	return TRUE;
}

void CWMADecPage::OnSize(UINT nType, int cx, int cy)
{
}


void CWMADecPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_CHECK_HIRES, check_hires);
}

HRESULT CWMADecPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IMediaObject, (void**)&dmo);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CWMADecPage::OnActivate()
{
	// read the _HIRESOUTPUT value
	CComPtr<IPropertyBag>		pbag;
	HRESULT hr = dmo->QueryInterface(IID_IPropertyBag, (void**)&pbag);
	if (SUCCEEDED(hr)) {

		VARIANT varg;
		VariantInit(&varg);		
		pbag->Read(L"_HIRESOUTPUT", &varg, NULL);

		if (varg.vt == VT_BOOL) {
			check_hires.SetCheck(varg.boolVal);
		}

		pbag = NULL;
	}

	return NOERROR;
}

HRESULT CWMADecPage::OnDisconnect()
{
	dmo = NULL;
	return NOERROR;
}

HRESULT CWMADecPage::OnApplyChanges()
{
	// set the _HIRESOUTPUT value
	CComPtr<IPropertyBag>		pbag;
	HRESULT hr = dmo->QueryInterface(IID_IPropertyBag, (void**)&pbag);
	if (SUCCEEDED(hr)) {

		VARIANT varg;
		VariantInit(&varg);		

		varg.vt			= VT_BOOL;
		varg.boolVal	= check_hires.GetCheck();

		hr = pbag->Write(L"_HIRESOUTPUT", &varg);
		if (FAILED(hr)) { 
			return E_FAIL;
		}
		pbag = NULL;
	}

	return NOERROR;
}

void CWMADecPage::OnHiResClick()
{
	SetDirty();
}



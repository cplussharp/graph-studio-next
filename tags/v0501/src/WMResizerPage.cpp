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
//	CWMResizerPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CWMResizerPage, CDSPropertyPage)
	ON_WM_SIZE()
    ON_COMMAND(IDC_CHECK_INTERLACED, &CWMResizerPage::OnCheckOrRadioClick)
    ON_EN_CHANGE(IDC_EDIT_DST_HEIGHT, &CWMResizerPage::OnChangeEdit)
    ON_COMMAND(IDC_RADIO_QUAL_FAST, &CWMResizerPage::OnCheckOrRadioClick)
    ON_COMMAND(IDC_RADIO_QUAL_HIGH, &CWMResizerPage::OnCheckOrRadioClick)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CWMResizerPage class
//
//-----------------------------------------------------------------------------
CWMResizerPage::CWMResizerPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("WMResizerPage"), pUnk, IDD, strTitle), isActiv(false)
{
	// retval
	if (phr) *phr = NOERROR;
	dmo = NULL;
}

CWMResizerPage::~CWMResizerPage()
{
	dmo = NULL;
}


BOOL CWMResizerPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	return TRUE;
}

void CWMResizerPage::OnSize(UINT nType, int cx, int cy)
{
}


void CWMResizerPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
}

HRESULT CWMResizerPage::OnConnect(IUnknown *pUnknown)
{
    HRESULT hr = pUnknown->QueryInterface(IID_IWMResizerProps, (void**)&dmo);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CWMResizerPage::OnActivate()
{
	// Read values
    RECT src = {0}, dst = {0};
    dmo->GetFullCropRegion(&src.left, &src.top, &src.right, &src.bottom, &dst.left, &dst.top, &dst.right, &dst.bottom);
    SetDlgItemInt(IDC_EDIT_SRC_LEFT,    src.left);
    SetDlgItemInt(IDC_EDIT_SRC_TOP,     src.top);
    SetDlgItemInt(IDC_EDIT_SRC_WIDTH,   src.right);
    SetDlgItemInt(IDC_EDIT_SRC_HEIGHT,  src.bottom);
    SetDlgItemInt(IDC_EDIT_DST_LEFT,    dst.left);
    SetDlgItemInt(IDC_EDIT_DST_TOP,     dst.top);
    SetDlgItemInt(IDC_EDIT_DST_WIDTH,   dst.right);
    SetDlgItemInt(IDC_EDIT_DST_HEIGHT,  dst.bottom);

    CComPtr<IPropertyStore> propstore;
    HRESULT hr = dmo->QueryInterface(IID_IPropertyStore, (void**)&propstore);
    if(SUCCEEDED(hr))
    {
        PROPVARIANT var;
        propstore->GetValue(MFPKEY_RESIZE_INTERLACE, &var);
        if(var.vt == VT_BOOL)
            CheckDlgButton(IDC_CHECK_INTERLACED, var.boolVal ? BST_CHECKED : BST_UNCHECKED);

        propstore->GetValue(MFPKEY_RESIZE_QUALITY, &var);
        if(var.vt == VT_BOOL)
            CheckRadioButton(IDC_RADIO_QUAL_HIGH, IDC_RADIO_QUAL_FAST, var.boolVal ? IDC_RADIO_QUAL_HIGH : IDC_RADIO_QUAL_FAST); 
    }

    isActiv = true;

	return NOERROR;
}

HRESULT CWMResizerPage::OnDisconnect()
{
    isActiv = false;
	dmo = NULL;
	return NOERROR;
}

HRESULT CWMResizerPage::OnApplyChanges()
{
	RECT src,dst;
    src.left    = GetDlgItemInt(IDC_EDIT_SRC_LEFT);
    src.top     = GetDlgItemInt(IDC_EDIT_SRC_TOP);
    src.right   = GetDlgItemInt(IDC_EDIT_SRC_WIDTH);
    src.bottom  = GetDlgItemInt(IDC_EDIT_SRC_HEIGHT);
    dst.left    = GetDlgItemInt(IDC_EDIT_DST_LEFT);
    dst.top     = GetDlgItemInt(IDC_EDIT_DST_TOP);
    dst.right   = GetDlgItemInt(IDC_EDIT_DST_WIDTH);
    dst.bottom  = GetDlgItemInt(IDC_EDIT_DST_HEIGHT);

    HRESULT hr = dmo->SetFullCropRegion(src.left, src.top, src.right, src.bottom, dst.left, dst.top, dst.right, dst.bottom);

    hr = dmo->SetInterlaceMode(IsDlgButtonChecked(IDC_CHECK_INTERLACED));

    int qual = GetCheckedRadioButton(IDC_RADIO_QUAL_HIGH, IDC_RADIO_QUAL_FAST);
    hr = dmo->SetResizerQuality(qual == IDC_RADIO_QUAL_HIGH ? TRUE : FALSE);

	return NOERROR;
}

void CWMResizerPage::OnCheckOrRadioClick()
{
    if(isActiv)
	    SetDirty();
}

void CWMResizerPage::OnChangeEdit()
{
    if(isActiv)
        SetDirty();
}


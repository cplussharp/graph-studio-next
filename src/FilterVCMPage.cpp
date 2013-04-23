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
//	CFilterVCMPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFilterVCMPage, CDSPropertyPage)
	ON_WM_SIZE()
	ON_COMMAND(IDC_BUTTON_CONFIG, &CFilterVCMPage::OnConfigClick)
	ON_COMMAND(IDC_BUTTON_ABOUT, &CFilterVCMPage::OnAboutClick)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFilterVCMPage class
//
//-----------------------------------------------------------------------------
CFilterVCMPage::CFilterVCMPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("FilterVCMDialogs"), pUnk, IDD, strTitle)
{
	// retval
	if (phr) *phr = NOERROR;
	vfwdialogs = NULL;
}

CFilterVCMPage::~CFilterVCMPage()
{
	vfwdialogs = NULL;
}


// overriden
BOOL CFilterVCMPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	HRESULT hr;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	// now let's see if there are any dialogs at all
	hr = vfwdialogs->ShowDialog(VfwCompressDialog_QueryConfig, NULL);
	btn_config.EnableWindow( hr == NOERROR );
	hr = vfwdialogs->ShowDialog(VfwCompressDialog_QueryAbout, NULL);
	btn_about.EnableWindow( hr == NOERROR );

	return TRUE;
}

void CFilterVCMPage::OnSize(UINT nType, int cx, int cy)
{
}


void CFilterVCMPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_BUTTON_CONFIG, btn_config);
	DDX_Control(pDX, IDC_BUTTON_ABOUT, btn_about);
}

HRESULT CFilterVCMPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IAMVfwCompressDialogs, (void**)&vfwdialogs);
	if (FAILED(hr)) return E_FAIL;

	return NOERROR;
}

HRESULT CFilterVCMPage::OnDisconnect()
{
	vfwdialogs = NULL;
	return NOERROR;
}

void CFilterVCMPage::OnConfigClick()
{
	if (!vfwdialogs) return ;
	HRESULT hr = vfwdialogs->ShowDialog(VfwCompressDialog_Config, *AfxGetMainWnd());
	if (FAILED(hr)) {
		DSUtil::ShowError(hr, _T("Graph must be in stopped state"));
	}
}

void CFilterVCMPage::OnAboutClick()
{
	if (!vfwdialogs) return ;
	HRESULT hr = vfwdialogs->ShowDialog(VfwCompressDialog_About, *AfxGetMainWnd());
	if (FAILED(hr)) {
		DSUtil::ShowError(hr, _T("Graph must be in stopped state"));
	}
}




//-----------------------------------------------------------------------------
//
//	CVideoCompressionPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CVideoCompressionPage, CDSPropertyPage)
	ON_WM_HSCROLL()
	ON_COMMAND(IDC_BUTTON_DEF_IRATE, &CVideoCompressionPage::OnIRateDefault)
	ON_COMMAND(IDC_BUTTON_DEF_PRATE, &CVideoCompressionPage::OnPRateDefault)
	ON_COMMAND(IDC_BUTTON_DEF_QUALITY, &CVideoCompressionPage::OnQualityDefault)
	ON_COMMAND(IDC_BUTTON_DEF_WINDOW, &CVideoCompressionPage::OnWindowDefault)
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
//
//	CVideoCompressionPage class
//
//-----------------------------------------------------------------------------

CVideoCompressionPage::CVideoCompressionPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("VCMPage"), pUnk, IDD, strTitle)
{
	// retval
	if (phr) *phr = NOERROR;
	comp = NULL;
}

CVideoCompressionPage::~CVideoCompressionPage()
{
	comp = NULL;
}

void CVideoCompressionPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_STATIC_DESC, label_desc);
	DDX_Control(pDX, IDC_STATIC_VERSION, label_version);

	DDX_Control(pDX, IDC_SLIDER_IRATE, tb_irate);
	DDX_Control(pDX, IDC_SLIDER_PRATE, tb_prate);
	DDX_Control(pDX, IDC_SLIDER_QUALITY, tb_quality);
	DDX_Control(pDX, IDC_SLIDER_WINDOW, tb_window);

	DDX_Control(pDX, IDC_STATIC_IRATE, label_irate);
	DDX_Control(pDX, IDC_STATIC_VAL_IRATE, label_irate_val);
	DDX_Control(pDX, IDC_STATIC_PRATE, label_prate);
	DDX_Control(pDX, IDC_STATIC_VAL_PRATE, label_prate_val);
	DDX_Control(pDX, IDC_STATIC_QUALITY, label_quality);
	DDX_Control(pDX, IDC_STATIC_VAL_QUALITY, label_quality_val);
	DDX_Control(pDX, IDC_STATIC_WINDOW, label_window);
	DDX_Control(pDX, IDC_STATIC_VAL_WINDOW, label_window_val);

	DDX_Control(pDX, IDC_BUTTON_DEF_IRATE, btn_def_irate);
	DDX_Control(pDX, IDC_BUTTON_DEF_PRATE, btn_def_prate);
	DDX_Control(pDX, IDC_BUTTON_DEF_QUALITY, btn_def_quality);
	DDX_Control(pDX, IDC_BUTTON_DEF_WINDOW, btn_def_window);
}

// overriden
BOOL CVideoCompressionPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	WCHAR		wstrDesc[1024];
	WCHAR		wstrVer[1024];
	int			cbDesc, cbVer;

	memset(wstrDesc, 0, sizeof(wstrDesc));
	memset(wstrVer, 0, sizeof(wstrVer));

	cbDesc = 1024;
	cbVer = 1024;

	comp->GetInfo(wstrVer, &cbVer, wstrDesc, &cbDesc,
				  &def_irate, &def_prate, &def_quality, 
				  &flags);
	def_window = 1;

	// keep the strings
	desc = CString(wstrDesc);
	version = CString(wstrVer);

	label_desc.SetWindowText(desc);
	label_version.SetWindowText(version);

	tb_irate.SetRange(-1, 250);
	tb_prate.SetRange(-1, 250);
	tb_quality.SetRange(0, 100);
	tb_window.SetRange(-1, 100);

	UpdateIRate();
	UpdatePRate();
	UpdateQuality();
	UpdateWindowSize();

	bool		kf = (flags & CompressionCaps_CanKeyFrame) == CompressionCaps_CanKeyFrame;
	bool		bf = (flags & CompressionCaps_CanBFrame) == CompressionCaps_CanBFrame;
	bool		cq = (flags & CompressionCaps_CanQuality) == CompressionCaps_CanQuality;
	bool		cw = (flags & CompressionCaps_CanWindow) == CompressionCaps_CanWindow;

	label_irate.EnableWindow(kf);	tb_irate.EnableWindow(kf);		label_irate_val.EnableWindow(kf);	btn_def_irate.EnableWindow(kf);
	label_prate.EnableWindow(bf);	tb_prate.EnableWindow(bf);		label_prate_val.EnableWindow(bf);	btn_def_prate.EnableWindow(bf);
	label_quality.EnableWindow(cq);	tb_quality.EnableWindow(cq);	label_quality_val.EnableWindow(cq);	btn_def_quality.EnableWindow(cq);
	label_window.EnableWindow(cw);	tb_window.EnableWindow(cw);		label_window_val.EnableWindow(cw);	btn_def_window.EnableWindow(cw);

	return TRUE;
}


HRESULT CVideoCompressionPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IAMVideoCompression, (void**)&comp);
	if (FAILED(hr)) return E_FAIL;

	return NOERROR;
}

HRESULT CVideoCompressionPage::OnDisconnect()
{
	comp = NULL;
	return NOERROR;
}


void CVideoCompressionPage::UpdateIRate()
{
	long		irate;

	comp->get_KeyFrameRate(&irate);
	tb_irate.SetPos(irate);

	CString		t;
	if (irate < 0) {
		t = _T("Default");
	} else {
		t.Format(_T("%d"), irate);
	}

	label_irate_val.SetWindowText(t);
}

void CVideoCompressionPage::UpdatePRate()
{
	long		prate;

	comp->get_PFramesPerKeyFrame(&prate);
	tb_prate.SetPos(prate);

	CString		t;
	if (prate < 0) {
		t = _T("Default");
	} else {
		t.Format(_T("%d"), prate);
	}

	label_prate_val.SetWindowText(t);
}

void CVideoCompressionPage::UpdateQuality()
{
	double		qual;

	comp->get_Quality(&qual);
	int	q = (int)(100.0 * qual);
	q = (q < 0 ? 0 : q > 100 ? 100 : q);
	tb_quality.SetPos(q);

	CString		t;
	if (qual < 0) {
		t = _T("Default");
	} else {
		t.Format(_T("%4.2f"), (float)qual);
	}
	label_quality_val.SetWindowText(t);
}

void CVideoCompressionPage::UpdateWindowSize()
{
	DWORDLONG	wndsize;
	comp->get_WindowSize(&wndsize);
	tb_window.SetPos(wndsize);

	CString		t;
	t.Format(_T("%d"), (int)wndsize);
	label_window_val.SetWindowText(t);
}

void CVideoCompressionPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	__super::OnHScroll(nSBCode, nPos, pScrollBar);
	if (!comp) return ;

	CSliderCtrl		*slider = (CSliderCtrl*)pScrollBar;
	long			pos;

	if (slider == &tb_irate) {
		pos = slider->GetPos();
		comp->put_KeyFrameRate(pos);
		UpdateIRate();
	} else
	if (slider == &tb_prate) {
		pos = slider->GetPos();
		comp->put_PFramesPerKeyFrame(pos);
		UpdatePRate();
	} else
	if (slider == &tb_quality) {
		pos = slider->GetPos();
		double	q = pos / 100.0;
		comp->put_Quality(q);
		UpdateQuality();
	} else
	if (slider == &tb_window) {
		pos = slider->GetPos();
		DWORDLONG	lpos = pos;
		comp->put_WindowSize(lpos);
		UpdateWindowSize();
	}
}

void CVideoCompressionPage::OnIRateDefault()
{
	comp->put_KeyFrameRate(-1);
	UpdateIRate();
}

void CVideoCompressionPage::OnPRateDefault()
{
	comp->put_PFramesPerKeyFrame(-1);
	UpdatePRate();
}

void CVideoCompressionPage::OnQualityDefault()
{
	comp->put_Quality(-1);
	UpdateQuality();
}

void CVideoCompressionPage::OnWindowDefault()
{
	comp->put_WindowSize(def_window);
	UpdateQuality();
}










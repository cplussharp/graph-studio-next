//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <math.h>

//-----------------------------------------------------------------------------
//
//	CVolumeBarForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CVolumeBarForm, CDialog)

BEGIN_MESSAGE_MAP(CVolumeBarForm, CDialog)
	ON_WM_ACTIVATE()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CVolumeBarForm class
//
//-----------------------------------------------------------------------------

CVolumeBarForm::CVolumeBarForm(CWnd* pParent)	: 
	CDialog(CVolumeBarForm::IDD, pParent),
	basic_audio(NULL)
{

}

CVolumeBarForm::~CVolumeBarForm()
{
}

void CVolumeBarForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, slider_volume);
	DDX_Control(pDX, IDC_SLIDER_BALANCE, slider_balance);
	DDX_Control(pDX, IDC_STATIC_VOLUME, label_volume);
	DDX_Control(pDX, IDC_STATIC_BALANCE, label_balance);
}

BOOL CVolumeBarForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);
	if (!ret) return FALSE;

	slider_volume.SetRange(0, 100);
	slider_balance.SetRange(0, 20);
	return TRUE;
}

void CVolumeBarForm::DoHide()
{
	ShowWindow(SW_HIDE);
	basic_audio = NULL;
}

void CVolumeBarForm::DisplayVolume(IBaseFilter *filter)
{
	if (!filter) {
		DoHide();
		return ;
	}

	// release what we might be holding
	basic_audio = NULL;

	HRESULT	hr = filter->QueryInterface(IID_IBasicAudio, (void**)&basic_audio);
	if (FAILED(hr)) {
		DoHide();
		return ;
	}

	// now do something
	RefreshLevels();

	POINT		pt;
	GetCursorPos(&pt);
	SetWindowPos(&CWnd::wndTop, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	SetForegroundWindow();
	UpdateLevels();
}

void CVolumeBarForm::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
	if (nState == WA_INACTIVE) {
		DoHide();
	}
}

void CVolumeBarForm::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	UpdateLevels();
}

void CVolumeBarForm::RefreshLevels()
{
	long		lv = 0, lb = 0;

	if (basic_audio) {
		basic_audio->get_Volume(&lv);
		basic_audio->get_Balance(&lb);
	}

	double	r = 1.0 - (pow(abs((double)lv / 10000.0), 0.25));
	int	vpos  = (int)(r * 100.0);
	if (vpos < 0) vpos = 0;
	if (vpos > 100) vpos = 100;

	slider_volume.SetPos(vpos);

	r = 10 + (lb / 1000.0);
	int bpos  = (int)r;
	if (bpos < 0) bpos = 0;
	if (bpos > 20) bpos = 20;
	slider_balance.SetPos(bpos);
}

void CVolumeBarForm::UpdateLevels()
{
	int		vpos = slider_volume.GetPos();
	int		bpos = slider_balance.GetPos();

	long	bperc = ((bpos - 10) * 100) / 10;
	CString	t;

	if (bperc == 0) {
		t = _T("Center");
	} else {
		t.Format(_T("%+d%%"), bperc);
	}
	label_balance.SetWindowText(t);

	t.Format(_T("%d%%"), vpos);
	label_volume.SetWindowText(t);

	if (basic_audio) {		
		double	vr = vpos / 100.0;
		double	v = 0 - (pow(1.0 - vr, 4) * 10000.0);
		long	lv = (long)v;

		basic_audio->put_Volume((long)v);

		lv = bperc * 100;
		basic_audio->put_Balance(lv);
	}
}


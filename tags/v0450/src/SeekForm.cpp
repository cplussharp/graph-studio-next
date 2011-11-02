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
//	CSeekForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CSeekForm, CDialog)
BEGIN_MESSAGE_MAP(CSeekForm, CDialog)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(IDC_RADIO_TIME, &CSeekForm::OnTimeClick)
	ON_COMMAND(IDC_RADIO_FRAME, &CSeekForm::OnFrameClick)

END_MESSAGE_MAP()

struct VALNAME
{
	DWORD		flag;
	LPCTSTR		name;
};

// our temp values
enum
{
	FLAG_FORMAT_FRAME		= 0x200,
	FLAG_FORMAT_SAMPLE		= 0x400,
	FLAG_FORMAT_FIELD		= 0x800,
	FLAG_FORMAT_BYTE		= 0x1000,
	FLAG_FORMAT_MEDIA_TIME	= 0x2000,
};

const VALNAME		CapsFlags[] = 
{
	{	AM_SEEKING_CanSeekAbsolute,		_T("AM_SEEKING_CanSeekAbsolute") },
	{	AM_SEEKING_CanSeekForwards,		_T("AM_SEEKING_CanSeekForwards") },
	{	AM_SEEKING_CanSeekBackwards,	_T("AM_SEEKING_CanSeekBackwards") },
	{	AM_SEEKING_CanGetCurrentPos,	_T("AM_SEEKING_CanGetCurrentPos") },	
	{	AM_SEEKING_CanGetStopPos,		_T("AM_SEEKING_CanGetStopPos") },
	{	AM_SEEKING_CanGetDuration,		_T("AM_SEEKING_CanGetDuration") },
	{	AM_SEEKING_CanPlayBackwards,	_T("AM_SEEKING_CanPlayBackwards") },
	{	AM_SEEKING_CanDoSegments,		_T("AM_SEEKING_CanDoSegments") },	
	{	AM_SEEKING_Source,				_T("AM_SEEKING_Source") },
	
	{	FLAG_FORMAT_FRAME,				_T("TIME_FORMAT_FRAME") },
	{	FLAG_FORMAT_SAMPLE,				_T("TIME_FORMAT_SAMPLE") },
	{	FLAG_FORMAT_FIELD,				_T("TIME_FORMAT_FIELD") },
	{	FLAG_FORMAT_BYTE,				_T("TIME_FORMAT_BYTE") },	
	{	FLAG_FORMAT_MEDIA_TIME,			_T("TIME_FORMAT_MEDIA_TIME") }
};
const int			CapsFlagsCount = sizeof(CapsFlags) / sizeof(CapsFlags[0]);


//-----------------------------------------------------------------------------
//
//	CSeekForm class
//
//-----------------------------------------------------------------------------

CSeekForm::CSeekForm(CWnd* pParent)	: 
	CDialog(CSeekForm::IDD, pParent)
{

}

CSeekForm::~CSeekForm()
{
}

void CSeekForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_STATIC_DURATION, label_duration);
	DDX_Control(pDX, IDC_STATIC_POSITION, label_position);
	DDX_Control(pDX, IDC_STATIC_FPS, label_fps);
	DDX_Control(pDX, IDC_RADIO_TIME, radio_time);
	DDX_Control(pDX, IDC_RADIO_FRAME, radio_frame);
	DDX_Control(pDX, IDC_EDIT_TIME, edit_time);
	DDX_Control(pDX, IDC_EDIT_FRAME, edit_frame);
	DDX_Control(pDX, IDC_CHECK_KEYFRAME, check_keyframe);
	DDX_Control(pDX, IDC_LIST_CAPS, list_caps);
}

BOOL CSeekForm::DoCreateDialog()
{
	BOOL ret = Create(IDD);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	SetTimer(0, 200, NULL);

	edit_time.SetWindowText(_T("00:00:00.000"));
	edit_frame.SetWindowText(_T("0"));

	list_caps.ResetContent();
	for (int i=0; i<CapsFlagsCount; i++) {
		list_caps.AddString(CapsFlags[i].name);
		list_caps.Enable(i, FALSE);
	}

	OnTimeClick();
	return TRUE;
}


void CSeekForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(title)) {
		title.GetClientRect(&rc2);
		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW);
		title.Invalidate();
	}
}

void CSeekForm::OnTimeClick()
{
	radio_time.SetCheck(TRUE);
	radio_frame.SetCheck(FALSE);
	edit_time.EnableWindow(TRUE);
	edit_frame.EnableWindow(FALSE);
}

void CSeekForm::OnFrameClick()
{
	radio_time.SetCheck(FALSE);
	radio_frame.SetCheck(TRUE);
	edit_time.EnableWindow(FALSE);
	edit_frame.EnableWindow(TRUE);
}

void CSeekForm::OnTimer(UINT_PTR id)
{
	switch (id) {
	case 0:
		{
			UpdateGraphPosition();

			// refresh caps
			__int64		c;
			GetCurrentCaps(c);

			if (c != caps) {
				caps = c;
				for (int i=0; i<CapsFlagsCount; i++) {
					bool active = (caps & CapsFlags[i].flag ? true : false);
					list_caps.SetCheck(i, (active ? 1 : 0));
				}
			}
		}
		break;
	}
}

void CSeekForm::OnOK()
{
	/*
		Let's find out the desired time.
	*/

	int			h, m, s, ms, ret;
	CString		t;
	BOOL		keyframe = check_keyframe.GetCheck();

	if (radio_time.GetCheck()) {
		edit_time.GetWindowText(t);

		int c = _stscanf_s(t.GetBuffer(), _T("%2d:%2d:%2d.%3d"), &h, &m, &s, &ms);
		if (c != 4) {			
			MessageBox(_T("Time format should be in the following form:\nHH:MM:SS.MS\ne.g.: 00:01:30.123"), _T("Error"));
			return ;
		} else {

			double	tms;
			tms = h * 3600;
			tms += m*60;
			tms += s;

			// to milliseconds
			tms *= 1000;
			tms += ms;

			view->graph.Seek(tms, keyframe);
		}
	} else {
		edit_frame.GetWindowText(t);

		__int64	frame;
		double	fps, ms;

		ret = view->graph.GetFPS(fps);
		if (ret < 0 || fps <= 0) {
			OnTimeClick();
			return ;
		}

		int c = _stscanf_s(t.GetBuffer(), _T("%I64d"), &frame);
		if (c == 1) {
			ms = ((double)(frame+0.5) * 1000.0 / fps);
			view->graph.Seek(ms, keyframe);
		} else {
			MessageBox(_T("Must be a number !!"), _T("Error"));
			return ;
		}
	}

	CDialog::OnOK();
}

void MakeNiceTimeMS(int time_ms, CString &v)
{
	int		ms = time_ms%1000;	
	time_ms -= ms;
	time_ms /= 1000;

	int		h, m, s;
	h = time_ms / 3600;		time_ms -= h*3600;
	m = time_ms / 60;		time_ms -= m*60;
	s = time_ms;

	v.Format(_T("%.2d:%.2d:%.2d.%.3d"), h, m, s, ms);
}

void CSeekForm::GetCurrentCaps(__int64 &c)
{
	c = 0;

	if (view) {
		if (view->graph.ms) {
			// get the caps
			DWORD		seekcaps = 0;
			if (FAILED(view->graph.ms->GetCapabilities(&seekcaps))) {
				seekcaps = 0;
			}

			// the lower values are exactly the same
			c |= seekcaps;

			// we query the time formats
			if (view->graph.ms->IsFormatSupported(&TIME_FORMAT_FRAME) == NOERROR) c |= FLAG_FORMAT_FRAME;
			if (view->graph.ms->IsFormatSupported(&TIME_FORMAT_SAMPLE) == NOERROR) c |= FLAG_FORMAT_SAMPLE;
			if (view->graph.ms->IsFormatSupported(&TIME_FORMAT_FIELD) == NOERROR) c |= FLAG_FORMAT_FIELD;
			if (view->graph.ms->IsFormatSupported(&TIME_FORMAT_BYTE) == NOERROR) c |= FLAG_FORMAT_BYTE;
			if (view->graph.ms->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME) == NOERROR) c |= FLAG_FORMAT_MEDIA_TIME;
		}
	}
}


void CSeekForm::UpdateGraphPosition()
{
	double	pos_ms, dur_ms, fps;
	int		ret;

	pos_ms = 0;
	dur_ms = 0;
	fps    = -1;

	if (view) {
		ret = view->graph.GetPositions(pos_ms, dur_ms);
		if (ret < 0) {
			pos_ms = 0;
			dur_ms = 0;
		}

		ret = view->graph.GetFPS(fps);
		if (ret == 0 && fps != 0) {
		} else {
			fps = -1.0;
		}
	}

	CString	cur, dur;
	MakeNiceTimeMS((int)pos_ms, cur);
	MakeNiceTimeMS((int)dur_ms, dur);

	if (fps <= 0) {
		label_fps.SetWindowText(_T("Not available"));

		OnTimeClick();
		radio_frame.EnableWindow(FALSE);

	} else {
		int64	tot_frames = (int64)(dur_ms * fps / 1000);
		int64	cur_frames = (int64)(pos_ms * fps / 1000);

		CString	t;
		t.Format(_T(" (%I64d frames)"), tot_frames);
		dur += t;
		t.Format(_T(" (%I64d frames)"), cur_frames);
		cur += t;

		t.Format(_T("%5.03f"), (float)fps);
		label_fps.SetWindowText(t);

		radio_frame.EnableWindow(TRUE);
	}


	label_duration.SetWindowText(dur);
	label_position.SetWindowText(cur);
}





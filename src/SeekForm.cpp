//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "SeekForm.h"

#include "time_utils.h"

//-----------------------------------------------------------------------------
//
//	CSeekForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CSeekForm, CDialog)
BEGIN_MESSAGE_MAP(CSeekForm, CDialog)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(IDC_RADIO_TIME, &CSeekForm::OnFormatTimeClick)
	ON_COMMAND(IDC_RADIO_FRAME, &CSeekForm::OnFormatFrameClick)
	ON_BN_CLICKED(IDC_RADIO_FIELD, &CSeekForm::OnFormatFieldClick)
	ON_BN_CLICKED(IDC_RADIO_SAMPLE, &CSeekForm::OnFormatSampleClick)
	ON_BN_CLICKED(IDC_RADIO_BYTE, &CSeekForm::OnFormatByteClick)
	ON_BN_CLICKED(IDC_CHECK_SET_CURRENT_POSITION, &CSeekForm::OnCheckSetCurrentPosition)
	ON_BN_CLICKED(IDC_CHECK_STOP_SET_POSITION, &CSeekForm::OnCheckSetStopPosition)
	ON_BN_CLICKED(IDC_CHECK_STOP_RELATIVE_TO_CURRENT, &CSeekForm::OnCheckStopRelativeToCurrent)
	ON_BN_CLICKED(IDC_CHECK_STOP_RELATIVE_TO_PREVIOUS, &CSeekForm::OnCheckStopRelativeToPrevious)
END_MESSAGE_MAP()

struct VALNAME
{
	DWORD		flag;
	LPCTSTR		name;
};

// our temp values
enum
{
	FLAG_FORMAT_FRAME		= AM_SEEKING_Source << 8,	// use higher bits above last defined bit AM_SEEKING_Source
	FLAG_FORMAT_SAMPLE		= AM_SEEKING_Source << 9,
	FLAG_FORMAT_FIELD		= AM_SEEKING_Source << 10,
	FLAG_FORMAT_BYTE		= AM_SEEKING_Source << 11,
	FLAG_FORMAT_MEDIA_TIME	= AM_SEEKING_Source << 12,
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

CSeekForm::CSeekForm(CGraphView* graph_view, CWnd* pParent)	: 
	CDialog(CSeekForm::IDD, pParent),
	view(graph_view),
	time_format(TIME_FORMAT_MEDIA_TIME),
	cached_caps(INT_MIN)
{
	ResetCachedValues();
}

void CSeekForm::ResetCachedValues()
{
	cached_cur_pos	= _I64_MIN;
	cached_stop		= _I64_MIN;
	cached_duration = _I64_MIN;
	cached_fps		= FLT_MIN;
}

CSeekForm::~CSeekForm()
{
}

void CSeekForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
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

	CheckDlgButton(IDC_RADIO_TIME,	true);
	SetTimeFormat(TIME_FORMAT_MEDIA_TIME);

	list_caps.ResetContent();
	for (int i=0; i<CapsFlagsCount; i++) {
		list_caps.AddString(CapsFlags[i].name);
		list_caps.Enable(i, FALSE);
	}

	CheckDlgButton(IDC_CHECK_SET_CURRENT_POSITION,	true);
	CheckDlgButton(IDC_CHECK_STOP_SET_POSITION,		false);
	EnableControls();

	GotoDlgCtrl(GetDlgItem(IDC_EDIT_CURRENT_POSITION));

	return TRUE;
}

void CSeekForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our header control to fit dialog...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(title)) {
		title.GetClientRect(&rc2);
		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
		title.Invalidate();
	}
}

void CSeekForm::EnableControls()
{
	const bool set_cur = IsDlgButtonChecked(IDC_CHECK_SET_CURRENT_POSITION) != 0;
	const bool set_stop = IsDlgButtonChecked(IDC_CHECK_STOP_SET_POSITION) != 0;

	GetDlgItem(IDC_EDIT_CURRENT_POSITION)			->EnableWindow(set_cur);
	GetDlgItem(IDC_CHECK_RELATIVE_TO_PREVIOUS)		->EnableWindow(set_cur);
	GetDlgItem(IDC_CHECK_KEYFRAME)					->EnableWindow(set_cur);
	GetDlgItem(IDC_CHECK_SEGMENT)					->EnableWindow(set_cur);
	GetDlgItem(IDC_CHECK_NO_FLUSH)					->EnableWindow(set_cur);

	// relative to previous and stop relative to current are mutually exclusive controls

	GetDlgItem(IDC_EDIT_STOP_POSITION)				->EnableWindow(set_stop);
	GetDlgItem(IDC_CHECK_STOP_RELATIVE_TO_PREVIOUS)	->EnableWindow(set_stop && !IsDlgButtonChecked(IDC_CHECK_STOP_RELATIVE_TO_CURRENT));
	GetDlgItem(IDC_CHECK_STOP_RELATIVE_TO_CURRENT)	->EnableWindow(set_stop && !IsDlgButtonChecked(IDC_CHECK_STOP_RELATIVE_TO_PREVIOUS));
	GetDlgItem(IDC_CHECK_STOP_KEYFRAME)				->EnableWindow(set_stop);
	GetDlgItem(IDC_CHECK_STOP_SEGMENT)				->EnableWindow(set_stop);
	GetDlgItem(IDC_CHECK_STOP_NO_FLUSH)				->EnableWindow(set_stop);

	// Can't seek if we don't set one of current and stop position so disable button as a cue to the user
	GetDlgItem(IDOK)								->EnableWindow(set_cur || set_stop);
}

void CSeekForm::SetTimeFormat(const GUID& new_time_format)
{
	HRESULT hr = view->graph.ms->SetTimeFormat(&new_time_format);
	if (SUCCEEDED(hr)) {
		time_format = new_time_format;
	} else {
		time_format = TIME_FORMAT_MEDIA_TIME;
		CheckDlgButton(IDC_RADIO_TIME,	1);
		CheckDlgButton(IDC_RADIO_FRAME, 0);
		CheckDlgButton(IDC_RADIO_FIELD, 0);
		CheckDlgButton(IDC_RADIO_SAMPLE,0);
		CheckDlgButton(IDC_RADIO_BYTE,	0);
		DSUtil::ShowError(hr, _T("Can't set time format"));
	}

	// invalidate our cached display values to force reformatting for new units
	ResetCachedValues();

	// set edit controls to hint format required to user
	const TCHAR * const time_str = TIME_FORMAT_MEDIA_TIME == time_format ? _T("00:00:00.000") : _T("0");
	SetDlgItemText(IDC_EDIT_CURRENT_POSITION,  time_str);
	SetDlgItemText(IDC_EDIT_STOP_POSITION, time_str);
}

void CSeekForm::OnTimer(UINT_PTR id)
{
	switch (id) {
	case 0:
		{
			if (!IsWindowVisible())		// no point doing this updating if seek form not shown
				return;

			UpdateGraphPosition();

			// refresh caps
			const int c = GetCurrentCaps();

			if (c != cached_caps) {
				cached_caps = c;
				for (int i=0; i<CapsFlagsCount; i++) {
					bool active = (cached_caps & CapsFlags[i].flag ? true : false);
					list_caps.SetCheck(i, (active ? 1 : 0));
				}
				
				GetDlgItem(IDC_RADIO_FRAME)	->EnableWindow(	(cached_caps & FLAG_FORMAT_FRAME		) != 0);
				GetDlgItem(IDC_RADIO_SAMPLE)->EnableWindow( (cached_caps & FLAG_FORMAT_SAMPLE		) != 0);
				GetDlgItem(IDC_RADIO_FIELD)	->EnableWindow(	(cached_caps & FLAG_FORMAT_FIELD		) != 0);
				GetDlgItem(IDC_RADIO_BYTE)	->EnableWindow(	(cached_caps & FLAG_FORMAT_BYTE			) != 0);
				GetDlgItem(IDC_RADIO_TIME)	->EnableWindow(	(cached_caps & FLAG_FORMAT_MEDIA_TIME	) != 0);
			}
		}
		break;
	}
}

void CSeekForm::OnOK()
{
	const CComPtr<IMediaSeeking> ims(view->graph.ms);
	if (!ims) {
		DSUtil::ShowError(_T("No IMediaSeeking interface available for seeking"));
		return;
	}

	LONGLONG cur_position = 0LL;
	int cur_flags = AM_SEEKING_NoPositioning;

	if (IsDlgButtonChecked(IDC_CHECK_SET_CURRENT_POSITION)) {
		CString time_str;
		GetDlgItemText(IDC_EDIT_CURRENT_POSITION, time_str);
		if (ParseTimeString(time_str, cur_position)) {
			cur_flags =	 IsDlgButtonChecked(IDC_CHECK_RELATIVE_TO_PREVIOUS) ? AM_SEEKING_RelativePositioning : AM_SEEKING_AbsolutePositioning;
			cur_flags |= IsDlgButtonChecked(IDC_CHECK_KEYFRAME)				? AM_SEEKING_SeekToKeyFrame : 0;
			cur_flags |= IsDlgButtonChecked(IDC_CHECK_SEGMENT)				? AM_SEEKING_Segment : 0;
			cur_flags |= IsDlgButtonChecked(IDC_CHECK_NO_FLUSH)				? AM_SEEKING_NoFlush : 0;
		}
	}

	LONGLONG stop_position = 0LL;
	int stop_flags = AM_SEEKING_NoPositioning;

	if (IsDlgButtonChecked(IDC_CHECK_STOP_SET_POSITION)) {
		CString time_str;
		GetDlgItemText(IDC_EDIT_STOP_POSITION, time_str);
		if (ParseTimeString(time_str, stop_position)) {
			stop_flags =	IsDlgButtonChecked(IDC_CHECK_STOP_RELATIVE_TO_CURRENT)		? AM_SEEKING_IncrementalPositioning :
							(IsDlgButtonChecked(IDC_CHECK_STOP_RELATIVE_TO_PREVIOUS)	? AM_SEEKING_RelativePositioning : AM_SEEKING_AbsolutePositioning);
			stop_flags |=	IsDlgButtonChecked(IDC_CHECK_STOP_KEYFRAME)					? AM_SEEKING_SeekToKeyFrame : 0;
			stop_flags |=	IsDlgButtonChecked(IDC_CHECK_STOP_SEGMENT)					? AM_SEEKING_Segment : 0;
			stop_flags |=	IsDlgButtonChecked(IDC_CHECK_STOP_NO_FLUSH)					? AM_SEEKING_NoFlush : 0;
		}
	}

	if (cur_flags || stop_flags) {
		HRESULT hr = ims->SetPositions(&cur_position, cur_flags, &stop_position, stop_flags);
		DSUtil::ShowError(hr, _T("IMediaSeeking::SetPositions returned error code"));
	}

	// Don't do default OK processing as this closes the modeless dialog
}

int CSeekForm::GetCurrentCaps()
{
	int c = 0;

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
			if (S_OK == view->graph.ms->IsFormatSupported(&TIME_FORMAT_FRAME))		c |= FLAG_FORMAT_FRAME;
			if (S_OK == view->graph.ms->IsFormatSupported(&TIME_FORMAT_SAMPLE))		c |= FLAG_FORMAT_SAMPLE;
			if (S_OK == view->graph.ms->IsFormatSupported(&TIME_FORMAT_FIELD))		c |= FLAG_FORMAT_FIELD;
			if (S_OK == view->graph.ms->IsFormatSupported(&TIME_FORMAT_BYTE))		c |= FLAG_FORMAT_BYTE;
			if (S_OK == view->graph.ms->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME))	c |= FLAG_FORMAT_MEDIA_TIME;
		}
	}
	return c;
}

bool CSeekForm::ParseTimeString(const CString& time_str, LONGLONG& time)
{
	bool parsed_ok = true;
	time = 0LL;

	if (TIME_FORMAT_MEDIA_TIME == time_format) {
		int	hours = 0, minutes = 0; 
		float seconds = 0.0f;
		if (3 != _stscanf_s(time_str, _T("%2d:%2d:%f"), &hours, &minutes, &seconds)) {			
			DSUtil::ShowError(_T("Time format should be in the following form:\nHH:MM:SS.<optional decimal places>\ne.g.: 00:01:30.1234567"));
			parsed_ok = false;
		} else {
			time =	hours	* (LONGLONG)UNITS * 60LL * 60LL;
			time += minutes	* (LONGLONG)UNITS * 60LL;
			time += seconds * (float)	UNITS;
		}
	} else {
		if (1 != _stscanf_s(time_str, _T("%I64d"), &time)) {
			DSUtil::ShowError(_T("Time must be an integer !!"));
			parsed_ok = false;
		}
	}
	return parsed_ok;
}

CString CSeekForm::FormatTimeString(LONGLONG time)
{
	CString time_str;
	if (TIME_FORMAT_MEDIA_TIME == time_format) {

		const int hours	=	time / (UNITS * 60 * 60);
		time -= (LONGLONG)hours * (UNITS * 60 * 60);

		const int minutes =	time / (UNITS * 60);
		time -= (LONGLONG)minutes * (UNITS * 60);

		const int seconds =	time / (UNITS);
		time -= (LONGLONG)seconds * UNITS;

		time_str.Format(_T("%.2d:%.2d:%.2d.%.7I64d"), hours, minutes, seconds, time);

	} else if (TIME_FORMAT_BYTE == time_format) {
		time_str = CommaFormat(time);
	} else {
		time_str.Format(_T("%I64d"), time);
	}
	return time_str;
}

// Update the current position display. Only updated if values have changed from last update
void CSeekForm::UpdateGraphPosition()
{
	const TCHAR* const na_string  = _T("Not available");

	const CComPtr<IMediaSeeking> ims(view->graph.ms);
	if (!ims)
		return;

	GUID current_format = GUID_NULL;
	HRESULT hr = ims->GetTimeFormatW(&current_format);
	if (FAILED(hr))
		return;

	// WARNING: only call SetTimeFormat if needed
	// Calling SetTimeFormat several times a second causes playback performance problems with some filters
	if (current_format == time_format || SUCCEEDED(ims->SetTimeFormat(&time_format))) {

		LONGLONG cur_pos = -1LL, stop = -1LL, dur = -1LL;
		hr = ims->GetPositions(&cur_pos, &stop);
		if (cur_pos != cached_cur_pos) {
			cached_cur_pos = cur_pos;
			SetDlgItemText(IDC_STATIC_POSITION,		FAILED(hr) || cur_pos<0	? na_string : FormatTimeString(cur_pos) );
		}

		if (stop != cached_stop) {
			cached_stop = stop;
			SetDlgItemText(IDC_STATIC_STOP_POSITION, FAILED(hr) || stop<0	? na_string : FormatTimeString(stop) );
		}

		hr = ims->GetDuration(&dur);
		if (dur != cached_duration) {
			cached_duration = dur;
			SetDlgItemText(IDC_STATIC_DURATION, 	FAILED(hr) || dur<0		? na_string : FormatTimeString(dur) );
		}

		double fps = -1.0;
		if (view->graph.GetFPS(fps) < 0 || fps == 0.0) {
			fps = -1.0;
		}

		if (fps != cached_fps) {
			cached_fps = fps;
			if (fps <= 0) {
				SetDlgItemText(IDC_STATIC_FPS, na_string);
			} else {
				CString str;
				str.Format(_T("%5.03f"), (float)fps);
				SetDlgItemText(IDC_STATIC_FPS, str);
			}
		}
	} else {
		SetDlgItemText(IDC_STATIC_POSITION,			na_string);
		SetDlgItemText(IDC_STATIC_STOP_POSITION,	na_string);
		SetDlgItemText(IDC_STATIC_DURATION,			na_string);
		SetDlgItemText(IDC_STATIC_FPS,				na_string);
	}
}

void CSeekForm::OnCheckSetCurrentPosition()
{
	EnableControls();
}

void CSeekForm::OnCheckSetStopPosition()
{
	EnableControls();
}

void CSeekForm::OnCheckStopRelativeToCurrent()
{
	EnableControls();
}

void CSeekForm::OnCheckStopRelativeToPrevious()
{
	EnableControls();
}

void CSeekForm::OnFormatTimeClick()
{
	SetTimeFormat(TIME_FORMAT_MEDIA_TIME);
}

void CSeekForm::OnFormatFrameClick()
{
	SetTimeFormat(TIME_FORMAT_FRAME);
}

void CSeekForm::OnFormatFieldClick()
{
	SetTimeFormat(TIME_FORMAT_FIELD);
}

void CSeekForm::OnFormatSampleClick()
{
	SetTimeFormat(TIME_FORMAT_SAMPLE);
}

void CSeekForm::OnFormatByteClick()
{
	SetTimeFormat(TIME_FORMAT_BYTE);
}

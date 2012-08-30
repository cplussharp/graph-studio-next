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
//	CScheduleForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CScheduleForm, CDialog)

BEGIN_MESSAGE_MAP(CScheduleForm, CDialog)
	ON_WM_SIZE()
	ON_WM_MEASUREITEM()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CScheduleForm::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CScheduleForm::OnBnClickedButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CScheduleForm::OnBnClickedButtonAdd)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CScheduleForm class
//
//-----------------------------------------------------------------------------

CScheduleForm::CScheduleForm(CWnd* pParent)	: 
	CDialog(CScheduleForm::IDD, pParent)
{

}

CScheduleForm::~CScheduleForm()
{
	ClearEvents();
}

void CScheduleForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SCHEDULE, list_schedule);
	DDX_Control(pDX, IDC_TITLEBAR, title);
}

BOOL CScheduleForm::DoCreateDialog()
{
	BOOL ret = Create(IDD);
	if (!ret) return FALSE;

	list_schedule.Initialize();

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	// create buttons
	CRect	rc;
	rc.SetRect(0, 0, 80, 25);
	btn_add.Create(_T("Add"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_ADD);
	btn_add.SetFont(GetFont());
	btn_remove.Create(_T("Remove"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_REMOVE);
	btn_remove.SetFont(GetFont());
	btn_close.Create(_T("Close"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_CLOSE);
	btn_close.SetFont(GetFont());

	SetWindowPos(NULL, 0, 0, 400, 250, SWP_NOMOVE);

	// make it happen
	last_time = GetTimeSec();
	SetTimer(0, 300, NULL);
	return TRUE;
}

void CScheduleForm::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT item)
{
	if (item->CtlType == ODT_LISTVIEW) {
		if (item->CtlID == IDC_LIST_SCHEDULE) {
			item->itemHeight = 18;
			return ;
		}
	}

	// base clasu
	__super::OnMeasureItem(nIDCtl, item);
}

void CScheduleForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(list_schedule)) {
		title.GetClientRect(&rc2);

		btn_add.SetWindowPos(NULL, cx - 2*(60+6), 4, 60, 25, SWP_SHOWWINDOW);
		btn_remove.SetWindowPos(NULL, cx - 1*(60+6), 4, 60, 25, SWP_SHOWWINDOW);
		btn_close.SetWindowPos(NULL, 6, 4, 60, 25, SWP_SHOWWINDOW);

		list_schedule.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW);

		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW);
		title.Invalidate();
		
		btn_add.Invalidate();
		btn_remove.Invalidate();
		btn_close.Invalidate();

		list_schedule.Invalidate();
	}
}

BOOL CScheduleForm::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}

void CScheduleForm::OnBnClickedButtonAdd()
{
	// add a new action
	ScheduleEvent	*ev = new ScheduleEvent();

	ev->action = ScheduleEvent::ACTION_NONE;
	ev->active = false;

	__int64	tn = GetTimeSec();
	int	h, m, s;
	ToHMS(tn, h, m, s);
	ev->time_pattern.Format(_T("%.2d:%.2d:%.2d"), h, m, s);

	// prihodime
	events.Add(ev);

	// supneme ho tam
	int item = list_schedule.InsertItem(events.GetCount(), _T("Item"));
	list_schedule.SetItemData(item, (DWORD_PTR)ev);
}

void CScheduleForm::OnBnClickedButtonRemove()
{
	// find the first selected item
	POSITION pos = list_schedule.GetFirstSelectedItemPosition();
	if (pos) {
		int item = list_schedule.GetNextSelectedItem(pos);

		// kick from the list
		list_schedule.DeleteItem(item);

		if (item >= 0 && item < events.GetCount()) {
			// kick out of the events
			ScheduleEvent	*ev = events[item];
			delete ev;
			events.RemoveAt(item);
		}

		item = min(item, events.GetCount() - 1);
		if (item >= 0) {
			list_schedule.SetItemState(item, ODS_SELECTED | ODS_FOCUS, ODS_SELECTED | ODS_FOCUS);
		}
	}
}

void CScheduleForm::OnBnClickedButtonClose()
{
	ShowWindow(SW_HIDE);
}

__int64 CScheduleForm::GetTimeSec()
{
	// time from the epoch
	return time(NULL);
}

void CScheduleForm::ToHMS(__int64 t, int &h, int &m, int &s)
{
	struct tm lt;
	if (localtime_s(&lt, &t) == NOERROR) {
		h = lt.tm_hour;
		m = lt.tm_min;
		s = lt.tm_sec;
	}
}

void CScheduleForm::OnTimer(UINT_PTR nIDEvent)
{
	__int64	tnow = GetTimeSec();
	if (tnow > last_time) {

		// iterate all seconds
		int		h, m, s;
		do {
			last_time++;			
			ToHMS(last_time, h, m, s);
			OnTime(h, m, s);
		} while (last_time < tnow);
	}
}

void CScheduleForm::OnTime(int h, int m, int s)
{
	// now loop all events
	for (int i=0; i<events.GetCount(); i++) {
		ScheduleEvent	*ev = events[i];
		if (ev->Check(h, m, s)) {

			switch (ev->action) {
			case ScheduleEvent::ACTION_START:	view->OnPlayClick(); break;
			case ScheduleEvent::ACTION_STOP:	view->OnStopClick(); break;
			case ScheduleEvent::ACTION_RESTART:
				{
					view->OnStopClick();
					view->OnPlayClick();
				}
				break;
			}
		}
	}
}

void CScheduleForm::ClearEvents()
{
	if (IsWindow(list_schedule)) {
		list_schedule.DeleteAllItems();
	}

	for (int i=0; i<events.GetCount(); i++) {
		ScheduleEvent	*ev = events[i];
		delete ev;
	}
	events.RemoveAll();
}

void CScheduleForm::AddEvent(CString time_pattern, int action)
{
	// add a new action
	ScheduleEvent	*ev = new ScheduleEvent();

	ev->action = action;
	ev->active = true;
	ev->time_pattern = time_pattern;

	// prihodime
	events.Add(ev);

	// supneme ho tam
	int item = list_schedule.InsertItem(events.GetCount(), _T("Item"));
	list_schedule.SetItemData(item, (DWORD_PTR)ev);
}


//-----------------------------------------------------------------------------
//
//	ScheduleEvent class
//
//-----------------------------------------------------------------------------

CString	GetPatternToken(CString &p)
{
	CString r;
	int	i = p.Find(_T(":"));
	if (i < 0) {
		r = p;
		p = _T("");
		return r;
	}

	r = p.Left(i);
	p.Delete(0, i+1);
	p.Trim();
	r.Trim();
	return r;
}

bool CheckPattern(CString p, int t)
{
	/*
		Checks whether the given integer matches the pattern
	*/

	if (p == _T("*")) return true;

	int i = p.Find(_T("/"));
	if (i < 0) {
	
		// this must be a single number
		int	val;
		if (_stscanf_s(p.GetBuffer(), _T("%d"), &val) == 1) {
			// only accept those that are the same
			if (val == t) return true;
		}

		return false;
	}

	p.Delete(0, i+1);
	p.Trim();

	// now let's check for the modulo
	int m;
	if (_stscanf_s(p.GetBuffer(), _T("%d"), &m) == 1) {
		// <=0 is not valid
		if (m <= 0) return false;
		if ((t % m) == 0) return true;
	}

	// do not accept
	return false;
}

bool ScheduleEvent::Check(int h, int m, int s)
{
	/*
		The pattern has following syntax :

		<H> : <M> : <S>								- the ':' character is the separator

		<H>, <M> and <S> can either be a constant number, or a wildcard '*' - that means 'every'.
		Also the '* / 10' value would mean 'every ten'
	*/

	if (!active) return false;

	CString	p = time_pattern;
	CString	hp, mp, sp;

	// check hour
	hp = GetPatternToken(p);
	if (hp == _T("")) return false;
	if (!CheckPattern(hp, h)) return false;

	// check minute
	mp = GetPatternToken(p);
	if (mp == _T("")) return false;
	if (!CheckPattern(mp, m)) return false;

	// check second
	sp = GetPatternToken(p);
	if (sp == _T("")) return false;
	if (!CheckPattern(sp, s)) return false;

	// okay, now it's fine
	return true;
}








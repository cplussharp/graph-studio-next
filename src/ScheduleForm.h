//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

class CGraphView;

//-----------------------------------------------------------------------------
//
//	ScheduleEvent class
//
//-----------------------------------------------------------------------------
class ScheduleEvent
{
public:

	enum {
		ACTION_NONE			= 0,
		ACTION_START		= 1,
		ACTION_STOP			= 2,
		ACTION_RESTART		= 3
	};

	int			action;
	bool		active;

	// time - events happen every day
	CString		time_pattern;

public:
	ScheduleEvent() : action(ScheduleEvent::ACTION_NONE), time_pattern(_T("")), active(false) { }
	ScheduleEvent(const ScheduleEvent &e) : action(e.action), time_pattern(e.time_pattern), active(e.active) { }
	ScheduleEvent &operator =(const ScheduleEvent &e) { action=e.action; time_pattern=e.time_pattern; active=e.active; return *this; }
	virtual ~ScheduleEvent() { }

	// check, if the event should happen
	bool Check(int h, int m, int s);
};


//-----------------------------------------------------------------------------
//
//	CScheduleForm class
//
//-----------------------------------------------------------------------------
class CScheduleForm : public CGraphStudioModelessDialog
{
protected:
	DECLARE_DYNAMIC(CScheduleForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	GraphStudio::TitleBar		title;
	GraphStudio::ScheduleList	list_schedule;
	CButton						btn_close;
	CButton						btn_add;
	CButton						btn_remove;

	__int64						last_time;

	// list of all events
	CArray<ScheduleEvent*>		events;

	__int64 GetTimeSec();
	void ToHMS(__int64 t, int &h, int &m, int &s);
public:
	CScheduleForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CScheduleForm();
	virtual CRect GetDefaultRect() const;

	enum { IDD = IDD_DIALOG_SCHEDULE };

	// initialization
	BOOL DoCreateDialog(CWnd* parent);

	void OnSize(UINT nType, int cx, int cy);
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT item);

	void OnBnClickedButtonClose();
	void OnBnClickedButtonAdd();
	void OnBnClickedButtonRemove();
	BOOL OnEraseBkgnd(CDC *pDC);

	// schedule timer
	void OnTimer(UINT_PTR nIDEvent);
	void OnTime(int h, int m, int s);

	// events
	void ClearEvents();
	void AddEvent(CString time_pattern, int action);
};

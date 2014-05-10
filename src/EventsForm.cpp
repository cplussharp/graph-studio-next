//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "EventsForm.h"


//-----------------------------------------------------------------------------
//
//	CEventsForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CEventsForm, CGraphStudioModelessDialog)

BEGIN_MESSAGE_MAP(CEventsForm, CGraphStudioModelessDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CEventsForm::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_COPY, &CEventsForm::OnBnClickedButtonCopy)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CEventsForm class
//
//-----------------------------------------------------------------------------

CEventsForm::CEventsForm(CWnd* pParent)	: 
	CGraphStudioModelessDialog(CEventsForm::IDD, pParent)
{

}

CEventsForm::~CEventsForm()
{
}

void CEventsForm::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_EVENTS, list_events);
	DDX_Control(pDX, IDC_TITLEBAR, title);
}

BOOL CEventsForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	// create buttons
	CRect	rc;
	rc.SetRect(0, 0, 60, 23);
    btn_copy.Create(_T("Copy"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_COPY);
	btn_copy.SetFont(GetFont());
    btn_copy.SetWindowPos(NULL, 4, 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	btn_clear.Create(_T("Clear"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_CLEAR);
    btn_clear.SetWindowPos(NULL, 8 + rc.Width(), 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_clear.SetFont(GetFont());

    OnInitialize();

	return TRUE;
}

CRect CEventsForm::GetDefaultRect() const 
{
	return CRect(50, 200, 450, 450);
}

void CEventsForm::OnInitialize()
{
	if(GraphStudio::HasFont(_T("Consolas")))
        GraphStudio::MakeFont(font_list, _T("Consolas"), 10, false, false);
    else
        GraphStudio::MakeFont(font_list, _T("Courier New"), 10, false, false);
	list_events.SetFont(&font_list);
}

void CEventsForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(list_events)) {
		title.GetClientRect(&rc2);

		list_events.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
		title.Invalidate();
	}
}

BOOL CEventsForm::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (message == WM_GRAPH_EVENT) {
		if (!lParam) return TRUE;

		GraphStudio::DisplayGraph	*graph = (GraphStudio::DisplayGraph*)lParam;
		if (!graph->me) return TRUE;

		long		evcode;
		LONG_PTR	param1, param2;

		while (graph->me->GetEvent(&evcode, &param1, &param2, 0) == NOERROR) {
			OnGraphEvent(evcode, param1, param2);			
			graph->me->FreeEventParams(evcode, param1, param2);
		}

		*pResult = 0;
		return TRUE;
	}
	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

void CEventsForm::OnGraphEvent(long evcode, LONG_PTR param1, LONG_PTR param2)
{
	CString	msg;

	switch (evcode) {
	case EC_ACTIVATE:		{ msg = _T("EC_ACTIVATE"); } break;
	case EC_BUFFERING_DATA:
		{
			BOOL	starts = (BOOL)param1;
			if (starts) {
				msg = _T("EC_BUFFERING_DATA (Buffering started)");
			} else {
				msg = _T("EC_BUFFERING_DATA (Buffering stopped)");
			}
		}
		break;
	case EC_BUILT:			{ msg = _T("EC_BUILT"); } break;
	case EC_CLOCK_CHANGED:	{ msg = _T("EC_CLOCK_CHANGED"); } break;
	case EC_CLOCK_UNSET:	{ msg = _T("EC_CLOCK_UNSET"); } break;
	case EC_COMPLETE:		
		{ 
			msg = _T("EC_COMPLETE"); 
			view->OnGraphStreamingComplete();
		} 
		break;
	case EC_DEVICE_LOST:
		{
			msg = _T("EC_DEVICE_LOST");
			// TODO: rebuild graph
		}
		break;
	case EC_DISPLAY_CHANGED: { msg = _T("EC_DISPLAY_CHANGED"); } break;
	case EC_END_OF_SEGMENT:	{ msg = _T("EC_END_OF_SEGMENT"); } break;
	case EC_ERROR_STILLPLAYING:	
		{
			HRESULT hr = (HRESULT)param1;
			msg.Format(_T("EC_ERROR_STILLPLAYING (hr = 0x%08x)"), hr);
		}
		break;
	case EC_ERRORABORT:		
		{	
			HRESULT hr = (HRESULT)param1;
			msg.Format(_T("EC_ERRORABORT (hr = 0x%08x)"), hr);
			view->OnStopClick();
		} 
		break;
	case EC_ERRORABORTEX:
		{
			HRESULT hr = (HRESULT)param1;
			BSTR str = (BSTR)param2;
			if (str) {
				msg.Format(_T("EC_ERRORABORTEX (hr = 0x%08x, %s)"), hr, str);
			} else {
				msg.Format(_T("EC_ERRORABORTEX (hr = 0x%08x)"), hr);
			}
			view->OnStopClick();
		}
		break;
	case EC_FILE_CLOSED:	{ msg = _T("EC_FILE_CLOSED"); } break;
	case EC_FULLSCREEN_LOST:	{ msg = _T("EC_FULLSCREEN_LOST"); } break;
	case EC_GRAPH_CHANGED:	
		{ 
			view->graph.RefreshFilters();
			view->graph.SmartPlacement(false);
			view->Invalidate();
			msg = _T("EC_GRAPH_CHANGED"); 
		} 
		break;
	case EC_LENGTH_CHANGED:	{ msg = _T("EC_LENGTH_CHANGED"); } break;
	case EC_LOADSTATUS:
		{
			long p1 = (long)param1;
			switch (p1) {
			case AM_LOADSTATUS_CLOSED:	msg = _T("EC_LOADSTATUS (code = AM_LOADSTATUS_CLOSED)"); break;
			case AM_LOADSTATUS_CONNECTING:	msg = _T("EC_LOADSTATUS (code = AM_LOADSTATUS_CONNECTING)"); break;
			case AM_LOADSTATUS_LOADINGDESCR:	msg = _T("EC_LOADSTATUS (code = AM_LOADSTATUS_LOADINGDESCR)"); break;
			case AM_LOADSTATUS_LOADINGMCAST:	msg = _T("EC_LOADSTATUS (code = AM_LOADSTATUS_LOADINGMCAST)"); break;
			case AM_LOADSTATUS_LOCATING:	msg = _T("EC_LOADSTATUS (code = AM_LOADSTATUS_LOCATING)"); break;
			case AM_LOADSTATUS_OPEN:	msg = _T("EC_LOADSTATUS (code = AM_LOADSTATUS_OPEN)"); break;
			case AM_LOADSTATUS_OPENING:	msg = _T("EC_LOADSTATUS (code = AM_LOADSTATUS_OPENING)"); break;
			default:
				msg = _T("EC_LOADSTATUS");
				break;
			}
		}
		break;
	case EC_NEED_RESTART:	{ msg = _T("EC_NEED_RESTART"); } break;
	case EC_OLE_EVENT:
		{
			msg.Format(_T("EC_OLE_EVENT  (%s = %s)"), (BSTR)param1, (BSTR)param2);
		}
		break;
	case EC_OPENING_FILE:
		{
			BOOL starting = (BOOL)param1;
			if (starting) {
				msg = _T("EC_OPENING_FILE (Starting)");
			} else {
				msg = _T("EC_OPENING_FILE (Finished)");
			}
		}
		break;
	case EC_PALETTE_CHANGED:	{ msg = _T("EC_PALETTE_CHANGED"); } break;
	case EC_PAUSED:				
		{
			msg = _T("EC_PAUSED"); 
			view->OnGraphStreamingStarted();
		} 
		break;
	case EC_PLEASE_REOPEN:		{ msg = _T("EC_PLEASE_REOPEN"); } break;
	case EC_PREPROCESS_COMPLETE:{ msg = _T("EC_PREPROCESS_COMPLETE"); } break;
	case EC_PROCESSING_LATENCY:
		{
			REFERENCE_TIME	latency = 0;
			if (param1) { latency = *((REFERENCE_TIME*)param1); }
			msg.Format(_T("EC_PROCESSING_LATENCY  (latency = %I64d)"), latency);
		}
		break;
	case EC_QUALITY_CHANGE:		{ msg = _T("EC_QUALITY_CHANGE"); } break;
	case EC_REPAINT:			{ msg = _T("EC_REPAINT"); } break;
	case EC_SAMPLE_LATENCY:
		{
			REFERENCE_TIME	latency = 0;
			if (param1) { latency = *((REFERENCE_TIME*)param1); }
			msg.Format(_T("EC_SAMPLE_LATENCY  (latency = %I64d)"), latency);
		}
		break;
	case EC_SAMPLE_NEEDED:		{ msg = _T("EC_SAMPLE_NEEDED"); } break;
	case EC_SCRUB_TIME:
		{
			REFERENCE_TIME	time = ((unsigned __int64)param1 << 32) | param2;
			msg.Format(_T("EC_SCRUB_TIME  (time = %I64d)"), time);
		}
		break;
	case EC_SEGMENT_STARTED:	{ msg = _T("EC_SEGMENT_STARTED"); } break;
	case EC_SHUTTING_DOWN:		{ msg = _T("EC_SHUTTING_DOWN"); } break;
	case EC_SNDDEV_IN_ERROR:
		{
			msg.Format(_T("EC_SNDDEV_IN_ERROR (access=%d, error=0x%08x)"), (DWORD )param1, (DWORD)param2);
		}
		break;
	case EC_SNDDEV_OUT_ERROR:
		{
			msg.Format(_T("EC_SNDDEV_OUT_ERROR (access=%d, error=0x%08x)"), (DWORD )param1, (DWORD)param2);
		}
		break;
	case EC_STARVATION:			{ msg = _T("EC_STARVATION"); } break;
	case EC_STATE_CHANGE:
		{
			FILTER_STATE	new_state = (FILTER_STATE)param1;
			switch (new_state) {
			case State_Stopped:	msg = _T("EC_STATE_CHANGE (State_Stopped)"); break;
			case State_Paused:	msg = _T("EC_STATE_CHANGE (State_Paused)"); break;
			case State_Running:	msg = _T("EC_STATE_CHANGE (State_Running)"); break;
			}
		}
		break;
	case EC_STATUS:
		{
			msg.Format(_T("EC_STATUS (%s, %s)"), (BSTR)param1, (BSTR)param2);
		}
		break;
	case EC_STEP_COMPLETE:
		{
			BOOL cancelled = (BOOL)param1;
			if (cancelled) {
				msg = _T("EC_STEP_COMPLETE (cancelled)");
			} else {
				msg = _T("EC_STEP_COMPLETE");
			}
		}
		break;
	case EC_STREAM_CONTROL_STARTED:	{ msg = _T("EC_STREAM_CONTROL_STARTED"); } break;
	case EC_STREAM_CONTROL_STOPPED:	{ msg = _T("EC_STREAM_CONTROL_STOPPED"); } break;
	case EC_STREAM_ERROR_STILLPLAYING:	
		{
			msg.Format(_T("EC_STREAM_ERROR_STILLPLAYING (hr = 0x%08x)"), (HRESULT)param1);
		}
		break;
	case EC_STREAM_ERROR_STOPPED:	
		{
			msg.Format(_T("EC_STREAM_ERROR_STOPPED (hr = 0x%08x)"), (HRESULT)param1);
		}
		break;
	case EC_UNBUILT:		{ msg = _T("EC_UNBUILT"); } break;
	case EC_USERABORT:		
		{ 
			msg = _T("EC_USERABORT"); 
			view->OnStopClick();
		} 
		break;
	case EC_VIDEO_SIZE_CHANGED:
		{
			int w, h;
			w = LOWORD((DWORD)param1);
			h = HIWORD((DWORD)param1);
			msg.Format(_T("EC_VIDEO_SIZE_CHANGED (%d x %d)"), w, h);
		}
		break;
	case EC_VIDEOFRAMEREADY:	{ msg = _T("EC_VIDEOFRAMEREADY"); } break;
	case EC_VMR_RECONNECTION_FAILED:
		{
			msg.Format(_T("EC_VMR_RECONNECTION_FAILED (hr = 0x%08x)"), (HRESULT)param1);
		}
		break;
	case EC_VMR_RENDERDEVICE_SET:
		{
			int p = (int)param1;
			switch (p) {
			case VMR_RENDER_DEVICE_OVERLAY:	msg = _T("EC_VMR_RENDERDEVICE_SET (Overlay)"); break;
			case VMR_RENDER_DEVICE_VIDMEM:	msg = _T("EC_VMR_RENDERDEVICE_SET (Video memory)"); break;
			case VMR_RENDER_DEVICE_SYSMEM:	msg = _T("EC_VMR_RENDERDEVICE_SET (System memory)"); break;
			default:
				msg = _T("EC_VMR_RENDERDEVICE_SET");
				break;
			}
		}	
		break;
	case EC_WMT_EVENT:	{ msg = _T("EC_WMT_EVENT"); } break;
	case EC_WMT_INDEX_EVENT:
		{
			msg = _T("EC_WMT_INDEX_EVENT");
		}
		break;
	default:
		{
			msg.Format(_T("Event Code = 0x%08x"), evcode);
		}
		break;
	}

	if (msg != _T("")) {
		CTime	time = CTime::GetCurrentTime();
		CString	ts;
		ts.Format(_T("[%02d:%02d:%02d] - %s"), time.GetHour(), time.GetMinute(), time.GetSecond(), msg); 
		list_events.InsertString(0, ts);
	}
}

void CEventsForm::OnBnClickedButtonClear()
{
	list_events.ResetContent();
}

void CEventsForm::OnBnClickedButtonCopy()
{
	// copy everything into clipboard
	CString	whole_text = _T("");
	if (list_events.GetCount() > 0) {
		for (int i=0; i<list_events.GetCount(); i++) {
			CString	line;
			list_events.GetText(i, line);
			whole_text = whole_text + line + _T("\r\n");
		}

		DSUtil::SetClipboardText(this->GetSafeHwnd(),whole_text);
	}
}

//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "seeking_bar.h"
#include "MainFrm.h"
#include "time_utils.h"


BEGIN_MESSAGE_MAP(CTransparentStatic, CStatic)
	// Standard printing commands
END_MESSAGE_MAP()


void CTransparentStatic::SetText(CString &t)
{
	CString tt;
	GetWindowText(tt);
	if (t != tt) {
		SetWindowText(t);
		CRect Rect;
		GetWindowRect(&Rect);
		GetParent()->ScreenToClient(&Rect);
		GetParent()->InvalidateRect(&Rect);
		GetParent()->UpdateWindow();
	}
}


HBRUSH CTransparentStatic::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
   pDC->SetBkMode(TRANSPARENT);
   return (HBRUSH)GetStockObject(NULL_BRUSH);
}

//-----------------------------------------------------------------------------
//
//	CSeekSlider class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CSeekSlider, CSliderCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

CSeekSlider::CSeekSlider()
{
}

CSeekSlider::~CSeekSlider()
{
}


void CSeekSlider::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW lpcd = (LPNMCUSTOMDRAW)pNMHDR;
	CDC *pDC = CDC::FromHandle(lpcd->hdc);
	DWORD dwStyle = this->GetStyle();
	
	switch(lpcd->dwDrawStage) {		
	case CDDS_PREPAINT:		
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
		}
		break;
	case CDDS_ITEMPREPAINT:
		{
			switch(lpcd->dwItemSpec) {
			case TBCD_TICS:		*pResult = CDRF_DODEFAULT; break;
			case TBCD_THUMB:	*pResult = CDRF_DODEFAULT; break;
			case TBCD_CHANNEL:	*pResult = CDRF_DODEFAULT; break;
			}
			break;
		}
	}
}

BOOL CSeekSlider::OnEraseBkgnd(CDC *pDC)
{
    CWnd* pParent = GetParent();
    ASSERT_VALID(pParent);

    CPoint pt(0, 0);
    MapWindowPoints(pParent, &pt, 1);
    pt = pDC->OffsetWindowOrg(pt.x, pt.y);
    LRESULT lResult = pParent->SendMessage(WM_ERASEBKGND, (WPARAM)pDC->m_hDC, 0L);
    pDC->SetWindowOrg(pt.x, pt.y);
    return lResult != 0;
}

void CSeekSlider::OnPaint()
{
	__super::OnPaint();
}

bool CSeekSlider::Create(CWnd *parent, CRect rc, UINT id)
{
	UINT	style = WS_CHILD | WS_VISIBLE | TBS_BOTH | TBS_ENABLESELRANGE |
					TBS_HORZ | TBS_NOTICKS | TBS_FIXEDLENGTH;

	bool ret = __super::Create(style, rc, parent, id) == TRUE;
	if (!ret) return false;

	// setup properties
	SetPageSize(8);
	SetLineSize(1);
	SetRange(0, rc.Width());

	// initialize slider properties
	SendMessage(TBM_SETTHUMBLENGTH, 18, 0);

	return true;
}

void CSeekSlider::SetChannelPos(double p)
{
	relative_pos = p;
	if (p < 0.0) p = 0.0;
	if (p > 1.0) p = 1.0;

	int lo, hi;
	GetRange(lo, hi);
	int pos = (int)(lo + (hi-lo)*relative_pos);

	int cslo, cshi;
	GetSelection(cslo, cshi);
	SetSelection(lo, pos);
	if (pos != cshi) Invalidate();
}


//-----------------------------------------------------------------------------
//
//	CSeekingBar class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CSeekingBar, CDialogBar)
	// Standard printing commands
	ON_WM_ERASEBKGND()
	ON_WM_MOVE()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
    ON_WM_SIZE()
END_MESSAGE_MAP()

CSeekingBar::CSeekingBar()
{
}

CSeekingBar::~CSeekingBar()
{
}

BOOL CSeekingBar::Create(CMainFrame* pParent, UINT nIDTemplate, UINT nStyle, UINT nID)
{
	frame = pParent;

	BOOL bReturn = CDialogBar::Create(pParent, nIDTemplate, nStyle, nID);
	back_brush.CreateSolidBrush(RGB(212, 219, 238));

	CRect	rc(0, 2, 450, 22);					// WARNING suspicious hard-coded pixel dimensions
	pending_seek_request = false;
	if (!seekbar.Create(this, rc, IDC_SLIDER_SEEK)) 
		return false;

	// seeking timer
	SetTimer(1, 200, NULL);
	return bReturn;
}

void CSeekingBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_TIME, label_time);
}

BOOL CSeekingBar::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (message == WM_INITDIALOG) {
		*pResult = OnInitDialog(wParam, lParam);
		return TRUE;
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

LONG CSeekingBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
    LRESULT bRet = HandleInitDialog(wParam, lParam);
    UpdateData(FALSE);
    return (LONG) bRet;
}

BOOL CSeekingBar::OnEraseBkgnd(CDC *pDC)
{
    CWnd* pParent = GetParent();
    ASSERT_VALID(pParent);

    CPoint pt(0, 0);
    MapWindowPoints(pParent, &pt, 1);
    pt = pDC->OffsetWindowOrg(pt.x, pt.y);
    LRESULT lResult = pParent->SendMessage(WM_ERASEBKGND, (WPARAM)pDC->m_hDC, 0L);
    pDC->SetWindowOrg(pt.x, pt.y);
    return lResult != 0;
}

HBRUSH CSeekingBar::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_TIME) {
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return back_brush;
}

void CSeekingBar::OnMove(int cx, int cy)
{
    Invalidate();
}

void CSeekingBar::SetGraphView(CGraphView *graph_view)
{
	view = graph_view;

	// position timer
	if (view) {
		SetTimer(0, 200, NULL);
	} else {
		KillTimer(0);
	}
}

void CSeekingBar::UpdateGraphPosition()
{
	double	pos_ms = 0.0, dur_ms = 0.0;
	FILTER_STATE GraphState = State_Running;

	if (view) {
		if (view->graph.GetState(GraphState, 0) != 0) {
			GraphState = State_Running;
		}
		int ret = view->graph.GetPositionAndDuration(pos_ms, dur_ms);
		if (ret < 0) {
			pos_ms = 0.0;
			dur_ms = 0.0;
		}
	}

	CString	cur, dur;
	MakeNiceTimeMS((int)pos_ms, cur);
	MakeNiceTimeMS((int)dur_ms, dur);
	cur = cur + _T(" / ") + dur;
	label_time.SetText(cur);

	// display how far through the playback
	const double proportion = dur_ms != 0.0 ? pos_ms / dur_ms : 0.0;

	if (frame && frame->GetTaskBarList())
	{
		TBPFLAG ProgressState = TBPF_NORMAL;
		switch (GraphState)
		{
		case State_Running: ProgressState = TBPF_NORMAL; break;
		case State_Paused: ProgressState = TBPF_PAUSED; break;
		default: ProgressState = TBPF_NOPROGRESS; break;
		}
		frame->GetTaskBarList()->SetProgressValue(*frame, static_cast<ULONGLONG>(pos_ms), static_cast<ULONGLONG>(dur_ms)); 
		frame->GetTaskBarList()->SetProgressState(*frame, dur_ms != 0.0 ? ProgressState : TBPF_NOPROGRESS);
	}

	if (view) {
		view->OnUpdateTimeLabel(cur);
		view->OnUpdateSeekbar(proportion);
	}
	seekbar.SetChannelPos(proportion);
}

void CSeekingBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	CSeekSlider	*sl = (CSeekSlider*)pScrollBar;
	if (sl == &seekbar) {
		pending_seek_request = true;
	}
}

void CSeekingBar::PerformSeek()
{
	pending_seek_request = false;

	if (view && view->graph.ms) {
		// let's do some seeking

		int lo, hi, cshi;
		seekbar.GetRange(lo, hi);
		cshi = seekbar.GetPos();

		double	cur, dur;
		view->graph.GetPositionAndDuration(cur, dur);
		cur = dur * cshi / hi;
		view->graph.Seek(cur);
	}
}

void CSeekingBar::OnTimer(UINT_PTR id)
{
	switch (id) {
	case 0:
		{
			UpdateGraphPosition();
		}
		break;
	case 1:
		{
			if (!pending_seek_request) return ;
			PerformSeek();
		}
		break;
	}
}



void CSeekingBar::OnSize(UINT nType, int cx, int cy)
{
    CDialogBar::OnSize(nType, cx, cy);

	if (!seekbar.m_hWnd ||
		!label_time.m_hWnd)
		return;					// no seekbar and label yet for sizing

	CRect seekbar_rect;
	seekbar.GetWindowRect(&seekbar_rect);
	CRect label_rect;
	label_time.GetWindowRect(&label_rect);

	const int gap_width = GetSystemMetrics(SM_CXVSCROLL) / 4;		// use a moderate system independent offset for a gap

	seekbar.SetWindowPos(NULL, gap_width, (cy-seekbar_rect.Height()) / 2, cx - label_rect.Width() - 3*gap_width, seekbar_rect.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	label_time.SetWindowPos(NULL, cx - label_rect.Width(), (cy-label_rect.Height()) / 2, 0,0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
}

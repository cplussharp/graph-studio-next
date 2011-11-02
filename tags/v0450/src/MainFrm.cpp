//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//-----------------------------------------------------------------------------
//
//	CMainFrame class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


//-----------------------------------------------------------------------------
//
//	CMainFrame class
//
//-----------------------------------------------------------------------------

CMainFrame::CMainFrame()
{
	view = NULL;
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	/*
	m_wndPlaybackBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT);
	m_wndPlaybackBar.LoadToolBar(IDR_PLAYBACK);
	*/

	if (!m_wndReBar.Create(this)) {
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_HIDE_INPLACE);

	// set rebar band info
	CReBarCtrl	&rebar = m_wndReBar.GetReBarCtrl();
	m_wndReBar.AddBar(&m_wndToolBar, _T(""), NULL, RBBS_NOGRIPPER | RBBS_BREAK);

	/*
	REBARBANDINFO	band;
	ZeroMemory(&band, sizeof(band));
	band.cbSize = sizeof(band);
	band.fMask = RBBIM_STYLE;
	rebar.GetBandInfo(0, &band);
	band.fStyle |= RBBS_NOGRIPPER;
	band.fStyle = band.fStyle &~ RBBS_GRIPPERALWAYS;
	rebar.SetBandInfo(0, &band);
	*/
	rebar.MaximizeBand(0);


	if (!m_wndSeekingBar.Create(this, IDD_BAR_SEEKING, 
		CBRS_ALIGN_BOTTOM | CBRS_TOOLTIPS |	
		CBRS_FLYBY | CBRS_HIDE_INPLACE, IDD_BAR_SEEKING)) {
		return -1;
	}

	m_wndReBar.AddBar(&m_wndSeekingBar, _T(""), NULL, RBBS_NOGRIPPER | RBBS_BREAK);
	rebar.MaximizeBand(1);

	if (!m_wndStatusBar.Create(this) ||	
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs)) return FALSE;

	cs.style = cs.style &~ WS_VISIBLE;

	cs.cx = 0;
	cs.cy = 0;
	cs.y = GetSystemMetrics(SM_CYSCREEN);
	cs.x = GetSystemMetrics(SM_CXSCREEN);

	return TRUE;
}

BOOL CMainFrame::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (message == WM_COMMAND) {
		if (view) {
			// tricky bypass
			view->OnWmCommand(wParam, lParam);
		}
	}
	return __super::OnWndMsg(message, wParam, lParam, pResult);
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const			{ CFrameWnd::AssertValid(); }
void CMainFrame::Dump(CDumpContext& dc) const	{ CFrameWnd::Dump(dc); }
#endif //_DEBUG





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

const UINT CMainFrame::m_uTaskbarBtnCreatedMsg = RegisterWindowMessage ( _T("TaskbarButtonCreated") );

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
    ON_CBN_SELCHANGE(ID_COMBO_RATE, &CMainFrame::OnComboRateChanged)
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


    // init Playrate ComboBox
    CRect rect;
	int nIndex = m_wndToolBar.GetToolBarCtrl().CommandToIndex(ID_COMBO_RATE);
	m_wndToolBar.SetButtonInfo(nIndex, ID_COMBO_RATE, TBBS_SEPARATOR, 205);
	m_wndToolBar.GetToolBarCtrl().GetItemRect(nIndex, &rect);
	rect.top = 1;
	rect.bottom = rect.top + 250 /*drop height*/;
    rect.right = rect.left + 58;
    if(!m_comboRate.Create(CBS_DROPDOWNLIST | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL, rect, &m_wndToolBar, ID_COMBO_RATE))
	{
		TRACE(_T("Failed to create combo-box for rate\n"));
		return FALSE;
	}
    m_comboRate.SetFont(m_wndToolBar.GetFont());
    int n = m_comboRate.AddString(_T("1600%"));
    m_comboRate.SetItemData(n, 16000);
    n = m_comboRate.AddString(_T(" 800%"));
    m_comboRate.SetItemData(n, 8000);
    n = m_comboRate.AddString(_T(" 400%"));
    m_comboRate.SetItemData(n, 4000);
    n = m_comboRate.AddString(_T(" 200%"));
    m_comboRate.SetItemData(n, 2000);
    n = m_comboRate.AddString(_T(" 150%"));
    m_comboRate.SetItemData(n, 1500);
    n = m_comboRate.AddString(_T(" 125%"));
    m_comboRate.SetItemData(n, 1250);
    n = m_comboRate.AddString(_T(" 110%"));
    m_comboRate.SetItemData(n, 1100);
    n = m_comboRate.AddString(_T(" 100%"));
    m_comboRate.SetItemData(n, 1000);
    m_comboRate.SetCurSel(n);
    m_comboRate_defaultSel = n;
    n = m_comboRate.AddString(_T("   90%"));
    m_comboRate.SetItemData(n, 900);
    n = m_comboRate.AddString(_T("   75%"));
    m_comboRate.SetItemData(n, 750);
    n = m_comboRate.AddString(_T("   50%"));
    m_comboRate.SetItemData(n, 500);
    n = m_comboRate.AddString(_T("   25%"));
    m_comboRate.SetItemData(n, 250);
    n = m_comboRate.AddString(_T("   10%"));
    m_comboRate.SetItemData(n, 100);
    n = m_comboRate.AddString(_T("  -10%"));
    m_comboRate.SetItemData(n, -100);
    n = m_comboRate.AddString(_T("  -25%"));
    m_comboRate.SetItemData(n, -250);
    n = m_comboRate.AddString(_T("  -50%"));
    m_comboRate.SetItemData(n, -500);
    n = m_comboRate.AddString(_T("  -75%"));
    m_comboRate.SetItemData(n, -750);
    n = m_comboRate.AddString(_T("-100%"));
    m_comboRate.SetItemData(n, -1000);


    // Register Internal Filters for COM-Creation
    (new DSUtil::CClassFactory(&CMonoTimeMeasure::g_Template))->Register();
    (new DSUtil::CClassFactory(&CMonoDump::g_Template))->Register();
    (new DSUtil::CClassFactory(&CFakeM2tsDevice::g_Template))->Register();
    (new DSUtil::CClassFactory(&CPsiConfigFilter::g_Template))->Register();
    (new DSUtil::CClassFactory(&CAnalyzerFilter::g_Template))->Register();

    // set Process ID in title
    CString strTitle;
    strTitle.Format(_T("GraphStudioNext (PID %08x)"), GetCurrentProcessId());
    m_strTitle = strTitle;

	// TODO: Remove this if you don't want tool tips
	return 0;
}

void CMainFrame::OnComboRateChanged()
{
    if(view == NULL)
        return;

    int sel = m_comboRate.GetCurSel();
    int ret = view->graph.SetRate((int)m_comboRate.GetItemData(sel) / 1000.0);
    if(ret != 0)
    {
        DSUtil::ShowError(ret, _T("Can't set rate."));
        OnUpdatePlayRate();
    }
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

LRESULT CMainFrame::OnTaskbarBtnCreated ( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // On pre-Win 7, anyone can register a message called "TaskbarButtonCreated"
    // and broadcast it, so make sure the OS is Win 7 or later before acting on
    // the message.
    if (DSUtil::IsOsWin7OrLater())
    {
        m_pTaskbarList.Release();
        m_pTaskbarList.CoCreateInstance ( CLSID_TaskbarList );
    }

    return 0;
}

BOOL CMainFrame::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (message == WM_COMMAND) {
		if (view) {
			// tricky bypass
			view->OnWmCommand(wParam, lParam);
		}
	} else if(message == m_uTaskbarBtnCreatedMsg) {
        *pResult = OnTaskbarBtnCreated(message, wParam, lParam);
        return TRUE;
    } else if(message == WM_UPDATEPLAYRATE) {
        OnUpdatePlayRate();
        return true;
    }
	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

void CMainFrame::OnUpdatePlayRate()
{
    int sel = m_comboRate_defaultSel;
    double curVal;
    if(view->graph.GetRate(&curVal) == 0)
    {
        int val = curVal * 1000;
        if(val != 1000)
        {
            int i = 0;
            int cv = 0;
            while((cv = m_comboRate.GetItemData(i)) != 0)
            {
                if(cv == val)
                {
                    sel = i;
                    break;
                }

                i++;
            }
        }
    }
    m_comboRate.SetCurSel(sel);
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const			{ CFrameWnd::AssertValid(); }
void CMainFrame::Dump(CDumpContext& dc) const	{ CFrameWnd::Dump(dc); }
#endif //_DEBUG





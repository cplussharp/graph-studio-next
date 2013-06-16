//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "MainFrm.h"

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
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_COMMAND(ID_CLOSE_WINDOW, &CMainFrame::OnFileClose)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

static void RegisterInternalFiltersForCurrentApartment(void)
{
    // Register Internal Filters for COM-Creation
    (new DSUtil::CClassFactory(&CMonoTimeMeasure::g_Template))->Register();
    (new DSUtil::CClassFactory(&CMonoDump::g_Template))->Register();
    (new DSUtil::CClassFactory(&CFakeM2tsDevice::g_Template))->Register();
    (new DSUtil::CClassFactory(&CPsiConfigFilter::g_Template))->Register();
    (new DSUtil::CClassFactory(&CAnalyzerFilter::g_Template))->Register();
    (new DSUtil::CClassFactory(&CAnalyzerWriterFilter::g_Template))->Register();
    (new DSUtil::CClassFactory(&CVideoAnalyzerFilter::g_Template))->Register();
    (new DSUtil::CClassFactory(&CAudioAnalyzerFilter::g_Template))->Register();

    //(new DSUtil::CClassFactory(&CAnalyzerPropPageConfig::g_Template))->Register();
}

DWORD WINAPI RegisterInternalFiltersForMTAThreadProc(_In_  LPVOID /*lpParameter*/ )
{
	// Register our internal filters for the single MTA in the process
	// so that registrations are accessible from the filter graph manager worker thread
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr)) {
		RegisterInternalFiltersForCurrentApartment();
		// Don't exit from this thread otherwise the registrations will die intermittently
		// Instead, sleep until process exist
		while(true)
			Sleep(0x8FFFFFFF);
	}
	return 0;
}

// Need to call this function when we switch filter graph models
// TODO call CoRevokeClassObject and free CClassFactory objects
static void RegisterInternalFilters()
{
	// Register internal filters for main thread and STA
	RegisterInternalFiltersForCurrentApartment();
	// Register internal filters for single MTA for filter graph manager to use when loading GRF files
	const HANDLE thread_handle = CreateThread(NULL, 0, RegisterInternalFiltersForMTAThreadProc, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
}

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

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!__super::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	CWinApp* pApp = AfxGetApp();
	if (pApp->m_pMainWnd == NULL)
		pApp->m_pMainWnd = this;

	//// enable customization button for all user toolbars
	//BOOL bNameValid;
	//CString strCustomize;
	//bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	//ASSERT(bNameValid);

	//for (int i = 0; i < iMaxUserToolbars; i ++)
	//{
	//	CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
	//	if (pUserToolbar != NULL)
	//	{
	//		pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	//	}
	//}

	return TRUE;
}

void CMainFrame::OnFileClose()
{
	// Call OnClose rather than DestroyWindow() and use different command ID to force calling CDocumentCanCloseFrame to fixup CWinApp::m_pMainWnd
	OnClose();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1) return -1;
	
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

	RegisterInternalFilters();

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
	if (!__super::PreCreateWindow(cs)) return FALSE;

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

BOOL CMainFrame::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if ((nFlags&MK_CONTROL) && (nFlags&MK_SHIFT)) {		// Control+shift wheel change speed
		if (zDelta <= -WHEEL_DELTA || zDelta >= WHEEL_DELTA) {
			const int sel = m_comboRate.GetCurSel() + ((zDelta > 0) ? -1 : 1);
			m_comboRate.SetCurSel(sel);
			OnComboRateChanged();
		}
		return 0;
	} else {
		return __super::OnMouseWheel(nFlags, zDelta, pt);
	}
}

void CMainFrame::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// This feature requires Windows Vista or greater.
	// The symbol _WIN32_WINNT must be >= 0x0600.
	// TODO: Add your message handler code here and/or call default

	__super::OnMouseHWheel(nFlags, zDelta, pt);
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const			{ __super::AssertValid(); }
void CMainFrame::Dump(CDumpContext& dc) const	{ __super::Dump(dc); }
#endif //_DEBUG





//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "graphstudio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//-----------------------------------------------------------------------------
//
//	CgraphstudioApp class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CgraphstudioApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CgraphstudioApp::OnAppAbout)
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

bool CgraphstudioApp::g_showGuidsOfKnownTypes = true;

//-----------------------------------------------------------------------------
//
//	CgraphstudioApp class
//
//-----------------------------------------------------------------------------

CgraphstudioApp::CgraphstudioApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

    SetAppID(_T("CPlusSharp.GraphStudioNext"));
}


// The one and only CgraphstudioApp object

CgraphstudioApp theApp;


// CgraphstudioApp initialization

BOOL CgraphstudioApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

    EnableTaskbarInteraction(TRUE);

	// Initialize OLE libraries
	if (!AfxOleInit()) {
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	AtlAxWinInit();

	SetRegistryKey(_T("MONOGRAM"));
	LoadStdProfileSettings(8);  // Load standard INI file options (including MRU)

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CGraphDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CGraphView));
	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	if (!ProcessShellCommand(cmdInfo)) return FALSE;

	// The one and only window has been initialized, so show and update it
	CMainFrame	*frame = (CMainFrame *)m_pMainWnd;
	CGraphView	*view  = (CGraphView *)frame->GetActiveView();

	// initialize the graph
	view->OnInit();

	m_pMainWnd->SetFocus();
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// if we've been started with a command line parameter
	// do open the file
	if (cmdInfo.m_strFileName != _T("")) {
		view->TryOpenFile(cmdInfo.m_strFileName);
	}

    // Jumplist
    TCHAR szModule[MAX_PATH];
    DWORD dwFLen = GetModuleFileName(nullptr, szModule, MAX_PATH);

    CJumpList m_jumpList;
    m_jumpList.InitializeList();
    m_jumpList.AddKnownCategory(KDC_FREQUENT);
    m_jumpList.AddKnownCategory(KDC_RECENT);
    m_jumpList.AddTask(szModule, L"/filters", L"Filters Dialog", szModule, 2);
    m_jumpList.CommitList();

    if (!wcscmp(m_lpCmdLine, L"/filters"))
        view->OnGraphInsertFilter();

	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	
	GraphStudio::TitleBar		titlebar;
	GraphStudio::URLLabel		url_label;

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, titlebar);
	DDX_Control(pDX, IDC_STATIC_URL, url_label);

	// navigate to this location
	url_label.url = _T("http://code.google.com/p/graph-studio-next/");
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CgraphstudioApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CgraphstudioApp message handlers



int CgraphstudioApp::ExitInstance()
{
    CMediaInfo::FreeInfoCache();

    return CWinApp::ExitInstance();
}

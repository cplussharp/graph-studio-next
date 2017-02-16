//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "graphstudio.h"

#include "VersionNo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Declare APIs for disabling user mode callback exception filter
// See https://support.microsoft.com/en-us/kb/976038 KB 976038
typedef BOOL WINAPI Sig_SetProcessUserModeExceptionPolicy(__in DWORD dwFlags);
		BOOL WINAPI SetProcessUserModeExceptionPolicy(__in DWORD dwFlags);

typedef BOOL WINAPI Sig_GetProcessUserModeExceptionPolicy(__out LPDWORD lpFlags);
		BOOL WINAPI GetProcessUserModeExceptionPolicy(__out LPDWORD lpFlags);

#define PROCESS_CALLBACK_FILTER_ENABLED     0x1

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

bool CgraphstudioApp::g_useInternalGrfParser    = false;
bool CgraphstudioApp::g_SaveXmlAndGrf           = true;
bool CgraphstudioApp::g_SaveInformation		    = true;
bool CgraphstudioApp::g_SaveScreenshot          = true;
bool CgraphstudioApp::g_ClearDocumentBeforeLoad = true;
bool CgraphstudioApp::g_showConsole             = false;
bool CgraphstudioApp::g_showGuidsOfKnownTypes   = true;
bool CgraphstudioApp::g_ReserveLowMemory		= false;	// this should definitely default to false at least on 32bit builds as some filters may not support LARGEADDRESSAWARE and may fail to work.
int	 CgraphstudioApp::g_ScreenshotFormat        = 0;

// Maintain separate settings for 32bit and 64bit reserve low memory as they have very different effects
const TCHAR * const CgraphstudioApp::g_ReserveLowMemoryOption = 
#ifdef _WIN64
	_T("ReserveLowMemory64");
#else
	_T("ReserveLowMemory32");
#endif


CgraphstudioApp::PinResolution CgraphstudioApp::g_ResolvePins = CgraphstudioApp::BY_NAME;

/*
Function to reserve address space to force memory allocations above a limit to reproduce pointer bugs
Source: Bruce Dawson https://randomascii.wordpress.com/2012/02/14/64-bit-made-easy/ - used with permission
*/
void ReserveLowMemory()
{
	static bool s_initialized = false;
	if (s_initialized)
		return;
	s_initialized = true;

	// Start by reserving large blocks of address space, and then
	// gradually reduce the size in order to capture all of the
	// fragments. Technically we should continue down to 64 KB but
	// stopping at 1 MB is sufficient to keep most allocators out.

#ifdef _WIN64
	const size_t LOW_MEM_LINE = 0x100000000LL;
#else
	const size_t LOW_MEM_LINE = 0x080000000LL;		// Test LARGEADDRESSAWARE code by reserving memory below 2GB
#endif

	size_t totalReservation = 0;
	size_t numVAllocs = 0;
	size_t numHeapAllocs = 0;
	size_t oneMB = 1024 * 1024;
	for (size_t size = 256 * oneMB; size >= oneMB; size /= 2)
	{
		for (;;)
		{
			void* p = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
			if (!p)
				break;

			if ((size_t)p >= LOW_MEM_LINE)
			{
				// We don't need this memory, so release it completely.
				VirtualFree(p, 0, MEM_RELEASE);
				break;
			}

			totalReservation += size;
			++numVAllocs;
		}
	}

	// Now repeat the same process but making heap allocations, to use up
	// the already reserved heap blocks that are below the 4 GB line.
	HANDLE heap = GetProcessHeap();
	for (size_t blockSize = 64 * 1024; blockSize >= 16; blockSize /= 2)
	{
		for (;;)
		{
			void* p = HeapAlloc(heap, 0, blockSize);
			if (!p)
				break;

			if ((size_t)p >= LOW_MEM_LINE)
			{
				// We don't need this memory, so release it completely.
				HeapFree(heap, 0, p);
				break;
			}

			totalReservation += blockSize;
			++numHeapAllocs;
		}
	}

	// Perversely enough the CRT doesn't use the process heap. Suck up
	// the memory the CRT heap has already reserved.
	for (size_t blockSize = 64 * 1024; blockSize >= 16; blockSize /= 2)
	{
		for (;;)
		{
			void* p = malloc(blockSize);
			if (!p)
				break;

			if ((size_t)p >= LOW_MEM_LINE)
			{
				// We don't need this memory, so release it completely.
				free(p);
				break;
			}

			totalReservation += blockSize;
			++numHeapAllocs;
		}
	}

	// Print diagnostics showing how many allocations we had to make in
	// order to reserve all of low memory, typically less than 200.
	char buffer[1000];
	sprintf_s(buffer, "Reserve Low Memory : Reserved %1.3f MB (%d vallocs,"
		"%d heap allocs) of low-memory.\n",
		totalReservation / (1024 * 1024.0),
		(int)numVAllocs, (int)numHeapAllocs);
	OutputDebugStringA(buffer);
}

//-----------------------------------------------------------------------------
//
//	CgraphstudioApp class
//
//-----------------------------------------------------------------------------

CgraphstudioApp::CgraphstudioApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
     
#ifdef _WIN64
    SetAppID(_T("CPlusSharp.GraphStudioNext64"));
#else
    SetAppID(_T("CPlusSharp.GraphStudioNext"));
#endif
}


// The one and only CgraphstudioApp object

CgraphstudioApp theApp;


// CgraphstudioApp initialization

BOOL CgraphstudioApp::InitInstance()
{
	// Disable user mode callback exception filter as this interferes with debugging
	HMODULE kernel = LoadLibrary(_T("kernel32.dll"));
	if (kernel) {
		Sig_GetProcessUserModeExceptionPolicy * const getApi = (Sig_GetProcessUserModeExceptionPolicy*)GetProcAddress(kernel, "GetProcessUserModeExceptionPolicy");
		Sig_SetProcessUserModeExceptionPolicy * const setApi = (Sig_SetProcessUserModeExceptionPolicy*)GetProcAddress(kernel, "SetProcessUserModeExceptionPolicy");
		DWORD dwFlags = 0;
		if (getApi && setApi && (*getApi)(&dwFlags)) {
			(*setApi)(dwFlags & (~PROCESS_CALLBACK_FILTER_ENABLED)); 
		}
	}

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

    m_nExitCode = 0;

	__super::InitInstance();

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

    // set exe location in registry
    TCHAR strExeLocation[MAX_PATH];
    if (GetModuleFileName(NULL, strExeLocation, MAX_PATH))
    {
        CPath pathExe(strExeLocation);
        pathExe.Canonicalize();
#ifdef _WIN64
        BOOL t = AfxGetApp()->WriteProfileStringW(_T(""),_T("exeLocation64"), pathExe);
#else
        BOOL t = AfxGetApp()->WriteProfileStringW(_T(""),_T("exeLocation"), pathExe);
#endif

		// if grfx filetype is not registered jet, do it now
		ATL::CRegKey regKey;
		CString strRegFileType = _T(".grfx");
		if (ERROR_SUCCESS != regKey.Open(HKEY_CLASSES_ROOT, strRegFileType, KEY_READ))
			CCliOptionsForm::AssociateFileType();
    }

	bool single_doc = false;

	CDocTemplate* pDocTemplate = NULL;

	if (single_doc) {
		pDocTemplate = new CSingleDocTemplate(
			IDR_MAINFRAME,
			RUNTIME_CLASS(CGraphDoc),
			RUNTIME_CLASS(CMainFrame),       // main SDI frame window
			RUNTIME_CLASS(CGraphView));
	} else {
		pDocTemplate = new CMultiDocTemplate(
			IDR_MAINFRAME,
			RUNTIME_CLASS(CGraphDoc),
			RUNTIME_CLASS(CMainFrame),       // main SDI frame window
			RUNTIME_CLASS(CGraphView));
	}
	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);

	// Do this as early in the process as possible to keep memory used above low memory
	g_ReserveLowMemory = GetProfileInt(_T("Settings"), g_ReserveLowMemoryOption, 0) ? true : false;
	if (g_ReserveLowMemory)
		ReserveLowMemory();

	// Parse command line for standard shell commands, DDE, file open
	ParseCommandLine(m_cmdInfo);
	if (!ProcessShellCommand(m_cmdInfo)) return FALSE;

	// The one and only window has been initialized, so show and update it
	CMainFrame	*frame = (CMainFrame *)m_pMainWnd;
	CGraphView	*view  = (CGraphView *)frame->GetActiveView();

	// initialize the graph
	view->OnInit();

	UINT showCmd = AfxGetApp()->GetProfileIntW(_T("Settings"), _T("ShowCmd"), SW_SHOWNORMAL);
	if (showCmd != SW_SHOWMAXIMIZED)		// Allow only maximized and normal
		showCmd = SW_SHOWNORMAL;

	m_pMainWnd->ShowWindow(showCmd);
	m_pMainWnd->UpdateWindow();
	m_pMainWnd->SetFocus();

    // command line optionen
    if (m_cmdInfo.m_bExitOnError)
        DSUtil::m_bExitOnError = true;

    if (m_cmdInfo.m_bShowCliHelp)
        view->OnShowCliOptions();

	// if we've been started with a command line parameter
	// do open the file
	if (m_cmdInfo.m_strFileName != _T("")) {
		if (view->ShouldOpenInNewDocument(m_cmdInfo.m_strFileName))
			view->OnNewClick();
		view->TryOpenFile(m_cmdInfo.m_strFileName);

        if (m_cmdInfo.m_bNoClock)
            view->RemoveClock();

        if (m_cmdInfo.m_bExitAfterRun)
            view->m_bExitOnStop = true;

        if (m_cmdInfo.m_bRunGraph)
            view->OnPlayClick();

        if (m_cmdInfo.m_bProgressView)
            view->OnViewProgressview();
    }
    else if (m_cmdInfo.m_bRemoteGraph)
    {
        view->OnNewClick();
        CComPtr<IRunningObjectTable>	rot;
        HRESULT hr = GetRunningObjectTable(0, &rot);
	    if (FAILED(hr)) 
        {
            // TODO Show error or exit?
        }
        else
        {
            CComPtr<IEnumMoniker>	emon;
	        CComPtr<IMoniker>		moniker;
	        CComPtr<IBindCtx>		bindctx;
	        ULONG					f;

            hr = CreateBindCtx(0, &bindctx);
	        if (FAILED(hr))
            {
		        // TODO Show error or exit?
	        }
            else
            {
                rot->EnumRunning(&emon);
	            emon->Reset();
	            while (emon->Next(1, &moniker, &f) == NOERROR)
                {
                    bool found = false;

		            // is this a graph object ?
		            LPOLESTR displayname;
		            moniker->GetDisplayName(bindctx, NULL, &displayname);

                    CString	name(displayname);
                    if (name.Find(m_cmdInfo.m_strRemoteGraph) == 0)
                    {
                        view->OnConnectRemote(moniker, name);
                        found = true;
                    }

                    if (displayname) {
			            CComPtr<IMalloc>	alloc;
			            if (SUCCEEDED(CoGetMalloc(0, &alloc))) {
				            alloc->Free(displayname);
			            }
		            }

		            moniker = NULL;

                    if (found)
                        break;
                }
            }
        }
    }

    // Jumplist
    TCHAR szModule[MAX_PATH];
    DWORD dwFLen = GetModuleFileName(nullptr, szModule, MAX_PATH);

    CJumpList m_jumpList;
    m_jumpList.InitializeList();
    m_jumpList.AddKnownCategory(KDC_FREQUENT);
    m_jumpList.AddKnownCategory(KDC_RECENT);
    m_jumpList.AddTask(szModule, L"/filters", L"Filters Dialog", szModule, -170);
    m_jumpList.CommitList();

    if (m_cmdInfo.m_bShowFilters)
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
public:
    CString m_strVerInfo;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	m_strVerInfo.Format(_T("GraphStudioNext %s\n\n"), (LPCTSTR)CString(VER_FILE_VERSION_STR));
    m_strVerInfo.Append(CString(VER_FILE_DESCRIPTION_STR));
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, titlebar);
    DDX_Control(pDX, IDC_STATIC_URL, url_label);
    DDX_Text(pDX, IDC_STATIC_VERSION, m_strVerInfo);

    // navigate to this location
    url_label.url = _T("https://github.com/cplussharp/graph-studio-next");
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

    int ret = __super::ExitInstance();
    if (m_nExitCode != 0)
        ret = m_nExitCode;

    return ret;
}

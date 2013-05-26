//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "graphView.h"
#include <atlbase.h>
#include <atlpath.h>

#include <io.h>
#include <fcntl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	void ShowConsole(bool show)
	{
		if (show) {
			// allocate a console for this app
			AllocConsole();

			static bool done_init = false;	// only do setup the first time the console is shown
			if (!done_init) {
				done_init = true;

				int hConHandle;
				long lStdHandle;
				FILE *fp;

				// redirect unbuffered STDOUT to the console
				lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
				hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
				fp = _fdopen( hConHandle, "w" );
				*stdout = *fp;

				setvbuf( stdout, NULL, _IONBF, 0 );
			}
		} else {

			FreeConsole();
		}
	}

// Modified from MFC viewscrll.cpp
UINT PASCAL _AfxGetMouseScrollLines()
{
	static UINT uCachedScrollLines;
	static BOOL _afxGotScrollLines; 

	// if we've already got it and we're not refreshing,
	// return what we've already got

	if (_afxGotScrollLines)
		return uCachedScrollLines;

	// see if we can find the mouse window

	_afxGotScrollLines = TRUE;

	static UINT msgGetScrollLines;
	static WORD nRegisteredMessage;

	// couldn't use the window -- try system settings
	uCachedScrollLines = 3; // reasonable default
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, 0);

	return uCachedScrollLines;
}

}

//-----------------------------------------------------------------------------
//
//	CGraphView class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CGraphView, DisplayView)

BEGIN_MESSAGE_MAP(CGraphView, GraphStudio::DisplayView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_BUTTON_PLAY, &CGraphView::OnPlayClick)
	ON_COMMAND(ID_BUTTON_PAUSE, &CGraphView::OnPauseClick)
	ON_COMMAND(ID_BUTTON_STOP, &CGraphView::OnStopClick)
	ON_COMMAND(ID_BUTTON_STEP, &CGraphView::OnFrameStepClick)
	ON_COMMAND(ID_BUTTON_PLAYPAUSE, &CGraphView::OnPlayPauseToggleClick)
    ON_COMMAND(ID_BUTTON_INTELLIGENT, &CGraphView::OnConnectModeIntelligentClick)
	ON_COMMAND(ID_BUTTON_DIRECT, &CGraphView::OnConnectModeDirectClick)
    ON_COMMAND(ID_BUTTON_DIRECT_WITHMT, &CGraphView::OnConnectModeDirectWmtClick)
	ON_COMMAND(ID_BUTTON_REFRESH, &CGraphView::OnRefreshFilters)
	ON_COMMAND(ID_BUTTON_SEEK, &CGraphView::OnSeekClick)
    ON_COMMAND(ID_BUTTON_ADDFILTER, &CGraphView::OnGraphInsertFilter)
    ON_COMMAND(ID_BUTTON_REMOVE_CONNECTIONS, &CGraphView::OnRemoveConnections)
	ON_COMMAND(ID_OPTIONS_EXACTMATCH, &CGraphView::OnOptionsExactMatchClick)
    ON_COMMAND(ID_OPTIONS_USEMEDIAINFO, &CGraphView::OnOptionsUseMediaInfoClick)
    ON_COMMAND(ID_OPTIONS_SHOWGUIDOFKNOWNTYPES, &CGraphView::OnOptionsShowGuidOfKnownTypesClick)
	ON_COMMAND(ID_FILE_NEW, &CGraphView::OnNewClick)
	ON_COMMAND(ID_FILE_OPEN, &CGraphView::OnFileOpenClick)
	ON_COMMAND(ID_FILE_SAVE, &CGraphView::OnFileSaveClick)
	ON_COMMAND(ID_FILE_SAVE_AS, &CGraphView::OnFileSaveAsClick)
	ON_COMMAND(ID_FILE_RENDERFILE, &CGraphView::OnRenderFileClick)
	ON_COMMAND(ID_FILE_RENDERURL, &CGraphView::OnRenderUrlClick)
	ON_COMMAND(ID_FILE_CONNECTTOREMOTEGRAPH, &CGraphView::OnConnectRemote)
	ON_COMMAND(ID_FILE_DISCONNECTFROMREMOTEGRAPH, &CGraphView::OnDisconnectRemote)
	ON_COMMAND(ID_GRAPH_INSERTFILTER, &CGraphView::OnGraphInsertFilter)
    ON_COMMAND(ID_GRAPH_INSERTFILTERFROMFILE, &CGraphView::OnGraphInsertFilterFromFile)
	ON_COMMAND(ID_VIEW_GRAPHEVENTS, &CGraphView::OnViewGraphEvents)
    ON_COMMAND(ID_VIEW_GRAPHSTATISTICS, &CGraphView::OnViewGraphStatistics)
	ON_COMMAND(ID_LIST_MRU_CLEAR, &CGraphView::OnClearMRUClick)
	ON_COMMAND(ID_GRAPH_MAKEGRAPHSCREENSHOT, &CGraphView::OnGraphScreenshot)
	ON_COMMAND(ID_GRAPH_USECLOCK, &CGraphView::OnUseClock)
	ON_COMMAND_RANGE(ID_LIST_MRU_FILE0, ID_LIST_MRU_FILE0+10, &CGraphView::OnDummyEvent)
    ON_COMMAND_RANGE(ID_AUDIO_SOURCE0, ID_AUDIO_SOURCE0+50, &CGraphView::OnDummyEvent)
	ON_COMMAND_RANGE(ID_VIDEO_SOURCE0, ID_VIDEO_SOURCE0+50, &CGraphView::OnDummyEvent)
	ON_COMMAND_RANGE(ID_AUDIO_RENDERER0, ID_AUDIO_RENDERER0+50, &CGraphView::OnDummyEvent)
	ON_COMMAND_RANGE(ID_VIDEO_RENDERER0, ID_VIDEO_RENDERER0+50, &CGraphView::OnDummyEvent)
    ON_COMMAND_RANGE(ID_INTERNAL_FILTER0, ID_INTERNAL_FILTER0+50, &CGraphView::OnDummyEvent)
	ON_COMMAND_RANGE(ID_FAVORITE_FILTER, ID_FAVORITE_FILTER+500, &CGraphView::OnDummyEvent)
	ON_COMMAND_RANGE(ID_PREFERRED_VIDEO_RENDERER, ID_PREFERRED_VIDEO_RENDERER+100, &CGraphView::OnDummyEvent)

	ON_WM_KEYDOWN()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_DROPFILES()

	ON_UPDATE_COMMAND_UI(ID_GRAPH_USECLOCK, &CGraphView::OnUpdateUseClock)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_INTELLIGENT, &CGraphView::OnUpdateConnectModeIntelligent)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DIRECT, &CGraphView::OnUpdateConnectModeDirect)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_DIRECT_WITHMT, &CGraphView::OnUpdateConnectModeDirectWmt)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_REMOVE_CONNECTIONS, &CGraphView::OnUpdateRemoveConnections)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_EXACTMATCH, &CGraphView::OnUpdateOptionsExactMatch)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_USEMEDIAINFO, &CGraphView::OnUpdateOptionsUseMediaInfo)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_SHOWGUIDOFKNOWNTYPES, &CGraphView::OnUpdateShowGuidOfKnownTypes)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PLAY, &CGraphView::OnUpdatePlayButton)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PAUSE, &CGraphView::OnUpdatePauseButton)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_STOP, &CGraphView::OnUpdateStopButton)
	ON_UPDATE_COMMAND_UI(ID_FILE_RENDERFILE, &CGraphView::OnUpdateRenderMediaFile)
	ON_UPDATE_COMMAND_UI(ID_FILE_CONNECTTOREMOTEGRAPH, &CGraphView::OnUpdateConnectRemote)
	ON_UPDATE_COMMAND_UI(ID_FILE_DISCONNECTFROMREMOTEGRAPH, &CGraphView::OnUpdateDisconnectRemote)
	
	ON_COMMAND(ID_VIEW_TEXTINFORMATION, &CGraphView::OnViewTextInformation)
	ON_COMMAND(ID_GRAPH_INSERTFILESOURCE, &CGraphView::OnGraphInsertFileSource)
    ON_COMMAND(ID_GRAPH_INSERTTEEFILTER, &CGraphView::OnGraphInsertTeeFilter)
	ON_COMMAND(ID_GRAPH_INSERTFILEWRITER, &CGraphView::OnGraphInsertFileSink)
	ON_COMMAND(ID_VIEW_50, &CGraphView::OnView50)
	ON_COMMAND(ID_VIEW_75, &CGraphView::OnView75)
	ON_COMMAND(ID_VIEW_100, &CGraphView::OnView100)
	ON_COMMAND(ID_VIEW_150, &CGraphView::OnView150)
	ON_COMMAND(ID_VIEW_200, &CGraphView::OnView200)
	ON_UPDATE_COMMAND_UI(ID_VIEW_50, &CGraphView::OnUpdateView50)
	ON_UPDATE_COMMAND_UI(ID_VIEW_75, &CGraphView::OnUpdateView75)
	ON_UPDATE_COMMAND_UI(ID_VIEW_100, &CGraphView::OnUpdateView100)
	ON_UPDATE_COMMAND_UI(ID_VIEW_150, &CGraphView::OnUpdateView150)
	ON_UPDATE_COMMAND_UI(ID_VIEW_200, &CGraphView::OnUpdateView200)
	ON_COMMAND(ID_FILE_ADDMEDIAFILE, &CGraphView::OnFileAddmediafile)
	ON_COMMAND(ID_FILTERS_DOUBLESELECTEDFILTERS, &CGraphView::OnFiltersDouble)
	ON_COMMAND(ID_VIEW_DECREASEZOOMLEVEL, &CGraphView::OnViewDecreasezoomlevel)
	ON_COMMAND(ID_VIEW_INCREASEZOOMLEVEL, &CGraphView::OnViewIncreasezoomlevel)
	ON_COMMAND(ID_FILTERS_MANAGEFAVORITES, &CGraphView::OnFiltersManageFavorites)
    ON_COMMAND(ID_FILTERS_MANAGEBLACKLIST, &CGraphView::OnFiltersManageBlacklist)
    ON_UPDATE_COMMAND_UI(ID_FILTERS_MANAGEBLACKLIST, &CGraphView::OnUpdateFiltersManageBlacklist)
	ON_COMMAND(ID_OPTIONS_DISPLAYASFILENAME, &CGraphView::OnOptionsDisplayFileName)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_DISPLAYASFILENAME, &CGraphView::OnUpdateOptionsDisplayFileName)
	ON_COMMAND(ID_VIEW_PROGRESSVIEW, &CGraphView::OnViewProgressview)
	ON_COMMAND(ID_FILE_SAVEASXML, &CGraphView::OnFileSaveasxml)
	ON_COMMAND(ID_AUTOMATICRESTART_SCHEDULE, &CGraphView::OnAutomaticrestartSchedule)
	ON_COMMAND(ID_VIEW_DECODERPERFORMANCE, &CGraphView::OnViewDecoderPerformance)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_ABORTRENDER, &CGraphView::OnUpdateOptionsAbortrender)
	ON_COMMAND(ID_OPTIONS_ABORTRENDER, &CGraphView::OnOptionsAbortrender)
	ON_COMMAND(ID_VIEW_GRAPHCONSTRUCTIONREPORT, &CGraphView::OnViewGraphconstructionreport)
    ON_COMMAND(ID_HELP_GUIDLOOKUP, &CGraphView::OnHelpGuidLookup)
    ON_COMMAND(ID_HELP_HRESULTLOOKUP, &CGraphView::OnHelpHresultLookup)
    ON_COMMAND(ID_HELP_COMMANDLINEOPTIONS, &CGraphView::OnShowCliOptions)
    ON_COMMAND(ID_OPTIONS_CONFIGURESBE, &CGraphView::OnConfigureSbe)
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_COMMAND(ID_VIEW_DECREASEHORIZONTALSPACING, &CGraphView::OnViewDecreaseHorizontalSpacing)
	ON_COMMAND(ID_VIEW_INCREASEHORIZONTALSPACING, &CGraphView::OnViewIncreaseHorizontalSpacing)
	ON_COMMAND(ID_VIEW_DECREASEVERTICALSPACING, &CGraphView::OnViewDecreaseVerticalSpacing)
	ON_COMMAND(ID_VIEW_INCREASEVERTICALSPACING, &CGraphView::OnViewIncreaseVerticalSpacing)
	ON_COMMAND(ID_FILEOPTIONS_LOADPINSBYNAME, &CGraphView::OnFileoptionsLoadpinsbyname)
	ON_UPDATE_COMMAND_UI(ID_FILEOPTIONS_LOADPINSBYNAME, &CGraphView::OnUpdateFileoptionsLoadpinsbyname)
	ON_COMMAND(ID_FILEOPTIONS_LOADPINSBYINDEX, &CGraphView::OnFileoptionsLoadpinsbyindex)
	ON_UPDATE_COMMAND_UI(ID_FILEOPTIONS_LOADPINSBYINDEX, &CGraphView::OnUpdateFileoptionsLoadpinsbyindex)
	ON_COMMAND(ID_FILEOPTIONS_LOADPINSBYID, &CGraphView::OnFileoptionsLoadpinsbyid)
	ON_UPDATE_COMMAND_UI(ID_FILEOPTIONS_LOADPINSBYID, &CGraphView::OnUpdateFileoptionsLoadpinsbyid)
	ON_COMMAND(ID_OPTIONS_SHOWCONSOLEWINDOW, &CGraphView::OnOptionsShowconsolewindow)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SHOWCONSOLEWINDOW, &CGraphView::OnUpdateOptionsShowconsolewindow)
	ON_COMMAND(ID_OPTIONS_USEINTERNALGRFFILEPARSER, &CGraphView::OnOptionsUseinternalgrffileparser)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_USEINTERNALGRFFILEPARSER, &CGraphView::OnUpdateOptionsUseinternalgrffileparser)
	ON_COMMAND(ID_FILE_ADDSOURCEFILTER, &CGraphView::OnFileAddSourceFilter)
	ON_COMMAND(ID_FILE_ADDFILESOURCE, &CGraphView::OnFileAddFileSourceAsync)
	ON_COMMAND(ID_CLSID_FILTERGRAPH, &CGraphView::OnClsidFiltergraph)
	ON_COMMAND(ID_CLSID_FILTERGRAPH_NO_THREAD, &CGraphView::OnClsidFiltergraphNoThread)
	ON_COMMAND(ID_CLSID_FILTERGRAPH_PRIVATE_THREAD, &CGraphView::OnClsidFiltergraphPrivateThread)
	ON_UPDATE_COMMAND_UI(ID_CLSID_FILTERGRAPH, &CGraphView::OnUpdateClsidFiltergraph)
	ON_UPDATE_COMMAND_UI(ID_CLSID_FILTERGRAPH_NO_THREAD, &CGraphView::OnUpdateClsidFiltergraphNoThread)
	ON_UPDATE_COMMAND_UI(ID_CLSID_FILTERGRAPH_PRIVATE_THREAD, &CGraphView::OnUpdateClsidFiltergraphPrivateThread)
	END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CGraphView class
//
//-----------------------------------------------------------------------------

CGraphView::CGraphView()
	: form_construction(NULL)
	, form_filters(NULL)
	, form_events(NULL)
	, form_textinfo(NULL)
	, form_schedule(NULL)
	, form_favorites(NULL)
    , form_blacklist(NULL)
	, form_progress(NULL)
	, form_volume(NULL)
	, form_seek(NULL)
    , form_statistic(NULL)
	, form_dec_performance(NULL)
    , form_guidlookup(NULL)
    , form_hresultlookup(NULL)
	, document_type(NONE)
	, last_start_time_ns(0LL)
	, last_stop_time_ns(0LL)
    , m_bExitOnStop(false)
{
}

CGraphView::~CGraphView()
{
	if (form_construction) { form_construction->DestroyWindow(); delete form_construction; form_construction = NULL; }
	if (form_volume) { form_volume->DestroyWindow(); delete form_volume; form_volume = NULL; }
	if (form_progress) { form_progress->DestroyWindow(); delete form_progress; form_progress = NULL; }
	if (form_dec_performance) { form_dec_performance->DestroyWindow(); delete form_dec_performance; form_dec_performance = NULL; }
	if (form_filters) { form_filters->DestroyWindow(); delete form_filters; form_filters = NULL; }
	if (form_events) { form_events->DestroyWindow(); delete form_events; form_events = NULL; }
	if (form_schedule) { form_schedule->DestroyWindow(); delete form_schedule; form_schedule = NULL; }
	if (form_seek) { form_seek->DestroyWindow(); delete form_seek; form_seek = NULL; }
    if (form_statistic) { form_statistic->DestroyWindow(); delete form_statistic; form_statistic = NULL; }
	if (form_textinfo) { form_textinfo->DestroyWindow(); delete form_textinfo; form_textinfo = NULL; }
	if (form_favorites) { form_favorites->DestroyWindow(); delete form_favorites; form_favorites = NULL; }
    if (form_blacklist) { form_blacklist->DestroyWindow(); delete form_blacklist; form_blacklist = NULL; }
    if (form_guidlookup) { form_guidlookup->DestroyWindow(); delete form_guidlookup; form_guidlookup = NULL; }
    if (form_hresultlookup) { form_hresultlookup->DestroyWindow(); delete form_hresultlookup; form_hresultlookup = NULL; }
}

BOOL CGraphView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!__super::PreCreateWindow(cs)) return FALSE;

	return TRUE;
}

// CGraphView printing

BOOL CGraphView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CGraphView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CGraphView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CGraphView::OnDestroy()
{
	SaveWindowPosition();

	ClosePropertyPages();
	graph.MakeNew();

	__super::OnDestroy();
}

BOOL CALLBACK MonitorEnum(HMONITOR monitor, HDC dc, LPRECT rect, LPARAM data)
{
	CGraphView	*view = (CGraphView*)data;
	if (!view) return FALSE;
	view->OnMonitorCallback(monitor, dc, rect);
	return TRUE;
}

void CGraphView::OnMonitorCallback(HMONITOR monitor, HDC dc, LPRECT rect)
{
	// append the monitor
	monitors.push_back(monitor);
}

void CGraphView::LoadWindowPosition()
{
	int	x,y,cx,cy;

	x = AfxGetApp()->GetProfileInt(_T("Settings"), _T("left"), 100);
	y = AfxGetApp()->GetProfileInt(_T("Settings"), _T("top"), 100);
	cx = AfxGetApp()->GetProfileInt(_T("Settings"), _T("width"), 640);
	cy = AfxGetApp()->GetProfileInt(_T("Settings"), _T("height"), 320);

	monitors.clear();
	EnumDisplayMonitors(NULL, NULL, MonitorEnum, (LPARAM)this);

	bool		visible = false;

	// make sure the window can be seen
	for (UINT i=0; i<monitors.size(); i++) {
		MONITORINFO			mi;
		memset(&mi, 0, sizeof(mi));
		mi.cbSize		= sizeof(mi);

		GetMonitorInfo(monitors[i], &mi);

		// 50-pixel borders 
		if (x+cx-50 > mi.rcWork.left	&& x+50 < mi.rcWork.right &&
			y+cy-50 > mi.rcWork.top		&& y+50 < mi.rcWork.bottom
			) {
			visible = true;
			break;
		}
	}

	// should be fine
	if (!visible) {
		x = 100;
		y = 100;
		cx = 640;
		cy = 320;
	}

	GetParentFrame()->SetWindowPos(NULL, x, y, cx, cy, SWP_SHOWWINDOW);
}

void CGraphView::SaveWindowPosition()
{
	CRect	rc;
	GetParentFrame()->GetWindowRect(&rc);

	WINDOWPLACEMENT placement;
	memset(&placement, 0, sizeof(placement));
	placement.length = sizeof(placement);

	if (GetParentFrame()->GetWindowPlacement(&placement)) {
		AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ShowCmd"), placement.showCmd);
		rc = placement.rcNormalPosition;
	}

	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("left"), rc.left);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("top"), rc.top);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("width"), rc.Width());
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("height"), rc.Height());
}

// CGraphView diagnostics

#ifdef _DEBUG
void CGraphView::AssertValid() const
{
	CView::AssertValid();
}

void CGraphView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGraphDoc* CGraphView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGraphDoc)));
	return (CGraphDoc*)m_pDocument;
}
#endif //_DEBUG


// CGraphView message handlers
void CGraphView::OnInit()
{
	DragAcceptFiles(TRUE);

	// set the parent view
	CMainFrame	*frm = (CMainFrame*)GetParent();
	frm->view = this;

	LoadWindowPosition();

	// initialize our event logger
	form_events = new CEventsForm(NULL);
	form_events->view = this;
	form_events->DoCreateDialog();

	// initialize schedule
	form_schedule = new CScheduleForm(NULL);
	form_schedule->view = this;
	form_schedule->DoCreateDialog();

	form_textinfo = new CTextInfoForm(NULL);
	form_textinfo->view = this;
    form_textinfo->DoCreateDialog();

	form_progress = new CProgressForm(NULL);
	form_progress->view = this;
	form_progress->Create(IDD_DIALOG_PROGRESS);

	graph.wndEvents = *form_events;
	graph.MakeNew();

	// set the seeking bar data
	CMainFrame *frame = (CMainFrame*)GetParentFrame();
	frame->m_wndSeekingBar.SetGraphView(this);

	mru.Load();

	int zoom_level = AfxGetApp()->GetProfileInt(_T("Settings"), _T("Zoom"), 100);
	switch (zoom_level) {
	case 200:	OnView200(); break;
	case 150:	OnView150(); break;
	case 75:	OnView75(); break;
	case 50:	OnView50(); break;
	default:	OnView100(); break;
	}

	render_params.use_media_info = AfxGetApp()->GetProfileInt(_T("Settings"), _T("UseMediaInfo"), 1) ? true : false;

	// load default renderer
	CString		def_vr = AfxGetApp()->GetProfileString(_T("Settings"), _T("Pref_Video_Renderer"), _T(""));
	render_params.preferred_video_renderer = def_vr;
	render_params.video_renderers = &video_renderers;

    int connectMode = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ConnectMode"), 0);
    if(connectMode<0) connectMode = 0;
    else if(connectMode>2) connectMode = 2;
    render_params.connect_mode = connectMode;

	CgraphstudioApp::g_showConsole = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ShowConsoleWindow"), 0) ? true : false;
	if (CgraphstudioApp::g_showConsole)
		ShowConsole(true);				// Don't do anything on startup unless show console setting is true

	CgraphstudioApp::g_useInternalGrfParser = AfxGetApp()->GetProfileInt(_T("Settings"), _T("UseInternalGrfParser"), 0) ? true : false;

	int showGuids = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ShowGuidsOfKnownTypes"), 1);
    CgraphstudioApp::g_showGuidsOfKnownTypes = showGuids != 0;

	CgraphstudioApp::g_ResolvePins = (CgraphstudioApp::PinResolution) AfxGetApp()->GetProfileInt(_T("Settings"), _T("ResolvePins"), 
		CgraphstudioApp::BY_NAME);

	UpdateGraphState();
	UpdateMRUMenu();

	// load favorites
	GraphStudio::BookmarkedFilters * const favorites = CFavoritesForm::GetFavoriteFilters();

	form_favorites = new CFavoritesForm();
	form_favorites->view = this;
	favorites->Load();
	form_favorites->DoCreateDialog();

	GraphStudio::BookmarkedFilters * const blacklisted = CFavoritesForm::GetBlacklistedFilters();
	blacklisted->Load();

	// trick to refresh menu in a while...
	SetTimer(1001, 20, NULL);
}

LRESULT CGraphView::OnWmCommand(WPARAM wParam, LPARAM lParam)
{
	/*
		For some strange reason MFC kept blocking several IDs
		from the command range so I had to do a little bypass.
	*/

	int		id = LOWORD(wParam);

    if (id >= ID_AUDIO_SOURCE0 && id < ID_AUDIO_SOURCE0 + 50) {
		OnAudioSourceClick(id);
	} else
	if (id >= ID_VIDEO_SOURCE0 && id < ID_VIDEO_SOURCE0 + 50) {
		OnVideoSourceClick(id);
	} else
	if (id >= ID_AUDIO_RENDERER0 && id < ID_AUDIO_RENDERER0 + 50) {
		OnAudioRendererClick(id);
	} else
	if (id >= ID_VIDEO_RENDERER0 && id < ID_VIDEO_RENDERER0 + 50) {
		OnVideoRendererClick(id);
	} else
        if (id >= ID_INTERNAL_FILTER0 && id < ID_INTERNAL_FILTER0 + 50) {
		OnInternalFilterClick(id);
	} else
	if (id >= ID_PREFERRED_VIDEO_RENDERER && id < ID_PREFERRED_VIDEO_RENDERER + 100) {
		OnPreferredVideoRendererClick(id);
	} else
	if (id >= ID_FAVORITE_FILTER && id < ID_FAVORITE_FILTER + 500) {
		OnFavoriteFilterClick(id);
	} else
	if (id >= ID_LIST_MRU_FILE0 && id < ID_LIST_MRU_FILE0 + 10) {
		OnMRUClick(id);
	}	

	return 0;
}

void CGraphView::OnPreferredVideoRendererClick(UINT nID)
{
	int		id = nID - ID_PREFERRED_VIDEO_RENDERER;

	// default ?
	if (id <= 0 || (id > video_renderers.filters.GetCount())) {
		render_params.preferred_video_renderer = _T("");
	} else {
		id -= 1;

		// store the name
		render_params.preferred_video_renderer = video_renderers.filters[id].moniker_name;

	}

	// save the value to the registry
	AfxGetApp()->WriteProfileString(_T("Settings"), _T("Pref_Video_Renderer"), render_params.preferred_video_renderer);

	UpdatePreferredVideoRenderersMenu();
}

void CGraphView::UpdatePreferredVideoRenderersMenu()
{
	/*	Not working yet :(
	*/

#if 0
	CMenu	newmenu;		
	CMenu	*menu;

	int		i;

	MENUITEMINFO		info;
	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask = MIIM_SUBMENU;		// we want to see if this menu item has an attached submenu

	optionsmenu->GetMenuItemInfo(ID_OPTIONS_PREFERREDVIDEORENDERER, &info);

	if (info.hSubMenu != NULL) { 
		// we can use the one that already exists
		menu = optionsmenu->GetSubMenu(5);
	} else {
		// create a new submenu

		newmenu.CreatePopupMenu();

		optionsmenu->ModifyMenu(ID_OPTIONS_PREFERREDVIDEORENDERER, MF_BYCOMMAND | MF_POPUP | MF_STRING, 
		 					   (UINT_PTR)newmenu.m_hMenu, _T("Preferred Video Renderer"));
		newmenu.Detach();
		menu = optionsmenu->GetSubMenu(5);
	}

	if (!menu) return ;

	// now we can kick all items that might be there
	while (menu->GetMenuItemCount() > 0) menu->RemoveMenu(0, MF_BYPOSITION);

	// now add the fresh new items
	bool	pref_default = (render_params.preferred_video_renderer == _T("") ? true : false);
	if (pref_default) {
		menu->InsertMenu(0, MF_CHECKED | MF_BYPOSITION | MF_STRING, ID_PREFERRED_VIDEO_RENDERER + 0, _T("Default"));
	} else {
		menu->InsertMenu(0, MF_BYPOSITION | MF_STRING, ID_PREFERRED_VIDEO_RENDERER + 0, _T("Default"));
	}
	menu->InsertMenu(1, MF_SEPARATOR);

	for (i=0; i<video_renderers.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = video_renderers.filters[i];

		int flags	= MF_STRING;
		if (filter.moniker_name == render_params.preferred_video_renderer) flags |= MF_CHECKED;
		menu->InsertMenu(menu->GetMenuItemCount(), flags, ID_PREFERRED_VIDEO_RENDERER + 1 + i, _T("&") + filter.name);
	}
#endif

}

void CGraphView::UpdateRenderersMenu()
{
	int		i;

	audio_sources.EnumerateAudioSources();
	video_sources.EnumerateVideoSources();
	audio_renderers.EnumerateAudioRenderers();
	video_renderers.EnumerateVideoRenderers();
    internal_filters.EnumerateInternalFilters();

	CMenu	*mainmenu  = GetParentFrame()->GetMenu();
	CMenu	*graphmenu = mainmenu->GetSubMenu(3);
	CMenu	audio_source_menu, video_source_menu, audio_render_menu, video_render_menu, internal_filter_menu;

    // fill in audio sources
	audio_source_menu.CreatePopupMenu();
	for (i=0; i<audio_sources.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = audio_sources.filters[i];
        if(!filter.file_exists) continue;
		audio_source_menu.InsertMenu(i, MF_STRING, ID_AUDIO_SOURCE0 + i, _T("&") + filter.name);
	}

	graphmenu->ModifyMenu(ID_GRAPH_INSERTAUDIOSOURCE, MF_BYCOMMAND | MF_POPUP | MF_STRING, 
						  (UINT_PTR)audio_source_menu.m_hMenu, _T("Insert Audio Source"));

	audio_source_menu.Detach();

	// fill in video sources
	video_source_menu.CreatePopupMenu();

	for (i=0; i<video_sources.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = video_sources.filters[i];
        if(!filter.file_exists) continue;
		video_source_menu.InsertMenu(i, MF_STRING, ID_VIDEO_SOURCE0 + i, _T("&") + filter.name);
	}

	graphmenu->ModifyMenu(ID_GRAPH_INSERTVIDEOSOURCE, MF_BYCOMMAND | MF_POPUP | MF_STRING, 
						  (UINT_PTR)video_source_menu.m_hMenu, _T("Insert Video Source"));

	video_source_menu.Detach();

	// fill in audio renderers
	audio_render_menu.CreatePopupMenu();
	for (i=0; i<audio_renderers.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = audio_renderers.filters[i];
        if(!filter.file_exists) continue;
		audio_render_menu.InsertMenu(i, MF_STRING, ID_AUDIO_RENDERER0 + i, _T("&") + filter.name);
	}

	graphmenu->ModifyMenu(ID_GRAPH_INSERTAUDIORENDERER, MF_BYCOMMAND | MF_POPUP | MF_STRING, 
						  (UINT_PTR)audio_render_menu.m_hMenu, _T("Insert Audio Renderer"));

	audio_render_menu.Detach();

	// fill in video renderers
	video_render_menu.CreatePopupMenu();

	for (i=0; i<video_renderers.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = video_renderers.filters[i];
        if(!filter.file_exists) continue;
		video_render_menu.InsertMenu(i, MF_STRING, ID_VIDEO_RENDERER0 + i, _T("&") + filter.name);
	}

	graphmenu->ModifyMenu(ID_GRAPH_INSERTVIDEORENDERER, MF_BYCOMMAND | MF_POPUP | MF_STRING, 
						  (UINT_PTR)video_render_menu.m_hMenu, _T("Insert Video Renderer"));

	video_render_menu.Detach();

    // fill in internal filters
	internal_filter_menu.CreatePopupMenu();

	for (i=0; i<internal_filters.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = internal_filters.filters[i];
		internal_filter_menu.InsertMenu(i, MF_STRING, ID_INTERNAL_FILTER0 + i, _T("&") + filter.name);
	}

	graphmenu->ModifyMenu(ID_GRAPH_INSERTINTERNALFILTER, MF_BYCOMMAND | MF_POPUP | MF_STRING, 
						  (UINT_PTR)internal_filter_menu.m_hMenu, _T("Insert Internal Filter"));

	internal_filter_menu.Detach();

	UpdatePreferredVideoRenderersMenu();
}

void CGraphView::UpdateMRUMenu()
{
	CMenu	*mainmenu = GetParentFrame()->GetMenu();
	if (!mainmenu) return ;
	CMenu	*filemenu = mainmenu->GetSubMenu(0);
	mru.GenerateMenu(filemenu);
}

void CGraphView::OnFrameStepClick()
{
	// do a frame step
	graph.DoFrameStep();

	// update the graph state timers
	UpdateGraphState();
}

void CGraphView::OnPlayClick()
{
	// backup way of setting start time for graphs that don't generate EC_PAUSED events when started
	last_stop_time_ns = last_start_time_ns = timer.GetTimeNS();

	const HRESULT hr = graph.DoPlay();
	DSUtil::ShowError(hr, _T("Error starting playback"));
	UpdateGraphState();
}

void CGraphView::OnStopClick()
{
	if (SUCCEEDED(graph.DoStop())) {
	}
	UpdateGraphState();
}

void CGraphView::OnPauseClick()
{
	graph.DoPause();
	UpdateGraphState();
}

void CGraphView::OnPlayPauseToggleClick()
{
	FILTER_STATE	state;
	int ret = graph.GetState(state, 20);
	if (ret < 0) {
		return ;
	}

	// toggle state
	if (state == State_Stopped || state == State_Paused) {
		OnPlayClick();
	} else {
		OnPauseClick();
	}
}

void CGraphView::OnViewDecoderPerformance()
{
	if (!form_dec_performance) {
		form_dec_performance = new CDecPerformanceForm(this);
		form_dec_performance->DoCreateDialog();
	}
	form_dec_performance->ShowWindow(SW_SHOW);
	form_dec_performance->SetActiveWindow();
}


void CGraphView::OnSeekClick()
{
	if (!form_seek) {
		form_seek = new CSeekForm();
		form_seek->view = this;
		form_seek->DoCreateDialog();
	}
	form_seek->ShowWindow(SW_SHOW);
	form_seek->UpdateGraphPosition();
	form_seek->SetActiveWindow();
}

void CGraphView::OnNewClick()
{
	if (form_schedule) {
		// reset schedule events for new graphs
		form_schedule->ClearEvents();
	}

	KillTimer(CGraphView::TIMER_REMOTE_GRAPH_STATE);

	if (!graph.is_remote) {
		OnStopClick();
	}

	// stop any running decoder tests
	if (form_dec_performance) {
		form_dec_performance->StopTiming();
	}

	ClosePropertyPages();
	graph.MakeNew();

	document_filename.Empty();
	document_type = NONE;
	UpdateTitleBar();

	Invalidate();
    AfxGetMainWnd()->SendMessage(WM_UPDATEPLAYRATE);
}

void CGraphView::UpdateTitleBar()
{
	if (!GetDocument())
		return;

	if (document_filename.IsEmpty()) {
		GetDocument()->SetTitle(_T("Untitled"));
		return;
	}

	const CPath document_path(document_filename);
	const int name_pos = document_path.FindFileName();
	CString	short_name = document_path;
	short_name.Delete(0, name_pos);

	GetDocument()->SetTitle(short_name);
}

void CGraphView::OnFileSaveClick()
{
	HRESULT hr = S_OK;

	switch (document_type) {
	case GRF:
		hr = graph.SaveGRF(document_filename);
		break;
	case XML:
		hr = graph.SaveXML(document_filename);
		break;
	case NONE:
		FileSaveAs(GRF);
		return;
	}

	if (SUCCEEDED(hr)) {
		// updatujeme MRU list
		mru.NotifyEntry(document_filename);
		UpdateMRUMenu();
	} else {
		DSUtil::ShowError(hr, _T("Can't save file"));
	}
}

void CGraphView::OnFileSaveAsClick()
{
	FileSaveAs(GRF);
}

void CGraphView::OnFileSaveasxml()
{
	FileSaveAs(XML);
}

void CGraphView::FileSaveAs(DocumentType input_type)
{
	// nabrowsujeme subor
	// NB references to indices below
	CString		filter;
	filter =	_T("GraphStudio XML Files (xml)|*.xml|");
	filter +=	_T("GraphEdit Files|*.grf|");
	filter +=	_T("All Graph Files|*.grf;*.xml|");
	filter +=	_T("All Files|*.*|");

	CFileDialog dlg(FALSE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,filter);

	// Pick initial type filter
	dlg.m_ofn.nFilterIndex = XML==input_type ? 1 : 2;

	// make sure document extension is not set by default (e.g. if graph created by render media file)
	CPath input_path(document_filename);
	input_path.RemoveExtension();

	// set default name
	CString input_filename = input_path;
	dlg.m_ofn.lpstrFile = input_filename.GetBufferSetLength(MAX_PATH + 1);
	dlg.m_ofn.nMaxFile = MAX_PATH + 1;

	if (dlg.DoModal() == IDOK) {
		CString filename = dlg.GetPathName();
		DocumentType save_as = GRF;

		CPath output_path = filename;
		const CString output_extension = output_path.GetExtension();

		// decide type of file to save
		if (output_extension.CompareNoCase(_T(".grf")) == 0) {
			// If GRF extension, save as GRF
			save_as = GRF;
		} else if (output_extension.CompareNoCase(_T(".xml")) == 0) {
			// If XML extension, save as XML
			save_as = XML;
		} else if (output_extension.IsEmpty()) {
			switch (dlg.m_ofn.nFilterIndex) {
			case 1:		save_as = XML;			break;
			case 2:		save_as = GRF;			break;
			default:	save_as = input_type;	break;	// ambigous, use type passed in
			}
		}

		// add file exension if none
		if (output_extension.IsEmpty()) {
			if (XML == save_as)
				output_path.AddExtension(_T(".xml"));
			else
				output_path.AddExtension(_T(".grf"));
		}

		filename = CString(output_path);

		HRESULT hr = XML==save_as ? graph.SaveXML(filename) : graph.SaveGRF(filename);
		if (FAILED(hr)) {
			DSUtil::ShowError(hr, _T("Can't save file"));
		} else {
			document_filename = filename;
			document_type = save_as;

			// update MRU list
			mru.NotifyEntry(filename);
			UpdateMRUMenu();
			UpdateTitleBar();
		}
	}
}

// render_media_file - default false but allows caller to force render as media file if needed
HRESULT CGraphView::TryOpenFile(CString fn, bool render_media_file)
{
	HRESULT hr = S_OK;
	CPath	path(fn);
	CString	ext = path.GetExtension();
	DocumentType save_as = NONE;

	ext = ext.MakeLower();
	if (ext == _T(".grf") && !render_media_file) {
		save_as = GRF;
		hr = graph.LoadGRF(fn);
		
	} else if (ext == _T(".xml") && !render_media_file) {
		save_as = XML;
		hr = graph.LoadXML(fn);

	} else {
		hr = graph.RenderFile(fn);
		graph.Dirty();
	}

	if (FAILED(hr))
		DSUtil::ShowError(hr, TEXT("Can't open file"));
	else {

		// Don't change document type if just rendering a media file
		if (save_as != NONE)
			document_type = save_as;

		if (save_as != NONE							// Save as XML or GRF always sets filename
				|| document_filename.IsEmpty())		// OR rendering media file only sets name if name currently empty
			document_filename = fn;

	    mru.NotifyEntry(fn);
	    UpdateMRUMenu();
		UpdateTitleBar();
    }

	UpdateGraphState();
	graph.SetClock(true, NULL);
	graph.RefreshFilters();
	graph.SmartPlacement();
	Invalidate();

    AfxGetMainWnd()->SendMessage(WM_UPDATEPLAYRATE);

	return hr;
}

CString CGraphView::PromptForFileToOpen(bool media_file)
{
	static int current_file_index = 1;
	static int current_media_index = 2;

	// nabrowsujeme subor
	CString		filter;

	filter +=	_T("All Files|*.*|");

	if (!media_file) {
		filter +=	_T("All Graph Files|*.grf;*.xml|");
		filter +=	_T("GraphEdit Files (grf)|*.grf|");
		filter +=	_T("GraphStudio XML Files (xml)|*.xml|");
	}

	filter +=	_T("Video Files |*.avi;*.mp4;*.mpg;*.mpeg;*.m2ts;*.mts;*.ts;*.mkv;*.ogg;*.ogm;*.pva;*.evo;*.flv;*.mov;*.hdmov;*.ifo;*.vob;*.rm;*.rmvb;*.wmv;*.asf|");
	filter +=	_T("Audio Files |*.aac;*.ac3;*.mp3;*.wma;*.mka;*.ogg;*.mpc;*.flac;*.ape;*.wav;*.ra;*.wv;*.m4a;*.tta;*.dts;*.spx;*.mp2;*.ofr;*.ofs;*.mpa;*.awb|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    dlg.m_ofn.nFilterIndex = media_file ? current_media_index : current_file_index;
    int ret = dlg.DoModal();

	if (media_file)
		current_media_index = dlg.m_ofn.nFilterIndex;
	else
		current_file_index = dlg.m_ofn.nFilterIndex;

	return ret == IDOK ? dlg.GetPathName() : CString();
}

void CGraphView::OnFileOpenClick()
{
	const CString filename = PromptForFileToOpen(false);
	if (!filename.IsEmpty()) {
		OnNewClick();
		TryOpenFile(filename, false);
	}
}

void CGraphView::OnFileAddmediafile()
{
	const CString filename = PromptForFileToOpen(false);
	if (!filename.IsEmpty()) {
		TryOpenFile(filename, false);
	}
}

void CGraphView::OnRenderUrlClick()
{
	CRenderUrlForm		dlg;
	int ret = dlg.DoModal();
	if (ret == IDOK) {
		OnNewClick();
		ret = TryOpenFile(dlg.result_file, /* render_media_file= */ true);
	}
    AfxGetMainWnd()->SendMessage(WM_UPDATEPLAYRATE);
}

void CGraphView::OnRenderFileClick()
{
	const CString filename = PromptForFileToOpen(true);
	if (!filename.IsEmpty()) {
		OnNewClick();
		TryOpenFile(filename, true);
	}
}

void CGraphView::OnGraphStreamingStarted()
{
	last_stop_time_ns = last_start_time_ns = timer.GetTimeNS();
}

void CGraphView::OnGraphStreamingComplete()
{
	last_stop_time_ns = timer.GetTimeNS();

	OnStopClick();

	// if there were any tests running, let the form know
	if (form_dec_performance) {
		form_dec_performance->OnPhaseComplete();
	}	

    // close application?
    if (m_bExitOnStop)
    {
        AfxGetMainWnd()->PostMessageW(WM_CLOSE);
        return;
    }
}

void CGraphView::OnGraphInsertFilter()
{
	if (!form_filters) {
		form_filters = new CFiltersForm();
		form_filters->view = this;
		form_filters->DoCreateDialog();
	}
	form_filters->ShowWindow(SW_SHOW);
	form_filters->SetActiveWindow();
}

void CGraphView::OnGraphInsertFilterFromFile()
{
	CFilterFromFile dlg(this);
    int ret = dlg.DoModal();
    if(IDOK == ret && dlg.filterFactory != NULL)
    {
        CComPtr<IBaseFilter> instance;
        HRESULT hr = dlg.filterFactory->CreateInstance(NULL, IID_IBaseFilter, (void**)&instance);
        if(FAILED(hr))
            DSUtil::ShowError(hr, _T("Error creating instance of filter"));
        else {
            // Get Filter name
            FILTER_INFO filterInfo = {0};   
            instance->QueryFilterInfo(&filterInfo);
            CString filterName = filterInfo.achName;
            if(filterName == _T(""))
                filterName = PathFindFileName(dlg.result_file);
			InsertNewFilter(instance, filterName, /* connectCurrentPin = */ false);
        }
    }
}

void CGraphView::OnDeleteSelection()
{
	FILTER_STATE	state = State_Running;
	if (graph.GetState(state, 0) != 0) {
		state = State_Running;
	}

	if (state != State_Stopped) {
		// play sound to warn the user
		MessageBeep(MB_ICONASTERISK);
		return ;
	}

	// avoid some unnecessary crashes
	overlay_filter = NULL;

	// delete selected objects
	graph.RemoveSelectionFromGraph();
	Invalidate();

    AfxGetMainWnd()->SendMessage(WM_UPDATEPLAYRATE);
}


void CGraphView::OnMpeg2DemuxCreatePsiPin()
{
    if(!current_filter) return;

    CComQIPtr<IMpeg2Demultiplexer> mp2demux = current_filter->filter;
    if(!mp2demux) return;

    // Define the media type.
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = KSDATAFORMAT_TYPE_MPEG2_SECTIONS;
    mt.subtype = MEDIASUBTYPE_None;

    // Create a new output pin.
    CComPtr<IPin> psiPin;
    HRESULT hr = mp2demux->CreateOutputPin(&mt, L"PSI Pin", &psiPin);
    if (SUCCEEDED(hr))
    {
        // Map the PID.
        CComQIPtr<IMPEG2PIDMap> pidMap = psiPin;
        if(pidMap)
        {
            ULONG Pid[] = { 0x00 }; // Map any desired PIDs. 
            ULONG cPid = 1;
            hr = pidMap->MapPID(cPid, Pid, MEDIA_MPEG2_PSI);
            if(SUCCEEDED(hr))
            {
                // Create PSI Filter
                DSUtil::FilterTemplate filter;
                if (internal_filters.FindTemplateByCLSID(CLSID_PsiConfig, &filter) >= 0)
                {
                    CComPtr<IBaseFilter> psiConfigFilter;
	                hr = filter.CreateInstance(&psiConfigFilter);
	                if (SUCCEEDED(hr))
                    {
                        // Set PSI Pin as current Pin
                        GraphStudio::Pin pin(NULL);
                        pin.Load(psiPin);
                        current_pin = &pin;

                        // Insert and connect PSI Config Filter
		                hr = InsertNewFilter(psiConfigFilter, filter.name, true /* connectCurrentPin */);
                    }
                }

                graph.RefreshFilters();
                graph.Dirty();
                Invalidate();
            }
            else
                DSUtil::ShowError(hr, _T("Can't Map PSI on Pin"));
        }
    }
    else
        DSUtil::ShowError(hr, _T("Can't create Pin"));
}

void CGraphView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CGraphView::OnRefreshFilters()
{
	graph.RefreshFilters();
	graph.SmartPlacement();
	Invalidate();
}


void CGraphView::OnAutomaticrestartSchedule()
{
	if (form_schedule) {
		form_schedule->ShowWindow(SW_SHOW);
		form_schedule->SetActiveWindow();
	}
}

void CGraphView::OnViewGraphEvents()
{
	if (form_events) { 
		form_events->ShowWindow(SW_SHOW);
		form_events->SetActiveWindow();
	}
}

void CGraphView::OnViewGraphStatistics()
{
    if (!form_statistic) {
		form_statistic = new CStatisticForm();
		form_statistic->view = this;
		form_statistic->DoCreateDialog();
	}

	form_statistic->ShowWindow(SW_SHOW);
	form_statistic->SetActiveWindow();
}

void CGraphView::OnViewGraphconstructionreport()
{
	if (!form_construction) {
		form_construction = new CGraphConstructionForm(NULL);
		form_construction->view = this;
		form_construction->DoCreateDialog();
		form_construction->Reload(&render_params);
	}
	form_construction->ShowWindow(SW_SHOW);
	form_construction->SetActiveWindow();
}

void CGraphView::OnHelpGuidLookup()
{
    if(!form_guidlookup) {
        form_guidlookup = new CLookupForm(NULL, FALSE);
		form_guidlookup->DoCreateDialog();
	}
	form_guidlookup->ShowWindow(SW_SHOW);
	form_guidlookup->SetActiveWindow();
}

void CGraphView::OnHelpHresultLookup()
{
    if(!form_hresultlookup) {
        form_hresultlookup = new CLookupForm(NULL, TRUE);
		form_hresultlookup->DoCreateDialog();
	}
	form_hresultlookup->ShowWindow(SW_SHOW);
	form_hresultlookup->SetActiveWindow();
}

void CGraphView::OnShowCliOptions()
{
    CCliOptionsForm dlg;
    dlg.DoModal();
}

void CGraphView::OnConfigureSbe()
{
    CSbeConfigForm dlg;
    dlg.DoModal();
}

void CGraphView::UpdateGraphState()
{
	// now we will ask the graph for the state.
	// if we're in the middle of change, we setup a timer
	// that will call us back
	int ret = graph.GetState(graph_state, 10);
	if (ret < 0) {
		// error ?

	} else
	if (ret == 0) {

		state_ready = true;
		switch (graph_state) {
		case State_Stopped:		OnGraphStopped(); break;
		case State_Paused:		OnGraphPaused(); break;
		case State_Running:		OnGraphRunning(); break;
		}

	} else
	if (ret == VFW_S_CANT_CUE) {
		// we handle this one as paused
		state_ready = true;
		OnGraphPaused();
	} else
	if (ret == VFW_S_STATE_INTERMEDIATE) {
		// schedule timer
		SetTimer(CGraphView::TIMER_GRAPH_STATE, 50, NULL);
		state_ready = false;
	}
}

void CGraphView::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case CGraphView::TIMER_GRAPH_STATE:
		{
			KillTimer(CGraphView::TIMER_GRAPH_STATE);
			UpdateGraphState();
		}
		break;
	case CGraphView::TIMER_REMOTE_GRAPH_STATE:
		{
			UpdateGraphState();
		}
		break;
	case 1001:
		{
			KillTimer(1001);
			UpdateRenderersMenu();
		}
		break;
	}
}

void CGraphView::OnUseClock()
{
	if (graph.uses_clock) {
		graph.SetClock(false, NULL);
	} else {
		graph.SetClock(true, NULL);
	}

	graph.Dirty();
	Invalidate();
}

void CGraphView::RemoveClock()
{
    if (graph.uses_clock)
    {
		graph.SetClock(false, NULL);
        graph.Dirty();
	    Invalidate();
    }
}

void CGraphView::OnUpdateUseClock(CCmdUI *ui)
{
	ui->SetCheck(graph.uses_clock);
}

void CGraphView::OnUpdateTimeLabel(CString text)
{
	if (form_progress) {
		form_progress->UpdateTimeLabel(text);
	}
}

void CGraphView::OnUpdateSeekbar(double pos)
{
	if (form_progress) {
		form_progress->UpdateProgress(pos);
	}
}

void CGraphView::OnUpdateRenderMediaFile(CCmdUI *ui)
{
	if (state_ready) {
		ui->Enable(graph_state == State_Stopped ? TRUE : FALSE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::OnUpdatePlayButton(CCmdUI *ui)
{
	if (state_ready) {
		ui->Enable(graph_state == State_Running ? FALSE : TRUE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::OnUpdatePauseButton(CCmdUI *ui)
{
	if (state_ready) {
		ui->Enable(graph_state == State_Paused ? FALSE : TRUE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::OnUpdateStopButton(CCmdUI *ui)
{
	if (state_ready) {
		ui->Enable(graph_state == State_Stopped ? FALSE : TRUE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::OnGraphRunning()
{
	CMainFrame	*frame = (CMainFrame*)GetParentFrame();
	CToolBarCtrl &toolbar = frame->m_wndToolBar.GetToolBarCtrl();

	toolbar.EnableButton(ID_BUTTON_PLAY, FALSE);
	toolbar.EnableButton(ID_BUTTON_PAUSE, TRUE);
	toolbar.EnableButton(ID_BUTTON_STOP, TRUE);
}

void CGraphView::OnGraphStopped()
{
    CMainFrame	*frame = (CMainFrame*)GetParentFrame();
	CToolBarCtrl &toolbar = frame->m_wndToolBar.GetToolBarCtrl();

	toolbar.EnableButton(ID_BUTTON_PLAY, TRUE);
	toolbar.EnableButton(ID_BUTTON_PAUSE, TRUE);
	toolbar.EnableButton(ID_BUTTON_STOP, FALSE);

	// send the event to the progress form
	if (form_progress) {
		form_progress->OnGraphStopped();
	}
}

void CGraphView::OnGraphPaused()
{
	CMainFrame	*frame = (CMainFrame*)GetParentFrame();
	CToolBarCtrl &toolbar = frame->m_wndToolBar.GetToolBarCtrl();

	if (toolbar.EnableButton(ID_BUTTON_PLAY, TRUE) == FALSE) {
		Sleep(1);
	}
	toolbar.EnableButton(ID_BUTTON_PAUSE, FALSE);
	toolbar.EnableButton(ID_BUTTON_STOP, TRUE);
}

void CGraphView::OnViewTextInformation()
{
	form_textinfo->ShowWindow(SW_SHOW);
    form_textinfo->OnBnClickedButtonRefresh();
	form_textinfo->SetActiveWindow();
}

void CGraphView::OnDropFiles(HDROP hDropInfo)
{
	CComQIPtr<IFileSourceFilter> source;
	CComQIPtr<IFileSinkFilter> sink;

	// Determine if we've dropped files on a filter
	GraphStudio::Filter* drop_filter = NULL;
	CPoint drop_point;
	if (DragQueryPoint(hDropInfo, &drop_point)) {
		drop_point += GetScrollPosition();
		drop_filter = graph.FindFilterByPos(drop_point);
		if (drop_filter) {
			source = drop_filter->filter;
			sink = drop_filter->filter;
		}
	}

	bool needs_refresh = false;

	// accept dropped files
	TCHAR	filename[MAX_PATH];
	int num_files = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, MAX_PATH);
	for (int i=0; i<num_files; i++) {
		const int name_length = DragQueryFile(hDropInfo, i, filename, MAX_PATH);
		if (name_length > 0) {
			filename[name_length] = _T('\0');

			HRESULT hr = S_OK;
			if (drop_filter) {
				if (source) {
					hr = source->Load(filename, NULL);
					needs_refresh = true;
				} else if (sink) {
					hr = sink->SetFileName(filename, NULL);
					needs_refresh = true;
				} else {
					// Let user know if they've dropped on an unsupported filter
					DSUtil::ShowError(_T("Filter under cursor does not support IFileSourceFilter or ISinkFilter"));
				}
			} else {
				if (!(GetKeyState(VK_CONTROL) & 0x80)) {
					OnNewClick();			// Clear graph before loading unless user has held down control when file is dropped
				}

				if (GetKeyState(VK_SHIFT) & 0x80) {
					hr = AddFileSourceAsync(filename);
				} else if (GetKeyState(VK_MENU) & 0x80) {
					hr = AddSourceFilter(filename);
				} else {
					hr = TryOpenFile(filename);
				}
			}
		}
	}
	if (needs_refresh) {
		// don't do smart placement if we've just changed files sources or sinks
		// if we're opening files then that does its own refresh 
		graph.RefreshFilters();
		graph.Dirty();
		Invalidate();
	}
	DragFinish(hDropInfo);
}

void CGraphView::OnGraphInsertFileSource()
{
	// directly insert a file source filter
	CComPtr<IBaseFilter>	instance;
	HRESULT					hr;

	hr = CoCreateInstance(CLSID_AsyncReader, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
	if (FAILED(hr))
        DSUtil::ShowError(hr, _T("Can't create File Source (Async.)"));
	else
		hr = InsertNewFilter(instance, _T("File Source (Async.)"));
}

void CGraphView::OnGraphInsertTeeFilter()
{
	// directly insert a file source filter
	CComPtr<IBaseFilter>	instance;
	HRESULT					hr;

    hr = CoCreateInstance(CLSID_InfTee, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
	if (FAILED(hr))
        DSUtil::ShowError(hr, _T("Can't create Inf Tee Filter"));
	else if (instance)
		hr = InsertNewFilter(instance, _T("Tee Filter"));
}

void CGraphView::OnGraphInsertFileSink()
{
	// directly insert a file source filter
	CComPtr<IBaseFilter>	instance;
	HRESULT					hr;

	hr = CoCreateInstance(CLSID_FileWriter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
	if (FAILED(hr))
		DSUtil::ShowError(hr, _T("Can't create File Writer"));
	else
		hr = InsertNewFilter(instance, _T("File Writer"));
}

void CGraphView::OnPropertyPageClosed(CPropertyForm *page)
{
	for (int i=0; i<property_pages.GetCount(); i++) {
		if (page == property_pages[i]) {
			if (IsWindow(*page)) {
				page->DestroyWindow();
			}
			delete page;
			property_pages.RemoveAt(i);
			return ;
		}
	}
}

void CGraphView::OnFilterRemoved(GraphStudio::DisplayGraph *sender, GraphStudio::Filter *filter)
{
	// close the property pages associated with this filter
	ClosePropertyPage(filter->filter);

	// Prevent any dangling references to Filters or Pins that are about to be deleted
	if (overlay_filter == filter)
		overlay_filter = NULL;
	if (current_filter == filter)
		current_filter = NULL;
	if (current_pin && filter->FindPin(current_pin->pin))
		current_pin = NULL;
}

void CGraphView::OnDisplayPropertyPage(IUnknown *object, IUnknown *filter, CString title)
{
	// scan through our objects...
	for (int i=0; i<property_pages.GetCount(); i++) {
		CPropertyForm	*page = property_pages[i];
		if (object == page->object) {
			property_pages[i]->ShowWindow(SW_SHOW);
			return ;
		}
	}

	CPropertyForm	*page = new CPropertyForm();
	int ret = page->DisplayPages(object, filter, title, this);
	if (ret < 0) {
		delete page;
		return ;
	}

	// add to the list
	property_pages.Add(page);
}

void CGraphView::ClosePropertyPage(IUnknown *filter)
{
	// scan through our objects...
	for (int i=0; i<property_pages.GetCount(); i++) {
		CPropertyForm	*page = property_pages[i];
		if (filter == page->filter) {

			// kill the page
			if (IsWindow(*page)) page->DestroyWindow();
			delete page;
			property_pages.RemoveAt(i);

			return ;
		}
	}
}

void CGraphView::ClosePropertyPages()
{
	for (int i=0; i<property_pages.GetCount(); i++) {
		CPropertyForm	*page = property_pages[i];
		if (IsWindow(*page)) page->DestroyWindow();
		delete page;
	}
	property_pages.RemoveAll();
}

void CGraphView::OnClearMRUClick()
{
	mru.Clear();
	UpdateMRUMenu();
}

void CGraphView::OnMRUClick(UINT nID)
{
	const int idx = nID - ID_LIST_MRU_FILE0;

	// let's try to open this one
	if (idx < 0 || idx >= mru.list.GetCount()) 
		return ;

	const CString	fn = mru.list[idx];
	OnNewClick();
	const HRESULT hr = TryOpenFile(fn);
}

void CGraphView::OnGraphScreenshot()
{
	MakeScreenshot(document_filename);
}

void CGraphView::OnConnectRemote()
{
	CRemoteGraphForm	remote_form;
	const int ret = remote_form.DoModal();
	if (ret == IDOK) {
		if (remote_form.sel_graph) {

			// get a graph object
			CComPtr<IRunningObjectTable>	rot;
			CComPtr<IUnknown>				unk;
			CComPtr<IFilterGraph>			fg;
			HRESULT							hr;

			hr = GetRunningObjectTable(0, &rot);
			ASSERT(SUCCEEDED(hr));

			hr = rot->GetObject(remote_form.sel_graph, &unk);
			if (SUCCEEDED(hr)) {

				hr = unk->QueryInterface(IID_IFilterGraph, (void**)&fg);
				if (SUCCEEDED(hr)) {

					hr = graph.ConnectToRemote(fg);
					if (SUCCEEDED(hr)) {
						SetTimer(CGraphView::TIMER_REMOTE_GRAPH_STATE, 200, NULL);
					} else {
						DSUtil::ShowError(hr, _T("Failed to connect to remote graph. Note, this can be caused by failing to register proppage.dll from the Windows SDK."));
						OnNewClick();
					}

					// get all filters
					graph.RefreshFilters();
					graph.SmartPlacement();
					Invalidate();
				}
				fg = NULL;
			}
			unk = NULL;

			rot = NULL;
		}
	}

    AfxGetMainWnd()->SendMessage(WM_UPDATEPLAYRATE);
}

void CGraphView::OnDisconnectRemote()
{
	OnNewClick();
}

void CGraphView::OnUpdateConnectRemote(CCmdUI *ui)
{
	ui->Enable(TRUE);
}

void CGraphView::OnUpdateDisconnectRemote(CCmdUI *ui)
{
	if (graph.is_remote) {
		ui->Enable(TRUE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::DoZoom(int z)
{	
	render_params.Zoom(z);
	graph.SmartPlacement();
	graph.Dirty();
	Invalidate();

	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("Zoom"), z);
}

void CGraphView::OnView50()		{ DoZoom(50);  }
void CGraphView::OnView75()		{ DoZoom(75);  }
void CGraphView::OnView100()	{ DoZoom(100); }
void CGraphView::OnView150()	{ DoZoom(150); }
void CGraphView::OnView200()	{ DoZoom(200); }

void CGraphView::OnUpdateView50(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 50);
}

void CGraphView::OnUpdateView75(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 75);
}

void CGraphView::OnUpdateView100(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 100);
}

void CGraphView::OnUpdateView150(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 150);
}

void CGraphView::OnUpdateView200(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 200);
}


void CGraphView::OnViewDecreasezoomlevel()
{
	int	zl = 100;
	switch (render_params.zoom) {
	case 50:	zl = 50; break;
	case 75:	zl = 50; break;
	case 100:	zl = 75; break;
	case 150:	zl = 100; break;
	case 200:	zl = 150; break;
	}
	DoZoom(zl);
}

void CGraphView::OnViewIncreasezoomlevel()
{
	int	zl = 100;
	switch (render_params.zoom) {
	case 50:	zl = 75; break;
	case 75:	zl = 100; break;
	case 100:	zl = 150; break;
	case 150:	zl = 200; break;
	case 200:	zl = 200; break;
	}
	DoZoom(zl);
}

void CGraphView::OnAudioSourceClick(UINT nID)
{
	int	n = nID - ID_AUDIO_SOURCE0;
	if (n < 0 || n >= audio_sources.filters.GetCount()) return ;

	InsertFilterFromTemplate(audio_sources.filters[n]);
}

void CGraphView::OnVideoSourceClick(UINT nID)
{
	int	n = nID - ID_VIDEO_SOURCE0;
	if (n < 0 || n >= video_sources.filters.GetCount()) return ;

	InsertFilterFromTemplate(video_sources.filters[n]);
}

void CGraphView::OnAudioRendererClick(UINT nID)
{
	int	n = nID - ID_AUDIO_RENDERER0;
	if (n < 0 || n >= audio_renderers.filters.GetCount()) return ;

	InsertFilterFromTemplate(audio_renderers.filters[n]);
}

void CGraphView::OnVideoRendererClick(UINT nID)
{
	int	n = nID - ID_VIDEO_RENDERER0;
	if (n < 0 || n >= video_renderers.filters.GetCount()) return ;

	InsertFilterFromTemplate(video_renderers.filters[n]);
}

void CGraphView::OnInternalFilterClick(UINT nID)
{
	int	n = nID - ID_INTERNAL_FILTER0;
	if (n < 0 || n >= internal_filters.filters.GetCount()) return ;

	InsertFilterFromTemplate(internal_filters.filters[n]);
}

void CGraphView::OnFavoriteFilterClick(UINT nID)
{
	CMenu	*mainmenu = GetParentFrame()->GetMenu();
	CMenu	*filtersmenu = mainmenu->GetSubMenu(4);

	MENUITEMINFO	info;
	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask = MIIM_DATA;
	filtersmenu->GetMenuItemInfo(nID, &info);

	// let's insert the filter
	GraphStudio::BookmarkedFilter	*filter = (GraphStudio::BookmarkedFilter *)info.dwItemData;
	InsertFilterFromFavorite(filter);
}

int CGraphView::InsertFilterFromFavorite(GraphStudio::BookmarkedFilter *filter)
{
	CComPtr<IMoniker>		moniker;
	CComPtr<IBaseFilter>	instance;
	CComPtr<IBindCtx>		bind;

	HRESULT					hr;
	ULONG					eaten = 0;

	hr = CreateBindCtx(0, &bind);
	if (FAILED(hr)) return -1;

	hr = MkParseDisplayName(bind, filter->moniker_name, &eaten, &moniker);
	if (hr != NOERROR) {
		bind = NULL;
		return -1;
	}

	hr = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&instance);
	if (SUCCEEDED(hr))
		hr = InsertNewFilter(instance, filter->name);

	bind = NULL;
	instance = NULL;
	moniker = NULL;
	return 0;
}

HRESULT CGraphView::InsertFilterFromTemplate(DSUtil::FilterTemplate &filter)
{
	// now create an instance of this filter
	CComPtr<IBaseFilter>	instance;
	HRESULT hr = filter.CreateInstance(&instance);
	if (SUCCEEDED(hr))
		hr = InsertNewFilter(instance, filter.name, /* connectCurrentPin = */ false);
	return hr;
}

void CGraphView::OnFiltersDouble()
{
	graph.DoubleSelected();
	graph.SmartPlacement();
	Invalidate();
}

void CGraphView::OnFiltersManageFavorites()
{
	form_favorites->ShowWindow(SW_SHOW);
	form_favorites->SetActiveWindow();
}

void CGraphView::OnUpdateFiltersManageBlacklist(CCmdUI *ui)
{
    ui->Enable(FALSE);
}

void CGraphView::OnFiltersManageBlacklist()
{
    if(!form_blacklist)
    {
        form_blacklist = new CBlacklistForm(NULL, FALSE);
		form_blacklist->DoCreateDialog();
	}
	form_blacklist->ShowWindow(SW_SHOW);
	form_blacklist->SetActiveWindow();
}

void CGraphView::OnOptionsDisplayFileName()
{
	render_params.display_file_name = !render_params.display_file_name;
	graph.RefreshFilters();
	graph.SmartPlacement();
	graph.Dirty();
	Invalidate();
}

void CGraphView::OnUpdateOptionsDisplayFileName(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.display_file_name ? true : false);
}


void CGraphView::OnConnectModeIntelligentClick()
{
	render_params.connect_mode = 0;
    AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ConnectMode"), render_params.connect_mode);
}

void CGraphView::OnUpdateConnectModeIntelligent(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.connect_mode == 0);
}

void CGraphView::OnConnectModeDirectClick()
{
	render_params.connect_mode = 1;
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ConnectMode"), render_params.connect_mode);
}

void CGraphView::OnUpdateConnectModeDirect(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.connect_mode == 1);
}

void CGraphView::OnConnectModeDirectWmtClick()
{
	render_params.connect_mode = 2;
    AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ConnectMode"), render_params.connect_mode);
}

void CGraphView::OnUpdateConnectModeDirectWmt(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.connect_mode == 2);
}


void CGraphView::OnOptionsExactMatchClick()
{
	render_params.exact_match_mode = !render_params.exact_match_mode;
}

void CGraphView::OnOptionsAbortrender()
{
	render_params.abort_timeout = !render_params.abort_timeout;
}

void CGraphView::OnUpdateOptionsExactMatch(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.exact_match_mode);
}

void CGraphView::OnUpdateOptionsAbortrender(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.abort_timeout);
}

void CGraphView::OnUpdateOptionsUseMediaInfo(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.use_media_info);
}

void CGraphView::OnOptionsUseMediaInfoClick()
{
	render_params.use_media_info = !render_params.use_media_info;
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("UseMediaInfo"), render_params.use_media_info);
}

void CGraphView::OnUpdateShowGuidOfKnownTypes(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(CgraphstudioApp::g_showGuidsOfKnownTypes);
}

void CGraphView::OnOptionsShowGuidOfKnownTypesClick()
{
    CgraphstudioApp::g_showGuidsOfKnownTypes = !CgraphstudioApp::g_showGuidsOfKnownTypes;

    // save the value to the registry
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ShowGuidsOfKnownTypes"), CgraphstudioApp::g_showGuidsOfKnownTypes ? 1 : 0);
}


void CGraphView::OnViewProgressview()
{
	CString name(_T("Untitled"));
	const CDocument * const doc  = GetDocument();
	if (doc)
		name = doc->GetTitle();

	form_progress->UpdateCaption(name);
	form_progress->ShowWindow(SW_SHOW);
	AfxGetMainWnd()->ShowWindow(SW_HIDE);
}

void CGraphView::OnOverlayIconClick(GraphStudio::OverlayIcon *icon, CPoint point)
{
	if (!icon) return ;

	switch (icon->id) {
	case GraphStudio::OverlayIcon::ICON_VOLUME:
		{
			// let's bring up some slider bar for the volume
			if (!form_volume) {
				form_volume = new CVolumeBarForm();
				form_volume->DoCreateDialog();
			}
			
			form_volume->DoHide();
			form_volume->DisplayVolume(icon->filter->filter);
		}
		break;
	case GraphStudio::OverlayIcon::ICON_CLOCK:
		{
			// set this new clock
			if (icon->filter && icon->filter->clock) {
				graph.SetClock(false, icon->filter->clock);
				graph.Dirty();
				Invalidate();
			}
		}
		break;
	}
}

void CGraphView::OnRenderFinished()
{
	if (form_construction) {
		form_construction->Reload(&render_params);
	}
}


void CGraphView::OnUpdateRemoveConnections(CCmdUI *pCmdUI)
{
    bool stateOk = false;
    if (state_ready)
		stateOk = graph_state == State_Stopped;

    bool hasConnections = FALSE;
    for(int i=0; i<graph.filters.GetCount() && stateOk; i++)
        if(graph.filters[i]->connected)
        {
            hasConnections = TRUE;
            break;
        }

    pCmdUI->Enable(stateOk && hasConnections ? TRUE : FALSE);
}

void CGraphView::OnRemoveConnections()
{
    for(int i=0; i<graph.filters.GetCount(); i++)
        if(graph.filters[i]->connected)
        {
            for(int j=0; j<graph.filters[i]->output_pins.GetCount(); j++)
                graph.filters[i]->output_pins[j]->Disconnect();
        }

    graph.RefreshFilters();
	Invalidate();
}


BOOL CGraphView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	const int alt = GetKeyState(VK_MENU) & 0x80;
	const int shift = nFlags & MK_SHIFT;
	const int ctrl = nFlags & MK_CONTROL;

	if (alt) {										// alt-wheel is synonym for horizontal wheel
		OnMouseHWheel(nFlags, -zDelta, pt);
		return 0;
	} else if (ctrl && !shift) {					// Control wheel (without shift)- zoom
		if (zDelta <= -WHEEL_DELTA) {
			OnViewDecreasezoomlevel();
		} else if (zDelta >= WHEEL_DELTA) {
			OnViewIncreasezoomlevel();
		}
		return 0;
	} else if (shift && !ctrl) {					// shift wheel- change x filter spacing
		int delta = (abs(zDelta) / WHEEL_DELTA);	// Round towards zero
		delta = (zDelta < 0 ? delta : -delta);
		ChangeFilterSpacing(GraphStudio::DisplayGraph::g_filterYGap, delta);
		return 0;
	} else {
		return __super::OnMouseWheel(nFlags, zDelta, pt);
	}
}

// This feature requires Windows Vista or greater.
// The symbol _WIN32_WINNT must be >= 0x0600.
// TODO: Add your message handler code here and/or call default
void CGraphView::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	const int shift = nFlags & MK_SHIFT;
	const int ctrl = nFlags & MK_CONTROL;

	if (shift && !ctrl) {							// shift wheel
		int delta = (abs(zDelta) / WHEEL_DELTA);	// Round towards zero
		delta = (zDelta < 0 ? -delta : delta);
		ChangeFilterSpacing(GraphStudio::DisplayGraph::g_filterXGap, delta);
	}
	// we don't handle anything but scrolling (no modifier keys)
	// if the parent is a splitter, it will handle the message
	else if (shift || ctrl || GetParentSplitter(this, TRUE) || !DoMouseHorzWheel(nFlags, zDelta, pt))
		__super::OnMouseHWheel(nFlags, zDelta, pt);
}

// Modified from CScrollView::DoMouseWheel which implements vertical mouse wheel scrolling
BOOL CGraphView::DoMouseHorzWheel(UINT fFlags, short zDelta, CPoint point)
{
	UNUSED_ALWAYS(point);
	UNUSED_ALWAYS(fFlags);

	// if we have a horizontal scroll bar, the horizontal wheel scrolls that
	// otherwise, don't do any work at all

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);
	if (!bHasHorzBar)
		return FALSE;

	BOOL bResult = FALSE;
	const UINT uWheelScrollLines = _AfxGetMouseScrollLines();
	int nDisplacement;

	if (uWheelScrollLines == WHEEL_PAGESCROLL)
	{
		nDisplacement = m_pageDev.cx;
		if (zDelta < 0)							// horizontal wheel seems to operate in opposite sign to vertical wheel
			nDisplacement = -nDisplacement;
	}
	else
	{
		// horizontal wheel seems to operate in opposite sign to vertical wheel
		const int nToScroll = ::MulDiv(zDelta, uWheelScrollLines, WHEEL_DELTA);
		nDisplacement = nToScroll * m_lineDev.cx;
		nDisplacement = min(nDisplacement, m_pageDev.cx);
	}
	bResult = OnScrollBy(CSize(nDisplacement, 0), TRUE);

	if (bResult)
		UpdateWindow();

	return bResult;
}

void CGraphView::ChangeFilterSpacing(int& value, int delta)
{
	int new_gap = value + (delta * GraphStudio::DisplayGraph::GRID_SIZE);
	new_gap = max(GraphStudio::DisplayGraph::GRID_SIZE, new_gap);

	if (new_gap != value) {
		value = new_gap;
		graph.SmartPlacement();
		Invalidate();
	}
}

void CGraphView::OnViewDecreaseHorizontalSpacing()
{
	ChangeFilterSpacing(GraphStudio::DisplayGraph::g_filterXGap, -1);
}

void CGraphView::OnViewIncreaseHorizontalSpacing()
{
	ChangeFilterSpacing(GraphStudio::DisplayGraph::g_filterXGap, +1);
}

void CGraphView::OnViewDecreaseVerticalSpacing()
{
	ChangeFilterSpacing(GraphStudio::DisplayGraph::g_filterYGap, -1);
}

void CGraphView::OnViewIncreaseVerticalSpacing()
{
	ChangeFilterSpacing(GraphStudio::DisplayGraph::g_filterYGap, +1);
}

static void SetResolvePins(CgraphstudioApp::PinResolution r)
{
	CgraphstudioApp::g_ResolvePins = r;
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ResolvePins"), r);
}

void CGraphView::OnFileoptionsLoadpinsbyname()
{
	SetResolvePins(CgraphstudioApp::BY_NAME);
}

void CGraphView::OnFileoptionsLoadpinsbyindex()
{
	SetResolvePins(CgraphstudioApp::BY_INDEX);
}

void CGraphView::OnFileoptionsLoadpinsbyid()
{
	SetResolvePins(CgraphstudioApp::BY_ID);
}

void CGraphView::OnUpdateFileoptionsLoadpinsbyname(CCmdUI *pCmdUI)
{
	const bool check = CgraphstudioApp::g_ResolvePins != CgraphstudioApp::BY_INDEX
			&& CgraphstudioApp::g_ResolvePins != CgraphstudioApp::BY_ID;
	pCmdUI->SetCheck(check);
}

void CGraphView::OnUpdateFileoptionsLoadpinsbyindex(CCmdUI *pCmdUI)
{
	const bool check = CgraphstudioApp::g_ResolvePins == CgraphstudioApp::BY_INDEX;
	pCmdUI->SetCheck(check);
}

void CGraphView::OnUpdateFileoptionsLoadpinsbyid(CCmdUI *pCmdUI)
{
	const bool check = CgraphstudioApp::g_ResolvePins == CgraphstudioApp::BY_ID;
	pCmdUI->SetCheck(check);
}

void CGraphView::OnOptionsShowconsolewindow()
{
	CgraphstudioApp::g_showConsole = ! CgraphstudioApp::g_showConsole;
	ShowConsole(CgraphstudioApp::g_showConsole);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ShowConsoleWindow"), CgraphstudioApp::g_showConsole);
}

void CGraphView::OnUpdateOptionsShowconsolewindow(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(CgraphstudioApp::g_showConsole);
}

void CGraphView::OnOptionsUseinternalgrffileparser()
{
	CgraphstudioApp::g_useInternalGrfParser = ! CgraphstudioApp::g_useInternalGrfParser;
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("UseInternalGrfParser"), CgraphstudioApp::g_useInternalGrfParser);
}

void CGraphView::OnUpdateOptionsUseinternalgrffileparser(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(CgraphstudioApp::g_useInternalGrfParser);
}

HRESULT CGraphView::AddSourceFilter(CString filename)
{
	CComPtr<IBaseFilter> ibasefilter;
	const HRESULT hr = graph.gb->AddSourceFilter(filename, filename, &ibasefilter);

	if (SUCCEEDED(hr)) {
		mru.NotifyEntry(filename);
		UpdateMRUMenu();
	}
	UpdateGraphState();
	graph.SetClock(true, NULL);
	graph.RefreshFilters();
	graph.SmartPlacement();
	Invalidate();
	AfxGetMainWnd()->SendMessage(WM_UPDATEPLAYRATE);

	return hr;
}

void CGraphView::OnFileAddSourceFilter()
{
	const CString filename = PromptForFileToOpen(true);
	if (!filename.IsEmpty()) {
		AddSourceFilter(filename);
    }
}

HRESULT CGraphView::AddFileSourceAsync(CString filename)
{
	HRESULT hr = E_NOINTERFACE;

	CComPtr<IBaseFilter> file_reader;
	file_reader.CoCreateInstance(CLSID_AsyncReader, NULL, CLSCTX_INPROC_SERVER);
	CComQIPtr<IFileSourceFilter> file_source(file_reader);
	if (file_reader && file_source) {
		hr = graph.AddFilter(file_reader, _T("File Source (Async.)"));
		if (SUCCEEDED(hr))
			hr = file_source->Load(filename, NULL);

		UpdateGraphState();
		graph.SetClock(true, NULL);
		graph.RefreshFilters();
		graph.SmartPlacement();
		Invalidate();
		AfxGetMainWnd()->SendMessage(WM_UPDATEPLAYRATE);
	}
	mru.NotifyEntry(filename);
	UpdateMRUMenu();

	return hr;
}

void CGraphView::OnFileAddFileSourceAsync()
{
	const CString filename = PromptForFileToOpen(true);
	if (!filename.IsEmpty()) {
		AddFileSourceAsync(filename);
    }
}


void CGraphView::OnClsidFiltergraph()
{
	graph.m_filter_graph_clsid = &CLSID_FilterGraph;
	OnNewClick();
}

void CGraphView::OnClsidFiltergraphNoThread()
{
	graph.m_filter_graph_clsid = &CLSID_FilterGraphNoThread;
	OnNewClick();
}

void CGraphView::OnClsidFiltergraphPrivateThread()
{
	graph.m_filter_graph_clsid = &CLSID_FilterGraphPrivateThread;
	OnNewClick();
}

void CGraphView::OnUpdateClsidFiltergraph(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(graph.m_filter_graph_clsid == &CLSID_FilterGraph);
}

void CGraphView::OnUpdateClsidFiltergraphNoThread(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(graph.m_filter_graph_clsid == &CLSID_FilterGraphNoThread);
}


void CGraphView::OnUpdateClsidFiltergraphPrivateThread(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(graph.m_filter_graph_clsid == &CLSID_FilterGraphPrivateThread);
}

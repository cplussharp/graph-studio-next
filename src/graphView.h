//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

#define WM_UPDATEPLAYRATE (WM_USER + 1)

//-----------------------------------------------------------------------------
//
//	CGraphView class
//
//-----------------------------------------------------------------------------

class CGraphView : public GraphStudio::DisplayView, public CustomToolTipCallback
{
protected: 
	CGraphView();
	DECLARE_DYNCREATE(CGraphView)
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

public:

	CFiltersForm				*form_filters;
	CEventsForm					*form_events;
	CGraphConstructionForm		*form_construction;
	CScheduleForm				*form_schedule;
	CTextInfoForm				*form_textinfo;
	CFavoritesForm				*form_favorites;
    CBlacklistForm				*form_blacklist;
	CProgressForm				*form_progress;
	CVolumeBarForm				*form_volume;
	CDecPerformanceForm			*form_dec_performance;
	CSeekForm					*form_seek;
    CStatisticForm				*form_statistic;
    CLookupForm                 *form_guidlookup;
    CLookupForm                 *form_hresultlookup;
    CFileTypesForm              *form_filetypes;

    CCustomToolTipCtrl	        m_ToolTip;
    CPoint                      m_lastToolTipPoint;

	// active property pages
	CArray<CPropertyForm*>		property_pages;

	// most recently used list
	GraphStudio::MRUList		mru;

	// enumerated audio & video sources/renderers
    DSUtil::FilterTemplates		audio_sources;
	DSUtil::FilterTemplates		video_sources;
	DSUtil::FilterTemplates		audio_renderers;
	DSUtil::FilterTemplates		video_renderers;
    DSUtil::FilterTemplates		internal_filters;

	enum {
		TIMER_GRAPH_STATE = 1,
		TIMER_REMOTE_GRAPH_STATE = 2,
		TIMER_AUTO_RESTART = 3
	};

	CString						document_filename;

	enum DocumentType
	{
		NONE,
		GRF,
		XML
	}							document_type;

	// filter state
	bool						state_ready;
	FILTER_STATE				graph_state;

	// known system monitors
	vector<HMONITOR>			monitors;

	HighResTimer				timer;

	__int64						last_start_time_ns;
	__int64						last_stop_time_ns;

    bool                        m_bExitOnStop;

public:
	virtual ~CGraphView();

	CGraphDoc* GetDocument() const;
	CMainFrame* GetParentFrame() const;

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    BOOL PreTranslateMessage(MSG* pMsg);
    void OnInitialUpdate();
    virtual void OnMouseMove(UINT nFlags, CPoint point);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Events
	void OnInit();
	void OnFileOpenClick();
	void OnFileSaveClick();
	void OnFileSaveAsClick();
	void OnRenderUrlClick();
	void OnPlayClick();
	void OnStopClick();
	void OnPauseClick();
	void OnPlayPauseToggleClick();
	void OnFrameStepClick();
	void OnRefreshFilters();
	void OnNewClick();
	void OnSeekClick();
	void OnRenderFileClick();
	void OnGraphInsertFilter();
	void OnFindInFiltersWindow();
    void OnGraphInsertFilterFromFile();
	void OnSaveAsXmlAndGrf();
	void OnAlwaysSaveScreenshot();
	void OnClearMRUClick();
	void OnMRUClick(UINT nID);
	void OnGraphScreenshot();
	void OnConnectRemote();
    void OnConnectRemote(CComPtr<IMoniker> moniker, CString graphName);
	void OnDisconnectRemote();
	void OnDummyEvent(UINT nID) { };
	void OnGraphStreamingStarted();
	void OnGraphStreamingComplete();

	CString PromptForFileToOpen(bool only_media_files);
	HRESULT FileSaveAs(DocumentType input_type);
	void UpdateTitleBar();
	HRESULT DoFileSave();

	// CustomToolTipCallback implementation
    virtual void GetToolTipLabelText(POINT cursor, CString& labelText, CString& descriptionText) const;

	// menu
	void UpdateMRUMenu();
	void UpdateRenderersMenu();
	void UpdatePreferredVideoRenderersMenu();
	virtual void PopulateAudioRenderersMenu(CMenu& menu);
	virtual void PopulateVideoRenderersMenu(CMenu& menu);

    void OnAudioSourceClick(UINT nID);
	void OnVideoSourceClick(UINT nID);
	void OnAudioRendererClick(UINT nID);
	void OnVideoRendererClick(UINT nID);
    void OnInternalFilterClick(UINT nID);
	void OnFavoriteFilterClick(UINT nID);
	void OnPreferredVideoRendererClick(UINT nID);
	HRESULT InsertFilterFromTemplate(DSUtil::FilterTemplate &filter);
	int InsertFilterFromFavorite(GraphStudio::BookmarkedFilter *filter);

	// keyboard events
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnViewGraphEvents();
    void OnViewGraphStatistics();
	void OnTimer(UINT_PTR nIDEvent);

	void UpdateGraphState();
	void OnGraphRunning();
	void OnGraphStopped();
	void OnGraphPaused();
	void OnUpdateTimeLabel(CString text);
	void OnUpdateSeekbar(double pos);
	void OnUseClock();
    void RemoveClock();

	void OnUpdateConnectRemote(CCmdUI *ui);
	void OnUpdateDisconnectRemote(CCmdUI *ui);
	void OnUpdatePlayButton(CCmdUI *ui);
	void OnUpdatePauseButton(CCmdUI *ui);
	void OnUpdateStopButton(CCmdUI *ui);
	void OnUpdateRenderMediaFile(CCmdUI *ui);
	void OnUpdateUseClock(CCmdUI *ui);
	void OnDropFiles(HDROP hDropInfo);
	HRESULT TryOpenFile(CString fn, bool render_media_file = false);
	int TryOpenXML(CString fn);
	HRESULT AddFileSourceAsync(CString source_file);
	HRESULT AddSourceFilter(CString source_file);

	virtual LRESULT OnWmCommand(WPARAM wParam, LPARAM lParam);

	// property pages
	virtual void OnDisplayPropertyPage(IUnknown *object, IUnknown *filter, CString title);
	virtual void OnFilterRemoved(GraphStudio::DisplayGraph *sender, GraphStudio::Filter *filter);
	virtual void OnPropertyPageClosed(CPropertyForm *page);
	virtual void OnRenderFinished();
	virtual void OnDeleteSelection();

    virtual void OnMpeg2DemuxCreatePsiPin();

	void ClosePropertyPages();
	void ClosePropertyPage(IUnknown *filter);
	virtual void OnOverlayIconClick(GraphStudio::OverlayIcon *icon, CPoint point);

	// save/load window position
	void LoadWindowPosition();
	void SaveWindowPosition();
	void OnMonitorCallback(HMONITOR monitor, HDC dc, LPRECT rect);

	void OnViewTextInformation();
	void OnGraphInsertFileSource();
    void OnGraphInsertTeeFilter();
	void OnGraphInsertFileSink();

	void OnDestroy();
	
	void OnView50();
	void OnView75();
	void OnView100();
	void OnView150();
	void OnView200();
	void DoZoom(int z);
	void SelectZoomItem(int idc);
	void OnUpdateView50(CCmdUI *pCmdUI);
	void OnUpdateView75(CCmdUI *pCmdUI);
	void OnUpdateView100(CCmdUI *pCmdUI);
	void OnUpdateView150(CCmdUI *pCmdUI);
	void OnUpdateView200(CCmdUI *pCmdUI);
	void OnFileAddmediafile();
	void OnFiltersDouble();
	void OnViewDecreasezoomlevel();
	void OnViewIncreasezoomlevel();
	void OnFiltersManageFavorites();
    void OnFiltersManageBlacklist();
    void OnUpdateFiltersManageBlacklist(CCmdUI *ui);
	void OnOptionsDisplayFileName();
	void OnUpdateOptionsDisplayFileName(CCmdUI *pCmdUI);
	
    void OnConnectModeIntelligentClick();
	void OnUpdateConnectModeIntelligent(CCmdUI *pCmdUI);
    void OnConnectModeDirectClick();
	void OnUpdateConnectModeDirect(CCmdUI *pCmdUI);
    void OnConnectModeDirectWmtClick();
	void OnUpdateConnectModeDirectWmt(CCmdUI *pCmdUI);

	void OnOptionsExactMatchClick();
	void OnUpdateOptionsExactMatch(CCmdUI *pCmdUI);
    void OnOptionsUseMediaInfoClick();
    void OnOptionsShowGuidOfKnownTypesClick();
    void OnUpdateOptionsUseMediaInfo(CCmdUI *pCmdUI);
    void OnUpdateShowGuidOfKnownTypes(CCmdUI *pCmdUI);

    void OnUpdateRemoveConnections(CCmdUI *pCmdUI);
    void OnRemoveConnections();

	void ChangeFilterSizeParam(int& value, int delta, int min_value=GraphStudio::DisplayGraph::GRID_SIZE);
	void SaveGraphLayoutSettings();
	BOOL DoMouseHorzWheel(UINT fFlags, short zDelta, CPoint point);

	afx_msg void OnViewProgressview();
	afx_msg void OnFileSaveasxml();
	afx_msg void OnAutomaticrestartSchedule();
	afx_msg void OnViewDecoderPerformance();
	afx_msg void OnUpdateOptionsAbortrender(CCmdUI *pCmdUI);
	afx_msg void OnOptionsAbortrender();
	afx_msg void OnUpdateOptionsAutoArrangeFilters(CCmdUI *pCmdUI);
	afx_msg void OnOptionsAutoArrangeFilters();
	afx_msg void OnUpdateOptionsResizeToFitGraph(CCmdUI *pCmdUI);
	afx_msg void OnOptionsResizeToFitGraph();
	afx_msg void OnViewGraphconstructionreport();
    afx_msg void OnHelpGuidLookup();
    afx_msg void OnHelpHresultLookup();
    afx_msg void OnHelpRegisteredFileTypes();
    afx_msg void OnShowCliOptions();
    afx_msg void OnConfigureSbe();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnViewDecreaseHorizontalSpacing();
	afx_msg void OnViewIncreaseHorizontalSpacing();
	afx_msg void OnViewDecreaseVerticalSpacing();
	afx_msg void OnViewIncreaseVerticalSpacing();
	afx_msg void OnViewDecreaseFilterWrapWidth();
	afx_msg void OnViewIncreaseFilterWrapWidth();
	afx_msg void OnResetGraphLayout();
	afx_msg void OnFileoptionsLoadpinsbyname();
	afx_msg void OnUpdateFileoptionsLoadpinsbyname(CCmdUI *pCmdUI);
	afx_msg void OnFileoptionsLoadpinsbyindex();
	afx_msg void OnUpdateFileoptionsLoadpinsbyindex(CCmdUI *pCmdUI);
	afx_msg void OnFileoptionsLoadpinsbyid();
	afx_msg void OnUpdateFileoptionsLoadpinsbyid(CCmdUI *pCmdUI);
	afx_msg void OnOptionsShowconsolewindow();
	afx_msg void OnUpdateOptionsShowconsolewindow(CCmdUI *pCmdUI);
	afx_msg void OnOptionsUseinternalgrffileparser();
	afx_msg void OnUpdateOptionsUseinternalgrffileparser(CCmdUI *pCmdUI);
	afx_msg void OnFileAddSourceFilter();
	afx_msg void OnFileAddFileSourceAsync();
	afx_msg void OnClsidFiltergraph();
	afx_msg void OnClsidFiltergraphNoThread();
	afx_msg void OnClsidFiltergraphPrivateThread();
	afx_msg void OnUpdateClsidFiltergraph(CCmdUI *pCmdUI);
	afx_msg void OnUpdateClsidFiltergraphNoThread(CCmdUI *pCmdUI);
	afx_msg void OnUpdateClsidFiltergraphPrivateThread(CCmdUI *pCmdUI);
	afx_msg void OnFilterFavorite();
	afx_msg void OnFilterBlacklist();
	afx_msg void OnSeekBackward1();
	afx_msg void OnSeekBackward2();
	afx_msg void OnSeekBackward3();
	afx_msg void OnSeekBackward4();
	afx_msg void OnSeekForward1();
	afx_msg void OnSeekForward2();
	afx_msg void OnSeekForward3();
	afx_msg void OnSeekForward4();
	afx_msg void OnUpdateSeekByFrame(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSeekByTime(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSaveAsXmlAndGrf(CCmdUI *pCmdUI);
	afx_msg void OnUpdateAlwaysSaveScreenshot(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in graphView.cpp
inline CGraphDoc* CGraphView::GetDocument() const
   { return reinterpret_cast<CGraphDoc*>(m_pDocument); }
#endif


//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	//-------------------------------------------------------------------------
	//
	//	DisplayView class
	//
	//-------------------------------------------------------------------------
	class DisplayView : public CScrollView, public GraphCallback
	{
	protected:
		DECLARE_DYNCREATE(DisplayView)
		DECLARE_MESSAGE_MAP()

		HRESULT	InsertNewFilter(IBaseFilter* newFilter, const CString& strFilterName, bool connectToCurrentPin = true);

		static Pin * GetPinFromFilterClick(Filter* filter, int clickFlags, bool findConnectedPin);

	public:

		RenderParameters	render_params;

		// graph currently displayed
		DisplayGraph		graph;

		// double buffered view
		CBitmap				backbuffer;
		CDC					memDC;
		int					back_width, back_height;

		CPoint				start_drag_point;
		CPoint				end_drag_point;

		// creating new connection
		CPoint				new_connection_start;
		CPoint				new_connection_end;
		bool				new_connection_start_connected;
		bool				new_connection_end_connected;
        Pin::PIN_CONNECTION_TYPE new_connection_start_type;
		CArray<Pin*>		connect_pins;

		enum {
			DRAG_GROUP = 0,
			DRAG_CONNECTION = 1,
			DRAG_SELECTION = 2,
			DRAG_OVERLAY_ICON = 3
		};
		int					drag_mode;

		// helpers for rightclick menu
		Filter						*overlay_filter;
		DSUtil::FilterTemplates		compatible_filters;


	public:
		DisplayView();
		~DisplayView();

		BOOL OnEraseBkgnd(CDC* pDC);
		virtual void OnDraw(CDC *pDC);
		void OnSize(UINT nType, int cx, int cy);

		virtual void OnMouseMove(UINT nFlags, CPoint point);
		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnLButtonUp(UINT nFlags, CPoint point);
		void OnMButtonDown(UINT nFlags, CPoint point);
		void OnRButtonDown(UINT nFlags, CPoint point);
		void OnLButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);	// forwarded from frame window

		void OnRenderPin();
        void OnRemovePin();
		void OnRenderNullStream();
        void OnRenderDxvaNullStream();
		void OnDumpStream();
		void OnTimeMeasureStream();
		void OnAnalyzeStream();
        void OnAnalyzeWriterStream();
        void OnTeeStream();
		void OnFileWriterStream();
		void OnPropertyPage();
		void OnDeleteFilter();
		void OnChooseSourceFile();
		void OnChooseDestinationFile();
		void OnConnectPin();

		void MakeScreenshot(const CString& image_filename, const GUID& gdiplus_format);
		bool SetSelectionFromClick(UINT nFlags, CPoint point, GraphStudio::Filter ** selected_filter = NULL, GraphStudio::Pin** selected_pin = NULL);
		void ScrollToMakeFilterVisible(Filter * filter);

		// stream selection
		void PrepareStreamSelectMenu(CMenu &menu, IUnknown *obj);
		void PrepareCompatibleFiltersMenu(CMenu &menu, Pin *pin);
		void UpdateFavoritesMenu();
		virtual void PopulateAudioRenderersMenu(CMenu& menu) {}
		virtual void PopulateVideoRenderersMenu(CMenu& menu) {};
		virtual int PopulateConnectMenu(CMenu& menu, GraphStudio::Pin &);
        void PrepareFavoriteFiltersMenu(CMenu &menu);
		void OnSelectStream(UINT id);
		void OnCompatibleFilterClick(UINT id);
		void OnConnectPinClick(UINT nID);
		void NavigateFilterGraph(bool pin, bool vertical, bool positive);
		void ShowContextMenu(CPoint pt, GraphStudio::Filter *, GraphStudio::Pin *);

		// scrolling aid
		void UpdateScrolling();
		void RepaintBackbuffer();

        // To get touch input
        virtual ULONG GetGestureStatus(CPoint ptTouch);

		// to be overriden
		virtual void OnDisplayPropertyPage(IUnknown *object, GraphStudio::Filter *filter, CString title);
		virtual void OnOverlayIconClick(OverlayIcon *icon, CPoint point);
		virtual void OnDeleteSelection();

		// GraphCallback implementation
		virtual void OnFiltersRefreshed();
		virtual void OnFilterRemoved(DisplayGraph *sender, Filter *filter);
		virtual void OnRenderFinished();
		virtual void OnSmartPlacement();

        virtual void OnMpeg2DemuxCreatePsiPin();
		afx_msg void OnFileSetlogfile();
		afx_msg void OnUpdateFileSetlogfile(CCmdUI *pCmdUI);
		afx_msg void OnViewMainwindow();
		afx_msg void OnFilterLeft();
		afx_msg void OnFilterRight();
		afx_msg void OnFilterUp();
		afx_msg void OnFilterDown();
		afx_msg void OnPinLeft();
		afx_msg void OnPinRight();
		afx_msg void OnPinUp();
		afx_msg void OnPinDown();
		afx_msg void OnContextMenu();
};


GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation



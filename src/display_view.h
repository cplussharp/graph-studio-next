//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

namespace GraphStudio
{

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

		enum {
			DRAG_GROUP = 0,
			DRAG_CONNECTION = 1,
			DRAG_SELECTION = 2,
			DRAG_OVERLAY_ICON = 3
		};
		int					drag_mode;

		// helpers for rightclick menu
		Filter						*overlay_filter;
		Filter						*current_filter;
		Pin							*current_pin;
		DSUtil::FilterTemplates		compatible_filters;


	public:
		DisplayView();
		~DisplayView();

		BOOL OnEraseBkgnd(CDC* pDC);
		virtual void OnDraw(CDC *pDC);
		void OnSize(UINT nType, int cx, int cy);

		void OnMouseMove(UINT nFlags, CPoint point);
		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnLButtonUp(UINT nFlags, CPoint point);
		void OnMButtonDown(UINT nFlags, CPoint point);
		void OnRButtonDown(UINT nFlags, CPoint point);
		void OnLButtonDblClk(UINT nFlags, CPoint point);

		void OnRenderPin();
        void OnRemovePin();
		void OnRenderNullStream();
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

		void MakeScreenshot(const CString& base_filename);

		// stream selection
		void PrepareStreamSelectMenu(CMenu &menu, IUnknown *obj);
		void PrepareCompatibleFiltersMenu(CMenu &menu, Pin *pin);
        void PrepareFavoriteFiltersMenu(CMenu &menu);
		void OnSelectStream(UINT id);
		void OnCompatibleFilterClick(UINT id);

		// scrolling aid
		void UpdateScrolling();
		void RepaintBackbuffer();

        // To get touch input
        virtual ULONG GetGestureStatus(CPoint ptTouch);

		// to be overriden
		virtual void OnDisplayPropertyPage(IUnknown *object, IUnknown *filter, CString title);
		virtual void OnFilterRemoved(DisplayGraph *sender, Filter *filter);
		virtual void OnOverlayIconClick(OverlayIcon *icon, CPoint point);
		virtual void OnRenderFinished();
		virtual void OnDeleteSelection();

        virtual void OnMpeg2DemuxCreatePsiPin();
	};


};



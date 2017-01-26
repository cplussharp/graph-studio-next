//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation


	class DisplayView;
	class DisplayGraph;
	class Filter;

	//-------------------------------------------------------------------------
	//
	//	Pin class
	//
	//-------------------------------------------------------------------------
	class Pin
	{
	public:
        typedef enum _PinConnectionType
        {	
            PIN_CONNECTION_TYPE_OTHER = 0,
            PIN_CONNECTION_TYPE_STREAM,
            PIN_CONNECTION_TYPE_AUDIO,
            PIN_CONNECTION_TYPE_VIDEO,
            PIN_CONNECTION_TYPE_SUBTITLE,
            PIN_CONNECTION_TYPE_MIXED,
        } 	PIN_CONNECTION_TYPE;

		RenderParameters		*params;
		Filter					*filter;
		CString					name;
		CString					id;
		CComPtr<IPin>			ipin;
		Pin						*peer;			// peer pin
		PIN_DIRECTION			dir;
		bool					connected;
        PIN_CONNECTION_TYPE     connectionType;
		bool					selected;

	public:
		Pin(Filter *parent);
		virtual ~Pin();

		// draw the pin
		void Draw(CDC *dc, bool input, int x, int y);
		void GetCenterPoint(CPoint *pt);

		int Load(IPin *pin);
		void LoadPeer();
		bool IsConnected();
		void Select(bool select);

		// operations
		int Disconnect();
	};

	//-------------------------------------------------------------------------
	//
	//	OverlayIcon class
	//
	//-------------------------------------------------------------------------
	class OverlayIcon
	{
	public:
		CBitmap			*icon_normal[2];
		CBitmap			*icon_hover[2];
		Filter			*filter;
		int				id;
		int				state;

		enum {
			ICON_VOLUME = 0,
			ICON_CLOCK = 1
		};

	public:
		OverlayIcon(Filter *parent, int icon_id);
		virtual ~OverlayIcon();
	};

	//-------------------------------------------------------------------------
	//
	//	Filter class
	//
	//-------------------------------------------------------------------------
	class Filter
	{
	public:
		DisplayGraph			*graph;
		RenderParameters		*params;
		CString					name;				// name of the filter
		CString					file_name;			// file name from IFileSourceFilter or IFileSinkFilter
		CString					display_name;		// name as it appears
		CLSID					clsid;				// it's CLASS_ID
		CString					clsid_str;			// string version
		mutable CString			dll_file;			// file name of the filter dll file (used as internal cache)
		bool					created_from_dll;	// If true, filters was created directly from DLL class factory

		CComPtr<IBaseFilter>	filter;
		CArray<Pin*>			input_pins;
		CArray<Pin*>			output_pins;

		int						posx, posy;			// position in the display
		int						width, height;		// drawn dimensions
		int						name_width;			// drawn width of filter name with word wrapping
		CPoint					start_drag_pos;

		int						column;				// The column index that we're arranged in. Not set if negative
		bool					selected;
		bool					connected;

		// Enhanced Video Renderer support
		EVR_VideoWindow			*videowindow;

		// special
		CComPtr<IBasicAudio>		basic_audio;
		CComPtr<IReferenceClock>	clock;
		bool						is_sync_source;

		// overlay icons
		CArray<OverlayIcon*>	overlay_icons;
		int						overlay_icon_active;	// index of icon

		enum {
			FILTER_UNKNOWN = 0,
			FILTER_STANDARD = 1,
			FILTER_WDM = 2,
			FILTER_DMO = 3
		};
		int						filter_type;				

		enum FilterPurpose {
			FILTER_SOURCE,
			FILTER_RENDERER,
			FILTER_OTHER
		} filter_purpose;

	public:
		Filter(DisplayGraph *parent);
		virtual ~Filter();

		// kreslenie filtra
		void Draw(CDC *dc);
		void DrawConnections(CDC *dc);

		// overlay icons
		void ReleaseIcons();
		void CreateIcons();

		// I/O
		void Release();
		void RemovePins();
		void LoadFromFilter(IBaseFilter *f);
		void Refresh();
		Pin *LoadPin(IPin *pin, PIN_DIRECTION dir);
		Pin *FindPin(IPin *pin);
		Pin *FindPinByPos(CPoint p, bool not_connected = true);
		Pin *FindPinByID(CString id);
		Pin *FindPinByMatchingID(CString id);
		Pin *FindPinByName(CString id);
		Pin* FindConnectionLineByPos(CPoint pt) const;
		bool HasPin(IPin *pin);
		void LoadPeers();
		void RemoveSelectedConnections();
		void RemoveFromGraph();
		void UpdateClock();
		CString GetDllFileName() const;

		// Helpers
		bool IsSource();
		bool IsRenderer();
		int NumOfDisconnectedPins(PIN_DIRECTION dir);
		int NumOfConnectedPins(PIN_DIRECTION dir);
		Pin* FirstUnconnectedInputPin();
		Pin* FirstUnconnectedOutputPin();

		// placement 
		void CalculatePlacementChain(int new_depth, int x, int y=-1);
		void UpdatePinPositions();

		// filter dragging
		void BeginDrag();
		void VerifyDrag(int *deltax, int *deltay);
		void Select(bool select);
		void SelectConnection(UINT flags, CPoint pt);
		bool AnyPinSelected() const;
		bool AnyOutputPinSelected() const;

		// overlay icons
		int CheckIcons(CPoint pt);

		static const int SELECTION_BORDER = 10;

		static const int OVERLAY_ICON_WIDTH = 16;
		static const int OVERLAY_ICON_HEIGHT = 16;
		static const int OVERLAY_ICON_OFFSET = 6;
	};


	//-------------------------------------------------------------------------
	//
	//	DisplayGraph callback classes
	//
	//-------------------------------------------------------------------------
	
	#define WM_GRAPH_EVENT		(WM_USER + 880)

	class GraphCallback
	{
	public:
		virtual void OnFiltersRefreshed() = 0;
		virtual void OnFilterRemoved(DisplayGraph *sender, Filter *filter) = 0;
		virtual void OnRenderFinished() = 0;
		virtual void OnSmartPlacement() = 0;
	};

	class GraphCallbackImpl : public CUnknown, protected IAMGraphBuilderCallback, protected IAMFilterGraphCallback 
	{
	public:
		DisplayGraph	*graph;

	public:
		GraphCallbackImpl(LPUNKNOWN punk, HRESULT *phr, DisplayGraph *parent);
		virtual ~GraphCallbackImpl();

		DECLARE_IUNKNOWN;
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

		// IAMGraphBuilderCallback overrides
		STDMETHODIMP SelectedFilter(IMoniker *pMon);
		STDMETHODIMP CreatedFilter(IBaseFilter *pFilter);

		// IAMFilterGraphCallback overrides
		HRESULT UnableToRender(IPin *pPin);			// This method uses the thiscall calling convention, rather than __stdcall.
	};


	//-------------------------------------------------------------------------
	//
	//	DisplayGraph class
	//
	//-------------------------------------------------------------------------

	class DisplayGraph
	{
	public:
		// Coordinates for graph are on a grid. 
		static const int GRID_SIZE = 8;
		// These helpers round the next previous grid coordinate. If already on a grid line same value is returned
		static inline int NextGridPos(const int coord) { return (coord+7) &~ 0x07; } 
		static inline int PrevGridPos(const int coord) { return coord &~ 0x07; } 

		// graph itself
		CComPtr<IGraphBuilder>			gb;
		CComPtr<ICaptureGraphBuilder2>	cgb;
		CComPtr<IMediaControl>			mc;
		CComPtr<IMediaEventEx>			me;
		CComPtr<IMediaSeeking>			ms;
		CComPtr<IVideoFrameStep>		fs;	

		CArray<Filter*>					filters;
		CDC								*dc;

		// render parameters
		RenderParameters				*params;
		GraphCallbackImpl				*graph_callback;

		// columns that filters are arranged in (smart placement). Columns can be different widths
		CArray<CPoint>					columns;

		// 
		HWND							wndEvents;
		GraphCallback					*callback;
		bool							is_remote;
		bool							is_frame_stepping;
		bool							dirty;

		// helpers
		bool							uses_clock;
		CString							clock_status_text;
        DWORD                           rotRegister;

		// cached seeking and playback information
		double							fps;
		bool							supports_time_seeking;
		bool							supports_frame_seeking;
		LONGLONG						min_time_per_frame;		// The minimum time gap between video frames across all pins in the graph. _I64_MAX if unavailable

		const GUID *					m_filter_graph_clsid;

	public:
		DisplayGraph();
		virtual ~DisplayGraph();

        // ROT
        void AddToRot();
        void RemoveFromRot();
		static bool IsOwnRotGraph(const CString& moniker_name);

		// let's build something
		int MakeNew();
		HRESULT RenderFile(CString fn);
		HRESULT LoadGRF(CString fn);
		int SaveGRF(CString fn);
		HRESULT ConnectToRemote(IFilterGraph *remote_graph);
		int AttachCaptureGraphBuilder();

		// XML-based graph construction
		int SaveXML(CString fn);

		HRESULT LoadXML(CString fn);
		HRESULT LoadXML_Filter(XML::XMLNode *node, CComPtr<IBaseFilter>& created_filter);
		HRESULT LoadXML_Render(XML::XMLNode *node);
		HRESULT LoadXML_Connect(XML::XMLNode *node, const CArray<IBaseFilter *> & indexed_filters, CString& error_string);
		HRESULT LoadXML_Schedule(XML::XMLNode *node);
		HRESULT LoadXML_Config(XML::XMLNode *node);
		HRESULT LoadXML_Interfaces(XML::XMLNode *node, IBaseFilter *filter);
		HRESULT LoadXML_ConfigInterface(XML::XMLNode *conf, IBaseFilter *filter);
		HRESULT LoadXML_Command(XML::XMLNode *node);
		HRESULT LoadXML_IAMGraphStreams(XML::XMLNode *node);

		// IAMBufferNegotiation 
		HRESULT LoadXML_IAMBufferNegotiation(XML::XMLNode *conf, IBaseFilter *filter);

		// adding filters
		HRESULT AddFilter(IBaseFilter *filter, CString proposed_name);

		Filter *FindFilter(IBaseFilter *filter);
		Filter *FindFilter(CString name);
		Filter *FindParentFilter(IPin *pin);
		Pin *FindPin(IPin *pin);
		Pin *FindPinByPath(CString pin_path);

		void RefreshFilters();
		void DeleteAllFilters();
		void LoadPeers();
		void RemoveSelectionFromGraph();

		// selection processing
		void SelectAllFilters(bool select);
		Filter * GetSelectedFilter(bool allow_multi_selection = false);
		Pin * GetSelectedPin(bool allow_multi_selection = false);
		void SetSelection(Filter * filter, Pin * pin, bool clear_existing_selection = true);

		int CalcDownstreamYPosition(Filter* const start_filter) const;
		void SmartPlacement(bool force = true);
		void DoubleSelected();
		HRESULT ConnectPins(Pin *p1, Pin *p2, bool chooseMediaType);
		HRESULT ReconnectPin(Pin *pin);

		// Clock manipulation
		void SetClock(bool default_clock, IReferenceClock *new_clock);
		void RefreshClock();

		// rendering the graph
		void Draw(CDC *dc);
		void DrawArrow(CDC *dc, CPoint p1, CPoint p2, DWORD color = RGB(0,0,0), int nPenStyle = PS_SOLID);

		// mouse interaction
		Filter *FindFilterByPos(CPoint pt) const;
		Pin *FindPinByPos(CPoint pt) const;

		// state information
		int GetState(FILTER_STATE &state, DWORD timeout=INFINITE);

		// seeking helpers
		int GetPositionAndDuration(double &current_ms, double &duration_ms);
		int GetFPS(double &fps);
		int RefreshFPS();
		int Seek(double time_ms, BOOL keyframe = FALSE);
        int GetRate(double* rate);
        int SetRate(double rate);

		// control
		void DoFrameStep();
		HRESULT DoPlay(bool full_screen = false);
		HRESULT DoStop();
		HRESULT DoPause(bool full_screen = false);

		// scrolling aid
		CRect GetGraphSize();

		inline void Dirty() { dirty = true; }

		bool IsLogFileOpen() const					{ return m_log_file != INVALID_HANDLE_VALUE; }
		HRESULT OpenLogFile(LPCTSTR file_name);
		HRESULT CloseLogFile();

	private:
		HRESULT ParseGRFFile(LPCWSTR filename);
		void PositionRowOfUnconnectedFilters(const CArray<Filter*> &, int row_length);

		HANDLE	m_log_file;
	};

	// helpers
	bool LineHit(CPoint p1, CPoint p2, CPoint hit); 
	void DoDrawArrow(CDC *dc, CPoint p1, CPoint p2, DWORD color, int nPenStyle = PS_SOLID);

	#ifndef GUID_LIST_SUPRESS_GUIDS

	// GUID helpers - can be excluded from filter builds
	bool NameGuid(GUID guid, CString &str, bool alsoAddGuid);
    bool InsertGuidLookup(int i, CListCtrl* pListCtrl);
	int GetFormatName(int wFormatTag, CString &str);

	#endif

    bool NameHResult(HRESULT hr, CString &str);
    bool InsertHresultLookup(int i, CListCtrl* pListCtrl);

	void MakeFont(CFont &f, CString name, int size, bool bold, bool italic);
    bool HasFont(CString fontName);

	// These should be moved to a separate utility module, really
	inline void MakeFont(CFont &f, CString name, int size, bool bold, bool italic)
	{
		HDC dc = CreateCompatibleDC(NULL);
		int nHeight    = -MulDiv(size, (int)(GetDeviceCaps(dc, LOGPIXELSY)), 72 );
		DeleteDC(dc);

		BYTE nBold   = (bold ? FW_BOLD : 0);
		BYTE nItalic = (italic ? TRUE : FALSE);

		f.CreateFont(nHeight, 0, 0, 0, nBold, nItalic, FALSE, FALSE, DEFAULT_CHARSET,
					  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, VARIABLE_PITCH, name);
	}

    inline int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* /*lpelfe*/, NEWTEXTMETRICEX* /*lpntme*/, int /*FontType*/, LPARAM lParam)
    {
        LPARAM* l = (LPARAM*)lParam;
        *l = TRUE;
        return TRUE;
    }


    inline bool HasFont(CString fontName)
    {
        // Get the screen DC
        CDC dc;
        if (!dc.CreateCompatibleDC(NULL))
        {
    	    return false;
        }
        LOGFONT lf = { 0 };
        // Any character set will do
        lf.lfCharSet = DEFAULT_CHARSET;
        // Set the facename to check for
        _tcscpy(lf.lfFaceName, fontName);
        LPARAM lParam = 0;
        // Enumerate fonts
        ::EnumFontFamiliesEx(dc.GetSafeHdc(), &lf,  (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&lParam, 0);
        return lParam ? true : false;
    }

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

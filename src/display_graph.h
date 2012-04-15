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
	class DisplayView;
	class DisplayGraph;
	class Filter;

	//-------------------------------------------------------------------------
	//
	//	RenderAction class
	//
	//-------------------------------------------------------------------------
	class RenderAction
	{
	public:
		DWORD		time_ms;			// relative to operation start time
		
		enum {
			ACTION_NONE			= 0,
			ACTION_SELECT		= 1,
			ACTION_CREATE		= 2
		};

		int			type;
		CString		displ_name;			// display name
		GUID		clsid;				// filter clsid

	public:
		RenderAction() : time_ms(0), displ_name(_T("")), type(RenderAction::ACTION_NONE), clsid(GUID_NULL) { }
		RenderAction(const RenderAction &f) : time_ms(f.time_ms), displ_name(f.displ_name), type(f.type), clsid(f.clsid) { }
		~RenderAction() { };
		RenderAction &operator =(const RenderAction &f) {
			time_ms = f.time_ms;
			displ_name = f.displ_name;
			type = f.type;
			clsid = f.clsid;
			return *this;
		}
	};

	//-------------------------------------------------------------------------
	//
	//	RenderParameters class
	//
	//-------------------------------------------------------------------------
	class RenderParameters
	{
	public:

		// color and font settings
		DWORD			color_back;

		// filter settings
		DWORD			filter_color;
		DWORD			color_filter_border_light;
		DWORD			color_filter_border_dark;
		DWORD			select_color;
		DWORD			filter_type_colors[4];

		CFont			font_filter;
		CFont			font_pin;

		// zoom
		int				zoom;
		
		// size of elements
		int				min_filter_width;
		int				min_filter_height;
		int				pin_spacing;

		// default size at 100%
		int				def_min_width;
		int				def_min_height;
		int				def_pin_spacing;
		int				def_filter_text_size;
		int				def_pin_text_size;

		// display as file name
		bool			display_file_name;
		bool			direct_connect;
		bool			exact_match_mode;
		bool			abort_timeout;								// abort rendering operation after 10 seconds

		// render operation state
		DWORD					render_start_time;
		bool					in_render;
		bool					render_can_proceed;
		vector<RenderAction>	render_actions;						// moniker name list for filters in the last render operation

		// Overlay Icons
		CBitmap			bmp_volume_hi;
		CBitmap			bmp_volume_lo;
		CBitmap			bmp_clock_active_hi;
		CBitmap			bmp_clock_active_lo;
		CBitmap			bmp_clock_inactive_hi;
		CBitmap			bmp_clock_inactive_lo;

		// preferred video renderer
		CString						preferred_video_renderer;		// display name
		DSUtil::FilterTemplates		*video_renderers;				// list of filters we consider video renderers

        bool            use_media_info;

	public:
		RenderParameters();
		virtual ~RenderParameters();

		// adjust sizes
		void Zoom(int z);
		void MarkRender(bool start);
	};

	//-------------------------------------------------------------------------
	//
	//	Pin class
	//
	//-------------------------------------------------------------------------
	class Pin
	{
	public:
		RenderParameters		*params;
		Filter					*filter;
		CString					name;
		CString					id;
		CComPtr<IPin>			pin;
		Pin						*peer;			// peer pin
		PIN_DIRECTION			dir;
		bool					connected;
		bool					selected;		// selected connection pin->peer (only valid for output pins)

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

		CComPtr<IBaseFilter>	filter;
		CArray<Pin*>			input_pins;
		CArray<Pin*>			output_pins;

		int						posx, posy;			// position in the display
		int						width, height;
		CPoint					start_drag_pos;

		int						tag;				// helper
		int						depth;	
		bool					selected;
		bool					connected;

		// Enhanced Video Renderer support
		EVR_VideoWindow			*videowindow;

		// special
		CComPtr<IBasicAudio>		basic_audio;
		CComPtr<IReferenceClock>	clock;

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
		void LoadPin(IPin *pin, PIN_DIRECTION dir);
		Pin *FindPin(IPin *pin);
		Pin *FindPinByPos(CPoint p, bool not_connected=true);
		Pin *FindPin(CString name);
		bool HasPin(IPin *pin);
		void LoadPeers();
		void DeleteSelectedConnections();
		void DeleteFilter();
		void UpdateClock();

		// Helpers
		bool IsSource();
		bool IsRenderer();
		int NumOfDisconnectedPins(PIN_DIRECTION dir);

		// placement 
		void CalculatePlacementChain(int new_depth, int x);
		void UpdatePinPositions();

		// filter dragging
		void BeginDrag();
		void VerifyDrag(int *deltax, int *deltay);
		void Select(bool select);
		void SelectConnection(UINT flags, CPoint pt);

		// overlay icons
		int CheckIcons(CPoint pt);
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
		virtual void OnFilterRemoved(DisplayGraph *sender, Filter *filter) = 0;
		virtual void OnRenderFinished() = 0;
	};

	class GraphCallbackImpl : public CUnknown, public IAMGraphBuilderCallback 
	{
	public:
		DisplayGraph	*graph;

	public:
		GraphCallbackImpl(LPUNKNOWN punk, HRESULT *phr, DisplayGraph *parent);
		virtual ~GraphCallbackImpl();

		DECLARE_IUNKNOWN;
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

		STDMETHODIMP SelectedFilter(IMoniker *pMon);
		STDMETHODIMP CreatedFilter(IBaseFilter *pFilter);
	};


	//-------------------------------------------------------------------------
	//
	//	DisplayGraph class
	//
	//-------------------------------------------------------------------------

	class DisplayGraph
	{
	public:

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

		// bins in filters are (smart placement)
		CArray<CPoint>					bins;

		// 
		HWND							wndEvents;
		GraphCallback					*callback;
		bool							is_remote;
		bool							is_frame_stepping;
		bool							dirty;

		// helpers
		CString							graph_name;
		bool							uses_clock;
		double							fps;

	public:
		DisplayGraph();
		virtual ~DisplayGraph();

		// let's build something
		int MakeNew();
		int RenderFile(CString fn);
		int LoadGRF(CString fn);
		int SaveGRF(CString fn);
		int ConnectToRemote(IFilterGraph *remote_graph);
		int AttachCaptureGraphBuilder();

		// XML-based graph construction
		int SaveXML(CString fn);
		int SaveXML_Filter(Filter *filter, XML::XMLWriter *writer);
		int LoadXML(CString fn);
		int LoadXML_Filter(XML::XMLNode *node);
		int LoadXML_Render(XML::XMLNode *node);
		int LoadXML_Connect(XML::XMLNode *node);
		int LoadXML_Schedule(XML::XMLNode *node);
		int LoadXML_Config(XML::XMLNode *node);
		int LoadXML_Interfaces(XML::XMLNode *node, IBaseFilter *filter);
		int LoadXML_ConfigInterface(XML::XMLNode *conf, IBaseFilter *filter);
		int LoadXML_Command(XML::XMLNode *node);
		int LoadXML_IAMGraphStreams(XML::XMLNode *node);

		// IAMBufferNegotiation 
		int LoadXML_IAMBufferNegotiation(XML::XMLNode *conf, IBaseFilter *filter);

		// adding filters
		HRESULT AddFilter(IBaseFilter *filter, CString proposed_name);

		void RefreshFilters();
		void ZeroTags();
		Filter *FindFilter(IBaseFilter *filter);
		Filter *FindFilter(CString name);
		Filter *FindParentFilter(IPin *pin);
		Pin *FindPin(IPin *pin);
		Pin *FindPin(CString pin_path);
		void RemoveUnusedFilters();
		void RemoveAllFilters();
		void SmartPlacement();
		void LoadPeers();
		void DeleteSelected();
		void DoubleSelected();
		int ConnectPins(Pin *p1, Pin *p2);

		// Clock manipulation
		void SetClock(bool default_clock, IReferenceClock *new_clock);
		void RefreshClock();

		// rendering the graph
		void Draw(CDC *dc);
		void DrawArrow(CDC *dc, CPoint p1, CPoint p2);

		// mouse interaction
		Filter *FindFilterByPos(CPoint pt);
		Pin *FindPinByPos(CPoint pt);

		// state information
		int GetState(FILTER_STATE &state, DWORD timeout=INFINITE);

		// seeking helpers
		int GetPositions(double &current_ms, double &duration_ms);
		int GetFPS(double &fps);
		int RefreshFPS();
		int Seek(double time_ms, BOOL keyframe = FALSE);
        int GetRate(double* rate);
        int SetRate(double rate);

		// control
		void DoFrameStep();
		HRESULT DoPlay();
		HRESULT DoStop();
		HRESULT DoPause();

		// scrolling aid
		CSize GetGraphSize();

		inline void Dirty() { dirty = true; }
	};

	// helpers
	bool LineHit(CPoint p1, CPoint p2, CPoint hit); 
	void DoDrawArrow(CDC *dc, CPoint p1, CPoint p2, DWORD color);

	// GUID helpers
	bool NameGuid(GUID guid, CString &str, bool alsoAddGuid);
    bool InsertGuidLookup(int i, CListCtrl* pListCtrl);
	int GetFormatName(int wFormatTag, CString &str);
    bool NameHResult(HRESULT hr, CString &str);
    bool InsertHresultLookup(int i, CListCtrl* pListCtrl);

	void MakeFont(CFont &f, CString name, int size, bool bold, bool italic);

};

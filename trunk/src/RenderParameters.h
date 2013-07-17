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
//	RenderAction class
//
//-------------------------------------------------------------------------
class RenderAction
{
public:
	DWORD		time_ms;			// relative to operation start time
		
	enum {
		ACTION_NONE				= 0,
		ACTION_SELECT			= 1,
		ACTION_CREATE			= 2,
		ACTION_REJECT			= 3,
		ACTION_TIMEOUT			= 4,
		ACTION_RENDER_FAILURE	= 5
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
	static const COLORREF			color_back;
    static const COLORREF			color_back_remote;
    static const COLORREF			color_select;

	// filter settings
	static const COLORREF			color_filter_border_light;
	static const COLORREF			color_filter_border_dark;
	static const COLORREF			color_filter_type[4];

    static const COLORREF			color_connection_break;
    static const COLORREF			color_connection_type[6];

	CFont			font_filter;
	CFont			font_pin;

	// zoom
	int				zoom;
		
	// size of elements
	int				min_filter_width;
	int				min_filter_height;
	int				filter_wrap_width;
	int				pin_spacing;

	// spacing between filters
	int				filter_x_gap;
	int				filter_y_gap;

	// default size at 100%
	static const int				def_min_width;
	static const int				def_min_height;
	static const int				def_pin_spacing;
	static const int				def_filter_text_size;
	static const int				def_pin_text_size;

    enum ConnectMode {
        ConnectMode_Intelligent,
        ConnectMode_Direct,
        ConnectMode_DirectWithMT
    };
    static ConnectMode     connect_mode;								// connect pins
    void SetConnectMode(ConnectMode mode)
    {
        connect_mode = mode;
        AfxGetApp()->WriteProfileInt(_T("Settings"), _T("ConnectMode"), connect_mode);
    }

	// display as file name
	static bool			display_file_name;
    void SetDisplayFileName(bool display)
    {
        display_file_name = display;
        AfxGetApp()->WriteProfileInt(_T("Settings"), _T("DisplayFileName"), display_file_name);
    }

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

    static bool     use_media_info;
    void SetUseMediaInfo(bool use)
    {
        use_media_info = use;
        AfxGetApp()->WriteProfileInt(_T("Settings"), _T("UseMediaInfo"), use_media_info);
    }

    bool            is_remote;

public:
	RenderParameters();
	virtual ~RenderParameters();

	// adjust sizes
	void Zoom(int z);
	void MarkRender(bool start);
};

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

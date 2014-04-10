//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

//-------------------------------------------------------------------------
//
//	RenderParameters class
//
//-------------------------------------------------------------------------

const COLORREF RenderParameters::color_back = RGB(192, 192, 192);		// default background color
const COLORREF RenderParameters::color_back_remote = RGB(255, 192, 192);		// default background color
const COLORREF RenderParameters::color_select = RGB(0,0,255);

// filter colors
const COLORREF RenderParameters::color_filter_border_light = RGB(255, 255, 255);
const COLORREF RenderParameters::color_filter_border_dark = RGB(128, 128, 128);
const COLORREF RenderParameters::color_filter_type[4] = {
    RGB(192,192,192), // Filter::FILTER_UNKNOWN
    RGB(192,192,255), // Filter::FILTER_STANDARD
    RGB(255,128,0), // Filter::FILTER_WDM
    RGB(0,192,64)   // Filter::FILTER_DMO
};

const COLORREF RenderParameters::color_connection_break = RGB(192,0,0);
const COLORREF RenderParameters::color_connection_type[6] = {
    RGB(255,0,128),     // PIN_CONNECTION_TYPE_OTHER
    RGB(0,0,0),         // PIN_CONNECTION_TYPE_STREAM
    RGB(0,128,128),     // PIN_CONNECTION_TYPE_AUDIO
    RGB(0,128,0),       // PIN_CONNECTION_TYPE_VIDEO
    RGB(255,128,0),      // PIN_CONNECTION_TYPE_SUBTITLE
    RGB(128,64,128)     // PIN_CONNECTION_TYPE_MIXED
};

// default size at 100%
const int RenderParameters::def_min_width = 92;
const int RenderParameters::def_min_height = 86;
const int RenderParameters::def_pin_spacing = 27;
const int RenderParameters::def_filter_text_size = 10;
const int RenderParameters::def_pin_text_size = 7;

const int RenderParameters::def_zoom		= 100;
const int RenderParameters::def_x_gap		= 40;
const int RenderParameters::def_y_gap		= 32;
const int RenderParameters::def_wrap_width	= 100;


RenderParameters::ConnectMode RenderParameters::connect_mode = ConnectMode_Intelligent;
bool RenderParameters::display_file_name = true;
bool RenderParameters::use_media_info = false;

RenderParameters::RenderParameters()
{
	display_file_name = AfxGetApp()->GetProfileInt(_T("Settings"), _T("DisplayFileName"), 1) ? true : false;
	exact_match_mode = false;
	abort_timeout = true;
	auto_arrange = AfxGetApp()->GetProfileInt(_T("Settings"), _T("AutoArrange"), false);
	resize_to_graph = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ResizeToGraph"), true);

    // load connect mode
    int connectMode = AfxGetApp()->GetProfileInt(_T("Settings"), _T("ConnectMode"), ConnectMode_Intelligent);
    if(connectMode<0) connectMode = 0;
    else if(connectMode>2) connectMode = 2;
    connect_mode = (ConnectMode)connectMode;

	render_start_time = 0;
	in_render = false;
	render_actions.clear();

	Zoom(def_zoom);
	filter_wrap_width	= def_wrap_width;
	filter_x_gap		= def_x_gap;
	filter_y_gap		= def_y_gap;

	// no preferred renderer
	preferred_video_renderer = _T("");
	video_renderers = NULL;

    use_media_info = AfxGetApp()->GetProfileInt(_T("Settings"), _T("UseMediaInfo"), 1) ? true : false;
    is_remote = false;

	// load bitmaps
	BOOL ok;	
	ok = bmp_volume_hi.LoadBitmap(IDB_BITMAP_VOLUME_HI);					if (!ok) return ;
	ok = bmp_volume_lo.LoadBitmap(IDB_BITMAP_VOLUME_LO);					if (!ok) return ;
	ok = bmp_clock_inactive_hi.LoadBitmap(IDB_BITMAP_CLOCK_INACTIVE_HI);	if (!ok) return ;
	ok = bmp_clock_inactive_lo.LoadBitmap(IDB_BITMAP_CLOCK_INACTIVE_LO);	if (!ok) return ;
	ok = bmp_clock_active_hi.LoadBitmap(IDB_BITMAP_CLOCK_ACTIVE_HI);		if (!ok) return ;
	ok = bmp_clock_active_lo.LoadBitmap(IDB_BITMAP_CLOCK_ACTIVE_LO);		if (!ok) return ;
}

RenderParameters::~RenderParameters()
{
}

void RenderParameters::Zoom(int z)
{
	zoom = z;

	min_filter_width = (((int)(z * def_min_width / 100.0)));
	min_filter_height = (((int)(z * def_min_height / 100.0)));
	pin_spacing = (int)(z * def_pin_spacing / 100.0);

	if (font_filter.m_hObject != 0) { font_filter.DeleteObject(); }
	if (font_pin.m_hObject != 0) { font_pin.DeleteObject(); }

	int size = 5 + (int) (5.0 * z / 100.0);
	MakeFont(font_filter, _T("Arial"), size, false, false); 
	size = 5 + (int) (2.0 * z / 100.0);
	MakeFont(font_pin, _T("Arial"), size, false, false);
}

void RenderParameters::MarkRender(bool start)
{
	if (in_render == start) return ;

	// remember the time
	in_render = start;
	if (start) {
		render_start_time = GetTickCount();
		render_can_proceed = true;
		render_actions.clear();
	}
}

void RenderParameters::ResetGraphLayout()
{
	filter_wrap_width = RenderParameters::def_wrap_width;
	filter_x_gap		= RenderParameters::def_x_gap;
	filter_y_gap		= RenderParameters::def_y_gap;
	Zoom(def_zoom);
}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation
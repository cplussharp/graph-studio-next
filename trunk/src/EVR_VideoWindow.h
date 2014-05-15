//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

class CMainFrame;

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	class Filter;

	//-------------------------------------------------------------------------
	//
	//	EVR_VideoWindow class
	//
	//-------------------------------------------------------------------------

	class EVR_VideoWindow : public CWnd
	{
	protected:
		DECLARE_DYNCREATE(EVR_VideoWindow)
		DECLARE_MESSAGE_MAP()

	public:

		Filter								*filter;
		CMainFrame							* parent_frame;
		CComPtr<IMFVideoDisplayControl>		video_control;		// EVR video control
		HCURSOR								cursor;

		// as we've seen them last time
		SIZE								native_size;
		SIZE								aspect_ratio;
		CRect								original_rect;		// used to restore position after full screen playback
		bool								full_screen;		// only true for duration of playback

	public:
		EVR_VideoWindow();
		virtual ~EVR_VideoWindow();

		// Init / Close
		int Open(Filter *filter);
		int Close();
		HRESULT Start(bool full_screen);
		HRESULT Stop();

		// methods
		void ResetSizePos();

		void OnSize(UINT nType, int cx, int cy);
		void OnPaint();
		BOOL OnEraseBkgnd(CDC *pDC);
		void OnClose();
		BOOL PreTranslateMessage(MSG *pmsg);
	};

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

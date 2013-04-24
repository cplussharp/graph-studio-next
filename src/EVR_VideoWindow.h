//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

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
		CComPtr<IMFVideoDisplayControl>		video_control;		// EVR video control
		HCURSOR								cursor;

		// as we've seen them last time
		SIZE								native_size;
		SIZE								aspect_ratio;

	public:
		EVR_VideoWindow();
		virtual ~EVR_VideoWindow();

		// Init / Close
		int Open(Filter *filter);
		int Close();
		int Start();

		// methods
		void ResetSizePos();

		void OnSize(UINT nType, int cx, int cy);
		void OnPaint();
		BOOL OnEraseBkgnd(CDC *pDC);
		void OnClose();
	};

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

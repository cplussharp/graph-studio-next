//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#pragma warning(disable: 4244)			// DWORD -> BYTE warning

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	//-------------------------------------------------------------------------
	//
	//	EVR_VideoWindow class
	//
	//-------------------------------------------------------------------------

	IMPLEMENT_DYNCREATE(EVR_VideoWindow, CWnd)
	
	BEGIN_MESSAGE_MAP(EVR_VideoWindow, CWnd)
		ON_WM_SIZE()
		ON_WM_PAINT()
		ON_WM_ERASEBKGND()
		ON_WM_CLOSE()
	END_MESSAGE_MAP()



	EVR_VideoWindow::EVR_VideoWindow() :
		CWnd()
	{
		video_control = NULL;

		native_size.cx = 0;
		native_size.cy = 0;
		aspect_ratio.cx = 1;
		aspect_ratio.cy = 1;
	}

	EVR_VideoWindow::~EVR_VideoWindow()
	{
		Close();
	}

	int EVR_VideoWindow::Open(Filter *filter)
	{
		CComPtr<IMFGetService>	getservice;
		HRESULT					hr;

		// no filter - no video window
		if (!filter || !filter->filter) return -1;

		hr = filter->filter->QueryInterface(IID_IMFGetService, (void**)&getservice);
		if (FAILED(hr)) return -1;

		// store the filter pointer
		this->filter = filter;

		// now let's try to get the IMFVideoDisplayControl interface
		hr = getservice->GetService(MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&video_control);
		if (FAILED(hr) || !video_control) return -1;

		cursor = AfxGetApp()->LoadCursor(IDC_ARROW);
		SetCursor(cursor);

		// now we can try to create the window
		BOOL	ok;		
		ok = CreateEx(0, AfxRegisterWndClass(0), _T("EVR Video Window"), WS_OVERLAPPEDWINDOW, 
					  CW_USEDEFAULT, CW_USEDEFAULT,
					  200, 200,
					  ::GetDesktopWindow(), 0);
		if (!ok) return -1;

		// set the clipping window
		hr = video_control->SetVideoWindow(m_hWnd);
		if (FAILED(hr)) return -1;
		
		CString		wndname;
		wndname = CString(_T("EVR Video Window: ")) + filter->name;
		SetWindowText(wndname);

		// just to be sure it's not visible
		ShowWindow(SW_HIDE);

		return 0;
	}

	int EVR_VideoWindow::Close()
	{
		// release all interfaces
		if (video_control) {
			video_control->SetVideoWindow(NULL);
			video_control = NULL;
		}

		filter = NULL;

		// kill the window
		if (::IsWindow(m_hWnd)) {
			DestroyWindow();
		}

		return 0;
	}

	int EVR_VideoWindow::Start()
	{
		if (!video_control) return -1;

		// check for the video stream change
		SIZE		new_size, new_ar;
		HRESULT		hr;

		hr = video_control->GetNativeVideoSize(&new_size, &new_ar);
		if (FAILED(hr)) return -1;

		if (new_size.cx != native_size.cx || new_size.cy != native_size.cy ||
			new_ar.cx != aspect_ratio.cx || new_ar.cy != aspect_ratio.cy
			) {

			native_size.cx = new_size.cx;
			native_size.cy = new_size.cy;
			aspect_ratio.cx = new_ar.cx;
			aspect_ratio.cy = new_ar.cy;

			// refresh the video window
			ResetSizePos();
		}

		ShowWindow(SW_SHOW);

		return 0;
	}

	void EVR_VideoWindow::ResetSizePos()
	{
		int		dx, dy;
		CRect	rcWnd, rcClient;
		GetWindowRect(&rcWnd);
		GetClientRect(&rcClient);

		dx = rcWnd.Width() - rcClient.Width();
		dy = rcWnd.Height() - rcClient.Height();

		this->SetWindowPos(NULL, 0, 0, native_size.cx + dx, native_size.cy + dy, SWP_NOMOVE | SWP_NOZORDER);
	}

	void EVR_VideoWindow::OnSize(UINT nType, int cx, int cy)
	{
		__super::OnSize(nType, cx, cy);
		if (!video_control) return ;

		int		width, height;
		CRect	rc;

		GetClientRect(&rc);
		width = rc.Width();
		height = rc.Height();

		// update the video control
		MFVideoNormalizedRect		src = { 0, 0, 1, 1 };

		video_control->SetVideoPosition(&src, &rc);
	}

	void EVR_VideoWindow::OnPaint()
	{
		ValidateRect(NULL);
		if (!video_control) return ;

		video_control->RepaintVideo();
	}

	BOOL EVR_VideoWindow::OnEraseBkgnd(CDC *pDC)
	{
		return TRUE;
	}

	void EVR_VideoWindow::OnClose()
	{
		ShowWindow(SW_HIDE);

		// send User Abort event
		if (filter->graph) {
			CComPtr<IMediaEventSink>	sink;
			HRESULT						hr;

			hr = filter->graph->gb->QueryInterface(IID_IMediaEventSink, (void**)&sink);
			if (SUCCEEDED(hr)) {
				sink->Notify(EC_USERABORT, 0, 0);
				sink = NULL;
			}
		}
	}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

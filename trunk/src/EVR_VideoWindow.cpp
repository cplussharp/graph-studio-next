//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "MainFrm.h"

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


	// Ideally we set parent frame to parent of of active document view but we don't have access to a document view at this point
	EVR_VideoWindow::EVR_VideoWindow() : 
		CWnd(),
		parent_frame(dynamic_cast<CMainFrame*>(AfxGetMainWnd())),
		full_screen(false)
	{
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
		if (!ok) 
			return E_FAIL;

		// set the clipping window
		hr = video_control->SetVideoWindow(m_hWnd);
		if (FAILED(hr)) 
			return hr;

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
			video_control->SetFullscreen(FALSE);
			ShowCursor(TRUE);
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
	HRESULT EVR_VideoWindow::Stop()
	{
		HRESULT hr = S_OK;
		if (video_control)
			hr = video_control->SetFullscreen(FALSE);
		
		ShowCursor(TRUE);
		if (full_screen)
			SetWindowPos(&wndNoTopMost, original_rect.left, original_rect.top, original_rect.Width(), original_rect.Height(), SWP_HIDEWINDOW);

		full_screen = false;
		return hr;
	}

	HRESULT EVR_VideoWindow::Start(bool fullscreen)
	{
		HRESULT hr = S_OK;

		CheckPointer(video_control, E_POINTER);

		full_screen = fullscreen;

		// check for the video stream change
		SIZE		new_size = { 0, 0 }, new_ar = { 0, 0 };

		hr = video_control->GetNativeVideoSize(&new_size, &new_ar);
		if (FAILED(hr)) 
			return hr;

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

		if (full_screen) {
			GetWindowRect(&original_rect);

			HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
			if (!monitor) {
				return E_FAIL;
			}

			 MONITORINFO info;
			 info.cbSize = sizeof(info);
			 if (!GetMonitorInfo(monitor, &info))
				 return E_FAIL;

			hr = video_control->SetFullscreen(TRUE);
			ShowCursor(FALSE);

			CRect rect(info.rcMonitor);
			SetWindowPos(&wndTopMost, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
			SetFocus();

		} else {
			ShowWindow(SW_SHOW);
			SetFocus();
		}

		return hr;
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

	BOOL EVR_VideoWindow::PreTranslateMessage(MSG *pmsg)
	{
		switch (pmsg->message) {
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				// Forward all key combinations to allow playback control including stopping full screen playback by pressing escape
				if (parent_frame && parent_frame->TranslateKeyboardAccelerator(pmsg)) {
					return TRUE;
				}
		}
		return __super::PreTranslateMessage(pmsg);
	}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

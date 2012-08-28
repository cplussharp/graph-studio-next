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
	//	URLLabel class
	//
	//-------------------------------------------------------------------------
	class URLLabel : public CStatic
	{
	protected:
		DECLARE_DYNCREATE(URLLabel)
		DECLARE_MESSAGE_MAP()

	public:

		CString			url;						// navigate to this page
		bool			reg_mouseleave;				// do we have the WM_MOUSELEAVE message registered ?
		bool			mouse_hover;				// is the mouse in our area ?

		HCURSOR			handpoint;

		DWORD			col_inactive;
		DWORD			col_active;

	public:
		URLLabel();
		virtual ~URLLabel();

		// handle mouse enter/leave
		void OnMouseMove(UINT nFlags, CPoint point);
		void OnMouseLeave();
		void OnLButtonDown(UINT nFlags, CPoint point);
		BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);

		HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

		virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	};



};


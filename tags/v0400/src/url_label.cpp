//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <math.h>

namespace GraphStudio
{

	IMPLEMENT_DYNCREATE(URLLabel, CStatic)

	BEGIN_MESSAGE_MAP(URLLabel, CStatic)
		ON_WM_MOUSEMOVE()
		ON_WM_CTLCOLOR_REFLECT()
		ON_WM_SETCURSOR()
		ON_WM_LBUTTONDOWN()
	END_MESSAGE_MAP()

	//-------------------------------------------------------------------------
	//
	//	URLLabel class
	//
	//-------------------------------------------------------------------------

	URLLabel::URLLabel()
	{
		url					= _T("");
		reg_mouseleave		= false;
		mouse_hover			= false;
		col_inactive		= RGB(0, 0, 0);
		col_active			= RGB(0, 0, 128);

		handpoint			= ::LoadCursor(NULL, IDC_HAND);
	}

	URLLabel::~URLLabel()
	{
	}

	void URLLabel::OnLButtonDown(UINT nFlags, CPoint point)
	{
		// open the link
		ShellExecute(NULL, _T("open"), url, _T(""), _T(""), SW_SHOWNORMAL);
	}

	// handle mouse enter/leave
	void URLLabel::OnMouseMove(UINT nFlags, CPoint point)
	{
		if (!reg_mouseleave) {
			reg_mouseleave = true;

			// we want to receive the WM_MOUSELEAVE notification
			TRACKMOUSEEVENT		tm;

			tm.cbSize		= sizeof(tm);
			tm.dwFlags		= TME_LEAVE;
			tm.dwHoverTime	= 0;
			tm.hwndTrack	= *this;

			TrackMouseEvent(&tm);
		}

		if (!mouse_hover) {
			mouse_hover = true;

			// repaint our window
			CWnd		*parent = GetParent();
			if (parent) {
				CPoint		pt(0, 0);
				parent->ClientToScreen(&pt);

				CPoint		op(0, 0);
				ClientToScreen(&op);

				CRect		rc;
				GetClientRect(&rc);
				rc.OffsetRect(op.x - pt.x, op.y - pt.y);
				parent->InvalidateRect(&rc);
				Invalidate();
			}
		}
	}

	void URLLabel::OnMouseLeave()
	{
		reg_mouseleave = false;
		mouse_hover = false;

		// repaint our window
		CWnd		*parent = GetParent();
		if (parent) {
			CPoint		pt(0, 0);
			parent->ClientToScreen(&pt);

			CPoint		op(0, 0);
			ClientToScreen(&op);

			CRect		rc;
			GetClientRect(&rc);
			rc.OffsetRect(op.x - pt.x, op.y - pt.y);
			parent->InvalidateRect(&rc);
			Invalidate();
		}
	}

	HBRUSH URLLabel::CtlColor(CDC *pDC, UINT nCtlColor)
	{
		if (mouse_hover) {
			pDC->SetTextColor(col_active);
		} else {
			pDC->SetTextColor(col_inactive);
		}
		pDC->SetBkMode(TRANSPARENT);
		return ((HBRUSH)GetStockObject(NULL_BRUSH));
	}

	BOOL URLLabel::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
	{
		::SetCursor(handpoint);
		return TRUE;
	}

	BOOL URLLabel::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
	{
		if (message == WM_MOUSELEAVE) {
			OnMouseLeave();
			return TRUE;
		} else
		return __super::OnWndMsg(message, wParam, lParam, pResult);
	}

};


//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

namespace GraphStudio
{

	IMPLEMENT_DYNCREATE(FilterListCtrl, CListCtrl)
	BEGIN_MESSAGE_MAP(FilterListCtrl, CListCtrl)
		ON_WM_LBUTTONDBLCLK()
	END_MESSAGE_MAP()

	//-------------------------------------------------------------------------
	//
	//	FilterListCtrl class
	//
	//-------------------------------------------------------------------------

	FilterListCtrl::FilterListCtrl()
	{
		MakeFont(font_filter, _T("Tahoma"), 8, false, false);
		MakeFont(font_info, _T("Tahoma"), 8, false, false);

		color_font = RGB(0,0,0);
		color_info = RGB(128, 128, 128);
		color_selected = RGB(178, 201, 211);
		color_error = RGB(255, 0, 0);

		type_colors[0] = RGB(0,0,0);		// Filter
		type_colors[1] = RGB(0,128,0);		// DMO
		type_colors[2] = RGB(128,0,0);		// KSProxy
		type_colors[3] = RGB(0,0,128);		// ACM/ICM
		type_colors[4] = RGB(128,0,128);	// PNP

		callback = NULL;
	}

	FilterListCtrl::~FilterListCtrl()
	{
	}

	void FilterListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
	{		
		int item, column;

		CListCtrl::OnLButtonDblClk(nFlags, point);
		item = HitTestEx(point, &column);
		if (item >= 0) {
			if (callback) callback->OnItemDblClk(item);
		}
	}

	// kreslenie itemov
	void FilterListCtrl::DrawItem(LPDRAWITEMSTRUCT item)
	{
		DSUtil::FilterTemplate	*filter = (DSUtil::FilterTemplate*)item->itemData;
		if (!filter) return ;

		CDC		dc;
		dc.Attach(item->hDC);

		// draw background ?
		if (item->itemState & (ODS_SELECTED | ODS_FOCUS)) {
			CBrush		brush(color_selected);
			CPen		pen(PS_SOLID, 1, color_selected);
			CRect		rc = item->rcItem;

			dc.SelectObject(brush);
			dc.SelectObject(pen);
			dc.Rectangle(&rc);
		}

		// draw filter name
		CRect	rcText = item->rcItem;
		rcText.left += 3;
		rcText.right -= 3;

		CString		type_text = _T("");
		int			idx       = 0;

		switch (filter->type) {
		case DSUtil::FilterTemplate::FT_PNP:		idx = 4; type_text = _T("PNP"); break;
		case DSUtil::FilterTemplate::FT_ACM_ICM:	idx = 3; type_text = _T("ACM/ICM"); break;
		case DSUtil::FilterTemplate::FT_KSPROXY:	idx = 2; type_text = _T("KS"); break;
		case DSUtil::FilterTemplate::FT_DMO:		idx = 1; type_text = _T("DMO"); break;
		case DSUtil::FilterTemplate::FT_FILTER:		idx = 0; type_text = _T(""); break;
		}
		dc.SetTextColor(type_colors[idx]);


		dc.SelectObject(&font_filter);
		dc.DrawText(filter->name, &rcText, DT_VCENTER | DT_SINGLELINE);

		// draw merit
		rcText.left = rcText.right - 70;
		dc.SetTextColor(color_info);
		dc.SelectObject(&font_info);
		CString info;
		info.Format(_T("(0x%08X)"), filter->merit);
		dc.DrawText(info, &rcText, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		if (!filter->file_exists) {
			CString	error_text = _T("(!)");
			CSize	extent = dc.GetTextExtent(error_text);

			rcText.right = rcText.left;
			rcText.left = rcText.right - extent.cx - 2*2;
			dc.SetTextColor(color_error);
			dc.DrawText(error_text, &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

		// draw type text
		int		rx   = rcText.left;
		rcText.left	 = rx - 66;
		rcText.right = rx - 16;
		dc.SelectObject(&font_filter);
		dc.SetTextColor(type_colors[idx]);
		dc.DrawText(type_text, &rcText, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

		dc.Detach();
		return ;
	}

	int FilterListCtrl::HitTestEx(CPoint &point, int *col) const
	{
		if (col) *col = 0;

		int row = HitTest(CPoint(0, point.y), NULL);

		if ((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT) return row;
		int nColumnCount = ((CHeaderCtrl*)GetDlgItem(0))->GetItemCount();

		for(int top = GetTopIndex(), bottom = GetBottomIndex(); top <= bottom; top++) {
			CRect r;
			GetItemRect(top, &r, LVIR_BOUNDS);

			if (r.top <= point.y && point.y < r.bottom)	{
				for (int colnum = 0; colnum < nColumnCount; colnum++) {
					int colwidth = GetColumnWidth(colnum);

					if (point.x >= r.left && point.x <= (r.left + colwidth)) {
						if(col) *col = colnum;
						return top;
					}
					r.left += colwidth;
				}
			}
		}

		return -1;
	}

	int FilterListCtrl::GetBottomIndex() const
	{
		CRect r;
		GetClientRect(r);

		int nBottomIndex = GetTopIndex() + GetCountPerPage() - 1;

		if (nBottomIndex >= GetItemCount()) {
			nBottomIndex = GetItemCount() - 1;
		} else 
		if (nBottomIndex < GetItemCount()) {
			CRect br;
			GetItemRect(nBottomIndex, br, LVIR_BOUNDS);

			if (br.bottom < r.bottom) nBottomIndex++;
		}

		return(nBottomIndex);
	}

};



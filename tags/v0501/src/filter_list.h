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

	class FilterListCallback
	{
	public:
		virtual void OnItemDblClk(int item) = 0;
	};

	//-------------------------------------------------------------------------
	//
	//	FilterListCtrl class
	//
	//-------------------------------------------------------------------------
	class FilterListCtrl : public CListCtrl
	{
	protected:
		DECLARE_DYNCREATE(FilterListCtrl)
		DECLARE_MESSAGE_MAP()
	public:
		CFont			font_filter;
		CFont			font_info;

		COLORREF		color_font;
		COLORREF		color_info;
		COLORREF		color_selected;
		COLORREF		color_error;

		COLORREF		type_colors[5];

		FilterListCallback	*callback;

	public:
		FilterListCtrl();
		~FilterListCtrl();

		// kreslenie itemov
		void DrawItem(LPDRAWITEMSTRUCT item);
		void OnLButtonDblClk(UINT nFlags, CPoint point);

		int GetBottomIndex() const;
		int HitTestEx(CPoint &point, int *col) const;

	};


};


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
		virtual void OnUpdateSearchString(const CString& search_string) = 0;
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
	
	private:
		CFont			font_filter;
		CFont			font_info;
		CFont			font_filter_sel;
		COLORREF		color_font;
		COLORREF		color_info;
		COLORREF		color_selected;
		COLORREF		color_error;
		COLORREF		type_colors[6];
		CString         search_str;

	public:
		FilterListCallback	*callback;
		CArray<DSUtil::FilterTemplate>  filters;

	public:
		FilterListCtrl();
		~FilterListCtrl();

		void UpdateList();
		void Initialize();
		void SetSearchString(const CString& search_string);
		int GetBottomIndex() const;
		COLORREF GetSelectionColor() const		{ return type_colors[5]; }

	private:
		// kreslenie itemov
		void DrawItem(LPDRAWITEMSTRUCT item);
		int HitTestEx(CPoint &point, int *col) const;

		afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	};
};


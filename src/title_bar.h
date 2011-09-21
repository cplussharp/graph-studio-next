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
	//	TitleBar class
	//
	//-------------------------------------------------------------------------
	class TitleBar : public CStatic
	{
	protected:
		DECLARE_DYNCREATE(TitleBar)
		DECLARE_MESSAGE_MAP()

	public:

		// double buffered view
		CBitmap			backbuffer;
		CDC				memDC;
		int				back_width, back_height;
		
		// colors
		DWORD			col_left, col_right;
		DWORD			col_glow_up, col_glow_bottom;
		int				topfade_up, topfade_bottom;
		int				bottomfade_up, bottomfade_bottom;

	public:
		TitleBar();
		virtual ~TitleBar();

		BOOL OnEraseBkgnd(CDC* pDC);
		BOOL PrepareBitmap(CDC *pDC);

		void DoPaint(CDC *dc, int cx, int cy);
		void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) { }

		virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	};



};


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

	IMPLEMENT_DYNCREATE(TitleBar, CStatic)

	BEGIN_MESSAGE_MAP(TitleBar, CStatic)
		// Standard printing commands
		ON_WM_ERASEBKGND()
	END_MESSAGE_MAP()

	//-------------------------------------------------------------------------
	//
	//	TitleBar class
	//
	//-------------------------------------------------------------------------

	#define MAKECOLOR(r,g,b)          ((COLORREF)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))

	TitleBar::TitleBar()
	{
		back_height = 0;
		back_width = 0;

		//col_left  = MAKECOLOR(4, 72, 117);
		//col_right = MAKECOLOR(25, 108, 119);
        col_left  = MAKECOLOR(200, 32, 32);
		col_right = MAKECOLOR(255, 64, 64);


		col_glow_up = MAKECOLOR(255,255,255);
		col_glow_bottom = MAKECOLOR(0,245,245);

		topfade_up = 128;
		topfade_bottom = 64;
		bottomfade_up = 0;
		bottomfade_bottom = 210;
	}

	TitleBar::~TitleBar()
	{
		if (memDC.GetSafeHdc()) {
			memDC.DeleteDC();
			backbuffer.DeleteObject();
		}
	}

	BOOL TitleBar::OnEraseBkgnd(CDC *pDC)
	{
		CRect		rc;
		GetClientRect(&rc);

		BOOL need_repaint = PrepareBitmap(pDC);
		if (need_repaint) {

			// paint
			DoPaint(&memDC, rc.Width(), rc.Height());
			pDC->BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

		} else {
			pDC->BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
		}
		
		return TRUE;
	}

	BOOL TitleBar::PrepareBitmap(CDC *pDC)
	{
		CRect	rc;
		GetClientRect(&rc);

		if (rc.Width() != back_width || rc.Height() != back_height) {

			if (memDC.GetSafeHdc()) {
				memDC.DeleteDC();
				backbuffer.DeleteObject();
			}

			memDC.CreateCompatibleDC(pDC);

			BITMAPINFO	bmi;
			HBITMAP		hb = NULL;
			DWORD		*bmiBits;

			ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
			bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth       = rc.Width();
			bmi.bmiHeader.biHeight      = -rc.Height();
			bmi.bmiHeader.biPlanes      = 1;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biBitCount    = 32;

			hb = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, (void**)&bmiBits, NULL, 0);
			if (!hb || !bmiBits) return FALSE;

			memDC.SelectObject(hb);
			backbuffer.Attach(hb);

			back_width = rc.Width();
			back_height = rc.Height();

			return TRUE;
		}

		return FALSE;
	}

	BOOL TitleBar::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
	{
		if (message == WM_COMMAND) {
			*pResult = GetParent()->SendMessage(message, wParam, lParam);
			return TRUE;
		}
		return __super::OnWndMsg(message, wParam, lParam, pResult);
	}


	#define COMP(a,b)		(((a)>>b)&0xff)
	inline DWORD BlendColor(DWORD c1, DWORD c2, int alpha)
	{
		int	r = (COMP(c1,16)*256 + (COMP(c2,16)-COMP(c1,16))*alpha) >> 8;
		int	g = (COMP(c1,8)*256 + (COMP(c2,8)-COMP(c1,8))*alpha) >> 8;
		int	b = (COMP(c1,0)*256 + (COMP(c2,0)-COMP(c1,0))*alpha) >> 8;
		return MAKECOLOR(r,g,b);
	}
	#undef COMP

	void TitleBar::DoPaint(CDC *dc, int cx, int cy)
	{
		int			x,y;
		BITMAP		b;
		GetObject(backbuffer, sizeof(b), &b);

		cy -= 1;

		// draw gradient
		int			stride = (b.bmWidthBytes / 4);
		DWORD		*pix = (DWORD*)(b.bmBits);
		DWORD		base_color;
		DWORD		color;
		int			y_mid = cy/2;

		for (y=0; y<cy; y++) {
			DWORD	*line = pix + y*stride;
			DWORD	glow_color;
			int		glow_alpha;

			// calculate glow parameters
			if (y < y_mid) {
				glow_color = col_glow_up;
				glow_alpha = (topfade_up + (((topfade_bottom-topfade_up)*(y+1))/y_mid));
			} else {
				glow_color = col_glow_bottom;
				glow_alpha = (bottomfade_up + (((bottomfade_bottom-bottomfade_up)*(y-y_mid+1))/y_mid));

				double	temp = glow_alpha / 256.0;
				temp = pow(temp, 3);
				glow_alpha = (int)(temp * 255.0);
			}

			for (x=0; x<cx; x++) {

				// base gradient color
				base_color = BlendColor(col_left, col_right, (x+1)*255/cx);
				color = BlendColor(base_color, glow_color, glow_alpha);

				line[x] = color;
			}
		}

		// draw border lines
		DWORD	*line1 = pix + 0*stride;
		DWORD	*line2 = pix + (cy-1)*stride;
		for (x=0; x<cx; x++) {
			line1[x] = BlendColor(line1[x], MAKECOLOR(255,255,255), 100);
			line2[x] = BlendColor(line2[x], MAKECOLOR(255,255,255), 100);
		}
		for (y=0; y<cy; y++) {
			line1 = pix + y*stride;
			line1[0]    = BlendColor(line1[0],    MAKECOLOR(255,255,255), 100);
			line1[cx-1] = BlendColor(line1[cx-1], MAKECOLOR(255,255,255), 100);
		}

		cy += 1;

		// black line at the bottom
		line1 = pix + (cy-1)*stride;
		for (x=0; x<cx; x++) {
			line1[x] = 0;
		}
	}

};


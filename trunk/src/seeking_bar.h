//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

class CGraphView;

//-----------------------------------------------------------------------------
//
//	CSeekSlider class
//
//-----------------------------------------------------------------------------
class CSeekSlider : public CSliderCtrl
{
protected:
	DECLARE_MESSAGE_MAP()

public:
	double	relative_pos;

public:
	CSeekSlider();
	~CSeekSlider();

	bool Create(CWnd *parent, CRect rc, UINT id);

	BOOL OnEraseBkgnd(CDC *pDC);
	void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

	// draw a larger channel
	void OnPaint();
	void SetChannelPos(double p);
};

//-----------------------------------------------------------------------------
//
//	CTransparentStatic class
//
//-----------------------------------------------------------------------------
class CTransparentStatic : public CStatic
{
	DECLARE_MESSAGE_MAP()
public:
	CTransparentStatic() { };
	~CTransparentStatic() { };
	HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	void SetText(CString &t);
};

//-----------------------------------------------------------------------------
//
//	CSeekingBar class
//
//-----------------------------------------------------------------------------

class CSeekingBar : public CDialogBar
{
protected:
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	CSeekSlider				seekbar;
	CTransparentStatic		label_time;
	CBrush					back_brush;

	CGraphView				*view;

	// accumulator for Seek Requests
	bool					pending_seek_request;

public:
	CSeekingBar();
	~CSeekingBar();

	virtual BOOL Create(CWnd *pParent, UINT nIDTemplate, UINT nStyle, UINT nID);
	BOOL OnEraseBkgnd(CDC *pDC);
	void OnMove(int cx, int cy);

	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	LONG OnInitDialog(UINT wParam, LONG lParam);
	HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

	// initialize the stuff
	void SetGraphView(CGraphView *graph_view);

	void OnTimer(UINT_PTR id);
	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	void UpdateGraphPosition();
	void PerformSeek();
};


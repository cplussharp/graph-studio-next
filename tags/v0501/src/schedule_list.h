//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

class ScheduleEvent;

namespace GraphStudio
{
	class ScheduleList;


	//-------------------------------------------------------------------------
	//
	//	ScheduleListEdit class
	//
	//-------------------------------------------------------------------------
	class ScheduleListEdit : public CEdit
	{
		friend class ScheduleList;
	protected:
		ScheduleEvent		*item;					// item, ktory editujeme
		int					column;					// column, ktoreho sa to tyka
		ScheduleList		*parent;				// list view, ktory sa edituje
		CString				def_text;				// defaultny text
		CFont				*font;

		virtual BOOL PreTranslateMessage(MSG* pMsg);
		DECLARE_MESSAGE_MAP()

	public:
		ScheduleListEdit(ScheduleList *pParent, ScheduleEvent *item, int column, CString default_text, CFont *font);
		virtual ~ScheduleListEdit();

		afx_msg void OnKillFocus(CWnd* pNewWnd);
		afx_msg void OnNcDestroy();
		afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	};

	//-------------------------------------------------------------------------
	//
	//	ScheduleListCombo class
	//
	//-------------------------------------------------------------------------
	class ScheduleListCombo : public CComboBox
	{
		friend class ScheduleList;
	protected:
		ScheduleEvent	*item;					// item, ktory editujeme
		int				column;					// column, ktoreho sa to tyka
		ScheduleList	*parent;				// list view, ktory sa edituje
		CFont			*font;
		int				default_value;
		bool			esc;

		virtual BOOL PreTranslateMessage(MSG* pMsg);
		DECLARE_MESSAGE_MAP()

	public:
		ScheduleListCombo(ScheduleList *pParent, ScheduleEvent *item, int column, CFont *font);
		virtual ~ScheduleListCombo();

		afx_msg void OnKillFocus(CWnd* pNewWnd);
		afx_msg void OnNcDestroy();
		afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		void OnCloseup();
	};


	//-------------------------------------------------------------------------
	//
	//	ScheduleList class
	//
	//-------------------------------------------------------------------------
	
	class ScheduleList : public CListCtrl
	{
	protected:
		DECLARE_DYNCREATE(ScheduleList)
		DECLARE_MESSAGE_MAP()

	public:

		ScheduleListEdit	*edit;
		ScheduleListCombo	*combo;

		int HitTestEx(CPoint &point, int *col) const;
		int GetBottomIndex() const;
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
		afx_msg BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnLButtonDblClk(UINT nFlags, CPoint point);

	public:
		ScheduleList();
		virtual ~ScheduleList();

		// inicializacia
		void Initialize();

		// kreslenie itemov
		void DrawItem(LPDRAWITEMSTRUCT item);
		void OnDrawColumn(CDC *dc, int index, CRect rc, LPDRAWITEMSTRUCT item);

		void DrawCheck(CRect rc, CDC &dc, int check_size, bool check, DWORD back_col, DWORD front_col);
		void DrawItemText(CString text, CRect rc, CDC &dc, int loffset=6, int roffset=6, CFont *font=NULL, DWORD col=0xffffffff);
		
		int EditItem(int item, int column, CString default_text, CFont *font);
		int ComboItem(int item, int column, int default_value, CFont *font);

	};

	bool VerifyTextPattern(CString newval);

};





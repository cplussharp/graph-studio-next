//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	//-------------------------------------------------------------------------
	//
	//	ScheduleList class
	//
	//-------------------------------------------------------------------------

	IMPLEMENT_DYNCREATE(ScheduleList, CListCtrl)

	BEGIN_MESSAGE_MAP(ScheduleList, CListCtrl)
		ON_WM_CREATE()
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_HSCROLL()
		ON_WM_VSCROLL()
	END_MESSAGE_MAP()

	//-------------------------------------------------------------------------
	//
	//	ScheduleList class
	//
	//-------------------------------------------------------------------------


	ScheduleList::ScheduleList() :
		CListCtrl(),
		edit(NULL),
		combo(NULL)
	{
	}

	ScheduleList::~ScheduleList()
	{
	}

	int ScheduleList::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		int ret = CListCtrl::OnCreate(lpCreateStruct);
		if (ret == -1) return -1;

		Initialize();
		return 0;
	}
		

	void ScheduleList::Initialize()
	{
		DWORD ex_style = GetExtendedStyle();
		ex_style = ex_style | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP;
		SetExtendedStyle(ex_style);

		// create header columns
		InsertColumn(0, _T("Active"), LVCFMT_LEFT, 50);
		InsertColumn(1, _T("Time"), LVCFMT_LEFT, 150);
		InsertColumn(2, _T("Action"), LVCFMT_LEFT, 140);
	}


	void ScheduleList::OnDrawColumn(CDC *dc, int index, CRect rc, LPDRAWITEMSTRUCT item)
	{
		ScheduleEvent	*data = (ScheduleEvent*)item->itemData;

		switch (index) {
		case 0:
			{
				DrawCheck(rc, *dc, 10, data->active, RGB(255,255,255), RGB(0,0,0));
			}
			break;
		case 1:
			{
				DrawItemText(data->time_pattern, rc, *dc);
			}
			break;
		case 2:
			{
				CString		act;
				switch (data->action) {
				case ScheduleEvent::ACTION_NONE:	act = _T("No action"); break;
				case ScheduleEvent::ACTION_RESTART:	act = _T("Restart"); break;
				case ScheduleEvent::ACTION_START:	act = _T("Start"); break;
				case ScheduleEvent::ACTION_STOP:	act = _T("Stop"); break;
				}

				DrawItemText(act, rc, *dc);
			}
			break;
		}
	}

	int ScheduleList::ComboItem(int item, int column, int default_value, CFont *font)
	{
		if (edit) {	edit->DestroyWindow(); delete edit;	edit = NULL; }
		if (combo) { 
			combo->DestroyWindow();
			delete combo;
			combo = NULL;
		}

		CRect	rect(0,0,0,0);
		GetItemRect(item, &rect, LVIR_BOUNDS);

		// spocitame suradnice rectu
		for (int i=0; i<column; i++) { rect.left += GetColumnWidth(i); }
		rect.right = rect.left + GetColumnWidth(column);

		// nova instancia kombacu
		ScheduleEvent		*data = (ScheduleEvent*)GetItemData(item);
		ScheduleListCombo *combo_new = new ScheduleListCombo(this, data, column, font);
		combo_new->Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL, rect, this, 2);

		combo_new->SetDroppedWidth(rect.Width() + 48);

		switch (column) {
		case 2:
			{
				int		ipos=0;

				combo_new->AddString(_T("No action"));
				combo_new->AddString(_T("Start"));
				combo_new->AddString(_T("Stop"));
				combo_new->AddString(_T("Restart"));

				ipos = data->action;

				// vyznacime toto
				combo_new->SetCurSel(ipos);
				combo_new->ShowDropDown();
			}
			break;
		}

		return 0;
	}


	int ScheduleList::EditItem(int item, int column, CString default_text, CFont *font)
	{
		if (edit) {	edit->DestroyWindow(); delete edit;	edit = NULL; }
		if (combo) { 
			combo->DestroyWindow();
			delete combo;
			combo = NULL;
		}

		CRect	rect(0,0,0,0);
		GetItemRect(item, &rect, LVIR_BOUNDS);

		// spocitame suradnice rectu
		for (int i=0; i<column; i++) { rect.left += GetColumnWidth(i); }
		rect.right = rect.left + GetColumnWidth(column);

		// nova instancia editboxu
		ScheduleListEdit *edit_new = new ScheduleListEdit(this, (ScheduleEvent*)GetItemData(item), column, default_text, font);
		edit_new->Create(WS_BORDER | WS_CHILD | WS_VISIBLE |
						 ES_LEFT | ES_WANTRETURN | ES_SUNKEN, 
						 rect, this, 1);

		return 0;
	}

	// kreslenie itemov
	void ScheduleList::DrawItem(LPDRAWITEMSTRUCT item)
	{
		ScheduleEvent	*data = (ScheduleEvent*)item->itemData;
		if (!data) return ;

		CDC		dc;
		dc.Attach(item->hDC);

		/*
			Ak je selectnuty, vyfarbime pozadie
		*/
		if (item->itemState & (ODS_SELECTED)) {
			CBrush		brush(RGB(178, 201, 211));
			CPen		pen(PS_SOLID, 1, RGB(178, 201, 211));
			CRect		rc = item->rcItem;

			dc.SelectObject(brush);
			dc.SelectObject(pen);
			dc.Rectangle(&rc);
		}

		// nakreslime vsetky columny
		int	col_index = 0;
		int	col_start = item->rcItem.left;
		do {
			LVCOLUMN		col;
			CRect			col_rect;
			col.mask = LVCF_WIDTH;

			if (GetColumn(col_index, &col)) {

				col_rect.left = col_start;
				col_rect.right = col_start + col.cx;
				col_rect.top = item->rcItem.top;
				col_rect.bottom = item->rcItem.bottom;

				// kreslime
				OnDrawColumn(&dc, col_index, col_rect, item);

				// posuvame sa
				col_index++;
				col_start += col.cx;

			} else {
				break;
			}
		} while (1);

		dc.Detach();
		return ;
	}

	void ScheduleList::DrawItemText(CString text, CRect rc, CDC &dc, int loffset, 
							   int roffset, CFont *font, DWORD col)
	{
		if (!font) font = GetFont();
		if (col == 0xffffffff) col = RGB(0,0,0);

		rc.left += loffset;			rc.right -= roffset;
		dc.SetTextColor(col);
		dc.SelectObject(*font);
		dc.DrawText(text, &rc, DT_VCENTER | DT_SINGLELINE);
	}

	void ScheduleList::DrawCheck(CRect rc, CDC &dc, int check_size, bool check, DWORD back_col, DWORD front_col)
	{
		CPen		pen(PS_SOLID, 2, front_col);
		CBrush		brush(back_col);

		CRect		rc_tick;

		rc_tick.left   = (rc.left + rc.right - check_size) / 2;
		rc_tick.right  = rc_tick.left + check_size;
		rc_tick.top	   = (rc.top + rc.bottom - check_size) / 2;
		rc_tick.bottom = rc_tick.top + check_size;
		
		dc.SelectObject(pen);
		dc.SelectObject(brush);
		dc.Rectangle(rc_tick);

		// nakreslime krizik
		if (check) {
			dc.MoveTo(rc_tick.left, rc_tick.top);	 dc.LineTo(rc_tick.right-2, rc_tick.bottom-2);
			dc.MoveTo(rc_tick.left, rc_tick.bottom-2); dc.LineTo(rc_tick.right-2, rc_tick.top);
		}
	}

	int ScheduleList::HitTestEx(CPoint &point, int *col) const
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

	int ScheduleList::GetBottomIndex() const
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

	void ScheduleList::OnLButtonDown(UINT nFlags, CPoint point)
	{
		if (combo) { 
			combo->DestroyWindow();
			delete combo;
			combo = NULL;
		}

		int item, column;

		CListCtrl::OnLButtonDown(nFlags, point);
		item = HitTestEx(point, &column);
		if (item >= 0) {

			// posleme hlasku o kliknuti
			ScheduleEvent	*item_obj = (ScheduleEvent*)GetItemData(item);
			if (item_obj) {
				switch (column) {
				case 0:
					{
						// zmenime
						item_obj->active = !item_obj->active;
						Update(item);
					}
					break;
				case 1:
					{
						// pattern
						EditItem(item, column, item_obj->time_pattern, GetFont());
					}
					break;
				case 2:
					{
						// action
						ComboItem(item, column, 0, GetFont());
					}
					break;
				}
			}

		}
	}

	void ScheduleList::OnLButtonDblClk(UINT nFlags, CPoint point)
	{		
		int item, column;

		CListCtrl::OnLButtonDblClk(nFlags, point);
		item = HitTestEx(point, &column);
		if (item >= 0) {

			ScheduleEvent	*item_obj = (ScheduleEvent*)GetItemData(item);
			if (item_obj) {

				switch (column) {
				case 1:
					{
						// pattern
						EditItem(item, column, item_obj->time_pattern, GetFont());
					}
					break;
				case 2:
					{
						// action
						ComboItem(item, column, 0, GetFont());
					}
					break;
				}
			}

		}
	}

	void ScheduleList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
	{
		if (edit) {	edit->DestroyWindow(); delete edit;	edit = NULL; }
		if (combo) { combo->DestroyWindow(); delete combo; combo = NULL; }
		__super::OnVScroll(nSBCode, nPos, pScrollBar);
	}

	void ScheduleList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
	{
		if (edit) {	edit->DestroyWindow(); delete edit;	edit = NULL; }
		if (combo) { 
			combo->DestroyWindow();
			delete combo;
			combo = NULL;
		}
		__super::OnHScroll(nSBCode, nPos, pScrollBar);
	}

	BOOL ScheduleList::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
	{
		HD_NOTIFY &nh = *(HD_NOTIFY*)lParam;
		if (nh.hdr.code == HDN_BEGINTRACKW || 
			nh.hdr.code == HDN_BEGINTRACKA || 
			nh.hdr.code == HDN_BEGINDRAG) {
			if (edit) {	edit->DestroyWindow(); delete edit;	edit = NULL; }
		}
		return __super::OnNotify(wParam, lParam, pResult);
	}


	//-------------------------------------------------------------------------
	//
	//	ScheduleListEdit class
	//
	//-------------------------------------------------------------------------

	BEGIN_MESSAGE_MAP(ScheduleListEdit, CEdit)
		ON_WM_CREATE()
		ON_WM_KILLFOCUS()
		ON_WM_CHAR()
		ON_WM_NCDESTROY()
	END_MESSAGE_MAP()


	ScheduleListEdit::ScheduleListEdit(ScheduleList *pParent, ScheduleEvent *item, int column, CString default_text, CFont *font) :
		CEdit(),
		parent(pParent)
	{
		this->item = item;
		this->column = column;
		this->def_text = default_text;
		this->font = font;

		// nastavime, ze mame editovaca
		parent->edit = this;
	}

	ScheduleListEdit::~ScheduleListEdit()
	{
		// zrusime editovaca
		parent->edit = NULL;
	}

	int ScheduleListEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
	{
		if (CEdit::OnCreate(lpCreateStruct) == -1) return -1;
		
		// nastavime font
		SetFont(font);

		// nastavime text
		SetFocus();
		SetWindowText(def_text);
		SetSel(0, -1);

		return 0;
	}

	BOOL ScheduleListEdit::PreTranslateMessage(MSG* pMsg) 
	{
		if (pMsg->message == WM_KEYDOWN) {
			if (!(GetKeyState(VK_CONTROL) & 0x8000)
					&& !(GetKeyState(VK_MENU) & 0x8000)) {
				if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) {
					::TranslateMessage(pMsg);
					::DispatchMessage(pMsg);
					return TRUE;				// koncime
				}
			}
		}
		return __super::PreTranslateMessage(pMsg);
	}

	void ScheduleListEdit::OnKillFocus(CWnd* pNewWnd)
	{
		CEdit::OnKillFocus(pNewWnd);		
		PostMessage(WM_CLOSE);
	}

	void ScheduleListEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		if (!(GetKeyState(VK_CONTROL) & 0x8000)
				&& !(GetKeyState(VK_MENU) & 0x8000)) {
			if (nChar == VK_ESCAPE || nChar == VK_RETURN) {

				// iba pri stlaceni ENTER sa prepise
				if (nChar == VK_RETURN) {
					CString	val;
					GetWindowText(val);
					if (item) {
						if (VerifyTextPattern(val)) {
							item->time_pattern = val;
						}
					}
				}

				GetParent()->SetFocus();
				return;
			}
		}
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}

	void ScheduleListEdit::OnNcDestroy() 
	{
		CEdit::OnNcDestroy();	
		delete this;
	}


	//-----------------------------------------------------------------------------
	//
	//	ScheduleListCombo class
	//
	//-----------------------------------------------------------------------------

	BEGIN_MESSAGE_MAP(ScheduleListCombo, CComboBox)
		ON_WM_CREATE()
		ON_WM_KILLFOCUS()
		ON_WM_CHAR()
		ON_WM_NCDESTROY()
		ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseup)
	END_MESSAGE_MAP()


	ScheduleListCombo::ScheduleListCombo(ScheduleList *pParent, ScheduleEvent *item, int column, CFont *font) :
		CComboBox(),
		parent(pParent)
	{
		this->item = item;
		this->column = column;
		this->default_value = 0;
		this->font = font;
		esc = false;

		// nastavime, ze mame kombaca
		parent->combo = this;
	}

	ScheduleListCombo::~ScheduleListCombo()
	{
		parent->combo = NULL;
	}

	int ScheduleListCombo::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		if (CComboBox::OnCreate(lpCreateStruct) == -1) return -1;

		// nastavime font
		SetFont(font);
		return 0;
	}

	BOOL ScheduleListCombo::PreTranslateMessage(MSG* pMsg)
	{
		if (pMsg->message == WM_KEYDOWN) {
			if (!(GetKeyState(VK_CONTROL) & 0x8000)
					&& !(GetKeyState(VK_MENU) & 0x8000)) {
				if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) {
					::TranslateMessage(pMsg);
					::DispatchMessage(pMsg);
					return TRUE;				// koncime
				}
			}
		}
		return CComboBox::PreTranslateMessage(pMsg);
	}

	void ScheduleListCombo::OnKillFocus(CWnd* pNewWnd)
	{
		CComboBox::OnKillFocus(pNewWnd);
	
		CString str;
		GetWindowText(str);

		// posielame niekam text
		if (!esc) {
			if (item) {
				int sel = GetCurSel();
				item->action = sel;
			}
		}
		PostMessage(WM_CLOSE);
	}

	void ScheduleListCombo::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
	{
		if (!(GetKeyState(VK_CONTROL) & 0x8000) 
				&& !(GetKeyState(VK_MENU) & 0x8000)) {
			if (nChar == VK_ESCAPE || nChar == VK_RETURN) {
				esc = (nChar == VK_ESCAPE);
				GetParent()->SetFocus();
				return;
			}
		}
		CComboBox::OnChar(nChar, nRepCnt, nFlags);
	}

	void ScheduleListCombo::OnNcDestroy() 
	{
		CComboBox::OnNcDestroy();	
		delete this;
	}

	void ScheduleListCombo::OnCloseup() 
	{
		GetParent()->SetFocus();
	}






	bool VerifyTextPattern(CString newval)
	{
		if (newval == _T("")) return false;

		return true;
	}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation


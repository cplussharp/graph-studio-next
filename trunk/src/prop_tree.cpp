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

	//-------------------------------------------------------------------------
	//
	//	PropItem class
	//
	//-------------------------------------------------------------------------
	
	PropItem::PropItem(CString n) :
		name(n),
		value(_T("")),
		type(TYPE_STRUCT),
        expand(true)
	{
	}

	PropItem::PropItem(CString n, GUID guid) :
		name(n),
		value(_T("")),
		type(TYPE_GUID),
        expand(false)
	{
		// convert to string
		LPOLESTR	str = NULL;
		StringFromCLSID(guid, &str);

		value = str;
		if (str) CoTaskMemFree(str);
	}

	PropItem::PropItem(CString n, int val) :
		name(n),
		type(TYPE_INT),
        expand(false)
	{
		value.Format(_T("%d"), val);
	}

	PropItem::PropItem(CString n, RECT rc) :
		name(n),
		type(TYPE_RECT),
        expand(false)
	{
		value.Format(_T("[%d, %d, %d, %d]"), rc.left, rc.top, rc.right, rc.bottom);
	}

	PropItem::PropItem(CString n, __int64 i) :
		name(n),
		type(TYPE_INT),
        expand(false)
	{
		value.Format(_T("%I64d"), i);
	}

	PropItem::PropItem(CString n, CString str) :
		name(n),
		type(TYPE_STRING),
        expand(false)
	{
		value = str;
	}

    PropItem::PropItem(CString n, CString str, bool isUrl) :
		name(n),
        expand(false)
	{
        type = isUrl ? TYPE_URL : TYPE_STRING;
		value = str;
	}

	PropItem::PropItem(CString n, bool val) :
		name(n),
		type(TYPE_BOOL),
        expand(false)
	{
		value = (val ? _T("TRUE") : _T("FALSE"));
	}

	PropItem::~PropItem()
	{
		Clear();
	}

	void PropItem::Clear()
	{
		for (int i=0; i<items.GetCount(); i++) {
			PropItem *item = items[i];
			delete item;
		}
		items.RemoveAll();
	}

	// build up the tree
	PropItem *PropItem::AddItem(PropItem *item)
	{
		items.Add(item);
		return item;
	}


	//-------------------------------------------------------------------------
	//
	//	PropTreeCtrl class
	//
	//-------------------------------------------------------------------------

	BEGIN_MESSAGE_MAP(PropTreeCtrl, CTreeCtrl)
		ON_WM_LBUTTONDOWN()
		ON_WM_VSCROLL()
	END_MESSAGE_MAP()

	PropTreeCtrl::PropTreeCtrl() :
		CTreeCtrl(),
		edit(NULL)
	{
	}

	PropTreeCtrl::~PropTreeCtrl()
	{
	}

	void PropTreeCtrl::OnSelChanged()
	{
		// display an edit field for the current item
		HTREEITEM	selitem = GetSelectedItem();
		if (selitem) {
			EditItem(selitem);
		} else {
			if (edit) { edit->DestroyWindow(); }
		}
	}

	void PropTreeCtrl::CancelEdit()
	{
		if (edit) { edit->DestroyWindow(); }
	}

	void PropTreeCtrl::EditItem(HTREEITEM item)
	{
		if (edit) { edit->DestroyWindow(); }

		PropItem	*pitem = (PropItem*)GetItemData(item);
		// don't display edit fields for groups
		if (!pitem) return ;
		if (pitem->type == GraphStudio::PropItem::TYPE_STRUCT) return ;

		CRect	rect(0,0,0,0);
		GetItemRect(item, &rect, FALSE);
		parent->AdjustTextRect(rect);

		// new instane of the Edit control
		PropTreeEdit	*edit_new = new PropTreeEdit(this, item, pitem, font_item);
		edit_new->Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT | ES_READONLY, rect, this, 1);
	}

	void PropTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
	{
		HTREEITEM	item = HitTest(point);
		if (item) {
			SelectItem(item);

			if (!ItemHasChildren(item)) return ;

			CRect	rc_check;
			CRect	rc;
			GetItemRect(item, &rc, FALSE);

			rc_check.left = rc.left + 2;
			rc_check.top  = rc.top  + 4;
			rc_check.right = rc_check.left + 10;
			rc_check.bottom = rc_check.top + 10;

			// toggle item
			if (rc_check.PtInRect(point)) {
				Expand(item, TVE_TOGGLE);
			}
		}
	}

	void PropTreeCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
	{
		CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
		if (edit) edit->UpdatePos();
	}


	//-------------------------------------------------------------------------
	//
	//	PropertyTree class
	//
	//-------------------------------------------------------------------------

	IMPLEMENT_DYNCREATE(PropertyTree, CStatic)
	BEGIN_MESSAGE_MAP(PropertyTree, CStatic)
		ON_WM_SIZE()
	END_MESSAGE_MAP()


	PropertyTree::PropertyTree() :
		CStatic()
	{
		MakeFont(font_group, _T("Tahoma"), 8, true, false);
		MakeFont(font_item, _T("Tahoma"), 8, false, false);

		color_group = RGB(0,0,0);
		color_item = RGB(0,0,0);
		color_item_selected = RGB(255,255,255);
		color_back_group = RGB(241, 239, 226);
		color_back_item = RGB(255, 255, 255);
		color_back_item_selected = RGB(51,153,255);

		left_offset = 14;
		left_width  = 130;
	}

	PropertyTree::~PropertyTree()
	{
	}

	void PropertyTree::Initialize()
	{
		if (tree.m_hWnd) return ;

		CRect rcClient;
		GetClientRect(&rcClient);

		// create tree and header controls as children
		tree.Create(WS_CHILD | WS_VISIBLE  | TVS_FULLROWSELECT |
					TVS_NOHSCROLL | TVS_NOTOOLTIPS,				
					CRect(), this, ID_TREE);
		tree.parent = this;
		tree.font_item = &font_item;

		RepositionControls();
	}

	void PropertyTree::OnSize(UINT nType, int cx, int cy)
	{
		RepositionControls();
	}

	void PropertyTree::BuildPropertyTree(PropItem *root)
	{
		tree.DeleteAllItems();
		tree.CancelEdit();
		BuildNode(root, tree.GetRootItem());
	}

	void PropertyTree::BuildNode(PropItem *node, HTREEITEM item)
	{
		int i;
		for (i=0; i<node->GetCount(); i++) {
			PropItem *pi = node->GetItem(i);

			HTREEITEM	newitem = tree.InsertItem(pi->name, item);
			tree.SetItemData(newitem, (DWORD_PTR)pi);

			BuildNode(pi, newitem);
		}

		// rozbalime
        if(node->expand)
		    tree.Expand(item, TVE_EXPAND);
	}

	void PropertyTree::RepositionControls()
	{
		if (tree.m_hWnd) {
			CRect		rcClient;
			GetClientRect(&rcClient);

			tree.MoveWindow(&rcClient);
		}
	}

	void PropertyTree::OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
	{
		NMCUSTOMDRAW	*pNMCustomDraw = (NMCUSTOMDRAW*)pNMHDR;
		NMTVCUSTOMDRAW	*pNMTVCustomDraw = (NMTVCUSTOMDRAW*)pNMHDR;

		switch (pNMCustomDraw->dwDrawStage) {
		case CDDS_PREPAINT:		*pResult = CDRF_NOTIFYITEMDRAW;	break;
		case CDDS_PREERASE:		*pResult = CDRF_SKIPDEFAULT; break;
		case CDDS_ITEMPREPAINT:	
			{
				// we paint the item manually
				HTREEITEM hItem = (HTREEITEM)pNMCustomDraw->dwItemSpec;
				PaintItem(hItem, pNMCustomDraw->uItemState, pNMCustomDraw);				

				*pResult = CDRF_SKIPDEFAULT; 
			}
			break;
		default:
			*pResult = CDRF_DODEFAULT;
		}
	}

	void PropertyTree::AdjustTextRect(CRect &rc)
	{
		// borders
		rc.top += 2;		rc.left += 0;
		rc.bottom -= 2;		rc.right -= 1;

		// left margin
		rc.left += left_offset;

		// left area width
		rc.left += left_width;

		// middle separator
		rc.left += 1;
	}

	void PropertyTree::PaintItem(HTREEITEM item, UINT state, NMCUSTOMDRAW *draw)
	{
		GraphStudio::PropItem	*prop = (GraphStudio::PropItem*)tree.GetItemData(item);
		if (!prop) return ;

		CRect	rc = draw->rc;
		CDC		dc;
		dc.Attach(draw->hdc);

		// we either draw a node or a leaf
		CBrush		brush_back(color_back_group);
		CBrush		brush_item(color_back_item);
		CBrush		brush_item_selected(color_back_item_selected);
		CPen		pen_back(PS_SOLID, 1, color_back_group);
		CPen		pen_item(PS_SOLID, 1, color_back_item);
		CPen		pen_item_selected(PS_SOLID, 1, color_back_item_selected);

		CBrush		*prev_brush = dc.SelectObject(&brush_back);
		CPen		*prev_pen   = dc.SelectObject(&pen_back);
		CFont		*prev_font	= dc.SelectObject(&font_group);

		int			text_left = left_offset + 2;

		dc.SetBkMode(TRANSPARENT);
		state = tree.GetItemState(item, TVIF_STATE);

		if (prop->type == GraphStudio::PropItem::TYPE_STRUCT) {
			// we paint the whole background
			dc.Rectangle(rc);
			dc.SetTextColor(color_group);

			// draw the text
			CRect	rc_text = rc;
			rc_text.left += text_left;
			rc_text.top += 1;
			rc_text.bottom -= 1;
			dc.DrawText(prop->name, &rc_text, DT_VCENTER | DT_SINGLELINE);

			// draw the + mark
			BOOL expanded = (state & TVIS_EXPANDED ? TRUE : FALSE);
			
			CPen	black_pen(PS_SOLID, 1, RGB(0,0,0));
			CRect	rc_mark;
			dc.SelectObject(&black_pen);

			rc_mark.left   = rc.left + 2;
			rc_mark.top    = rc.top  + 4;
			rc_mark.right  = rc_mark.left + 9;
			rc_mark.bottom = rc_mark.top + 9;
			dc.Rectangle(&rc_mark);

			dc.MoveTo(rc_mark.left + 2, rc_mark.top + 4);
			dc.LineTo(rc_mark.right - 2, rc_mark.top + 4);
			if (!expanded) {
				dc.MoveTo(rc_mark.left + 4, rc_mark.top + 2);
				dc.LineTo(rc_mark.left + 4, rc_mark.bottom - 2);
			}

			// draw the selection rectangle
			if (state & TVIS_SELECTED) {
				CSize		s = dc.GetTextExtent(prop->name);
				CRect		rc_bound = rc_text;
				rc_bound.left -= 2;
				rc_bound.right = rc_bound.left + s.cx + 4;
				rc_bound.bottom = rc_bound.top + s.cy + 3;
				dc.SelectObject(&pen_item_selected);
				dc.DrawFocusRect(&rc_bound);
			}

		} else {
			// we paint just the left margin
			CRect	rc_left = rc;
			rc_left.right = rc_left.left + left_offset;
			dc.Rectangle(&rc_left);
			dc.MoveTo(rc.left, rc.bottom-1);
			dc.LineTo(rc.right, rc.bottom-1);

			// middle line
			int	mid_line = rc.left + left_offset + left_width;

			dc.MoveTo(mid_line, rc.top);
			dc.LineTo(mid_line, rc.bottom);

			dc.SelectObject(&brush_item);
			dc.SelectObject(&pen_item);
			dc.Rectangle(mid_line+1, rc_left.top, rc.right, rc.bottom -1);

			// is the item selected /
			if (state & TVIS_SELECTED) {
				dc.SelectObject(&brush_item_selected);
				dc.SelectObject(&pen_item_selected);
				dc.SetTextColor(color_item_selected);
			} else {
				dc.SetTextColor(color_item);
			}
			dc.Rectangle(rc_left.right, rc_left.top, mid_line, rc.bottom -1);

			dc.SelectObject(&font_item);
			text_left += 2;

			CRect	rc_text = rc;
			rc_text.top += 1;
			rc_text.bottom -= 1;

			rc_text.left += text_left;
			rc_text.right = mid_line - 2;
			dc.DrawText(prop->name, &rc_text, DT_VCENTER | DT_SINGLELINE);

			rc_text.left = mid_line + 4;
			rc_text.right = rc.right - 2;
			dc.SetTextColor(color_item);
			dc.DrawText(prop->value, &rc_text, DT_VCENTER | DT_SINGLELINE);			
		}

		dc.SelectObject(prev_brush);
		dc.SelectObject(prev_pen);
		dc.SelectObject(prev_font);
		dc.Detach();
	}

	BOOL PropertyTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
	{
		LPNMHDR pHdr = (LPNMHDR)lParam;

		switch (pHdr->code) {
		case NM_CUSTOMDRAW:
			{
				OnTreeCustomDraw(pHdr, pResult);
				return TRUE;
			}
			break;
		case HDN_ITEMCHANGING:
		case HDN_ITEMCHANGED:
			{
				// header ?
				return TRUE;
			}
			break;
		case TVN_SELCHANGED:	
			{
				tree.OnSelChanged();
			}
			break;
		case TVN_ITEMEXPANDING:	Invalidate();
		case TVN_ITEMEXPANDED:	RepositionControls();
		}

		// forward notifications from children to the control owner
		pHdr->hwndFrom = GetSafeHwnd();
		pHdr->idFrom = GetWindowLong(GetSafeHwnd(),GWL_ID);
		return (BOOL)GetParent()->SendMessage(WM_NOTIFY,wParam,lParam);			
	}

	//-------------------------------------------------------------------------
	//
	//	PropTreeEdit class
	//
	//-------------------------------------------------------------------------

	BEGIN_MESSAGE_MAP(PropTreeEdit, CEdit)
		ON_WM_CREATE()
		ON_WM_NCDESTROY()
		ON_WM_CTLCOLOR_REFLECT()
		ON_WM_KEYDOWN()
	END_MESSAGE_MAP()

	PropTreeEdit::PropTreeEdit(PropTreeCtrl *pParent, HTREEITEM htItem, PropItem *pitem, CFont *font) :
		CEdit(),
		parent(pParent),
		item(pitem)
	{
		this->htItem = htItem;
		this->font = font;

		// assign a new edit field
		parent->edit = this;

		DWORD	back_color = RGB(255, 255, 255);
		brush_back.CreateSolidBrush(back_color);
	}

	PropTreeEdit::~PropTreeEdit()
	{
		// cancel the edit field
		parent->edit = NULL;
	}

	void PropTreeEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		// we forward several keys to the parent
		switch (nChar) {
		case VK_UP:
		case VK_DOWN:
		case VK_NEXT:
		case VK_PRIOR:
		case VK_ESCAPE:
			{
				parent->PostMessage(WM_KEYDOWN, nChar, ((nFlags << 16) | nRepCnt));
				return ;
			}
			break;
		}
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
	}

	void PropTreeEdit::UpdatePos()
	{
		CRect		rc;
		parent->GetItemRect(htItem, rc, FALSE);
		parent->parent->AdjustTextRect(rc);
		MoveWindow(&rc);
		Invalidate();
	}

	int PropTreeEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
	{
		if (CEdit::OnCreate(lpCreateStruct) == -1) return -1;
		
		SetFont(font);
		SetFocus();

		CString	val = _T("");
		if (item) val = item->value;
		SetWindowText(val);
		SetSel(0, -1);
		return 0;
	}

	BOOL PropTreeEdit::PreTranslateMessage(MSG* pMsg) 
	{
		if (pMsg->message == WM_KEYDOWN) {
			if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) {
				::TranslateMessage(pMsg);
				::DispatchMessage(pMsg);
				return TRUE;				// koncime
			}
		}
		return __super::PreTranslateMessage(pMsg);
	}

	void PropTreeEdit::OnNcDestroy() 
	{
		CEdit::OnNcDestroy();	
		delete this;
	}

	HBRUSH PropTreeEdit::CtlColor(CDC *dc, UINT nCtlColor)
	{
		// override readonly background color
		return brush_back;	
	}

};


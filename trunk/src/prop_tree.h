//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	class PropTreeCtrl;
	class PropertyTree;

	//-------------------------------------------------------------------------
	//
	//	PropItem class
	//
	//-------------------------------------------------------------------------
	
	class PropItem
	{
	public:
		CArray<PropItem*>	items;

		CString				name;
		CString				value;

		enum {
			TYPE_UNKNOWN	= 0,
			TYPE_STRUCT		= 1,
			TYPE_INT		= 2,
			TYPE_STRING		= 3,
			TYPE_BOOL		= 4,
			TYPE_GUID		= 5,
			TYPE_RECT		= 6,
            TYPE_URL        = 7,
            TYPE_DOUBLE     = 8,
            TYPE_LARGEINT   = 9,
            TYPE_TIME       = 10
		};

		int					type;
        bool                expand;

	public:
		// struct
		PropItem(CString n);

		// constructors for separate types
		PropItem(CString n, GUID guid);
		PropItem(CString n, int val);
        PropItem(CString n, unsigned int val);
        PropItem(CString n, double val);
		PropItem(CString n, CString str);
        PropItem(CString n, CString str, bool isUrl);
		PropItem(CString n, bool val);
		PropItem(CString n, RECT rc);
		PropItem(CString n, __int64 val);
        PropItem(CString n, unsigned __int64 val);
        PropItem(CString n, CTime val);

		virtual ~PropItem();

		void Clear();

		int GetCount() { return (int)items.GetCount(); }
		PropItem *GetItem(int n) { return ( n<0 || n >= items.GetCount() ? NULL : items[n]); }
        PropItem *GetItemByName(const CString& name);

		// build up the tree
		PropItem *AddItem(PropItem *item);
	};

	//-------------------------------------------------------------------------
	//
	//	PropTreeEdit class
	//
	//-------------------------------------------------------------------------

	class PropTreeEdit : public CEdit
	{
	protected:
		HTREEITEM		htItem;
		PropItem		*item;					// item we're displaying
		CFont			*font;
		PropTreeCtrl	*parent;
		CBrush			brush_back;

		virtual BOOL PreTranslateMessage(MSG* pMsg);
		DECLARE_MESSAGE_MAP()

	public:
		PropTreeEdit(PropTreeCtrl *pParent, HTREEITEM htItem, PropItem *pitem, CFont *font);
		virtual ~PropTreeEdit();

		afx_msg void OnNcDestroy();
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		HBRUSH CtlColor(CDC *dc, UINT nCtlColor);
		void UpdatePos();
	};

	//-------------------------------------------------------------------------
	//
	//	PropTreeCtrl class
	//
	//-------------------------------------------------------------------------

	class PropTreeCtrl : public CTreeCtrl
	{
	protected:
		DECLARE_MESSAGE_MAP()
	public:

		PropertyTree		*parent;
		PropTreeEdit		*edit;
		CFont				*font_item;

	public:
		PropTreeCtrl();
		~PropTreeCtrl();

		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnSelChanged();
		void EditItem(HTREEITEM item);
		void CancelEdit();
		void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	};

	//-------------------------------------------------------------------------
	//
	//	PropertyTree class
	//
	//-------------------------------------------------------------------------

	class PropertyTree : public CStatic
	{
	protected:
		DECLARE_MESSAGE_MAP()

		virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

        bool dontDrawItem;

	public:
		DECLARE_DYNCREATE(PropertyTree)

		PropTreeCtrl		tree;

		enum {
			ID_TREE = 1
		};

		// rendering stuff
		CFont				font_group;
		CFont				font_item;

		DWORD				color_group;
		DWORD				color_item;
		DWORD				color_item_selected;
		DWORD				color_back_item;
		DWORD				color_back_item_selected;
		DWORD				color_back_group;

		int					left_offset;
		int					left_width;

	public:
		PropertyTree();
		~PropertyTree();

		virtual void Initialize();
		virtual void RepositionControls();

		void AdjustTextRect(CRect &rc);
		void OnSize(UINT nType, int cx, int cy);
		void OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

		BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
		void PaintItem(HTREEITEM item, UINT state, NMCUSTOMDRAW *draw);

		void BuildPropertyTree(PropItem *root);
		void BuildNode(PropItem *node, HTREEITEM item);
	};

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

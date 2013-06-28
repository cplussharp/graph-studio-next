//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	class BookmarkedItem
	{
	public:
		int			type;
		CString		name;				// filter name
	public:
		BookmarkedItem() : type(0), name(_T("")) { }
		virtual ~BookmarkedItem() { }
	};

	void SortBookmarkedItems(CArray<BookmarkedItem*> *ar);

	class BookmarkedGroup;

	//-------------------------------------------------------------------------
	//
	//	BookmarkedFilter class
	//
	//-------------------------------------------------------------------------
	class BookmarkedFilter : public BookmarkedItem
	{
	public:
		CString			moniker_name;
		HTREEITEM		item;				// helper
		BookmarkedGroup	*parent;			// parent group
	public:
		BookmarkedFilter();
		virtual ~BookmarkedFilter();

		// helpers
		void FromTemplate(DSUtil::FilterTemplate &ft);
	};

	//-------------------------------------------------------------------------
	//
	//	BookmarkedGroup class
	//
	//-------------------------------------------------------------------------
	class BookmarkedGroup : public BookmarkedItem
	{
	public:
		CArray<BookmarkedFilter*>	filters;
		HTREEITEM				item;
	public:
		BookmarkedGroup();
		virtual ~BookmarkedGroup();

		// removing bookmarked items
		HTREEITEM RemoveBookmark(DSUtil::FilterTemplate &ft);
		bool IsBookmarked(DSUtil::FilterTemplate &ft);
		bool ContainsMoniker(const CString& moniker_name);
		void RemoveFilter(BookmarkedFilter *filter);
	};

	//-------------------------------------------------------------------------
	//
	//	BookmarkedFilters
	//
	//-------------------------------------------------------------------------
	class BookmarkedFilters
	{
	public:
		CArray<BookmarkedGroup*>	groups;			// our groups
		CArray<BookmarkedFilter*>	filters;		// our filters
		CString						registry_name;

	public:
		BookmarkedFilters(const CString& reg_name);
		virtual ~BookmarkedFilters();


		// load/save from registry
		int Load();
		int Save();
		void Clear();

		// I/O on bookmarked filters
		int AddBookmark(DSUtil::FilterTemplate &ft);
		HTREEITEM RemoveBookmark(DSUtil::FilterTemplate &ft);
		bool IsBookmarked(DSUtil::FilterTemplate &ft);
		bool ContainsMoniker(const CString& moniker_display_name);
		void RemoveFilter(BookmarkedFilter *filter);

		BookmarkedGroup *AddGroup(CString name);
		void Sort();
	};

	//-------------------------------------------------------------------------
	//
	//	FavoritesTree
	//
	//-------------------------------------------------------------------------

	class FavoritesTree : public CTreeCtrl
	{
	protected:
		DECLARE_MESSAGE_MAP()

	protected:
		CImageList*	m_pDragImage;
		BOOL		m_bLDragging;
		HTREEITEM	m_hitemDrag,m_hitemDrop;

	public:
		FavoritesTree();
		~FavoritesTree();

		void OnBeginDrag(NMHDR *pNMHDR, LRESULT *pResult);
		void OnMouseMove(UINT nFlags, CPoint point);
		void OnLButtonUp(UINT nFlags, CPoint point);
	};

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation


//-----------------------------------------------------------------------------
//
//	CFavoritesForm class
//
//-----------------------------------------------------------------------------
class CFavoritesForm : public CGraphStudioModelessDialog
{
protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CFavoritesForm)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:

	GraphStudio::TitleBar		title;
	GraphStudio::FavoritesTree	tree;
	CImageList					image_list;
	CButton						btn_add_filters;
	HTREEITEM					menu_fired_item;

public:
	CFavoritesForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFavoritesForm();
	CRect GetDefaultRect() const;

	// Access to favorite filters singleton
	static GraphStudio::BookmarkedFilters * GetFavoriteFilters();

	// Access to blacklisted filters singleton TODO move somewhere better
	static GraphStudio::BookmarkedFilters * GetBlacklistedFilters();

	// Dialog Data
	enum { IDD = IDD_DIALOG_FAVORITES };

	BOOL DoCreateDialog(CWnd* parent);
	void OnInitialize();
	void OnSize(UINT nType, int cx, int cy);

	// update favorite filters
	void UpdateTree();
	void RemoveFilter(HTREEITEM item);

    static int FillMenu(CMenu* filters_menu, GraphStudio::BookmarkedFilters* favorites, int offset = 0);

	void OnNMRclickTreeFavorites(NMHDR *pNMHDR, LRESULT *pResult);
	void OnMenuCreategroup();
	void OnBeginDrag(NMHDR *pNMHDR, LRESULT *pResult);
	void OnMenuRemovegroup();
	void OnMenuRemovefilter();
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnTvnKeydownTreeFavorites(NMHDR *pNMHDR, LRESULT *pResult);
	void OnAddFilters();

};


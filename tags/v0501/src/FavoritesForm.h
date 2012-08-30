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

	class FavoriteItem
	{
	public:
		int			type;
		CString			name;				// filter name
	public:
		FavoriteItem() : type(0), name(_T("")) { }
		virtual ~FavoriteItem() { }
	};

	void SortFavoriteItems(CArray<FavoriteItem*> *ar);

	class FavoriteGroup;

	//-------------------------------------------------------------------------
	//
	//	FavoriteFilter class
	//
	//-------------------------------------------------------------------------
	class FavoriteFilter : public FavoriteItem
	{
	public:
		CString			moniker_name;
		HTREEITEM		item;				// helper
		FavoriteGroup	*parent;			// parent group
	public:
		FavoriteFilter();
		virtual ~FavoriteFilter();

		// helpers
		void FromTemplate(DSUtil::FilterTemplate &ft);
	};

	//-------------------------------------------------------------------------
	//
	//	FavoriteGroup class
	//
	//-------------------------------------------------------------------------
	class FavoriteGroup : public FavoriteItem
	{
	public:
		CArray<FavoriteFilter*>	filters;
		HTREEITEM				item;
	public:
		FavoriteGroup();
		virtual ~FavoriteGroup();

		// removing favorite filte
		HTREEITEM RemoveFavorite(DSUtil::FilterTemplate &ft);
		bool IsFavorite(DSUtil::FilterTemplate &ft);
		void RemoveFilter(FavoriteFilter *filter);
	};

	//-------------------------------------------------------------------------
	//
	//	Favorites
	//
	//-------------------------------------------------------------------------
	class Favorites
	{
	public:
		CArray<FavoriteGroup*>	groups;			// our groups
		CArray<FavoriteFilter*>	filters;		// our filters
	public:
		Favorites();
		virtual ~Favorites();
		static Favorites *GetInstance();

		// load/save from registry
		int Load();
		int Save();
		void Clear();

		// I/O on favorite filters
		int AddFavorite(DSUtil::FilterTemplate &ft);
		HTREEITEM RemoveFavorite(DSUtil::FilterTemplate &ft);
		bool IsFavorite(DSUtil::FilterTemplate &ft);
		void RemoveFilter(FavoriteFilter *filter);

		FavoriteGroup *AddGroup(CString name);
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

};


//-----------------------------------------------------------------------------
//
//	CFavoritesForm class
//
//-----------------------------------------------------------------------------
class CFavoritesForm : public CDialog
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
	CGraphView					*view;
	HTREEITEM					menu_fired_item;

public:
	CFavoritesForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFavoritesForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_FAVORITES };

	BOOL DoCreateDialog();
	void OnInitialize();
	void OnSize(UINT nType, int cx, int cy);

	// update favorite filters
	void UpdateTree();
	void UpdateFavoriteMenu();
	void RemoveFilter(HTREEITEM item);

    static int FillMenu(CMenu* filters_menu, GraphStudio::Favorites* favorites, int offset = 0);

	void OnNMRclickTreeFavorites(NMHDR *pNMHDR, LRESULT *pResult);
	void OnMenuCreategroup();
	void OnBeginDrag(NMHDR *pNMHDR, LRESULT *pResult);
	void OnMenuRemovegroup();
	void OnMenuRemovefilter();
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnTvnKeydownTreeFavorites(NMHDR *pNMHDR, LRESULT *pResult);
	void OnAddFilters();
};


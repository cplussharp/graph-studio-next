//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FavoritesForm.h"


//-----------------------------------------------------------------------------
//
//	CFavoritesForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CFavoritesForm, CGraphStudioModelessDialog)

BEGIN_MESSAGE_MAP(CFavoritesForm, CGraphStudioModelessDialog)
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_NOTIFY(NM_RCLICK, IDC_TREE_FAVORITES, &CFavoritesForm::OnNMRclickTreeFavorites)
	ON_COMMAND(ID_MENU_CREATEGROUP, &CFavoritesForm::OnMenuCreategroup)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_TREE_FAVORITES, &CFavoritesForm::OnBeginDrag)
	ON_COMMAND(ID_MENU_REMOVEGROUP, &CFavoritesForm::OnMenuRemovegroup)
	ON_COMMAND(ID_MENU_REMOVEFILTER, &CFavoritesForm::OnMenuRemovefilter)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_FAVORITES, &CFavoritesForm::OnTvnKeydownTreeFavorites)
	ON_COMMAND(ID_BUTTON_ADD_FILTERS, &CFavoritesForm::OnAddFilters)
	ON_COMMAND(IDC_BUTTON_INSERT, &CFavoritesForm::InsertSelectedFilter)
	ON_COMMAND(IDC_DELETE_SELECTED, &CFavoritesForm::DeleteSelected)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_FAVORITES, &CFavoritesForm::OnDblclkTreeFavorites)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFavoritesForm class
//
//-----------------------------------------------------------------------------

GraphStudio::BookmarkedFilters	g_favorites(_T("Favorites"));
GraphStudio::BookmarkedFilters	g_blacklisted(_T("Blacklisted"));

GraphStudio::BookmarkedFilters* CFavoritesForm::GetBlacklistedFilters()
{
	return &g_blacklisted;
}

GraphStudio::BookmarkedFilters* CFavoritesForm::GetFavoriteFilters()
{
	return &g_favorites;
}

CFavoritesForm::CFavoritesForm(CWnd* pParent) : 
	CGraphStudioModelessDialog(CFavoritesForm::IDD, pParent)
{
}

CFavoritesForm::~CFavoritesForm()
{
}

void CFavoritesForm::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_TREE_FAVORITES, tree);
}

BOOL CFavoritesForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	CBitmap	bmp;
	bmp.LoadBitmap(IDB_BITMAP_TREE);
	image_list.Create(17, 20, ILC_COLOR32, 1, 1);
	
	image_list.Add(&bmp, RGB(255,0,255));

	tree.SetImageList(&image_list, TVSIL_NORMAL);
	tree.SetImageList(&image_list, TVSIL_STATE);
	tree.SetItemHeight(20);

	CRect rect_title;
	title.GetWindowRect(rect_title);

	// create buttons
	const int button_height = 25, button_width = 90;
	const int border = (rect_title.Height() - button_height) / 2;

	CRect	rc(border, border, button_width, button_height);

	btn_add_filters.Create(_T("&Filters..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, ID_BUTTON_ADD_FILTERS);
	btn_add_filters.SetFont(GetFont());

	rc.OffsetRect(button_width + (2 * border), 0);

	btn_insert_filters.Create(_T("&Insert Filter"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_INSERT);
	btn_insert_filters.SetFont(GetFont());

	rc.OffsetRect(button_width + (2 * border), 0);

	btn_delete_selected.Create(_T("&Delete"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_DELETE_SELECTED);
	btn_delete_selected.SetFont(GetFont());

	OnInitialize();

	return TRUE;
};

CRect CFavoritesForm::GetDefaultRect() const 
{
	return CRect(50, 200, 450, 500);
}

void CFavoritesForm::OnInitialize()
{
	UpdateTree();
}

void CFavoritesForm::OnAddFilters()
{
	view->OnGraphInsertFilter();
}

void CFavoritesForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	// compute anchor lines
	int	right_x = rc.Width() - 320;
	int merit_combo_width = 180;

	if (IsWindow(title)) {

		title.GetClientRect(&rc2);
		title.SetWindowPos(NULL, 0, 0, cx, rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		tree.SetWindowPos(NULL, 0, rc2.Height(), cx, rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		// invalidate all controls
		title.Invalidate();

		title.GetClientRect(&rc2);
	}
}


static int CALLBACK FavoriteCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CTreeCtrl	*tree				= (CTreeCtrl*)lParamSort;
	GraphStudio::BookmarkedItem *i1	= (GraphStudio::BookmarkedItem *)lParam1;
	GraphStudio::BookmarkedItem *i2	= (GraphStudio::BookmarkedItem *)lParam2;

	if (i1->type != i2->type) {
		if (i2->type > i1->type) return 1; else return -1;
	}

	// both of the same type
	if (i1->type == 0) {

		GraphStudio::BookmarkedFilter	*f1 = (GraphStudio::BookmarkedFilter *)i1;
		GraphStudio::BookmarkedFilter	*f2 = (GraphStudio::BookmarkedFilter *)i2;

		CString	t1 = f1->name;	t1.MakeLower();
		CString	t2 = f2->name;	t2.MakeLower();

		if (t2 < t1) return 1; else return -1;

	} else
	if (i1->type == 1) {

		GraphStudio::BookmarkedGroup	*g1 = (GraphStudio::BookmarkedGroup *)i1;
		GraphStudio::BookmarkedGroup	*g2 = (GraphStudio::BookmarkedGroup *)i2;

		CString	t1 = g1->name;	t1.MakeLower();
		CString	t2 = g2->name;	t2.MakeLower();
		if (t2 < t1) return 1; else return -1;

	}

	return 0;
}

int CFavoritesForm::FillMenu(CMenu* filters_menu, GraphStudio::BookmarkedFilters* favorites, int offset)
{
	int		i, id;
	int		c=0;
		
	// add groups
	id = 1;
	for (i=0; i<favorites->groups.GetCount(); i++) {
		GraphStudio::BookmarkedGroup	*group = favorites->groups[i];

		filters_menu->InsertMenu(offset + c, MF_BYPOSITION | MF_STRING, 0, _T("&") + group->name);

		if (group->filters.GetCount() > 0) {
				
			// fill in audio renderers
			CMenu	group_menu;
			group_menu.CreatePopupMenu();
			for (int j=0; j<group->filters.GetCount(); j++) {
				GraphStudio::BookmarkedFilter	*filter = group->filters[j];
				group_menu.InsertMenu(j, MF_STRING, ID_FAVORITE_FILTER + id, _T("&") + filter->name);

				// associate the menu item with the gavorite filter
				MENUITEMINFO	info;
				memset(&info, 0, sizeof(info));
				info.cbSize = sizeof(info);
				info.fMask = MIIM_DATA;
				info.dwItemData = (ULONG_PTR)filter;
				group_menu.SetMenuItemInfo(j, &info, TRUE);
				id++;
			}

			filters_menu->ModifyMenu(offset + c, MF_BYPOSITION | MF_POPUP | MF_STRING, 
									(UINT_PTR)group_menu.m_hMenu, group->name);

			group_menu.Detach();

		} else {

			filters_menu->EnableMenuItem(offset + c, MF_BYPOSITION | MF_GRAYED);
		}
		c++;			
	}

	// add filters
	for (i=0; i<favorites->filters.GetCount(); i++) {
		filters_menu->InsertMenu(offset + c, MF_BYPOSITION | MF_STRING, ID_FAVORITE_FILTER + id, _T("&") + favorites->filters[i]->name);

		// associate the menu item with the favorite filter
		MENUITEMINFO	info;
		memset(&info, 0, sizeof(info));
		info.cbSize = sizeof(info);
		info.fMask = MIIM_DATA;
		info.dwItemData = (ULONG_PTR)favorites->filters[i];
		filters_menu->SetMenuItemInfo(offset + c, &info, TRUE);

		id++;
		c++;			
	}

    return c;
}

void CFavoritesForm::UpdateTree()
{
	GraphStudio::BookmarkedFilters	*favorites = CFavoritesForm::GetFavoriteFilters();
	if (!favorites) return ;

	int i, j;
	for (i=0; i<favorites->groups.GetCount(); i++) {
		GraphStudio::BookmarkedGroup	*group = favorites->groups[i];
		if (group->item == NULL) {
			group->item = tree.InsertItem(group->name, 1, 1, TVI_ROOT);
			tree.SetItemData(group->item, (DWORD_PTR)group);
		}
	
		// scan through filters
		for (j=0; j<group->filters.GetCount(); j++) {
			GraphStudio::BookmarkedFilter *filter = group->filters[j];
			if (filter->item == NULL) {
				filter->item = tree.InsertItem(filter->name, 0, 0, group->item);
				tree.SetItemData(filter->item, (DWORD_PTR)filter);
			}
		}
	}

	// scan through filters
	for (j=0; j<favorites->filters.GetCount(); j++) {
		GraphStudio::BookmarkedFilter *filter = favorites->filters[j];
		if (filter->item == NULL) {
			filter->item = tree.InsertItem(filter->name, 0, 0, TVI_ROOT);
			tree.SetItemData(filter->item, (DWORD_PTR)filter);
		}
	}

	TVSORTCB tvs;
	tvs.hParent		= TVI_ROOT;
	tvs.lpfnCompare = FavoriteCompare;
	tvs.lParam		= (LPARAM)&tree;
	tree.SortChildrenCB(&tvs);
}

void CFavoritesForm::RemoveFilter(HTREEITEM item)
{
	tree.DeleteItem(item);
}

void CFavoritesForm::OnMenuRemovegroup()
{
	if (!menu_fired_item) return ;

	// remove whole group
	GraphStudio::BookmarkedFilters		*favorites = CFavoritesForm::GetFavoriteFilters();
	GraphStudio::BookmarkedItem	*it = (GraphStudio::BookmarkedItem*)tree.GetItemData(menu_fired_item);
	if (it->type != 1) {
		ASSERT(FALSE);
	}

	int i;
	GraphStudio::BookmarkedGroup	*group = (GraphStudio::BookmarkedGroup*)it;
	for (i=0; i<group->filters.GetCount(); i++) {
		GraphStudio::BookmarkedFilter	*filter = group->filters[i];
		if (filter->item) {
			tree.DeleteItem(filter->item);
			filter->item = NULL;
		}
	}

	for (i=0; i<favorites->groups.GetCount(); i++) {
		if (favorites->groups[i] == group) {
			favorites->groups.RemoveAt(i);
			break;
		}
	}

	if (group->item) tree.DeleteItem(group->item);
	delete group;
	favorites->Save();
	UpdateTree();
}

void CFavoritesForm::OnMenuRemovefilter()
{
	if (!menu_fired_item) return ;

	GraphStudio::BookmarkedFilters		*favorites = CFavoritesForm::GetFavoriteFilters();
	GraphStudio::BookmarkedItem	*it = (GraphStudio::BookmarkedItem*)tree.GetItemData(menu_fired_item);
	if (it->type != 0) {
		ASSERT(FALSE);
	}

	GraphStudio::BookmarkedFilter	*filter = (GraphStudio::BookmarkedFilter*)it;

	// if it has no parent it's a top group filter
	if (filter->parent == NULL) {
		
		favorites->RemoveFilter(filter);
		tree.DeleteItem(menu_fired_item);

	} else {

		filter->parent->RemoveFilter(filter);
		tree.DeleteItem(menu_fired_item);

	}

	// delete instance
	delete filter;

	favorites->Save();
	UpdateTree();
}


void CFavoritesForm::OnNMRclickTreeFavorites(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	CPoint	ptScreen, ptClient;
	GetCursorPos(&ptScreen);

	ptClient = ptScreen;
	tree.ScreenToClient(&ptClient);

	UINT		flags = 0;
	HTREEITEM	item = tree.HitTest(ptClient, &flags);

	// create a menu
	CMenu		menu;		menu.LoadMenu(IDR_MENU_FAVORITES);
	CMenu		*popup;		popup = menu.GetSubMenu(0);

	// check if it is a group or not
	if (item) {
		GraphStudio::BookmarkedItem	*it = (GraphStudio::BookmarkedItem*)tree.GetItemData(item);
		if (!it) return ;

		if (it->type == 0) {
			// it's a filter
			popup->EnableMenuItem(ID_MENU_REMOVEGROUP, MF_BYCOMMAND | MF_GRAYED);
		} else {
			// it's a group
			popup->EnableMenuItem(ID_MENU_REMOVEFILTER, MF_BYCOMMAND | MF_GRAYED);
		}
	} else {
		popup->EnableMenuItem(ID_MENU_REMOVEGROUP, MF_BYCOMMAND | MF_GRAYED);
		popup->EnableMenuItem(ID_MENU_REMOVEFILTER, MF_BYCOMMAND | MF_GRAYED);
	}

	menu_fired_item = item;
	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, this, NULL);
}

void CFavoritesForm::OnMenuCreategroup()
{
	CNewGroupForm	newgroupdlg;
	INT_PTR ret = newgroupdlg.DoModal();
	if (ret == IDOK) {

		CString	new_name = newgroupdlg.text;

		// try to add the group
		GraphStudio::BookmarkedFilters		*favorites = CFavoritesForm::GetFavoriteFilters();
		GraphStudio::BookmarkedGroup	*group = favorites->AddGroup(new_name);
		if (!group) {
			// error, this name already exists
			MessageBeep(MB_ICONERROR);
			return ;
		}

		UpdateTree();

		// select the item
		if (group->item) {
			tree.SelectItem(group->item);
		}

		favorites->Save();
	}
}

void CFavoritesForm::OnBeginDrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	// just redirect the event
	tree.OnBeginDrag(pNMHDR, pResult);
}

void CFavoritesForm::OnTvnKeydownTreeFavorites(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	*pResult = 0;

	if (VK_DELETE == pTVKeyDown->wVKey) {
		OnKeyDown(pTVKeyDown->wVKey, 1, 0);
	}
}

void CFavoritesForm::OnDblclkTreeFavorites(NMHDR *pNMHDR, LRESULT *pResult)
{
	InsertSelectedFilter();
	*pResult = 0;
}
void CFavoritesForm::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_DELETE 
			&& !(GetKeyState(VK_SHIFT)		& 0x8000)
			&& !(GetKeyState(VK_CONTROL)	& 0x8000)
			&& !(GetKeyState(VK_MENU)		& 0x8000)) {
		DeleteSelected();
	}
}

void CFavoritesForm::OnOK()
{
	InsertSelectedFilter();
	// Don't call base class as this closes the dialog
}

void CFavoritesForm::DeleteSelected()
{
	HTREEITEM	item = tree.GetSelectedItem();
	if (item != NULL) {

		// delete this item
		GraphStudio::BookmarkedFilters		*favorites = CFavoritesForm::GetFavoriteFilters();
		GraphStudio::BookmarkedItem	*it = (GraphStudio::BookmarkedItem*)tree.GetItemData(item);
		if (it->type == 0) {
			// it's a filter
			GraphStudio::BookmarkedFilter	*filter = (GraphStudio::BookmarkedFilter*)it;

			// if it has no parent it's a top group filter
			if (filter->parent == NULL) {					
				favorites->RemoveFilter(filter);
				tree.DeleteItem(item);
			} else {
				filter->parent->RemoveFilter(filter);
				tree.DeleteItem(item);
			}

			// delete instance
			delete filter;

			favorites->Save();
			UpdateTree();

		} else 
		if (it->type == 1) {
			// it's a group
			int i;
			GraphStudio::BookmarkedGroup	*group = (GraphStudio::BookmarkedGroup*)it;
			for (i=0; i<group->filters.GetCount(); i++) {
				GraphStudio::BookmarkedFilter	*filter = group->filters[i];
				if (filter->item) {
					tree.DeleteItem(filter->item);
					filter->item = NULL;
				}
			}

			for (i=0; i<favorites->groups.GetCount(); i++) {
				if (favorites->groups[i] == group) {
					favorites->groups.RemoveAt(i);
					break;
				}
			}

			if (group->item) tree.DeleteItem(group->item);
			delete group;
			favorites->Save();
			UpdateTree();
		}

	}
}

void CFavoritesForm::InsertSelectedFilter()
{
	HTREEITEM	item = tree.GetSelectedItem();
	if (item && view) {
		// delete this item
		GraphStudio::BookmarkedFilters		*favorites = CFavoritesForm::GetFavoriteFilters();
		GraphStudio::BookmarkedItem	*it = (GraphStudio::BookmarkedItem*)tree.GetItemData(item);
		if (it->type == 0) {
			// it's a filter (not a group)
			GraphStudio::BookmarkedFilter	*filter = (GraphStudio::BookmarkedFilter*)it;
			view->InsertFilterFromFavorite(filter);
		}
	}
}


GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	int _FavoriteCompare(BookmarkedItem *f1, BookmarkedItem *f2)
	{
		CString s1 = f1->name; s1.MakeUpper();
		CString s2 = f2->name; s2.MakeUpper();
		return s1.Compare(s2);
	}

	void _FavoriteSwapItems(CArray<BookmarkedItem*> *ar, SSIZE_T i, SSIZE_T j)
	{	
		if (i == j) return ;
		BookmarkedItem	*temp = ar->GetAt(i);
		ar->SetAt(i, ar->GetAt(j));
		ar->SetAt(j, temp);
	}

	void _FavoriteSort(CArray<BookmarkedItem*> *ar, SSIZE_T lo, SSIZE_T hi)
	{
		SSIZE_T i = lo, j = hi;
		BookmarkedItem *m;

		// pivot
		m = ar->GetAt( (lo+hi)>>1 );

		do {
			while (_FavoriteCompare(m, ar->GetAt(i))>0) i++;
			while (_FavoriteCompare(ar->GetAt(j), m)>0) j--;

			if (i <= j) {
				_FavoriteSwapItems(ar, i, j);
				i++;
				j--;
			}
		} while (i <= j);

		if (j > lo) _FavoriteSort(ar, lo, j);
		if (i < hi) _FavoriteSort(ar, i, hi);
	}

	void SortBookmarkedItems(CArray<BookmarkedItem*> *ar)
	{
		if (ar->GetCount() == 0) return ;
		_FavoriteSort(ar, 0, ar->GetCount()-1);
	}

	//-------------------------------------------------------------------------
	//
	//	BookmarkedFilter class
	//
	//-------------------------------------------------------------------------

	BookmarkedFilter::BookmarkedFilter() :
		item(NULL),
		parent(NULL)
	{
		type = 0;
	}

	BookmarkedFilter::~BookmarkedFilter()
	{
		// nic
	}

	void BookmarkedFilter::FromTemplate(DSUtil::FilterTemplate &ft)
	{
		name = ft.name;
		moniker_name = ft.moniker_name;
	}

	//-------------------------------------------------------------------------
	//
	//	BookmarkedGroup class
	//
	//-------------------------------------------------------------------------

	BookmarkedGroup::BookmarkedGroup() :
		item(NULL)
	{
		type = 1;
	}

	BookmarkedGroup::~BookmarkedGroup()
	{
		for (int i=0; i<filters.GetCount(); i++) {
			BookmarkedFilter	*filter = filters[i];
			delete filter;
		}
		filters.RemoveAll();
	}

	bool BookmarkedGroup::IsBookmarked(const DSUtil::FilterTemplate &ft)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			BookmarkedFilter *f = filters[i];
			if (f->moniker_name == ft.moniker_name) return true;
		}
		return false;
	}

	bool BookmarkedGroup::ContainsMoniker(const CString& moniker_name)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i]->moniker_name == moniker_name) 
				return true;
		}
		return false;
	}

	void BookmarkedGroup::RemoveFilter(BookmarkedFilter *filter)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i] == filter) {
				filters.RemoveAt(i);
				return ;
			}
		}
	}

	HTREEITEM BookmarkedGroup::RemoveBookmark(DSUtil::FilterTemplate &ft)
	{
		HTREEITEM ret = NULL;
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i]->moniker_name == ft.moniker_name) {
				ret = filters[i]->item;
				filters.RemoveAt(i);
				return ret;
			}
		}
		// not found
		return ret;
	}

	//-------------------------------------------------------------------------
	//
	//	BookmarkedFilters
	//
	//-------------------------------------------------------------------------

	BookmarkedFilters::BookmarkedFilters(const CString& reg_name)
		: registry_name(reg_name)
	{
	}

	BookmarkedFilters::~BookmarkedFilters()
	{
		Clear();
	}

	void BookmarkedFilters::Clear()
	{
		int i;
		for (i=0; i<groups.GetCount(); i++) delete groups[i];
		for (i=0; i<filters.GetCount(); i++) delete filters[i];
		groups.RemoveAll();
		filters.RemoveAll();
	}

	#define REGISTRY_BASE_KEY "Software\\MONOGRAM\\MONOGRAM GraphStudio"

	int BookmarkedFilters::Load()
	{
		Clear();

		CString		keyname = CString(_T(REGISTRY_BASE_KEY)) + _T('\\') + registry_name;
		CRegKey		key;
		LONG		ok;

		// open the favorites key
		ok = key.Open(HKEY_CURRENT_USER, keyname);
		if (ok != ERROR_SUCCESS) return -1;

		// scan the groups and filters
		TCHAR		achClass[1024] = TEXT(""); 
		DWORD		cchClassName = 1024; 
		DWORD		cSubKeys=0;              
		DWORD		cbMaxSubKey;             
		DWORD		cchMaxClass;             
		DWORD		cValues=0;             
		DWORD		cchMaxValue;         
		DWORD		cbMaxValueData;      
		DWORD		cbSecurityDescriptor;
		FILETIME	ftLastWriteTime;     
		TCHAR		achValue[1024]; 
		DWORD		cchValue = 1024;  
		DWORD		i, retCode; 
 
		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(key, achClass, &cchClassName, NULL, 
								  &cSubKeys, &cbMaxSubKey, &cchMaxClass, 
								  &cValues, &cchMaxValue, &cbMaxValueData,
								  &cbSecurityDescriptor, &ftLastWriteTime); 

		// load groups
		for (DWORD j=0; j<cSubKeys; j++) {
			DWORD len = 1024;
			retCode = RegEnumKey(key, j, achValue, len);
			if (retCode != ERROR_SUCCESS) continue;

			// load filters in this group
			CString	path = keyname + _T("\\") + CString(achValue);
			CRegKey	key_group;
			retCode = key_group.Open(HKEY_CURRENT_USER, path);
			DWORD	this_SubKeys;
			if (retCode == ERROR_SUCCESS) {

				// Get the class name and the value count. 
				retCode = RegQueryInfoKey(key_group, achClass, &cchClassName, NULL, 
										  &this_SubKeys, &cbMaxSubKey, &cchMaxClass, 
										  &cValues, &cchMaxValue, &cbMaxValueData,
										  &cbSecurityDescriptor, &ftLastWriteTime); 

				// add new group instance
				BookmarkedGroup	*group = new BookmarkedGroup();
				group->name = CString(achValue);
				groups.Add(group);

				// load filters
				for (i=0; i<cValues; i++) {
					DWORD len = 1024;
					retCode = RegEnumValue(key_group, i, achValue, &len, NULL, NULL, NULL, NULL);
					if (retCode != ERROR_SUCCESS) continue;

					CString	value_name, value;
					TCHAR	val[1024];
					value_name = CString(achValue);

					if (value_name != _T("")) {
						len = 1024;
						retCode = key_group.QueryStringValue(value_name, val, &len);
						if (retCode == ERROR_SUCCESS) {

							BookmarkedFilter	*filter = new BookmarkedFilter();
							filter->name = CString(val);
							filter->moniker_name = CString(value_name);
							filter->item = NULL;		// will be added later by the form
							filter->parent = group;

							group->filters.Add(filter);
						}	
					}
				}

				key_group.Close();
			}
		}


		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(key, achClass, &cchClassName, NULL, 
								  &cSubKeys, &cbMaxSubKey, &cchMaxClass, 
								  &cValues, &cchMaxValue, &cbMaxValueData,
								  &cbSecurityDescriptor, &ftLastWriteTime); 

		// load filters
		for (i=0; i<cValues; i++) {
			DWORD len = 1024;
			retCode = RegEnumValue(key, i, achValue, &len, NULL, NULL, NULL, NULL);
			if (retCode != ERROR_SUCCESS) continue;

			CString	value_name, value;
			TCHAR	val[1024];
			value_name = CString(achValue);

			if (value_name != _T("")) {
				len = 1024;
				retCode = key.QueryStringValue(value_name, val, &len);
				if (retCode == ERROR_SUCCESS) {

					BookmarkedFilter	*filter = new BookmarkedFilter();
					filter->name = CString(val);
					filter->moniker_name = CString(value_name);
					filter->item = NULL;		// will be added later by the form

					filters.Add(filter);
				}	
			}
		}

		return 0;
	}

	int BookmarkedFilters::Save()
	{
		CString		keyname = _T(REGISTRY_BASE_KEY);
		CRegKey		key;
		LONG		ok;
		int			i;

		// we delete all we had written before...
		ok = key.Create(HKEY_CURRENT_USER, keyname);
		if (ok != ERROR_SUCCESS) return -1;

		key.RecurseDeleteKey(registry_name);
		key.Close();

		keyname += CString(_T('\\')) + registry_name;
		ok = key.Create(HKEY_CURRENT_USER, keyname);
		if (ok != ERROR_SUCCESS) return -1;

		// now save the current state

		// iterate through groups
		for (i=0; i<groups.GetCount(); i++) {
			BookmarkedGroup	*group = groups[i];
			CString	groupname = keyname + _T("\\") + registry_name + group->name;

			CRegKey	groupkey;
			ok = groupkey.Create(key, group->name);
			if (ok == ERROR_SUCCESS) {

				// now write filters
				for (int j=0; j<group->filters.GetCount(); j++) {
					BookmarkedFilter	*filter = group->filters[j];
					groupkey.SetStringValue(filter->moniker_name, filter->name);
				}

				groupkey.Close();
			}
		}

		// iterate through filters
		for (int j=0; j<filters.GetCount(); j++) {
			BookmarkedFilter	*filter = filters[j];
			key.SetStringValue(filter->moniker_name, filter->name);
		}

		key.Close();
		return 0;
	}

	// I/O on favorite filters
	int BookmarkedFilters::AddBookmark(DSUtil::FilterTemplate &ft)
	{
		// we already have this one
		if (IsBookmarked(ft)) 
			return 0;

		// add a new filter
		BookmarkedFilter	*filt = new BookmarkedFilter();
		filt->FromTemplate(ft);
		filt->parent = NULL;
		filters.Add(filt);

		return 0;
	}

	void BookmarkedFilters::RemoveFilter(BookmarkedFilter *filter)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i] == filter) {
				filters.RemoveAt(i);
				return ;
			}
		}
	}

	HTREEITEM BookmarkedFilters::RemoveBookmark(DSUtil::FilterTemplate &ft)
	{
		int i;

		// search in groups
		for (i=0; i<groups.GetCount(); i++) {
			HTREEITEM ret = groups[i]->RemoveBookmark(ft);
			if (ret != NULL) return ret;
		}

		for (i=0; i<filters.GetCount(); i++) {
			if (filters[i]->moniker_name == ft.moniker_name) {
				BookmarkedFilter	*filter = filters[i];
				filters.RemoveAt(i);
				HTREEITEM ret = filter->item;
				delete filter;
				return ret;
			}
		}

		// we don't have this one
		return NULL;
	}

	bool BookmarkedFilters::IsBookmarked(const DSUtil::FilterTemplate &ft)
	{
		int i;
		for (i=0; i<groups.GetCount(); i++) {
			bool ok = groups[i]->IsBookmarked(ft);
			if (ok) return true;
		}

		for (i=0; i<filters.GetCount(); i++) {
			if (filters[i]->moniker_name == ft.moniker_name) return true;
		}
		return false;
	}

	bool BookmarkedFilters::ContainsMoniker(const CString & moniker_name)
	{
		int i;
		for (i=0; i<groups.GetCount(); i++) {
			if (groups[i]->ContainsMoniker(moniker_name))
				return true;
		}

		for (i=0; i<filters.GetCount(); i++) {
			if (filters[i]->moniker_name == moniker_name) 
				return true;
		}
		return false;
	}

	BookmarkedGroup *BookmarkedFilters::AddGroup(CString name)
	{
		int i;
		for (i=0; i<groups.GetCount(); i++) {
			BookmarkedGroup *g = groups[i];
			if (g->name == name) return NULL;
		}

		BookmarkedGroup *g = new BookmarkedGroup();
		g->name = name;
		g->item = NULL;
		groups.Add(g);
		return g;
	}

	void BookmarkedFilters::Sort()
	{
		// have the items sorted...
		SortBookmarkedItems((CArray<BookmarkedItem*>*)&filters);
		SortBookmarkedItems((CArray<BookmarkedItem*>*)&groups);

		for (int i=0; i<groups.GetCount(); i++) {
			SortBookmarkedItems((CArray<BookmarkedItem*>*)&groups[i]->filters);
		}
	}


	//-------------------------------------------------------------------------
	//
	//	FavoritesTree
	//
	//-------------------------------------------------------------------------

	BEGIN_MESSAGE_MAP(FavoritesTree, CTreeCtrl)
		ON_WM_MOUSEMOVE()
		ON_WM_LBUTTONUP()
	END_MESSAGE_MAP()

	FavoritesTree::FavoritesTree()
	{
		m_bLDragging = FALSE;
		m_pDragImage = NULL;
	}

	FavoritesTree::~FavoritesTree()
	{
	}

	void FavoritesTree::OnBeginDrag(NMHDR *pNMHDR, LRESULT *pResult)
	{
		NM_TREEVIEW* pTreeView = (NM_TREEVIEW*)pNMHDR;
		*pResult = 0;
	    
		BookmarkedItem	*item = (BookmarkedItem*)GetItemData(pTreeView->itemNew.hItem);
		if (!item) return ;
		if (item->type == 1) return ;			// we don't allow dragging of groups

		// Item user started dragging ...
		m_hitemDrag = pTreeView->itemNew.hItem;
		m_hitemDrop = NULL;

		// get the image list for dragging
		m_pDragImage = CreateDragImage(m_hitemDrag);  
		if (!m_pDragImage)	return;
		m_bLDragging = TRUE;

		m_pDragImage->BeginDrag(0, CPoint(-15,-15));

		POINT pt = pTreeView->ptDrag;
		ClientToScreen(&pt);
		m_pDragImage->DragEnter(NULL, pt);

		SetCapture();
	}

	void FavoritesTree::OnMouseMove(UINT nFlags, CPoint point) 
	{
		HTREEITEM	hitem;
		UINT		flags = 0;

		if (m_bLDragging) {
			POINT pt = point;
			ClientToScreen(&pt);

			CImageList::DragMove(pt);
			if ((hitem = HitTest(point, &flags)) != NULL) {
				CImageList::DragShowNolock(FALSE);
				SelectDropTarget(hitem);
				m_hitemDrop = hitem;
				CImageList::DragShowNolock(TRUE);
			} else {
				// drop in to the root group
				m_hitemDrop = NULL;
			}
		}

		CTreeCtrl::OnMouseMove(nFlags, point);
	}

	void FavoritesTree::OnLButtonUp(UINT nFlags, CPoint point) 
	{
		CTreeCtrl::OnLButtonUp(nFlags, point);

		if (m_bLDragging) {
			m_bLDragging = FALSE;
			CImageList::DragLeave(this);
			CImageList::EndDrag();
			ReleaseCapture();

			delete m_pDragImage;

			// Remove drop target highlighting
			SelectDropTarget(NULL);
			if (m_hitemDrag == m_hitemDrop) return;

			// find the group where the filter was dropped
			BookmarkedFilters		*favorites = CFavoritesForm::GetFavoriteFilters();
			BookmarkedFilter	*filter = (BookmarkedFilter*)GetItemData(m_hitemDrag);
			ASSERT(filter && filter->type == 0);


			//-----------------------------------------------------------------
			//	Remove filter from old position
			//-----------------------------------------------------------------
			if (filter->parent) {
				filter->parent->RemoveFilter(filter);
				filter->parent = NULL;
			} else {
				favorites->RemoveFilter(filter);
				filter->parent = NULL;
			}

			// remove tree node
			if (filter->item) {
				DeleteItem(filter->item);
				filter->item = NULL;
			}
			filter->item = NULL;


			//-----------------------------------------------------------------
			//	Find a new position for it
			//-----------------------------------------------------------------
			if (!m_hitemDrop) {

				// topmost group
				favorites->filters.Add(filter);

			} else {
				BookmarkedItem	*drop_item = (BookmarkedItem*)GetItemData(m_hitemDrop);
				if (drop_item->type == 1) {

					// dropped into a group
					BookmarkedGroup *group = (BookmarkedGroup *)drop_item;
					filter->parent = group;
					group->filters.Add(filter);

				} else {

					// dropped into a filter - check if it has a parent
					HTREEITEM		parent = GetParentItem(m_hitemDrop);
					if (parent == NULL) {

						// topmost group
						favorites->filters.Add(filter);

					} else {

						// custom group
						BookmarkedGroup	*group = (BookmarkedGroup*)GetItemData(parent);
						filter->parent = group;
						group->filters.Add(filter);

					}

				}
			}

			// update the tree
			CFavoritesForm *parent_form = (CFavoritesForm*)GetParent();
			parent_form->UpdateTree();
			favorites->Save();
		}

	}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

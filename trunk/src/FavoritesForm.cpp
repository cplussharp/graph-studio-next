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
IMPLEMENT_DYNAMIC(CFavoritesForm, CDialog)

BEGIN_MESSAGE_MAP(CFavoritesForm, CDialog)
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_NOTIFY(NM_RCLICK, IDC_TREE_FAVORITES, &CFavoritesForm::OnNMRclickTreeFavorites)
	ON_COMMAND(ID_MENU_CREATEGROUP, &CFavoritesForm::OnMenuCreategroup)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_TREE_FAVORITES, &CFavoritesForm::OnBeginDrag)
	ON_COMMAND(ID_MENU_REMOVEGROUP, &CFavoritesForm::OnMenuRemovegroup)
	ON_COMMAND(ID_MENU_REMOVEFILTER, &CFavoritesForm::OnMenuRemovefilter)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_FAVORITES, &CFavoritesForm::OnTvnKeydownTreeFavorites)
	ON_COMMAND(ID_BUTTON_ADD_FILTERS, &CFavoritesForm::OnAddFilters)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFavoritesForm class
//
//-----------------------------------------------------------------------------

CFavoritesForm::CFavoritesForm(CWnd* pParent) : 
	CDialog(CFavoritesForm::IDD, pParent)
{

}

CFavoritesForm::~CFavoritesForm()
{
}

void CFavoritesForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_TREE_FAVORITES, tree);
}

BOOL CFavoritesForm::DoCreateDialog()
{
	BOOL ret = Create(IDD);
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

	// create buttons
	CRect	rc;
	rc.SetRect(0, 0, 80, 25);
	btn_add_filters.Create(_T("Add Filter(s)"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, ID_BUTTON_ADD_FILTERS);
	btn_add_filters.SetFont(GetFont());

	OnInitialize();

	SetWindowPos(NULL, 0, 0, 400, 300, SWP_NOMOVE);
	return TRUE;
};

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

		btn_add_filters.SetWindowPos(NULL, cx - 1*(80+6), 4, 80, 25, SWP_SHOWWINDOW | SWP_NOZORDER);

		tree.SetWindowPos(NULL, 0, rc2.Height(), cx, rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		// invalidate all controls
		title.Invalidate();

		title.GetClientRect(&rc2);

		btn_add_filters.Invalidate();

	}
}


static int CALLBACK FavoriteCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CTreeCtrl	*tree				= (CTreeCtrl*)lParamSort;
	GraphStudio::FavoriteItem *i1	= (GraphStudio::FavoriteItem *)lParam1;
	GraphStudio::FavoriteItem *i2	= (GraphStudio::FavoriteItem *)lParam2;

	if (i1->type != i2->type) {
		if (i2->type > i1->type) return 1; else return -1;
	}

	// both of the same type
	if (i1->type == 0) {

		GraphStudio::FavoriteFilter	*f1 = (GraphStudio::FavoriteFilter *)i1;
		GraphStudio::FavoriteFilter	*f2 = (GraphStudio::FavoriteFilter *)i2;

		CString	t1 = f1->name;	t1.MakeLower();
		CString	t2 = f2->name;	t2.MakeLower();

		if (t2 < t1) return 1; else return -1;

	} else
	if (i1->type == 1) {

		GraphStudio::FavoriteGroup	*g1 = (GraphStudio::FavoriteGroup *)i1;
		GraphStudio::FavoriteGroup	*g2 = (GraphStudio::FavoriteGroup *)i2;

		CString	t1 = g1->name;	t1.MakeLower();
		CString	t2 = g2->name;	t2.MakeLower();
		if (t2 < t1) return 1; else return -1;

	}

	return 0;
}

void CFavoritesForm::UpdateFavoriteMenu()
{
	// first erase all added items
	CMenu	*main_menu		= view->GetParentFrame()->GetMenu();
	CMenu	*filters_menu	= main_menu->GetSubMenu(4);

	// remove all
	while (filters_menu->GetMenuItemCount() > 4) {
		filters_menu->DeleteMenu(filters_menu->GetMenuItemCount() - 3, MF_BYPOSITION);
	}

	GraphStudio::Favorites	*favorites = GraphStudio::Favorites::GetInstance();
	favorites->Sort();

	int	cnt = favorites->filters.GetCount() + favorites->groups.GetCount();

	if (cnt > 0) {
		int	offset = filters_menu->GetMenuItemCount() - 2;
		int c = FillMenu(filters_menu, favorites, offset);

		// separator at the end
		filters_menu->InsertMenu(offset + c, MF_BYPOSITION | MF_SEPARATOR, 0);
	}
}

int CFavoritesForm::FillMenu(CMenu* filters_menu, GraphStudio::Favorites* favorites, int offset)
{
	int		i, id;
	int		c=0;
		
	// add groups
	id = 1;
	for (i=0; i<favorites->groups.GetCount(); i++) {
		GraphStudio::FavoriteGroup	*group = favorites->groups[i];

		filters_menu->InsertMenu(offset + c, MF_BYPOSITION | MF_STRING, 0, _T("&") + group->name);

		if (group->filters.GetCount() > 0) {
				
			// fill in audio renderers
			CMenu	group_menu;
			group_menu.CreatePopupMenu();
			for (int j=0; j<group->filters.GetCount(); j++) {
				GraphStudio::FavoriteFilter	*filter = group->filters[j];
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
	GraphStudio::Favorites	*favorites = GraphStudio::Favorites::GetInstance();
	if (!favorites) return ;

	int i, j;
	for (i=0; i<favorites->groups.GetCount(); i++) {
		GraphStudio::FavoriteGroup	*group = favorites->groups[i];
		if (group->item == NULL) {
			group->item = tree.InsertItem(group->name, 1, 1, TVI_ROOT);
			tree.SetItemData(group->item, (DWORD_PTR)group);
		}
	
		// scan through filters
		for (j=0; j<group->filters.GetCount(); j++) {
			GraphStudio::FavoriteFilter *filter = group->filters[j];
			if (filter->item == NULL) {
				filter->item = tree.InsertItem(filter->name, 0, 0, group->item);
				tree.SetItemData(filter->item, (DWORD_PTR)filter);
			}
		}
	}

	// scan through filters
	for (j=0; j<favorites->filters.GetCount(); j++) {
		GraphStudio::FavoriteFilter *filter = favorites->filters[j];
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

	UpdateFavoriteMenu();
}

void CFavoritesForm::RemoveFilter(HTREEITEM item)
{
	tree.DeleteItem(item);
}

void CFavoritesForm::OnMenuRemovegroup()
{
	if (!menu_fired_item) return ;

	// remove whole group
	GraphStudio::Favorites		*favorites = GraphStudio::Favorites::GetInstance();
	GraphStudio::FavoriteItem	*it = (GraphStudio::FavoriteItem*)tree.GetItemData(menu_fired_item);
	if (it->type != 1) {
		ASSERT(FALSE);
	}

	int i;
	GraphStudio::FavoriteGroup	*group = (GraphStudio::FavoriteGroup*)it;
	for (i=0; i<group->filters.GetCount(); i++) {
		GraphStudio::FavoriteFilter	*filter = group->filters[i];
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

	GraphStudio::Favorites		*favorites = GraphStudio::Favorites::GetInstance();
	GraphStudio::FavoriteItem	*it = (GraphStudio::FavoriteItem*)tree.GetItemData(menu_fired_item);
	if (it->type != 0) {
		ASSERT(FALSE);
	}

	GraphStudio::FavoriteFilter	*filter = (GraphStudio::FavoriteFilter*)it;

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

	UINT		flags;
	HTREEITEM	item = tree.HitTest(ptClient, &flags);

	// create a menu
	CMenu		menu;		menu.LoadMenu(IDR_MENU_FAVORITES);
	CMenu		*popup;		popup = menu.GetSubMenu(0);

	// check if it is a group or not
	if (item) {
		GraphStudio::FavoriteItem	*it = (GraphStudio::FavoriteItem*)tree.GetItemData(item);
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
	int ret = newgroupdlg.DoModal();
	if (ret == IDOK) {

		CString	new_name = newgroupdlg.text;

		// try to add the group
		GraphStudio::Favorites		*favorites = GraphStudio::Favorites::GetInstance();
		GraphStudio::FavoriteGroup	*group = favorites->AddGroup(new_name);
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

	if (pTVKeyDown->wVKey == VK_DELETE) {
		OnKeyDown(VK_DELETE, 1, 0);
	}
}

void CFavoritesForm::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// handle DEL key
	if (nChar == VK_DELETE) {
	
		HTREEITEM	item = tree.GetSelectedItem();
		if (item != NULL) {

			// delete this item
			GraphStudio::Favorites		*favorites = GraphStudio::Favorites::GetInstance();
			GraphStudio::FavoriteItem	*it = (GraphStudio::FavoriteItem*)tree.GetItemData(item);
			if (it->type == 0) {
				// it's a filter
				GraphStudio::FavoriteFilter	*filter = (GraphStudio::FavoriteFilter*)it;

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
				GraphStudio::FavoriteGroup	*group = (GraphStudio::FavoriteGroup*)it;
				for (i=0; i<group->filters.GetCount(); i++) {
					GraphStudio::FavoriteFilter	*filter = group->filters[i];
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
}


namespace GraphStudio
{

	int _FavoriteCompare(FavoriteItem *f1, FavoriteItem *f2)
	{
		CString s1 = f1->name; s1.MakeUpper();
		CString s2 = f2->name; s2.MakeUpper();
		return s1.Compare(s2);
	}

	void _FavoriteSwapItems(CArray<FavoriteItem*> *ar, int i, int j)
	{	
		if (i == j) return ;
		FavoriteItem	*temp = ar->GetAt(i);
		ar->SetAt(i, ar->GetAt(j));
		ar->SetAt(j, temp);
	}

	void _FavoriteSort(CArray<FavoriteItem*> *ar, int lo, int hi)
	{
		int i = lo, j = hi;
		FavoriteItem *m;

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

	void SortFavoriteItems(CArray<FavoriteItem*> *ar)
	{
		if (ar->GetCount() == 0) return ;
		_FavoriteSort(ar, 0, ar->GetCount()-1);
	}

	//-------------------------------------------------------------------------
	//
	//	FavoriteFilter class
	//
	//-------------------------------------------------------------------------

	FavoriteFilter::FavoriteFilter() :
		moniker_name(_T("")),
		item(NULL),
		parent(NULL)
	{
		type = 0;
	}

	FavoriteFilter::~FavoriteFilter()
	{
		// nic
	}

	void FavoriteFilter::FromTemplate(DSUtil::FilterTemplate &ft)
	{
		name = ft.name;
		moniker_name = ft.moniker_name;
	}

	//-------------------------------------------------------------------------
	//
	//	FavoriteGroup class
	//
	//-------------------------------------------------------------------------

	FavoriteGroup::FavoriteGroup() :
		item(NULL)
	{
		type = 1;
	}

	FavoriteGroup::~FavoriteGroup()
	{
		for (int i=0; i<filters.GetCount(); i++) {
			FavoriteFilter	*filter = filters[i];
			delete filter;
		}
		filters.RemoveAll();
	}

	bool FavoriteGroup::IsFavorite(DSUtil::FilterTemplate &ft)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			FavoriteFilter *f = filters[i];
			if (f->moniker_name == ft.moniker_name) return true;
		}
		return false;
	}

	void FavoriteGroup::RemoveFilter(FavoriteFilter *filter)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i] == filter) {
				filters.RemoveAt(i);
				return ;
			}
		}
	}

	HTREEITEM FavoriteGroup::RemoveFavorite(DSUtil::FilterTemplate &ft)
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
	//	Favorites
	//
	//-------------------------------------------------------------------------

	Favorites		g_favorites;

	Favorites::Favorites()
	{
	}

	Favorites::~Favorites()
	{
		Clear();
	}

	Favorites *Favorites::GetInstance()
	{
		return &g_favorites;
	}

	void Favorites::Clear()
	{
		int i;
		for (i=0; i<groups.GetCount(); i++) delete groups[i];
		for (i=0; i<filters.GetCount(); i++) delete filters[i];
		groups.RemoveAll();
		filters.RemoveAll();
	}

	int Favorites::Load()
	{
		Clear();

		CString		keyname = _T("Software\\MONOGRAM\\MONOGRAM GraphStudio\\Favorites");
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
				FavoriteGroup	*group = new FavoriteGroup();
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

							FavoriteFilter	*filter = new FavoriteFilter();
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

					FavoriteFilter	*filter = new FavoriteFilter();
					filter->name = CString(val);
					filter->moniker_name = CString(value_name);
					filter->item = NULL;		// will be added later by the form

					filters.Add(filter);
				}	
			}
		}

		return 0;
	}

	int Favorites::Save()
	{
		CString		keyname = _T("Software\\MONOGRAM\\MONOGRAM GraphStudio");
		CRegKey		key;
		LONG		ok;
		int			i;

		// we delete all we had written before...
		ok = key.Create(HKEY_CURRENT_USER, keyname);
		if (ok != ERROR_SUCCESS) return -1;

		key.RecurseDeleteKey(_T("Favorites"));
		key.Close();

		keyname += _T("\\Favorites");
		ok = key.Create(HKEY_CURRENT_USER, keyname);
		if (ok != ERROR_SUCCESS) return -1;

		// now save the current state

		// iterate through groups
		for (i=0; i<groups.GetCount(); i++) {
			FavoriteGroup	*group = groups[i];
			CString	groupname = keyname + _T("\\Favorites") + group->name;

			CRegKey	groupkey;
			ok = groupkey.Create(key, group->name);
			if (ok == ERROR_SUCCESS) {

				// now write filters
				for (int j=0; j<group->filters.GetCount(); j++) {
					FavoriteFilter	*filter = group->filters[j];
					groupkey.SetStringValue(filter->moniker_name, filter->name);
				}

				groupkey.Close();
			}
		}

		// iterate through filters
		for (int j=0; j<filters.GetCount(); j++) {
			FavoriteFilter	*filter = filters[j];
			key.SetStringValue(filter->moniker_name, filter->name);
		}

		key.Close();
		return 0;
	}

	// I/O on favorite filters
	int Favorites::AddFavorite(DSUtil::FilterTemplate &ft)
	{
		// we already have this one
		if (IsFavorite(ft)) return 0;

		// add a new filter
		FavoriteFilter	*filt = new FavoriteFilter();
		filt->FromTemplate(ft);
		filt->parent = NULL;
		filters.Add(filt);

		return 0;
	}

	void Favorites::RemoveFilter(FavoriteFilter *filter)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i] == filter) {
				filters.RemoveAt(i);
				return ;
			}
		}
	}

	HTREEITEM Favorites::RemoveFavorite(DSUtil::FilterTemplate &ft)
	{
		int i;

		// search in groups
		for (i=0; i<groups.GetCount(); i++) {
			HTREEITEM ret = groups[i]->RemoveFavorite(ft);
			if (ret != NULL) return ret;
		}

		for (i=0; i<filters.GetCount(); i++) {
			if (filters[i]->moniker_name == ft.moniker_name) {
				FavoriteFilter	*filter = filters[i];
				filters.RemoveAt(i);
				HTREEITEM ret = filter->item;
				delete filter;
				return ret;
			}
		}

		// we don't have this one
		return NULL;
	}

	bool Favorites::IsFavorite(DSUtil::FilterTemplate &ft)
	{
		int i;
		for (i=0; i<groups.GetCount(); i++) {
			bool ok = groups[i]->IsFavorite(ft);
			if (ok) return true;
		}

		for (i=0; i<filters.GetCount(); i++) {
			if (filters[i]->moniker_name == ft.moniker_name) return true;
		}
		return false;
	}

	FavoriteGroup *Favorites::AddGroup(CString name)
	{
		int i;
		for (i=0; i<groups.GetCount(); i++) {
			FavoriteGroup *g = groups[i];
			if (g->name == name) return NULL;
		}

		FavoriteGroup *g = new FavoriteGroup();
		g->name = name;
		g->item = NULL;
		groups.Add(g);
		return g;
	}

	void Favorites::Sort()
	{
		// have the items sorted...
		SortFavoriteItems((CArray<FavoriteItem*>*)&filters);
		SortFavoriteItems((CArray<FavoriteItem*>*)&groups);

		for (int i=0; i<groups.GetCount(); i++) {
			SortFavoriteItems((CArray<FavoriteItem*>*)&groups[i]->filters);
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
	    
		FavoriteItem	*item = (FavoriteItem*)GetItemData(pTreeView->itemNew.hItem);
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
		UINT		flags;

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
			Favorites		*favorites = Favorites::GetInstance();
			FavoriteFilter	*filter = (FavoriteFilter*)GetItemData(m_hitemDrag);
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
				FavoriteItem	*drop_item = (FavoriteItem*)GetItemData(m_hitemDrop);
				if (drop_item->type == 1) {

					// dropped into a group
					FavoriteGroup *group = (FavoriteGroup *)drop_item;
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
						FavoriteGroup	*group = (FavoriteGroup*)GetItemData(parent);
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
};


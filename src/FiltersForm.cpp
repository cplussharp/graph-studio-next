//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FiltersForm.h"


typedef HRESULT (_stdcall *DllUnregisterServerProc)(); 
typedef HRESULT (_stdcall *DllRegisterServerProc)(); 


//-----------------------------------------------------------------------------
//
//	CFiltersForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CFiltersForm, CGraphStudioModelessDialog)

BEGIN_MESSAGE_MAP(CFiltersForm, CGraphStudioModelessDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORIES, &CFiltersForm::OnComboCategoriesChange)
	ON_WM_SIZE()
	ON_WM_MEASUREITEM()
	ON_BN_CLICKED(IDC_BUTTON_INSERT, &CFiltersForm::OnBnClickedButtonInsert)
	ON_CBN_SELCHANGE(IDC_COMBO_MERIT, &CFiltersForm::OnComboMeritChange)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FILTERS, &CFiltersForm::OnFilterItemClick)
	ON_EN_UPDATE(IDC_SEARCH_STRING, &CFiltersForm::OnEnUpdateSearchString)
	ON_BN_CLICKED(IDC_BUTTON_PROPERTYPAGE, &CFiltersForm::OnBnClickedButtonPropertypage)
	ON_BN_CLICKED(IDC_CHECK_FAVORITE, &CFiltersForm::OnBnClickedCheckFavorite)
	ON_BN_CLICKED(IDC_BUTTON_LOCATE, &CFiltersForm::OnLocateClick)
	ON_BN_CLICKED(IDC_BUTTON_UNREGISTER, &CFiltersForm::OnUnregisterClick)
    ON_BN_CLICKED(IDC_BUTTON_REGISTER, &CFiltersForm::OnRegisterClick)
	ON_BN_CLICKED(IDC_BUTTON_MERIT, &CFiltersForm::OnMeritClick)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_BLACKLIST, &CFiltersForm::OnBnClickedCheckBlacklist)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFiltersForm class
//
//-----------------------------------------------------------------------------

DSUtil::FilterTemplates		CFiltersForm::cached_templates;

const DSUtil::FilterCategories & CFiltersForm::GetFilterCategories()
{
	static DSUtil::FilterCategories theCategories;		// singleton wrapped in getter, constructed on demand
	return theCategories;
}

CFiltersForm::CFiltersForm(CWnd* pParent) : 
	CGraphStudioModelessDialog(CFiltersForm::IDD, pParent),
	info(CString(_T("root"))), m_pToolTip(NULL)
{

}

CFiltersForm::~CFiltersForm()
{
    if(m_pToolTip)
        delete m_pToolTip;
}

void CFiltersForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
    DDX_Control(pDX, IDC_LIST_FILTERS, list_filters);
    DDX_Control(pDX, IDC_BUTTON_INSERT, btn_insert);
    DDX_Control(pDX, IDC_BUTTON_LOCATE, btn_locate);
    DDX_Control(pDX, IDC_BUTTON_MERIT, btn_merit);
    DDX_Control(pDX, IDC_BUTTON_PROPERTYPAGE, btn_propertypage);
    DDX_Control(pDX, IDC_BUTTON_UNREGISTER, btn_unregister);
    DDX_Control(pDX, IDC_CHECK_FAVORITE, check_favorite);
    DDX_Control(pDX, IDC_CHECK_BLACKLIST, check_blacklist);
    DDX_Control(pDX, IDC_MFCLINK_SEARCH_ONLINE, m_search_online);
}

BOOL CFiltersForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	// Create combos
	const CRect	combo_rect(0, 0, 100, 300);
	combo_categories.Create(WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_SORT | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, combo_rect, &title, IDC_COMBO_CATEGORIES);
	combo_merit.Create(WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL, combo_rect, &title, IDC_COMBO_MERIT);

	// create buttons
	const CRect	button_rect(0, 0, 100, 23);
	btn_register.Create(_T("&Register"), WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, button_rect, &title, IDC_BUTTON_REGISTER);

	tree_details.left_width = 110;		// Set left width to allow more room
	tree_details.Create(_T("Filter &Details"), WS_TABSTOP | WS_CHILD | WS_VISIBLE, button_rect, this, IDC_TREE);

	CRect edit_rect(0,0,16,16);		// recommended edit control height is 14 but add a bit as 14 looks cramped
	::MapDialogRect(title.m_hWnd, &edit_rect);
	edit_search.Create(WS_BORDER | WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LOWERCASE, edit_rect, &title, IDC_SEARCH_STRING);

	// change z order put tree_details directly after filter list in tab order
	tree_details.SetWindowPos(&list_filters, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	CFont * const current_font = GetFont();
	combo_categories.SetFont(current_font);
	combo_merit.SetFont(current_font);
	btn_register.SetFont(current_font);
	edit_search.SetFont(current_font);

	tree_details.Initialize();

	OnInitialize();
	return TRUE;
};

CRect CFiltersForm::GetDefaultRect() const 
{
	return CRect(50, 200, 750, 650);
}

// CFiltersForm message handlers
void CFiltersForm::OnInitialize()
{
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ADDFILTER);

    SetIcon(m_hIcon, TRUE);

	// Fill the categories combo
	DragAcceptFiles(TRUE);

	list_filters.callback = this;

	// add some columns
	CRect	rc;
	list_filters.GetClientRect(&rc);

	DWORD	style = list_filters.GetExtendedStyle() | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP;
	list_filters.SetExtendedStyle(style);
	list_filters.InsertColumn(0, _T("Filter Name"), 0, rc.Width() - 20);

	// merits
	merit_mode = CFiltersForm::MERIT_MODE_ALL;
	combo_merit.ResetContent();
	combo_merit.AddString(_T("All Merit Values"));
	combo_merit.AddString(_T("= MERIT_DO_NOT_USE"));
	combo_merit.AddString(_T(">= MERIT_DO_NOT_USE"));
	combo_merit.AddString(_T("> MERIT_DO_NOT_USE"));
	combo_merit.AddString(_T("= MERIT_UNLIKELY "));
	combo_merit.AddString(_T(">= MERIT_UNLIKELY "));
	combo_merit.AddString(_T("> MERIT_UNLIKELY "));
	combo_merit.AddString(_T("= MERIT_NORMAL"));
	combo_merit.AddString(_T(">= MERIT_NORMAL"));
	combo_merit.AddString(_T("> MERIT_NORMAL"));
	combo_merit.AddString(_T("= MERIT_PREFERRED"));
	combo_merit.AddString(_T(">= MERIT_PREFERRED"));
	combo_merit.AddString(_T("> MERIT_PREFERRED"));
	combo_merit.AddString(_T("Non-Standard Merit"));
	combo_merit.SetCurSel(0);

	DSUtil::FilterTemplates filters;

	const DSUtil::FilterCategories & categories = GetFilterCategories();

	for (int i=0; i<categories.categories.GetCount(); i++) {
		const DSUtil::FilterCategory	&cat = categories.categories[i];

		// ignore empty categories
		filters.Enumerate(cat);
		if (filters.filters.GetCount() > 0) {
			CString	n;
			n.Format(_T("%s (%d filters)"), cat.name, filters.filters.GetCount());
			int item_index = combo_categories.AddString(n);
			combo_categories.SetItemDataPtr(item_index, (void*)&cat);

			if (cat.clsid == GUID_NULL && !cat.is_dmo) {	// Set ALL filters as the default entry
				combo_categories.SetCurSel(item_index);
				RefreshFilterList(filters);
			}
		}
	}

	combo_categories.InsertString(0, _T("-- Favorite Filters"));
	combo_categories.SetItemDataPtr(0, (void*)CATEGORY_FAVORITES);

	combo_categories.InsertString(1, _T("-- Blacklisted Filters"));
	combo_categories.SetItemDataPtr(1, (void*)CATEGORY_BLACKLIST);

	combo_categories.SetMinVisibleItems(40);
	combo_merit.SetMinVisibleItems(40);

    btn_register.SetShield(TRUE);
    btn_unregister.SetShield(TRUE);
    btn_merit.SetShield(TRUE);

    //Set up the tooltip
    m_pToolTip = new CToolTipCtrl;
    if(!m_pToolTip->Create(this))
    {
        TRACE("Unable To create ToolTip\n");
        return;
    }

    m_pToolTip->Activate(TRUE);
}

static void EnumerateBookmarks(const CArray<GraphStudio::BookmarkedFilter*>& bookmarks, DSUtil::FilterTemplates& filters)
{
	for (int i=0; i<bookmarks.GetCount(); i++) {
		GraphStudio::BookmarkedFilter* const bm = bookmarks[i];
		DSUtil::FilterTemplate filter_template;
		filter_template.LoadFromMonikerName(bm->moniker_name);
		filters.filters.Add(filter_template);
	}
}

void CFiltersForm::OnComboCategoriesChange()
{
	const int item = combo_categories.GetCurSel();
	DSUtil::FilterCategory* const item_data = (DSUtil::FilterCategory*)combo_categories.GetItemDataPtr(item);
	DSUtil::FilterTemplates	filters;

	if (item_data == (DSUtil::FilterCategory*)CATEGORY_FAVORITES) {
		EnumerateBookmarks(CFavoritesForm::GetFavoriteFilters()->filters, filters); 
	} else if (item_data == (DSUtil::FilterCategory*)CATEGORY_BLACKLIST) {
		EnumerateBookmarks(CFavoritesForm::GetBlacklistedFilters()->filters, filters); 
	} else if (item_data) {
		filters.Enumerate(*item_data);
	}
	RefreshFilterList(filters);
	info.Clear();
	tree_details.BuildPropertyTree(&info);

}

void CFiltersForm::RefreshFilterList(const DSUtil::FilterTemplates &filters)
{
	list_filters.Initialize();   

	int i;
	for (i=0; i<filters.filters.GetCount(); i++) {
		const DSUtil::FilterTemplate	&filter = filters.filters[i];

		// pridame itemu
		if (CanBeDisplayed(filter)) {
			list_filters.filters.Add(filter);
		}
	}
	list_filters.UpdateList();
}

void CFiltersForm::FindFilterWithCLSID(const CLSID & filter_clsid)
{
	CString clsid_str;
	GraphStudio::CLSIDToString(filter_clsid, clsid_str);

	edit_search.SetWindowText(clsid_str);
	list_filters.SetSearchString(clsid_str);
	if (list_filters.GetItemCount() <= 0) {								// if nothing found
		if (	combo_categories.GetCurSel()	!= CATEGORY_ALL			// if category isn't ALL DirectShow filters
			||	combo_merit.GetCurSel()			!= MERIT_MODE_ALL)		// OR merit isn't 'ANY'
		{						
			combo_categories.SetCurSel(CATEGORY_ALL);					// set category to ALL DirectShow filters
			merit_mode = MERIT_MODE_ALL;
 			combo_merit.SetCurSel(merit_mode);							// and merit to 'ANY'
			OnComboCategoriesChange();									// and refresh
		}
	}

	if (list_filters.GetItemCount() > 0) {								// If any found
		list_filters.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);		// select first filter found

		DSUtil::FilterTemplate * const filter = (DSUtil::FilterTemplate*)list_filters.GetItemData(0);
		ASSERT(filter);
		if (filter)
			UpdateFilterDetails(*filter);								// update rest of page
	} else {
		info.Clear();
		tree_details.BuildPropertyTree(&info);
	}
}


void CFiltersForm::OnEnUpdateSearchString()
{
	CString search_string;
	edit_search.GetWindowText(search_string);
	list_filters.SetSearchString(search_string);
	info.Clear();
	tree_details.BuildPropertyTree(&info);
}

bool CFiltersForm::CanBeDisplayed(const DSUtil::FilterTemplate &filter)
{
	switch (merit_mode) {
	case CFiltersForm::MERIT_MODE_ALL:				return true;
	case CFiltersForm::MERIT_MODE_DONOTUSE:			return (filter.merit == MERIT_DO_NOT_USE);
	case CFiltersForm::MERIT_MODE_DONOTUSE_GE:		return (filter.merit >= MERIT_DO_NOT_USE);
	case CFiltersForm::MERIT_MODE_DONOTUSE_G:		return (filter.merit > MERIT_DO_NOT_USE);
	case CFiltersForm::MERIT_MODE_UNLIKELY:			return (filter.merit == MERIT_UNLIKELY);
	case CFiltersForm::MERIT_MODE_UNLIKELY_GE:		return (filter.merit >= MERIT_UNLIKELY);
	case CFiltersForm::MERIT_MODE_UNLIKELY_G:		return (filter.merit > MERIT_UNLIKELY);
	case CFiltersForm::MERIT_MODE_NORMAL:			return (filter.merit == MERIT_NORMAL);
	case CFiltersForm::MERIT_MODE_NORMAL_GE:		return (filter.merit >= MERIT_NORMAL);
	case CFiltersForm::MERIT_MODE_NORMAL_G:			return (filter.merit > MERIT_NORMAL);
	case CFiltersForm::MERIT_MODE_PREFERRED:		return (filter.merit == MERIT_PREFERRED);
	case CFiltersForm::MERIT_MODE_PREFERRED_GE:		return (filter.merit >= MERIT_PREFERRED);
	case CFiltersForm::MERIT_MODE_PREFERRED_G:		return (filter.merit > MERIT_PREFERRED);
	case CFiltersForm::MERIT_MODE_NON_STANDARD:
		{
			if (filter.merit == MERIT_DO_NOT_USE ||
				filter.merit == MERIT_UNLIKELY ||
				filter.merit == MERIT_NORMAL ||
				filter.merit == MERIT_PREFERRED) return false;
			return true;
		}
		break;

	default:
		return false;
	}
}

#define MIN_WIDTH_RIGHT 320

void CFiltersForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	// compute anchor lines
	int	right_x		= rc.Width() / 2;
    int	right_width = right_x;
    if(right_x < MIN_WIDTH_RIGHT)
    {
        right_x = rc.Width() - MIN_WIDTH_RIGHT;
        right_width = MIN_WIDTH_RIGHT;
    }
	const int merit_combo_width = 130;
	const int gap = 8;

	title.GetClientRect(&rc2);
	title.SetWindowPos(NULL, 0, 0, cx, rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	const int title_height = rc2.Height();

	const int details_top = rc2.Height();
	list_filters.SetWindowPos(NULL, 0, rc2.Height(), right_x, rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	list_filters.GetClientRect(&rc2);
	list_filters.SetColumnWidth(0, rc2.Width()-10);

	// details
	tree_details.SetWindowPos(NULL, right_x, details_top, rc.Width()-right_x, rc.Height() - 100-details_top, SWP_SHOWWINDOW | SWP_NOZORDER);

	check_favorite.GetWindowRect(&rc2);
    check_favorite.SetWindowPos(NULL, right_x+gap, rc.Height()-100+gap, rc2.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	// combo boxes
	combo_categories.GetWindowRect(&rc2);
	combo_categories.SetWindowPos(NULL, 4, 6, right_x - gap - merit_combo_width, rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	combo_merit.GetWindowRect(&rc2);
	combo_merit.SetWindowPos(NULL, right_x - merit_combo_width, 6, merit_combo_width, rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	combo_categories.SetMinVisibleItems(40);
	combo_merit.SetMinVisibleItems(40);

	// sizing
	btn_register.GetWindowRect(&rc2);
	const int	btn_height = rc2.Height();
	const int btn_register_width = rc2.Width();
	int current_x = right_x + (gap*2);

	// edit control
	const int edit_search_width = rc.Width() - (2*gap) - current_x - btn_register_width;
	edit_search.GetWindowRect(&rc2);
	edit_search.SetWindowPos(NULL, current_x, (title_height-rc2.Height()) / 2, edit_search_width, rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	// buttons
	btn_register.GetWindowRect(&rc2);
	btn_register.SetWindowPos(NULL, rc.Width() - 4 - rc2.Width(), 5, rc2.Width(), btn_height, SWP_SHOWWINDOW | SWP_NOZORDER);

	btn_insert.GetWindowRect(&rc2);
	btn_insert.SetWindowPos(NULL, right_x+gap, rc.Height() - 2*(gap+btn_height), rc2.Width(), btn_height, SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_propertypage.SetWindowPos(NULL, right_x+gap, rc.Height() - 1*(gap+btn_height), rc2.Width(), btn_height, SWP_SHOWWINDOW | SWP_NOZORDER);

	btn_merit.SetWindowPos(NULL, rc.Width() - gap - rc2.Width(), rc.Height() - 3*(gap+btn_height), rc2.Width(), btn_height, SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_locate.SetWindowPos(NULL, rc.Width() - gap - rc2.Width(), rc.Height() - 2*(gap+btn_height), rc2.Width(), btn_height, SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_unregister.SetWindowPos(NULL, rc.Width() - gap - rc2.Width(), rc.Height() - 1*(gap+btn_height), rc2.Width(), btn_height, SWP_SHOWWINDOW | SWP_NOZORDER);

    check_blacklist.GetWindowRect(&rc2);
    check_blacklist.SetWindowPos(NULL, right_x + right_width / 2 - rc2.Width() / 2, rc.Height()- 100 +gap, rc2.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

    m_search_online.GetWindowRect(&rc2);
    m_search_online.SetWindowPos(NULL, right_x + right_width / 2 - rc2.Width() / 2, rc.Height()- 70 +gap, rc2.Width(), btn_height, SWP_SHOWWINDOW | SWP_NOZORDER);

	// invalidate all controls
	title.Invalidate();
	//combo_categories.Invalidate();
	//combo_merit.Invalidate();
	btn_register.Invalidate();
	btn_insert.Invalidate();
	btn_propertypage.Invalidate();
	btn_locate.Invalidate();
	btn_merit.Invalidate();
	btn_unregister.Invalidate();

	list_filters.Invalidate();
	tree_details.Invalidate();

    m_search_online.Invalidate();
}

void CFiltersForm::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT item)
{
	if (item->CtlType == ODT_LISTVIEW) {
		if (item->CtlID == IDC_LIST_FILTERS) {
		
			// fixed height
			item->itemHeight = 18;
			return ;

		}
	}

	// base clasu
	__super::OnMeasureItem(nIDCtl, item);
}

void CFiltersForm::OnItemDblClk(int item)
{
	OnBnClickedButtonInsert();
}

void CFiltersForm::OnUpdateSearchString(const CString& search_string)
{
	edit_search.SetWindowText(search_string);
}

void CFiltersForm::OnBnClickedButtonInsert()
{
    POSITION pos = list_filters.GetFirstSelectedItemPosition();
	while (pos)
    {
		const int item = list_filters.GetNextSelectedItem(pos);
		DSUtil::FilterTemplate * const filter = (DSUtil::FilterTemplate*)list_filters.GetItemData(item);
        if (filter)
			view->InsertFilterFromTemplate(*filter);
	}
}

void CFiltersForm::OnComboMeritChange()
{
	merit_mode = combo_merit.GetCurSel();
	OnComboCategoriesChange();
}

BOOL CFiltersForm::PreTranslateMessage(MSG *pmsg)
{
	if (pmsg->message == WM_KEYDOWN) {
		if (pmsg->wParam == VK_RETURN) {
			OnBnClickedButtonInsert();
			return TRUE;
		} else if (pmsg->wParam == VK_F5) {
			OnComboCategoriesChange();				// Refresh filter list on F5
			cached_templates.filters.RemoveAll();	// Refresh cache for lookup of CLSID
		}
	}

    if (NULL != m_pToolTip)
        m_pToolTip->RelayEvent(pmsg);

	return __super::PreTranslateMessage(pmsg);
}

void CFiltersForm::OnFilterItemClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (!(pNMLV->uOldState & ODS_SELECTED) &&
		(pNMLV->uNewState & ODS_SELECTED)) {
		DSUtil::FilterTemplate	*filter = (DSUtil::FilterTemplate*)list_filters.GetItemData(pNMLV->iItem);

		if (filter)
			UpdateFilterDetails(*filter);
	}
}

void CFiltersForm::UpdateFilterDetails(const DSUtil::FilterTemplate &filter)
{
	// display information
	info.Clear();
	const DSUtil::FilterCategories & categories = GetFilterCategories();

	if (filter.category != GUID_NULL) {
		GraphStudio::PropItem * const category_item = info.AddItem(new GraphStudio::PropItem(_T("Category")));

		CString category_name;
		for (int i=0; i<categories.categories.GetCount(); i++) {
			const DSUtil::FilterCategory	&cat = categories.categories[i];
			if (cat.clsid == filter.category) {
				category_name = cat.name;
			}
		}
		CString category_guid;
		GraphStudio::NameGuid(filter.category, category_guid, false);

		category_item->AddItem(new GraphStudio::PropItem(_T("Category Name"), category_name));
		category_item->AddItem(new GraphStudio::PropItem(_T("Category GUID"), category_guid));
	}

	GraphStudio::PropItem * const group= info.AddItem(new GraphStudio::PropItem(_T("Filter Details")));
	{
		CString	type;
		switch (filter.type) {
			case DSUtil::FilterTemplate::FT_DMO:		type = _T("DMO"); break;
			case DSUtil::FilterTemplate::FT_KSPROXY:	type = _T("WDM"); break;
			case DSUtil::FilterTemplate::FT_FILTER:		type = _T("Standard"); break;
			case DSUtil::FilterTemplate::FT_ACM_ICM:	type = _T("ACM/ICM"); break;
			case DSUtil::FilterTemplate::FT_PNP:		type = _T("Plug && Play"); break;
		}	
		group->AddItem(new GraphStudio::PropItem(_T("Display Name"), filter.moniker_name));

		// The following properties are only present for some hardware devices
		if (!filter.description.IsEmpty())
			group->AddItem(new GraphStudio::PropItem(_T("Description"), filter.description));
		if (!filter.device_path.IsEmpty())
			group->AddItem(new GraphStudio::PropItem(_T("Device Path"), filter.device_path));
		if (filter.wave_in_id >= 0)
			group->AddItem(new GraphStudio::PropItem(_T("waveIn ID"), filter.wave_in_id));

		group->AddItem(new GraphStudio::PropItem(_T("Type"), type));

		// Use the values found during enumeration rather than values found in the registry
		if (GUID_NULL != filter.clsid)
			group->AddItem(new GraphStudio::PropItem(_T("CLSID"), filter.clsid));
		if (!filter.name.IsEmpty())
			group->AddItem(new GraphStudio::PropItem(_T("Object Name"), filter.name));
		if (!filter.file.IsEmpty())
			group->AddItem(new GraphStudio::PropItem(_T("File"), filter.file));
	}
	GraphStudio::PropItem * const template_info = info.AddItem(new GraphStudio::PropItem(_T("Filter Data")));
	{
		GraphStudio::GetFilterInformationFromTemplate(filter, template_info);
	}

	tree_details.BuildPropertyTree(&info);

	// favorite filter ?
	GraphStudio::BookmarkedFilters * const favorites = CFavoritesForm::GetFavoriteFilters();
	check_favorite.SetCheck(favorites->IsBookmarked(filter));

	// blacklisted filter
	GraphStudio::BookmarkedFilters * const blacklisted = CFavoritesForm::GetBlacklistedFilters();
	check_blacklist.SetCheck(blacklisted->IsBookmarked(filter));

	// create search url
	LPOLESTR str;
	StringFromCLSID(filter.clsid, &str);
	CString	str_clsid(str);
	CString str_name = filter.name;
	str_name.Replace(' ', '+');
	CString url;
	CString file = filter.file.Mid(CPath(filter.file).FindFileName());
	url.Format(TEXT("http://www.google.com/search?q=%s+OR+\\\"%s\\\"+OR+%s"), str_clsid, str_name, file);
	m_search_online.SetURL(url);
	if (str) 
		CoTaskMemFree(str);
}

void CFiltersForm::OnBnClickedCheckBlacklist()
{
	DSUtil::FilterTemplate *filter = GetSelected();
	if (filter) {
		const BOOL	check = check_blacklist.GetCheck();
		GraphStudio::BookmarkedFilters	* const blacklist	= CFavoritesForm::GetBlacklistedFilters();

		if (check) {
			blacklist->AddBookmark(*filter);
		} else {
			HTREEITEM item = blacklist->RemoveBookmark(*filter);
		}
		blacklist->Save();
	}
}

void CFiltersForm::OnBnClickedCheckFavorite()
{
	DSUtil::FilterTemplate *filter = GetSelected();
	if (filter) {

		const BOOL						check		= check_favorite.GetCheck();
		GraphStudio::BookmarkedFilters*	const favorites	= CFavoritesForm::GetFavoriteFilters();

		if (check) {
			favorites->AddBookmark(*filter);
			if (view && view->form_favorites) {
				view->form_favorites->UpdateTree();
			}
		} else {
			HTREEITEM item = favorites->RemoveBookmark(*filter);
			if (item != NULL) {
				// remove this item
				if (view && view->form_favorites) {
					view->form_favorites->RemoveFilter(item);
				}
			}
		}

		// todo: update tree
		favorites->Save();
	}
}

DSUtil::FilterTemplate *CFiltersForm::GetSelected()
{
	POSITION pos = list_filters.GetFirstSelectedItemPosition();
	if (pos) {
		int item = list_filters.GetNextSelectedItem(pos);
		DSUtil::FilterTemplate *filter = (DSUtil::FilterTemplate*)list_filters.GetItemData(item);
		return filter;
	}

	return NULL;
}

void CFiltersForm::OnBnClickedButtonPropertypage()
{
	// create a new instance of a filter and display
	// it's property page.
	// the filter will be destroyed once the page is closed
	// now we try to add a filter

	DSUtil::FilterTemplate *filter = GetSelected();
	if (filter) {

		// now create an instance of this filter
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr;

		hr = filter->CreateInstance(&instance);
		if (FAILED(hr)) {
			// display error message
		} else {


			CString			title = filter->name + _T(" Properties");
			CPropertyForm	*page = new CPropertyForm();
			int ret = page->DisplayPages(instance, instance, title, view);
			if (ret < 0) {
				delete page;
				return ;
			}
			view->property_pages.Add(page);
		}
		instance = NULL;
	}
}


int ConfigureInsertedFilter(IBaseFilter *filter, const CString& filterName)
{
	int	ret = 0;
	HRESULT hr = S_OK;

	//-------------------------------------------------------------
	//	IFileSourceFilter
	//-------------------------------------------------------------
	CComQIPtr<IFileSourceFilter> fs = filter;
	if (fs) {
		CFileSrcForm form(filterName);
		hr = form.ChooseSourceFile(fs);					// not fatal error if file not chosen yet
	}

	//-------------------------------------------------------------
	//	IFileSinkFilter
	//-------------------------------------------------------------
	CComQIPtr<IFileSinkFilter> fsink = filter;
	if (fsink) {
		CFileSinkForm form(filterName);
		HRESULT hr = form.ChooseSinkFile(fsink);	// not fatal error if file not chosen yet
	}

    //-------------------------------------------------------------
	//	IStreamBufferConfigure
	//-------------------------------------------------------------
    CComQIPtr<IStreamBufferInitialize> pInitSbe = filter;
    if(pInitSbe) {
		DSUtil::InitSbeObject(pInitSbe);
	}

	return hr;
}


void CFiltersForm::OnLocateClick()
{
	DSUtil::FilterTemplate *filter = GetSelected();
	if (filter) {

		// get the file name
		CString		filename = filter->file;

		// open the explorer with the location
		CString		param;
		param = _T("/select, \"");
		param += filename;
		param += _T("\"");

		ShellExecute(NULL, _T("open"), _T("explorer.exe"), param, NULL,	SW_NORMAL);
	}
}

BOOL IsUserAdmin(VOID)
/*++ 
Routine Description: This routine returns TRUE if the caller's
process is a member of the Administrators local group. Caller is NOT
expected to be impersonating anyone and is expected to be able to
open its own process and process token. 
Arguments: None. 
Return Value: 
   TRUE - Caller has Administrators local group. 
   FALSE - Caller does not have Administrators local group. --
*/ 
{
BOOL b;
SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
PSID AdministratorsGroup; 
b = AllocateAndInitializeSid(
    &NtAuthority,
    2,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_ADMINS,
    0, 0, 0, 0, 0, 0,
    &AdministratorsGroup); 
if(b) 
{
    if (!CheckTokenMembership( NULL, AdministratorsGroup, &b)) 
    {
         b = FALSE;
    } 
    FreeSid(AdministratorsGroup); 
}

return(b);
}

void CFiltersForm::OnUnregisterClick()
{
    if(!IsUserAdmin())
    {
        DSUtil::ShowInfo(_T("Admin rights required to unregister a filter.\nPlease restart the program as admin."));
        return;
    }

	POSITION pos = list_filters.GetFirstSelectedItemPosition();
    int sel = list_filters.GetSelectionMark();
    bool changed = false;
	while (pos)
    {
		int item = list_filters.GetNextSelectedItem(pos);
		DSUtil::FilterTemplate *filter = (DSUtil::FilterTemplate*)list_filters.GetItemData(item);
		
        if (filter)
        {
		    HRESULT				hr;

		    // DMOs do it differently
		    if (filter->type == DSUtil::FilterTemplate::FT_DMO) {

			    hr = DMOUnregister(filter->clsid, filter->category);
			    if (SUCCEEDED(hr)) {
                    changed = true;
				    DSUtil::ShowInfo(_T("Unregister succeeded."));
			    } else {
				    CString		msg;
				    msg.Format(_T("Unregister failed: 0x%08x"), hr);
				    DSUtil::ShowError(hr, msg);
			    }

		    } else
		    if (filter->type == DSUtil::FilterTemplate::FT_FILTER) {

			    /*
				    We either call DllUnregisterServer in the file.
				    Or simply delete the entries if the file is no longer 
				    available.
			    */

			    CString		fn = filter->file;
			    fn = fn.MakeLower();
			
			    if (fn.Find(_T("quartz.dll")) >= 0 ||
				    fn.Find(_T("qdvd.dll")) >= 0 ||
				    fn.Find(_T("qdv.dll")) >= 0 ||
				    fn.Find(_T("qedit.dll")) >= 0 || 
				    fn.Find(_T("qasf.dll")) >= 0 ||
				    fn.Find(_T("qcap.dll")) >= 0
				    ) {

				    // we simply won't let the users unregister these files...
				    // If they really try to do this, perhaps they should be
				    // doing something else than computers...
				    DSUtil::ShowWarning(_T("This file is essential to the system.\nPermission denied."));
				    continue;
			    }

			    // ask the user for confirmation
                BOOL unregisterAll;
			    if (!ConfirmUnregisterFilter(filter->name, &unregisterAll)) {
				    continue;
			    }

                // prepare dll search path
                CString libPath = fn;
                PathRemoveFileSpec (libPath.GetBuffer()); 
                libPath.ReleaseBuffer(); 
                SetDllDirectory(libPath);

			    HMODULE		library = LoadLibrary(fn);
			    if (library && unregisterAll) {
				    DllUnregisterServerProc		unreg = (DllUnregisterServerProc)GetProcAddress(library, "DllUnregisterServer");
				    if (unreg) {
					    hr = unreg();
					    if (SUCCEEDED(hr)) {
                            changed = true;
                            CString		msg;
                            msg.Format(_T("Unregister '%s' succeeded."), PathFindFileName(fn));
						    DSUtil::ShowInfo(msg);
					    } else {
						    CString		msg;
                            msg.Format(_T("Unregister '%s' failed: 0x%08x"), PathFindFileName(fn), hr);
						    DSUtil::ShowError(hr, msg);
					    }
				    }
				    FreeLibrary(library);

			    } else {

				    // dirty removing...
				    hr = DSUtil::UnregisterFilter(filter->clsid, filter->category);
				    if (SUCCEEDED(hr)) {
					    hr = DSUtil::UnregisterCOM(filter->clsid);
				    }
				    if (SUCCEEDED(hr)) {
                        changed = true;
                        CString		msg;
                        msg.Format(_T("Unregister '%s' succeeded."), filter->name);
					    DSUtil::ShowInfo(msg);
				    } else {
					    CString		msg;
					    msg.Format(_T("Unregister '%s' failed: 0x%08x"), filter->name, hr);
					    DSUtil::ShowError(hr, msg);
				    }
			    }
		    }
        }
	}

    // reload the filters
    if(changed)
    {
	    OnComboCategoriesChange();
        list_filters.SetItemState(sel, LVIS_SELECTED, LVIS_SELECTED);
        list_filters.SetSelectionMark(sel);
        list_filters.EnsureVisible(sel, TRUE);
    }
}

void CFiltersForm::OnMeritClick()
{
    if(!IsUserAdmin())
    {
        DSUtil::ShowInfo(_T("Admin rights required to change the merit of a filter.\nPlease restart the program as admin."));
        return;
    }

	DSUtil::FilterTemplate *filter = GetSelected();
	if (filter) {

		if (filter->type == DSUtil::FilterTemplate::FT_DMO ||
			filter->type == DSUtil::FilterTemplate::FT_FILTER
			) {

			DWORD		oldmerit = filter->merit;
			DWORD		newmerit = filter->merit;

			if (ChangeMeritDialog(filter->name, oldmerit, newmerit)) {

				// try to change the merit
				filter->merit = newmerit;
				int ret = filter->WriteMerit();
				if (ret < 0) {
					DSUtil::ShowError(_T("Failed to update merit value"));
					filter->merit = oldmerit;
				} else {
					DSUtil::ShowInfo(_T("Merit change succeeded."));

					// remember the display name so we can select the same filter again
					CString		displayname = filter->moniker_name;
					int			bottom = list_filters.GetBottomIndex();

					// reload the filters
					OnComboCategoriesChange();

					int			idx = -1;
					for (int i=0; i<list_filters.GetItemCount(); i++) {
						DSUtil::FilterTemplate	*filt = (DSUtil::FilterTemplate*)list_filters.GetItemData(i);
						if (filt->moniker_name == displayname) {
							idx = i;
							break;
						}
					}

					if (idx >= 0) {
						list_filters.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						list_filters.SetSelectionMark(idx);

						// it always scrolls one item down :Z
						if (bottom > 0) bottom -= 1;
						list_filters.EnsureVisible(bottom, TRUE);
					}
				}
			}
		}
	}
}

void CFiltersForm::OnRegisterClick()
{
    if(!IsUserAdmin())
    {
        DSUtil::ShowInfo(_T("Admin rights required to register a filter.\nPlease restart the program as admin."));
        return;
    }

    CString	filter = _T("Filter Files (*.dll,*.ocx,*.ax)|*.dll;*.ocx;*.ax|All Files|*.*|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_ALLOWMULTISELECT|OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);

    int ret = dlg.DoModal();
    if (ret != IDOK) return;

    bool changed = false;
    int sel = list_filters.GetSelectionMark();

    POSITION pos = dlg.GetStartPosition();
    CString firstFilterFile;
    while(pos)
    {
        CString filename = dlg.GetNextPathName(pos);
        firstFilterFile = filename;

        // prepare dll search path
        CString libPath = filename;
        PathRemoveFileSpec (libPath.GetBuffer()); 
        libPath.ReleaseBuffer(); 
        SetDllDirectory(libPath);

        HMODULE	library = LoadLibrary(filename);
		if (library)
        {
			DllRegisterServerProc reg = (DllUnregisterServerProc)GetProcAddress(library, "DllRegisterServer");
			if (reg)
            {
				HRESULT hr = reg();
				if (SUCCEEDED(hr))
                {
                    changed = true;
                    CString	msg;
                    msg.Format(_T("Register '%s' succeeded."), PathFindFileName(filename));
					DSUtil::ShowInfo(msg);
				} else {
					CString	msg;
					msg.Format(_T("Register '%s' failed: 0x%08x"), PathFindFileName(filename), hr);
					DSUtil::ShowError(hr, msg);
				}
			}
			FreeLibrary(library);
		} 
    }

    // reload the filters
    if(changed)
    {
	    OnComboCategoriesChange();

        DSUtil::FilterTemplate*	filter;
        for (int i=0;i < list_filters.GetItemCount();i++)
        {
            filter = (DSUtil::FilterTemplate*)(list_filters.GetItemData(i));
            if (filter->file.CompareNoCase(firstFilterFile) == 0)
            {
                list_filters.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
                list_filters.SetSelectionMark(i);
                list_filters.EnsureVisible(i, TRUE);
                return;
            }
        }

        if(sel >= 0)
        {
            list_filters.SetItemState(sel, LVIS_SELECTED, LVIS_SELECTED);
            list_filters.SetSelectionMark(sel);
            list_filters.EnsureVisible(sel, TRUE);
        }
    }
}

BOOL CFiltersForm::OnInitDialog()
{
	BOOL res = __super::OnInitDialog();
	list_filters.Initialize();
	return res;
}

HBRUSH CFiltersForm::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	const HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	const int id = pWnd ? pWnd->GetDlgCtrlID() : 0;
	if (IDC_SEARCH_STRING == id) {
		pDC->SetTextColor(list_filters.GetSelectionColor());
	}
   return hbr;
}

static inline void EnumerateAllFilters(DSUtil::FilterTemplates& templates)
{
	if (templates.filters.GetCount() == 0) {
		templates.EnumerateDMO(GUID_NULL);
		templates.EnumerateAllRegisteredFilters();
	}
}

bool CFiltersForm::FilterTemplateFromCLSID(const GUID& clsid, DSUtil::FilterTemplate& filter_template)
{
	bool found =false;
	EnumerateAllFilters(cached_templates);
	return 0 == cached_templates.FindTemplateByCLSID(clsid, &filter_template);
}

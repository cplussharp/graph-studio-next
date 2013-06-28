//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
//
//	CFiltersForm class
//
//-----------------------------------------------------------------------------
class CFiltersForm : 
	public CGraphStudioModelessDialog,
	public GraphStudio::FilterListCallback
{
private:
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CFiltersForm)

	void RefreshFilterList(const DSUtil::FilterTemplates &filters);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    HICON m_hIcon;

	CComboBox					combo_categories;
	CComboBox					combo_merit;
	CEdit						edit_search;
	CButton						btn_register;
	CButton						btn_insert;
	CButton						btn_propertypage;
	CButton						btn_unregister;
	CButton						btn_locate;
	CButton						btn_merit;
	CButton						check_favorite;
	CButton						check_blacklist;
	GraphStudio::FilterListCtrl	list_filters;
	GraphStudio::TitleBar		title;
	GraphStudio::PropertyTree	tree_details;
	GraphStudio::PropItem		info;
    CMFCLinkCtrl                m_search_online;
    CToolTipCtrl*               m_pToolTip;

	// enumerated stuff
	DSUtil::FilterCategories	categories;

	static DSUtil::FilterTemplates		cached_templates;	// cache for searching

	// Fake filter values
	enum Category
	{
		CATEGORY_FAVORITES = 1,
		CATEGORY_BLACKLIST = 3
	};

	enum {
		MERIT_MODE_ALL = 0,
		MERIT_MODE_DONOTUSE = 1,
		MERIT_MODE_DONOTUSE_GE = 2,
		MERIT_MODE_DONOTUSE_G = 3,
		MERIT_MODE_UNLIKELY = 4,
		MERIT_MODE_UNLIKELY_GE = 5,
		MERIT_MODE_UNLIKELY_G = 6,
		MERIT_MODE_NORMAL = 7,
		MERIT_MODE_NORMAL_GE = 8,
		MERIT_MODE_NORMAL_G = 9,
		MERIT_MODE_PREFERRED = 10,
		MERIT_MODE_PREFERRED_GE = 11,
		MERIT_MODE_PREFERRED_G = 12,
		MERIT_MODE_NON_STANDARD = 13
	};
	int		merit_mode;

public:
	CFiltersForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFiltersForm();

	BOOL DoCreateDialog(CWnd* parent);

	static bool FilterTemplateFromCLSID(const GUID& clsid, DSUtil::FilterTemplate& filter_template);

private:
	// Dialog Data
	enum { IDD = IDD_DIALOG_FILTERS };

	// overrides
	void OnInitialize();
	void OnSize(UINT nType, int cx, int cy);
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT item);
	BOOL PreTranslateMessage(MSG *pmsg);
	virtual  BOOL OnInitDialog( );

	// filtering
	DSUtil::FilterTemplate *GetSelected();
	bool CanBeDisplayed(const DSUtil::FilterTemplate &filter);


	// filterlist callback
	virtual void OnItemDblClk(int item);
	virtual void OnUpdateSearchString(const CString& search_string);

	// command handlers
	afx_msg void OnComboCategoriesChange();
	afx_msg void OnBnClickedButtonInsert();
	afx_msg void OnComboMeritChange();
	afx_msg void OnFilterItemClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonPropertypage();
	afx_msg void OnBnClickedCheckFavorite();
	afx_msg void OnLocateClick();
	afx_msg void OnUnregisterClick();
	afx_msg void OnRegisterClick();
	afx_msg void OnMeritClick();
	afx_msg void OnEnUpdateSearchString();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedCheckBlacklist();

};

int ConfigureInsertedFilter(IBaseFilter *filter, const CString& strFilterName);



//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

class CPropertyForm;
class CPageSite;
class CPageContainer;

//-----------------------------------------------------------------------------
//
//	CPageContainer class
//
//-----------------------------------------------------------------------------
class CPageContainer : public CUnknown
{
public:
	CPropertyForm			*form;
	CArray<CPageSite*>		pages;
	int						current;

public:
	CPageContainer(CPropertyForm *parent);
	virtual ~CPageContainer();

	void Clear();
	void ActivatePage(int i);
	void DeactivatePage(int i);
	int AddPage(IPropertyPage *page);
	void ResizeToFitPage(int i);
};


//-----------------------------------------------------------------------------
//
//	CPageSite class
//
//-----------------------------------------------------------------------------
class CPageSite : 
	public CUnknown,
	public IPropertyPageSite
{
public:
	CPageContainer				*parent;
	CComPtr<IPropertyPage>		page;
	bool						active;

	// page properties
	CString						title;
	CSize						size;

public:
	CPageSite(LPUNKNOWN pUnk, CPageContainer *container);
	virtual ~CPageSite();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	// IPropertyPageSite
	STDMETHODIMP OnStatusChange(DWORD dwFlags);
    STDMETHODIMP GetLocaleID(LCID *pLocaleID);
	STDMETHODIMP GetPageContainer(IUnknown **ppUnk);
	STDMETHODIMP TranslateAccelerator(MSG *pMsg);

	// I/O
	int CloseSite();
	int Deactivate();
	int Activate(HWND owner, CRect &rc);
	int AssignPage(IPropertyPage *page);
};


//-----------------------------------------------------------------------------
//
//	CPropertyForm class
//
//-----------------------------------------------------------------------------
class CPropertyForm : public CGraphStudioModelessDialog
{
protected:
	DECLARE_DYNAMIC(CPropertyForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX); 

public:
	CTabCtrl				tabs;
	CButton					button_apply;
	CButton					button_ok;
	CButton					button_close;
	CButton					button_auto_size;

	// control alignment helpers
	int						tab_x, tab_y, tab_cx, tab_cy;
	int						button_bottom_offset;
	int						bok_cx, bcancel_cx, bapply_cx;
	static CSize			previous_size;

	// we're associated with this object (filter/pin)
	IUnknown				*object;
	IUnknown				*filter;

	CPageContainer			*container;

public:
	CPropertyForm(CWnd* pParent = NULL);  
	virtual ~CPropertyForm();
	virtual bool ShouldRestorePosition() const { return false; }		// leave this to the main window (and we have may have multiple instances)

	enum { IDD = IDD_DIALOG_PROPERTYPAGE };

	// activate property page for objects
	int DisplayPages(IUnknown *obj, IUnknown *filt, CString title, CGraphView *view);
	int AnalyzeObject(IUnknown *obj);
    void AddPropertyPage(CDSPropertyPage *prop_page, IUnknown *obj);
	int LoadPinPage(IPin *pin);
    int LoadInterfacePage(IUnknown *obj, const CString& strTitle);
	int LoadDbgLogPage(IUnknown *obj, const CString& strTitle);
    int LoadMediaInfoPage(IUnknown *obj);
    void LoadCustomInterfacePropertyPages(IUnknown *obj);

	// check for DMO pages
	int AnalyzeDMO(IUnknown *obj);

	void OnDestroy();
	CSize GetFormSizeToFitPage(CSize size);
	void OnSize(UINT nType, int cx, int cy);
	void OnClose();
	afx_msg void OnTabSelected(NMHDR *pNMHDR, LRESULT *pResult);

	virtual BOOL PreTranslateMessage(MSG *pMsg);

	void OnOK();
	void OnCancel();
	afx_msg void OnBnClickedButtonApply();
	LRESULT OnPressButton(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedAutoSize();
};

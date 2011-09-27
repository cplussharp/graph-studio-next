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
//	CDSPropertyPage class
//
//-----------------------------------------------------------------------------
class CDSPropertyPage : 
	public CDialog,
	public CUnknown,
	public IPropertyPage
{
protected:

    LPPROPERTYPAGESITE		page_site;
    HWND					hwnd_page;
	BOOL					dirty;
	BOOL					object_set;
	int						dialog_id;
	CString					title;

public:
	CDSPropertyPage(LPCTSTR pName, LPUNKNOWN pUnk, int DialogID, LPCTSTR title);
	virtual ~CDSPropertyPage();

    STDMETHODIMP QueryInterface(REFIID riid, __deref_out void **ppv) { 
		return CUnknown::GetOwner()->QueryInterface(riid,ppv);            
    }                                                         
    STDMETHODIMP_(ULONG) AddRef() {                             
        return CUnknown::GetOwner()->AddRef();                            
    }                                                          
    STDMETHODIMP_(ULONG) Release() {                            
        return CUnknown::GetOwner()->Release();                           
    }

	STDMETHODIMP_(ULONG) NonDelegatingRelease();
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    // Override these virtual methods
    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
    virtual HRESULT OnActivate();
    virtual HRESULT OnDeactivate();
    virtual HRESULT OnApplyChanges();
	virtual void SetDirty();

	virtual BOOL PreTranslateMessage(MSG *pMsg);

    // These implement an IPropertyPage interface
    STDMETHODIMP SetPageSite(LPPROPERTYPAGESITE pPageSite);
    STDMETHODIMP Activate(HWND hwndParent, LPCRECT prect,BOOL fModal);
    STDMETHODIMP Deactivate();
    STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);
    STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN *ppUnk);
    STDMETHODIMP Show(UINT nCmdShow);
    STDMETHODIMP Move(LPCRECT prect);
    STDMETHODIMP IsPageDirty();
    STDMETHODIMP Apply();
    STDMETHODIMP Help(LPCWSTR lpszHelpDir);
    STDMETHODIMP TranslateAccelerator(LPMSG lpMsg);

};
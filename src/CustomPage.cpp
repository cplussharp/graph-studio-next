//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

//-----------------------------------------------------------------------------
//
//	CDSPropertyPage class
//
//-----------------------------------------------------------------------------

CDSPropertyPage::CDSPropertyPage(LPCTSTR pName, LPUNKNOWN pUnk, int DialogId, LPCTSTR title) :
    CUnknown(pName,pUnk),
	CDialog(),
	page_site(NULL),
	hwnd_page(NULL),
	dirty(FALSE),
	object_set(FALSE),
	dialog_id(DialogId)
{
	this->title = CString(title);
}

CDSPropertyPage::~CDSPropertyPage()
{
}

BOOL CDSPropertyPage::PreTranslateMessage(MSG *pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_ESCAPE) {
			// block ESC keypresses
			return TRUE;
		} else if (pMsg->wParam == VK_TAB						
				&& (0x8000 & GetKeyState(VK_CONTROL))			
				&& !(0x8000 & GetKeyState(VK_MENU))) {
			// Process Ctrl+Tab and Ctrl+Shift+Tab in parent property form for tab switching
			CWnd * const parent = GetParent();
			if (parent)
				return parent->PreTranslateMessage(pMsg);
		}
	} 
	return __super::PreTranslateMessage(pMsg);
}

STDMETHODIMP_(ULONG) CDSPropertyPage::NonDelegatingAddRef()
{
    LONG lRef = InterlockedIncrement(&m_cRef);
    ASSERT(lRef > 0);
    return max(ULONG(m_cRef),1ul);
}

STDMETHODIMP_(ULONG) CDSPropertyPage::NonDelegatingRelease()
{
    LONG lRef = InterlockedDecrement(&m_cRef);
    if (lRef == 0) {
        m_cRef++;
        SetPageSite(NULL);
        SetObjects(0, NULL);
        delete this;
        return ULONG(0);
    } else {
        return max(ULONG(lRef),1ul);
    }
}

STDMETHODIMP CDSPropertyPage::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (riid == IID_IPropertyPage) {
		return ::GetInterface(static_cast<IPropertyPage *>(this),ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid,ppv);
    }
}

STDMETHODIMP CDSPropertyPage::GetPageInfo(LPPROPPAGEINFO pPageInfo)
{
    WCHAR	wszTitle[STR_MAX_LENGTH];
	memset(wszTitle, 0, sizeof(wszTitle));
	lstrcat(wszTitle, title.GetBuffer());

    // Allocate dynamic memory for the property page title
    LPOLESTR pszTitle;
    HRESULT hr = AMGetWideString(wszTitle, &pszTitle);
    if (FAILED(hr)) return hr;
    
    pPageInfo->cb               = sizeof(PROPPAGEINFO);
    pPageInfo->pszTitle         = pszTitle;
    pPageInfo->pszDocString     = NULL;
    pPageInfo->pszHelpFile      = NULL;
    pPageInfo->dwHelpContext    = 0;

    // Set defaults in case GetDialogSize fails
    pPageInfo->size.cx          = 340;
    pPageInfo->size.cy          = 150;

	// let's retrieve the size
	HWND	hwnd_temp;
	hwnd_temp = ::CreateDialogParam(::AfxGetInstanceHandle(), 
								  MAKEINTRESOURCE(dialog_id), 
								  ::GetDesktopWindow(),
								  NULL,
								  0);
	if (hwnd_temp != NULL) {

		RECT	rc;
		rc.left = rc.right = rc.top = rc.bottom = 0;
		::GetWindowRect(hwnd_temp, &rc);

		// get the size
		pPageInfo->size.cx		= rc.right - rc.left;
		pPageInfo->size.cy		= rc.bottom - rc.top;

		// kill the window
		::DestroyWindow(hwnd_temp);
	}

    return NOERROR;
}

STDMETHODIMP CDSPropertyPage::SetObjects(ULONG cObjects, LPUNKNOWN *ppUnk)
{
    if (cObjects == 1) {
        if ((ppUnk == NULL) || (*ppUnk == NULL)) return E_POINTER;
        object_set = TRUE ;
        return OnConnect(*ppUnk);
    } else 
	if (cObjects == 0) {
        object_set = FALSE;
        return OnDisconnect();
    }
    return E_UNEXPECTED;
}

STDMETHODIMP CDSPropertyPage::Activate(HWND hwndParent, LPCRECT pRect, BOOL fModal)
{
    CheckPointer(pRect,E_POINTER);
    if (object_set == FALSE) return E_UNEXPECTED;

	// if we already exist - exit
	if (IsWindow(*this)) return E_UNEXPECTED;

	CWnd		*parent = CWnd::FromHandle(hwndParent);
	BOOL ok = Create(dialog_id, parent);

	if (!ok) return E_OUTOFMEMORY;

    OnActivate();
    Move(pRect);
    return Show(SW_SHOWNORMAL);
}


STDMETHODIMP CDSPropertyPage::Move(LPCRECT pRect)
{
    CheckPointer(pRect,E_POINTER);

	if (m_hWnd == NULL) return E_UNEXPECTED;

	MoveWindow(pRect, TRUE);
    return NOERROR;
}

STDMETHODIMP CDSPropertyPage::Show(UINT nCmdShow)
{
	if (m_hWnd == NULL) return E_UNEXPECTED;

    // Ignore wrong show flags
    if ((nCmdShow != SW_SHOW) && (nCmdShow != SW_SHOWNORMAL) && (nCmdShow != SW_HIDE)) {
        return E_INVALIDARG;
    }

    ShowWindow(nCmdShow);
    InvalidateRect(NULL,TRUE);
    return NOERROR;
}

STDMETHODIMP CDSPropertyPage::Deactivate()
{
	if (m_hWnd == NULL) return E_UNEXPECTED;

    // Remove WS_EX_CONTROLPARENT before DestroyWindow call
    DWORD dwStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
    dwStyle = dwStyle & (~WS_EX_CONTROLPARENT);
    SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyle);
    OnDeactivate();
    DestroyWindow();

    return NOERROR;
}

STDMETHODIMP CDSPropertyPage::SetPageSite(LPPROPERTYPAGESITE pPageSite)
{
    if (pPageSite) {
        if (page_site) return E_UNEXPECTED;

        page_site = pPageSite;
        page_site->AddRef();

    } else {
        if (page_site == NULL) return E_UNEXPECTED;

        page_site->Release();
        page_site = NULL;
    }
    return NOERROR;
}

STDMETHODIMP CDSPropertyPage::Apply()
{
    if (object_set == FALSE) return E_UNEXPECTED;
    if (page_site == NULL) return E_UNEXPECTED;
    if (dirty == FALSE) return NOERROR;

    HRESULT hr = OnApplyChanges();
    if (SUCCEEDED(hr)) {
        dirty = FALSE;
    }
    return hr;
}

HRESULT CDSPropertyPage::OnConnect(IUnknown *pUnknown)			{ return NOERROR; }
HRESULT CDSPropertyPage::OnDisconnect()						{ return NOERROR; }
HRESULT CDSPropertyPage::OnActivate()							{ return NOERROR; }
HRESULT CDSPropertyPage::OnDeactivate()						{ return NOERROR; }
HRESULT CDSPropertyPage::OnApplyChanges()						{ return NOERROR; }

STDMETHODIMP CDSPropertyPage::IsPageDirty() 
{ 
	return (dirty ? S_OK : S_FALSE); 
}

STDMETHODIMP CDSPropertyPage::TranslateAccelerator(LPMSG lpMsg) 
{ 
	return E_NOTIMPL; 
}

STDMETHODIMP CDSPropertyPage::Help(LPCWSTR lpszHelpDir)
{
	return E_NOTIMPL;
}

void CDSPropertyPage::SetDirty()
{
	dirty = TRUE;
    if (page_site) {
        page_site->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }
}






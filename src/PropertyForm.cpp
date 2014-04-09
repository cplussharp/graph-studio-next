//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "PropertyForm.h"


//-----------------------------------------------------------------------------
//
//	CPropertyForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CPropertyForm, CGraphStudioModelessDialog)
BEGIN_MESSAGE_MAP(CPropertyForm, CGraphStudioModelessDialog)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PAGES, &CPropertyForm::OnTabSelected)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CPropertyForm::OnBnClickedButtonApply)
	ON_MESSAGE(PSM_PRESSBUTTON, &CPropertyForm::OnPressButton)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CPropertyForm class
//
//-----------------------------------------------------------------------------

CPropertyForm::CPropertyForm(CWnd *pParent) : 
	CGraphStudioModelessDialog(CPropertyForm::IDD, pParent)
{
	container = NULL;
	object = NULL;
	filter = NULL;
}

CPropertyForm::~CPropertyForm()
{
	if (object) object->Release(); object = NULL;
}

void CPropertyForm::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_PAGES, tabs);
	DDX_Control(pDX, IDC_BUTTON_APPLY, button_apply);
	DDX_Control(pDX, IDOK, button_ok);
	DDX_Control(pDX, IDCANCEL, button_close);
}

void CPropertyForm::OnClose()
{
    view->graph.RefreshFilters();
    view->graph.Dirty();
    view->Invalidate();

    // report that we're being closed
    view->ClosePropertyPage(filter);
}

void CPropertyForm::OnOK()
{
	OnBnClickedButtonApply();
	OnClose();
}

void CPropertyForm::OnCancel()
{
	OnClose();
}

void CPropertyForm::OnDestroy()
{
	// destroy our pages
	if (container) {
		container->Clear();
		delete container;
	}
	if (object) object->Release(); object = NULL;
	if (filter) filter->Release(); filter = NULL;

	__super::OnDestroy();
}

void CPropertyForm::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (tabs.m_hWnd) {
		const int i = tabs.GetCurSel();
		if (container && i>=0 && i<container->pages.GetSize()) {

			const CSize page_size = GetFormSizeToFitPage(container->pages[i]->size);
			lpMMI->ptMinTrackSize.x = page_size.cx;
			lpMMI->ptMinTrackSize.y = page_size.cy;
			return;
		}
	}
	CGraphStudioModelessDialog::OnGetMinMaxInfo(lpMMI);
}

CSize CPropertyForm::GetFormSizeToFitPage(CSize size)
{
	// compute alignment helpers
	CRect	rc_client;
	CRect	rc_display;

	GetWindowRect(&rc_client);
	tabs.GetClientRect(&rc_display);
	tabs.AdjustRect(FALSE, &rc_display);

	int	dx = rc_client.Width() - rc_display.Width();
	int	dy = rc_client.Height()- rc_display.Height();

	return CSize(size.cx + dx, size.cy + dy);
}

void CPropertyForm::OnSize(UINT nType, int cx, int cy)
{
	if (IsWindow(tabs)) {
		tabs.SetWindowPos(NULL, tab_x, tab_y, cx-tab_cx, cy-tab_cy, SWP_SHOWWINDOW | SWP_NOZORDER);

		// now resize the page
		CRect	rc_client;
		tabs.GetClientRect(&rc_client);
		tabs.AdjustRect(FALSE, &rc_client);

		if (container && container->current>=0) {
			container->pages[container->current]->page->Move(rc_client);
		}

		button_ok.SetWindowPos(NULL, cx-bok_cx, cy-button_bottom_offset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		button_close.SetWindowPos(NULL, cx-bcancel_cx, cy-button_bottom_offset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		button_apply.SetWindowPos(NULL, cx-bapply_cx, cy-button_bottom_offset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		button_ok.Invalidate();
		button_close.Invalidate();
		button_apply.Invalidate();
	}
}

int CPropertyForm::DisplayPages(IUnknown *obj, IUnknown *filt, CString title, CGraphView *view)
{
	// create a new window
	BOOL bret = Create(IDD_DIALOG_PROPERTYPAGE, view);
	if (!bret) return -1;
	SetWindowText(title);

	// we need to set WS_EX_CONTROLPARENT for the tabs
	tabs.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	if (object) object->Release(); object = NULL;
	if (filter) filter->Release(); filter = NULL;

	object = obj;
	object->AddRef();
	this->view = view;

	if (filt) {
		filter = filt;
		filter->AddRef();
	}

	// compute alignment helpers
	CRect	rc_client;
	CRect	rc_display;

	GetWindowRect(&rc_client);

	CPoint	p1(0,0), p2(0,0);
	ClientToScreen(&p1);
	tabs.ClientToScreen(&p2);
	GetClientRect(&rc_client);
	tabs.GetWindowRect(&rc_display);
	tab_x = (p2.x-p1.x);		tab_y = (p2.y-p1.y);
	tab_cx = rc_client.Width() - rc_display.Width();
	tab_cy = rc_client.Height() - rc_display.Height();

	p2 = CPoint(0,0);
	button_ok.ClientToScreen(&p2);
	button_bottom_offset = p1.y + rc_client.Height() - p2.y;
	bok_cx = p1.x + rc_client.Width() - p2.x;

	p2 = CPoint(0,0);
	button_close.ClientToScreen(&p2);
	bcancel_cx = p1.x + rc_client.Width() - p2.x;

	p2 = CPoint(0,0);
	button_apply.ClientToScreen(&p2);
	bapply_cx = p1.x + rc_client.Width() - p2.x;


	// let's create a new container
	container = new CPageContainer(this);
	container->NonDelegatingAddRef();			// we may be exposing this object so make sure it won't go away

	int ret = AnalyzeObject(object);
	if (ret < 0) return -1;

	if (container->pages.GetCount() > 0) {
		container->ActivatePage(0);
	}

	// show page
	ShowWindow(SW_SHOW);
	return ret;
}

int CPropertyForm::AnalyzeObject(IUnknown *obj)
{
	CComPtr<ISpecifyPropertyPages>	specify;
	HRESULT							hr;

	CComPtr<IBaseFilter>			filter;
	hr = obj->QueryInterface(IID_IBaseFilter, (void**)&filter);
	if (FAILED(hr)) filter = NULL;

	CLSID		clsid;
	if (filter) {
		filter->GetClassID(&clsid);
	}

	bool		can_specify_pp = true;

	if (view->graph.is_remote) {

		// some filters don't like showing property page via remote connection
		if (clsid == CLSID_DSoundRender || clsid == CLSID_AudioRender) {
			can_specify_pp = false;
		}

	}

	if (can_specify_pp) {
		hr = obj->QueryInterface(IID_ISpecifyPropertyPages, (void**)&specify);
		if (SUCCEEDED(hr)) {
			CAUUID	pagelist;
			hr = specify->GetPages(&pagelist);
			if (SUCCEEDED(hr)) {

				// now create all pages
				for (int i=0; i<(int)pagelist.cElems; i++) {
					CComPtr<IPropertyPage>	page;
					hr = CoCreateInstance(pagelist.pElems[i], NULL, CLSCTX_INPROC_SERVER, IID_IPropertyPage, (void**)&page);
					if (SUCCEEDED(hr)) {
						// assign the object
						hr = page->SetObjects(1, &obj);
						if (SUCCEEDED(hr)) {
							// and add the page to our container
							container->AddPage(page);
						}
					}

					page = NULL;
				}

				// free used memory
				if (pagelist.pElems) CoTaskMemFree(pagelist.pElems);
			}
		}
		specify = NULL;
	}

	if (filter) {
		// display the filter details page
		HRESULT					hr;

		//---------------------------------------------------------------------
		//	Support for Video For Windows & ACM objects
		//---------------------------------------------------------------------
		CComPtr<IAMVfwCompressDialogs>		vfw_dialogs;
		if (SUCCEEDED(obj->QueryInterface(IID_IAMVfwCompressDialogs, (void**)&vfw_dialogs))) {
			AddPropertyPage(new CFilterVCMPage(NULL, &hr, _T("VFW Dialogs")), obj);
		}
		vfw_dialogs = NULL;

        // Internal Property Pages
        CComQIPtr<IStreamBufferSink> sbeSink = obj;
        if(sbeSink)
            AddPropertyPage(new CSbeSinkPage(NULL, &hr, _T("SbeSink")), obj);

        CComQIPtr<IAnalyzerCommon> analyzer = obj;
        if(analyzer)
            AddPropertyPage(new CAnalyzerPage(NULL, &hr, _T("Analyzer")), obj);

        // Filter Details
        AddPropertyPage(new CFilterDetailsPage(NULL, &hr), obj);

		// check for DMO pages
		AnalyzeDMO(obj);

        // Interfaces
        LoadInterfacePage(obj, TEXT("Interfaces"));

        // MediaInfo
        if(view->render_params.use_media_info)
            LoadMediaInfoPage(obj);

        // Property Pages for some selected interfaces
        LoadCustomInterfacePropertyPages(obj);

		// let's enumerate all pins
		CComPtr<IEnumPins>		epins;
		hr = filter->EnumPins(&epins);
		if (SUCCEEDED(hr)) {
			epins->Reset();

			ULONG			f;
			CComPtr<IPin>	pin;
			while (epins->Next(1, &pin, &f) == NOERROR) {
				LoadPinPage(pin);
				pin = NULL;
			}
			epins = NULL;
		}

	}

	CComPtr<IPin>	pin;
	if (SUCCEEDED(obj->QueryInterface(IID_IPin, (void**)&pin))) {
		LoadPinPage(pin);
		pin = NULL;
        LoadInterfacePage(obj, TEXT("Interfaces"));
	}
	
	filter = NULL;
	return 0;
}

int CPropertyForm::AnalyzeDMO(IUnknown *obj)
{
	CComPtr<IMediaObject>			dmo;
	HRESULT							hr;

	hr = obj->QueryInterface(IID_IMediaObject, (void**)&dmo);
	if (FAILED(hr)) return -1;

	// fine - it's a DMO object. now check what kind of it is.
	int						i = 0;
	int						dmo_type = -1;			// unknown
	DMO_MEDIA_TYPE			dmt;
	memset(&dmt, 0, sizeof(dmt));

	// first check if it is connected
	hr = dmo->GetInputCurrentType(0, &dmt);
	if (FAILED(hr)) {
	
		// or try to enumerate through the types
		while (dmo_type == -1 && hr != DMO_E_NO_MORE_ITEMS) {
			hr = dmo->GetInputType(0, i++, &dmt);
			if (hr == NOERROR) {

				// currently we're only interested in WMA Decoder
				if (dmt.majortype == MEDIATYPE_Audio &&
					(
						dmt.subtype		== GraphStudio::MEDIASUBTYPE_WMA9_00 ||
						dmt.subtype		== GraphStudio::MEDIASUBTYPE_WMA9_01 ||
						dmt.subtype		== GraphStudio::MEDIASUBTYPE_WMA9_02 ||
						dmt.subtype		== GraphStudio::MEDIASUBTYPE_WMA9_03
					)
					){
					dmo_type = 0;			// WMA
				}
			}
			MoFreeMediaType(&dmt);
		}
	} else {

		// currently we're only interested in WMA Decoder
		if (dmt.majortype == MEDIATYPE_Audio &&
			(
				dmt.subtype		== GraphStudio::MEDIASUBTYPE_WMA9_00 ||
				dmt.subtype		== GraphStudio::MEDIASUBTYPE_WMA9_01 ||
				dmt.subtype		== GraphStudio::MEDIASUBTYPE_WMA9_02 ||
				dmt.subtype		== GraphStudio::MEDIASUBTYPE_WMA9_03
			)
			){
			dmo_type = 0;			// WMA
		}

		MoFreeMediaType(&dmt);
	}

	switch (dmo_type) {
	case 0:
		AddPropertyPage(new CWMADecPage(NULL, &hr, _T("WMA Decoder")), obj);
		break;
	}

    // Check if Resizer DMO
    CComPtr<IWMResizerProps> resizer;
    hr = obj->QueryInterface(IID_IWMResizerProps, (void**)&resizer);
	if (SUCCEEDED(hr) && resizer)
		AddPropertyPage(new CWMResizerPage(NULL, &hr, _T("IWMResizer")), obj);

    // Check if it has IDMOQualityControl
    CComPtr<IDMOQualityControl> qualctrl;
    hr = obj->QueryInterface(IID_IDMOQualityControl, (void**)&qualctrl);
	if (SUCCEEDED(hr) && qualctrl)
		AddPropertyPage(new CDMOQualCtrlPage(NULL, &hr, _T("DMOQualCtrl")), obj);

	return 0;
}

void CPropertyForm::OnTabSelected(NMHDR *pNMHDR, LRESULT *pResult)
{
	int i = tabs.GetCurSel();
	if (container) container->ActivatePage(i);
	*pResult = 0;
}


void CPropertyForm::OnBnClickedButtonApply()
{
	if (container->current != -1) {
		CPageSite *site = container->pages[container->current];
		if (site->page) {
			// NOTE: A property page's Apply might or otherwise the page might indicate dirtiness again, so we need to let Apply button stay enabled if needed
			button_apply.EnableWindow(FALSE);
			site->page->Apply();
			return;
		}
	}
	button_apply.EnableWindow(FALSE);
}

LRESULT CPropertyForm::OnPressButton(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case PSBTN_OK:
		if(button_ok.IsWindowEnabled())
			PostMessage(WM_COMMAND, MAKEWPARAM(button_ok.GetDlgCtrlID(), BN_CLICKED));
		break;
	case PSBTN_CANCEL:
		PostMessage(WM_COMMAND, MAKEWPARAM(button_close.GetDlgCtrlID(), BN_CLICKED));
		break;
	case PSBTN_APPLYNOW:
		if(button_apply.IsWindowEnabled())
			PostMessage(WM_COMMAND, MAKEWPARAM(button_apply.GetDlgCtrlID(), BN_CLICKED));
		break;
	}
	return 0;
}

int CPropertyForm::LoadPinPage(IPin *pin)
{
	PIN_INFO		info;
	pin->QueryPinInfo(&info);
	if (info.pFilter) 
		info.pFilter->Release();

	CString		title(info.achName);

	// display the filter details page
	HRESULT					hr;

	AddPropertyPage(new CPinDetailsPage(NULL, &hr, title), (IUnknown*)pin);
	
	//---------------------------------------------------------------------
	//
	//	Support for Buffer Negotiation
	//
	//---------------------------------------------------------------------
	CComPtr<IAMBufferNegotiation>		buf_neg;
	if (SUCCEEDED(pin->QueryInterface(IID_IAMBufferNegotiation, (void**)&buf_neg))) {
		AddPropertyPage(new CBufferNegotiationPage(NULL, &hr, _T("Latency")), (IUnknown*)pin);
	}
	buf_neg = NULL;


	//---------------------------------------------------------------------
	//
	//	Support for Video For Windows
	//
	//---------------------------------------------------------------------
	CComPtr<IAMVideoCompression>		vfw_comp;
	if (SUCCEEDED(pin->QueryInterface(IID_IAMVideoCompression, (void**)&vfw_comp))) {
        AddPropertyPage(new CVideoCompressionPage(NULL, &hr, _T("Video Compression")), (IUnknown*)pin);
	}
	vfw_comp = NULL;	

	//-------------------------------------------------------------------------
	//
	//	Support for ACM audio codecs
	//
	//-------------------------------------------------------------------------
	CComPtr<IAMStreamConfig>	stream_config;
	if (SUCCEEDED(pin->QueryInterface(IID_IAMStreamConfig, (void**)&stream_config))) {
		
		// if the parent filter is the ACM Wrapper Filter then we will show
		// the ACM Compression page
		PIN_INFO	info;
		bool		is_acm_wrapper = false;

		memset(&info, 0, sizeof(info));

		pin->QueryPinInfo(&info);
		if (info.pFilter) {
			CLSID	clsid;
			info.pFilter->GetClassID(&clsid);

			if (clsid == CLSID_ACMWrapper) {
				is_acm_wrapper = true;
			}

			info.pFilter->Release();
		}

		if (is_acm_wrapper) {

			// now add the ACM Compression page
			AddPropertyPage(new CAudioCompressionPage(NULL, &hr, _T("Audio Compression")), (IUnknown*)pin);		

		} else {
			// I'll think of some nice page later ...
		}
	}
	stream_config = NULL;

	return 0;
}

int CPropertyForm::LoadInterfacePage(IUnknown *obj, const CString& strTitle)
{
	// display the details page
    HRESULT hr;
	AddPropertyPage(new CInterfaceDetailsPage(NULL, &hr, strTitle), obj);

    return 0;
}

int CPropertyForm::LoadMediaInfoPage(IUnknown *obj)
{
	// display the details page
    CComQIPtr<IFileSourceFilter> pI = obj;
    if(pI)
    {
        LPOLESTR strFile = NULL;
        CMediaType media_type;
        HRESULT hr = pI->GetCurFile(&strFile, &media_type);
        if(hr == S_OK && strFile != NULL)
            AddPropertyPage(CMediaInfoPage::CreateInstance(NULL, &hr, CString(strFile)), obj);

        if(strFile)
            CoTaskMemFree(strFile);
    }

    return 0;
}

void CPropertyForm::LoadCustomInterfacePropertyPages(IUnknown *obj)
{
    if(!obj) return;
    // display the details page
	HRESULT					hr;

    CComPtr<IAMExtendedSeeking> extseek;
    if(SUCCEEDED(obj->QueryInterface(IID_IAMExtendedSeeking, (void**)&extseek)))
        AddPropertyPage(new CAMExtendedSeekingPage(NULL, &hr, TEXT("Marker")), obj);

    CComQIPtr<ITuner> tuner = obj;
    if(tuner)
        AddPropertyPage(new CTunerInfoPage(NULL, &hr, TEXT("TunerInfo")), obj);
}

void CPropertyForm::AddPropertyPage(CDSPropertyPage *prop_page, IUnknown *obj)
{
    if (prop_page && obj) {
		prop_page->AddRef();

        CComPtr<IPropertyPage>	page;
		HRESULT hr = prop_page->QueryInterface(IID_IPropertyPage, (void**)&page);
		if (SUCCEEDED(hr)) {
			// assign the object
			hr = page->SetObjects(1, &obj);
			if (SUCCEEDED(hr)) {
				// and add the page to our container
				container->AddPage(page);
			}
		}
		page = NULL;

		// don't care anymore
		prop_page->Release();
	}
}

BOOL CPropertyForm::PreTranslateMessage(MSG *pMsg)
{
	if (pMsg && pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_TAB && container) {
			if ((0x8000 & GetKeyState(VK_CONTROL)) && !(0x8000 & GetKeyState(VK_MENU))) {	// control-tab to change page
				const int numPages = container->pages.GetCount();
				if (numPages > 1) {
					const int increment = (0x8000 & GetKeyState(VK_SHIFT)) ? -1 : 1;		// added shift iterates backwards
					int newPage = tabs.GetCurSel() + increment;

					if (newPage < 0)
						newPage = numPages - 1;
					else if (newPage >= numPages)
						newPage = 0;
					container->ActivatePage(newPage);
				}
				return TRUE;
			}
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

//-----------------------------------------------------------------------------
//
//	CPageContainer class
//
//-----------------------------------------------------------------------------

CPageContainer::CPageContainer(CPropertyForm *parent) :
	CUnknown(NAME("Page Container"), NULL),
	form(parent)
{
	// we don't have any pages yet
	pages.RemoveAll();
	current = -1;
}

CPageContainer::~CPageContainer()
{
	Clear();
}

void CPageContainer::Clear()
{
	if (current != -1) DeactivatePage(current);

	// release all pages
	for (int i=0; i<pages.GetCount(); i++) {
		CPageSite	*site = pages[i];

		// make it go away
		site->CloseSite();
		delete site;
	}
	pages.RemoveAll();
	current = -1;

	// kick all pages in the tab control
	form->tabs.DeleteAllItems();
}

void CPageContainer::ActivatePage(int i)
{
	if (i == current) return ;

	CPageSite	*site = pages[i];

	if (current != -1) DeactivatePage(current);
	current = i;

	// select specified tab
	form->tabs.SetCurSel(i);

	// resize parent
	const CSize formSize = form->GetFormSizeToFitPage(site->size);
	form->SetWindowPos(NULL, 0, 0, formSize.cx, formSize.cy, SWP_NOMOVE | SWP_NOZORDER);

	// now display the page
	CRect	rc_client;
	form->tabs.GetClientRect(&rc_client);
	form->tabs.AdjustRect(FALSE, &rc_client);

	// show the page
	site->Activate(form->tabs, rc_client);

	form->Invalidate();
}

void CPageContainer::DeactivatePage(int i)
{
	CPageSite	*site = pages[i];
	site->Deactivate();
	current = -1;
}

int CPageContainer::AddPage(IPropertyPage *page)
{
	// get some info first...
	PROPPAGEINFO	info;
	memset(&info, 0, sizeof(info));
	info.cb = sizeof(info);

	HRESULT	hr = page->GetPageInfo(&info);
	if (FAILED(hr)) return -1;

	CString	title = CString(info.pszTitle);
	CSize	size = info.size;

	// release buffers...
	if (info.pszTitle) CoTaskMemFree(info.pszTitle);
	if (info.pszDocString) CoTaskMemFree(info.pszDocString);
	if (info.pszHelpFile) CoTaskMemFree(info.pszHelpFile);

	// create site object for this page
	CPageSite	*site = new CPageSite(NULL, this);
	site->NonDelegatingAddRef();					// keep an extra reference
	site->AssignPage(page);
	site->title = title;
	site->size = size;

	// insert into tab control
	form->tabs.InsertItem((int) pages.GetCount(),title); 

	// we're done
	pages.Add(site);

	return 0;
}

//-----------------------------------------------------------------------------
//
//	CPageSite class
//
//-----------------------------------------------------------------------------

CPageSite::CPageSite(LPUNKNOWN pUnk, CPageContainer *container) :
	CUnknown(NAME("Page Site"), pUnk),
	parent(container),
	title(_T(""))
{
	page = NULL;
	size.cx = 0;
	size.cy = 0;
	active = false;
}

CPageSite::~CPageSite()
{
	page = NULL;
}

int CPageSite::CloseSite()
{
	if (page) {
		if (active) Deactivate();
		page = NULL;
	}
	return 0;
}

int CPageSite::AssignPage(IPropertyPage *page)
{
	// keep a reference
	page->QueryInterface(IID_IPropertyPage, (void**)&this->page);

	// pair the site object with page
	page->SetPageSite(this);
	return 0;
}

STDMETHODIMP CPageSite::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IPropertyPageSite) {
		return GetInterface((IPropertyPageSite*)this, ppv);
	} else
		return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CPageSite::OnStatusChange(DWORD dwFlags)
{
	if (dwFlags & PROPPAGESTATUS_DIRTY) {
		parent->form->button_apply.EnableWindow(TRUE);
	}
	return NOERROR;
}

STDMETHODIMP CPageSite::GetLocaleID(LCID *pLocaleID)
{
	return E_FAIL;
}

STDMETHODIMP CPageSite::GetPageContainer(IUnknown **ppUnk)
{
	return parent->NonDelegatingQueryInterface(IID_IUnknown, (void**)ppUnk);
}

STDMETHODIMP CPageSite::TranslateAccelerator(MSG *pMsg)
{
	return NOERROR;
}

int CPageSite::Deactivate()
{
	if (page) {
		page->Show(SW_HIDE);
		page->Deactivate();
		active = false;
	}
	return 0;
}

int CPageSite::Activate(HWND owner, CRect &rc)
{
	parent->form->button_apply.EnableWindow(FALSE);
	if (page) {
		HRESULT hr = page->Activate(owner, &rc, FALSE);
		if (FAILED(hr)) return -1;
		page->Show(SW_SHOW);
		active = true;
	}
	return 0;
}

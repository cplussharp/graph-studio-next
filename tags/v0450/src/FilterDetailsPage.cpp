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
//	CDetailsPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CDetailsPage, CDSPropertyPage)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CDetailsPage class
//
//-----------------------------------------------------------------------------
CDetailsPage::CDetailsPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("FilterDetails"), pUnk, IDD, strTitle),
	info(_T("root"))
{
	// retval
	if (phr) *phr = NOERROR;

}

CDetailsPage::~CDetailsPage()
{
	// todo
}


// overriden
BOOL CDetailsPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// create the tree
	CRect	rc;
	GetClientRect(&rc);

	ok = tree.Create(NULL, WS_CHILD | WS_VISIBLE, rc, this, IDC_TREE);
	if (!ok) return FALSE;

	info.Clear();
	OnBuildTree();

	tree.Initialize();
	tree.BuildPropertyTree(&info);

	return TRUE;
}

void CDetailsPage::OnBuildTree()
{
}

void CDetailsPage::OnSize(UINT nType, int cx, int cy)
{
	if (IsWindow(tree)) tree.MoveWindow(0, 0, cx, cy);
}



//-----------------------------------------------------------------------------
//
//	CFilterDetailsPage class
//
//-----------------------------------------------------------------------------

CFilterDetailsPage::CFilterDetailsPage(LPUNKNOWN pUnk, HRESULT *phr) :
	CDetailsPage(pUnk, phr, _T("Filter")),
	filter(NULL)
{
	// retval
	if (phr) *phr = NOERROR;

}

CFilterDetailsPage::~CFilterDetailsPage()
{
	// todo
}

HRESULT CFilterDetailsPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IBaseFilter, (void**)&filter);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CFilterDetailsPage::OnDisconnect()
{
	filter = NULL;
	return NOERROR;
}

void CFilterDetailsPage::OnBuildTree()
{
	GraphStudio::PropItem	*group;
	GraphStudio::Filter		gfilter(NULL);

	gfilter.LoadFromFilter(filter);

	group = info.AddItem(new GraphStudio::PropItem(_T("Filter Details")));
		CString	type;
		switch (gfilter.filter_type) {
		case GraphStudio::Filter::FILTER_DMO:		type = _T("DMO"); break;
		case GraphStudio::Filter::FILTER_WDM:		type = _T("WDM"); break;
		case GraphStudio::Filter::FILTER_STANDARD:	type = _T("Standard"); break;
		case GraphStudio::Filter::FILTER_UNKNOWN:	type = _T("Unknown"); break;
		}	
		group->AddItem(new GraphStudio::PropItem(_T("Type"), type));
		GraphStudio::GetFilterDetails(gfilter.clsid, group);
}

//-----------------------------------------------------------------------------
//
//	CPinDetailsPage class
//
//-----------------------------------------------------------------------------

CPinDetailsPage::CPinDetailsPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDetailsPage(pUnk, phr, strTitle),
	pin(NULL)
{
	// retval
	if (phr) *phr = NOERROR;

}

CPinDetailsPage::~CPinDetailsPage()
{
	// todo
}

HRESULT CPinDetailsPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IPin, (void**)&pin);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CPinDetailsPage::OnDisconnect()
{
	pin = NULL;
	return NOERROR;
}

void CPinDetailsPage::OnBuildTree()
{
	//GraphStudio::PropItem	*group;
	GraphStudio::Pin		gpin(NULL);
	int						ret;

	ret = gpin.Load(pin);
	if (ret == 0) {
		GetPinDetails(gpin.pin, &info);
	}
}



//-----------------------------------------------------------------------------
//
//	CInterfaceDetailsPage class
//
//-----------------------------------------------------------------------------

CInterfaceDetailsPage::CInterfaceDetailsPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDetailsPage(pUnk, phr, strTitle),
	pInterfaces(NULL)
{
	// retval
	if (phr) *phr = NOERROR;

}

CInterfaceDetailsPage::~CInterfaceDetailsPage()
{
	if(pInterfaces) delete pInterfaces;
}

HRESULT CInterfaceDetailsPage::OnConnect(IUnknown *pUnknown)
{
    pInterfaces = new CInterfaceScanner(pUnknown);
	return NOERROR;
}

HRESULT CInterfaceDetailsPage::OnDisconnect()
{
    if(pInterfaces)
    {
        delete pInterfaces;
        pInterfaces = NULL;
    }
	return NOERROR;
}

void CInterfaceDetailsPage::OnBuildTree()
{
	pInterfaces->GetDetails(&info);
}

//-----------------------------------------------------------------------------
//
//	CMediaInfoPage class
//
//-----------------------------------------------------------------------------

CMediaInfoPage* CMediaInfoPage::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR pszFile)
{
    CMediaInfo* info = CMediaInfo::GetInfoForFile(pszFile);
    if(info == NULL) return NULL;
    return new CMediaInfoPage(pUnk, phr, info);
}

CMediaInfoPage::CMediaInfoPage(LPUNKNOWN pUnk, HRESULT *phr, CMediaInfo* pInfo) :
	CDetailsPage(pUnk, phr, _T("MediaInfo")),
	m_pInfo(pInfo)
{
	// retval
	if (phr) *phr = NOERROR;

}

CMediaInfoPage::~CMediaInfoPage()
{
}

HRESULT CMediaInfoPage::OnConnect(IUnknown *pUnknown)
{
	return NOERROR;
}

HRESULT CMediaInfoPage::OnDisconnect()
{
	return NOERROR;
}

void CMediaInfoPage::OnBuildTree()
{
	m_pInfo->GetDetails(&info);
}


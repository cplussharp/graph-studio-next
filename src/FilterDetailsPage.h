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
//	CDetailsPage class
//
//-----------------------------------------------------------------------------
class CDetailsPage : public CDSPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

public:

	GraphStudio::PropertyTree		tree;
	GraphStudio::PropItem			info;

	enum { IDD = IDD_DIALOG_FILTERDETAILS };
public:
	CDetailsPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CDetailsPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void OnBuildTree();

	void OnSize(UINT nType, int cx, int cy);
};


//-----------------------------------------------------------------------------
//
//	CFilterDetailsPage class
//
//-----------------------------------------------------------------------------
class CFilterDetailsPage : public CDetailsPage
{
public:
	CComPtr<IBaseFilter>			filter;

public:
	CFilterDetailsPage(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CFilterDetailsPage();
	
    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual void OnBuildTree();

};

//-----------------------------------------------------------------------------
//
//	CPinDetailsPage class
//
//-----------------------------------------------------------------------------
class CPinDetailsPage : public CDetailsPage
{
public:
	CComPtr<IPin>			pin;

public:
	CPinDetailsPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CPinDetailsPage();
	
    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual void OnBuildTree();

};



//-----------------------------------------------------------------------------
//
//	CInterfaceDetailsPage class
//
//-----------------------------------------------------------------------------
class CInterfaceDetailsPage : public CDetailsPage
{
public:
	CInterfaceScanner*			pInterfaces;

public:
	CInterfaceDetailsPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CInterfaceDetailsPage();
	
    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual void OnBuildTree();
};


//-----------------------------------------------------------------------------
//
//	CMediaInfoPage class
//
//-----------------------------------------------------------------------------
class CMediaInfoPage : public CDetailsPage
{
protected:
	CMediaInfo*			m_pInfo;
    CMediaInfoPage(LPUNKNOWN pUnk, HRESULT *phr, CMediaInfo* pInfo);

public:
	static CMediaInfoPage* CreateInstance(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR pszFile, VARIANT_BOOL useCache);
	virtual ~CMediaInfoPage();
	
    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual void OnBuildTree();
};


//-----------------------------------------------------------------------------
//
//	CAMExtendedSeekingPage class
//
//-----------------------------------------------------------------------------
class CAMExtendedSeekingPage : public CDetailsPage
{
public:
	CComPtr<IAMExtendedSeeking>   		filter;

public:
	CAMExtendedSeekingPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CAMExtendedSeekingPage();
	
    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual void OnBuildTree();
};


//-----------------------------------------------------------------------------
//
//	CTunerInfoPage class
//
//-----------------------------------------------------------------------------
class CTunerInfoPage : public CDetailsPage
{
public:
	CComQIPtr<ITuner>   		tuner;

public:
	CTunerInfoPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CTunerInfoPage();
	
    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual void OnBuildTree();

    static void GetLocatorInfo(CComPtr<ILocator> loc, GraphStudio::PropItem* info);
};

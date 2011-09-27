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








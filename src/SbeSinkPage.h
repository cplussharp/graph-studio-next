//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once
#include "afxwin.h"

//-----------------------------------------------------------------------------
//
//	CSbeSinkPage class
//
//-----------------------------------------------------------------------------
class CSbeSinkPage : public CDSPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

public:

	GraphStudio::TitleBar				title;

	CComPtr<IStreamBufferSink>			filter;
    bool                                isActiv;

    enum { IDD = IDD_PROPPAGE_STREAMBUFFERSINK };
public:
	CSbeSinkPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CSbeSinkPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnSize(UINT nType, int cx, int cy);

    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual HRESULT OnActivate();
	virtual HRESULT OnApplyChanges();
};


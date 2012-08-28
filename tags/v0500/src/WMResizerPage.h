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
//	CWMResizerPage class
//
//-----------------------------------------------------------------------------
class CWMResizerPage : public CDSPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

public:

	GraphStudio::TitleBar				title;

	CComPtr<IWMResizerProps>   			dmo;
    bool                                isActiv;

    enum { IDD = IDD_PROPPAGE_WMRESIZER };
public:
	CWMResizerPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CWMResizerPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCheckOrRadioClick();
    afx_msg void OnChangeEdit();

    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual HRESULT OnActivate();
	virtual HRESULT OnApplyChanges();
};


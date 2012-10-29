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
//	CAnalyzerPage class
//
//-----------------------------------------------------------------------------
class CAnalyzerPage : public CDSPropertyPage //, public IAnalyzerFilterCallback
{
protected:
	DECLARE_MESSAGE_MAP()

    void FreeEntryData(StatisticRecordEntry &entry) const;
    const CString GetEntryString(__int64 entryNr, int field) const;

public:
	GraphStudio::TitleBar				title;

	CComPtr<IAnalyzerFilter>   	        filter;
    bool                                isActiv;
    CListCtrl                           m_listCtrl;
    __int64                             m_firstTimeStamp;

    enum { IDD = /*IDD_DIALOG_WMADECODER*/ IDD_PROPPAGE_ANALYZER };
public:
	CAnalyzerPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CAnalyzerPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCheckClick();

    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual HRESULT OnActivate();
	virtual HRESULT OnApplyChanges();
    afx_msg void OnBnClickedButtonReset();
    afx_msg void OnLvnGetdispinfoListData(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedButtonRefresh();
    afx_msg void OnBnClickedButtonSave();
};


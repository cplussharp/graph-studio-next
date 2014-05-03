//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once
//#include "afxwin.h"

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
    const CString GetEntryString(__int64 entryNr, int field, bool commaFormattedTimestamps) const;
	static CString FormatSetPositionsFlags(DWORD flags);

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

	enum Columns
	{
		Number = 0,				
		TimeStamp,
        TimeStampDif,
		Kind,
		Discontinuity,
		Sync,
		Preroll,
		Start,
		Stop,
		MediaStart,
		MediaStop,
		TypeSpecificFlags,
		SampleFlags,
		StreamID,
		DataLength,
        DataCrc,
		Data,
		NumColumns			// Must be last member of enum
	};


public:
	GraphStudio::TitleBar				title;

	CComPtr<IAnalyzerCommon>   	        filter;
    bool                                isActiv;
    CListCtrl                           m_listCtrl;
	CRect								m_listCtrlBorder;
    CSpinButtonCtrl                     m_spinPreviewByteCount;
    DWORD                               m_nPreviewByteCount;
    __int64                             m_firstTimeStamp;
    CFont			                    font_entries;

    enum { IDD = /*IDD_DIALOG_WMADECODER*/ IDD_PROPPAGE_ANALYZER };
public:

    static const CFactoryTemplate g_Template;

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
    afx_msg void OnCustomDrawListData(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedButtonRefresh();
    afx_msg void OnBnClickedButtonSave();
    afx_msg void OnSpinDeltaByteCount(NMHDR* pNMHDR, LRESULT* pResult);
};


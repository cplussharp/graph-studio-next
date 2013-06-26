//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

class CGraphView;

struct GraphStatisticEntry
{
    long lIndex;
    BSTR szName;
    long lCount;
    double dLast;
    double dAverage;
    double dStdDev;
    double dMin;
    double dMax;
};

//-----------------------------------------------------------------------------
//
//	CStatisticForm class
//
//-----------------------------------------------------------------------------
class CStatisticForm : public CGraphStudioModelessDialog
{
protected:
	DECLARE_DYNAMIC(CStatisticForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    const CString CStatisticForm::GetEntryString(LONG entryNr, int field);
    GraphStatisticEntry* GetEntry(LONG entryNr);

    void FreeCachedStatisticEntry();
    GraphStatisticEntry m_cachedEntry;

public:
	GraphStudio::TitleBar	title;
    CButton					btn_reset;
	CButton					btn_export;
    CListCtrl               m_listCtrl;

public:
	CStatisticForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStatisticForm();

	enum { IDD = IDD_DIALOG_STATISTIC };

	// initialization
	BOOL DoCreateDialog();
	void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnTimer(UINT_PTR id);
	afx_msg void OnResetClick();
    afx_msg void OnExportClick();
    afx_msg void OnLvnGetdispinfoListData(NMHDR *pNMHDR, LRESULT *pResult);
};

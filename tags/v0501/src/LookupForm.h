//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once
#include "afxwin.h"

struct SortStruct { // this just keeps all the sorting data together so you don't have to initialise it in the main constructor.
    int  SortCol;
    bool Ascending;
    SortStruct() : SortCol(0), Ascending(true) {}
    void SetCol(int Col) {Ascending=(SortCol==Col ? !Ascending : true); SortCol=Col;}
};

//-----------------------------------------------------------------------------
//
//	CLookupForm class
//
//-----------------------------------------------------------------------------

class CLookupForm : public CDialog
{
	DECLARE_DYNAMIC(CLookupForm)

public:
	CLookupForm(CWnd* pParent = NULL, BOOL forHR = FALSE);
	virtual ~CLookupForm();

	enum { IDD = IDD_DIALOG_LOOKUP };
    BOOL DoCreateDialog();
    void OnInitialize();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

    SortStruct sortData;

public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedButtonSearch();

    GraphStudio::TitleBar	m_title;
    CListCtrl m_listCtrl;
    CButton m_btnSearch;
    CEdit m_editSearch;

    BOOL m_isHR;
    afx_msg void OnLvnColumnclickListLookup(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLvnItemchangedListLookup(NMHDR *pNMHDR, LRESULT *pResult);
    virtual void OnOK();
};

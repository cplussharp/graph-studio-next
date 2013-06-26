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
//	CBlacklistForm class
//
//-----------------------------------------------------------------------------

class CBlacklistForm : public CGraphStudioModelessDialog
{
	DECLARE_DYNAMIC(CBlacklistForm)

public:
	CBlacklistForm(CWnd* pParent = NULL, BOOL forHR = FALSE);
	virtual ~CBlacklistForm();

	enum { IDD = IDD_DIALOG_BLACKLIST };
    BOOL DoCreateDialog();
    void OnInitialize();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedButtonAdd();
    afx_msg void OnBnClickedButtonRemove();
    afx_msg void OnBnClickedButtonImport();
    afx_msg void OnBnClickedButtonExport();

    GraphStudio::TitleBar	m_title;
    CListCtrl m_listCtrl;
    CButton m_btnAdd;
    CButton m_btnRemove;
    CButton m_btnImport;
    CButton m_btnExport;
    CEdit m_editEntry;

    BOOL m_isHR;
};

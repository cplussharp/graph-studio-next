//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : cplussharp
//
//-----------------------------------------------------------------------------
#pragma once
#include "afxcmn.h"

class CGraphView;


//-----------------------------------------------------------------------------
//
//	CPageFileTypes class
//
//-----------------------------------------------------------------------------
class CPageFileTypes : public CDialog
{
protected:
	DECLARE_MESSAGE_MAP()

public:
	GraphStudio::PropertyTree	tree;
	GraphStudio::PropItem		info;
    CString                     title;

    enum { IDD = IDD_PROPPAGE_FILETYPES };

public:
	CPageFileTypes(CString title);
	virtual ~CPageFileTypes();
	
	// overriden
	virtual BOOL OnInitDialog();

    void UpdateTree();

	void OnSize(UINT nType, int cx, int cy);
};


//-----------------------------------------------------------------------------
//
//	CFileTypesForm class
//
//-----------------------------------------------------------------------------
class CFileTypesForm : public CGraphStudioModelessDialog
{
protected:
	DECLARE_DYNAMIC(CFileTypesForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	GraphStudio::TitleBar	title;
	CButton			btn_reload;
	CButton			btn_copy;
    CTabCtrl        tab;

    CPageFileTypes*	page_protocols;
    CPageFileTypes*	page_extensions;
    CPageFileTypes*	page_bytes;
    CPageFileTypes*	page_mediaplayer;

    CPageFileTypes* pages[3];
    DWORD           pageCount;

public:
	CFileTypesForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileTypesForm();
	CRect GetDefaultRect() const;

	enum { IDD = IDD_DIALOG_FILETYPES };

	// initialization
	BOOL DoCreateDialog(CWnd* parent);
    void OnInitialize();

	void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnBnClickedButtonReload();
	afx_msg void OnBnClickedButtonCopy();
    afx_msg void OnTabChanged(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTabChanging(NMHDR *pNMHDR, LRESULT *pResult);
};

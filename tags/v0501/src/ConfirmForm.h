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
//	CConfirmDialog class
//
//-----------------------------------------------------------------------------
class CConfirmDialog : public CDialog
{
protected:
	DECLARE_DYNAMIC(CConfirmDialog)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);
public:

	CStatic					label_filter;
	CFont					font_filter;
	CString					filter_name;

public:
	CConfirmDialog(CWnd* pParent = NULL);   
	virtual ~CConfirmDialog();

	// Dialog Data
	enum { IDD = IDD_DIALOG_UNREGCONFIRM };

	virtual BOOL OnInitDialog();
    BOOL m_bUnregisterAll;
};


bool ConfirmUnregisterFilter(CString name, BOOL* pbUnregisterAll);

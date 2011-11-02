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
//	CMeritChangeDialog class
//
//-----------------------------------------------------------------------------
class CMeritChangeDialog : public CDialog
{
protected:
	DECLARE_DYNAMIC(CMeritChangeDialog)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);
public:

	CStatic					label_filter;
	CFont					font_filter;
	CString					filter_name;
	CEdit					edit_original;
	CComboBox				cb_newmerit;

	DWORD					old_merit;
	DWORD					new_merit;

public:
	CMeritChangeDialog(CWnd* pParent = NULL);   
	virtual ~CMeritChangeDialog();

	// Dialog Data
	enum { IDD = IDD_DIALOG_MERITCHANGE };

	virtual BOOL OnInitDialog();
	virtual void OnOK();
};


bool ChangeMeritDialog(CString name, DWORD original_merit, DWORD &new_merit);
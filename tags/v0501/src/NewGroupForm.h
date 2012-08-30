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
//	CNewGroupForm class
//
//-----------------------------------------------------------------------------

class CNewGroupForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CNewGroupForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);

public:

	GraphStudio::TitleBar		title;
	CEdit						edit;
	CString						text;

public:
	CNewGroupForm(CWnd* pParent = NULL); 
	virtual ~CNewGroupForm();
	enum { IDD = IDD_DIALOG_NEWGROUP };

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	void OnShowWindow(BOOL bShow, UINT nStatus);

};

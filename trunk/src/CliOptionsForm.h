//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once
#include "afxcmn.h"


//-----------------------------------------------------------------------------
//
//	CSbeConfigForm class
//
//-----------------------------------------------------------------------------
class CCliOptionsForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CCliOptionsForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);

public:
	GraphStudio::TitleBar	title;

public:
	CCliOptionsForm(CWnd* pParent = NULL);  
	virtual ~CCliOptionsForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_CLIOPTIONS };

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	void OnAssociateFileType();
	static void AssociateFileType();

    CFont			font_options;
    CEdit			edit_options;
};

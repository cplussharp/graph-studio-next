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
class CSbeConfigForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CSbeConfigForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);

    CComPtr<IStreamBufferConfigure> m_pConfig;

public:
	GraphStudio::TitleBar	title;

public:
	CSbeConfigForm(CWnd* pParent = NULL);  
	virtual ~CSbeConfigForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_SBECONFIG };

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonBrowse();
    CString m_strDir;
    DWORD m_nFileDuration;
    DWORD m_nFileCountMin;
    DWORD m_nFileCountMax;
    CSpinButtonCtrl m_spinFileCountMin;
    CSpinButtonCtrl m_spinFileCountMax;
};

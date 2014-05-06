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
//	CCbgLogConfigForm class
//
//-----------------------------------------------------------------------------
class CDbgLogConfigForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CDbgLogConfigForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);

public:
	GraphStudio::TitleBar	title;

public:
	CDbgLogConfigForm(const CString& strFileName, CWnd* pParent = NULL);
	virtual ~CDbgLogConfigForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_DBGLOGCONFIG };

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonBrowse();
    CString m_strLogFile;
	DWORD m_nTrace;
	DWORD m_nError;
	DWORD m_nMemory;
	DWORD m_nLocking;
	DWORD m_nTiming;
	int m_nTimeout;
	DWORD m_nCustom1;
	DWORD m_nCustom2;
	DWORD m_nCustom3;
	DWORD m_nCustom4;
	DWORD m_nCustom5;
    CSpinButtonCtrl m_spinTrace;
	CSpinButtonCtrl m_spinError;
	CSpinButtonCtrl m_spinMemory;
	CSpinButtonCtrl m_spinLocking;
	CSpinButtonCtrl m_spinTiming;
	CSpinButtonCtrl m_spinTimeout;
	CSpinButtonCtrl m_spinCustom1;
	CSpinButtonCtrl m_spinCustom2;
	CSpinButtonCtrl m_spinCustom3;
	CSpinButtonCtrl m_spinCustom4;
	CSpinButtonCtrl m_spinCustom5;
	CButton m_btnOK;

	const CString m_strFileName;
	CString m_strRegKey;
};

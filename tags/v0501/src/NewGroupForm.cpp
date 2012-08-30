//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CNewGroupForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CNewGroupForm, CDialog)
BEGIN_MESSAGE_MAP(CNewGroupForm, CDialog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CNewGroupForm class
//
//-----------------------------------------------------------------------------

CNewGroupForm::CNewGroupForm(CWnd *pParent) : 
	CDialog(CNewGroupForm::IDD, pParent)
{
}

CNewGroupForm::~CNewGroupForm()
{
}

void CNewGroupForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_EDIT_NAME, edit);
}

BOOL CNewGroupForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	text = _T("");
	edit.SetFocus();
	
	return TRUE;
}

void CNewGroupForm::OnShowWindow(BOOL bShow, UINT nStatus)
{
	edit.SetFocus();
}

void CNewGroupForm::OnOK()
{
	edit.GetWindowText(text);
	__super::OnOK();
}
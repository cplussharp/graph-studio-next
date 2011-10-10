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
//	CConfirmDialog class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CConfirmDialog, CDialog)
BEGIN_MESSAGE_MAP(CConfirmDialog, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CConfirmDialog class
//
//-----------------------------------------------------------------------------

CConfirmDialog::CConfirmDialog(CWnd* pParent)	: 
	CDialog(CConfirmDialog::IDD, pParent)
        , m_bUnregisterAll(TRUE)
    {

}

CConfirmDialog::~CConfirmDialog()
{
}

void CConfirmDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_FILTER, label_filter);
    DDX_Check(pDX, IDC_CHECK_UNREGALL, m_bUnregisterAll);
}

BOOL CConfirmDialog::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	// create a nice big font
	GraphStudio::MakeFont(font_filter, _T("Arial"), 12, true, false);
	label_filter.SetFont(&font_filter);
	label_filter.SetWindowText(filter_name);

	return TRUE;
}



bool ConfirmUnregisterFilter(CString name, BOOL* pbUnregisterAll)
{
	CConfirmDialog		dlg;
	dlg.filter_name = name;

	if (dlg.DoModal() == IDOK)
    {
        if(pbUnregisterAll) *pbUnregisterAll = dlg.m_bUnregisterAll;
        return true;
    }
	return false;
}


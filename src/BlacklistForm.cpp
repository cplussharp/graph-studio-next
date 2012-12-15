//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "BlacklistForm.h"

//-----------------------------------------------------------------------------
//
//	CLookupForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CBlacklistForm, CDialog)

CBlacklistForm::CBlacklistForm(CWnd* pParent /*=NULL*/, BOOL isHR /*=FALSE*/)
	: CDialog(CBlacklistForm::IDD, pParent), m_isHR(isHR)
{

}

CBlacklistForm::~CBlacklistForm()
{
}

void CBlacklistForm::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, m_title);
    DDX_Control(pDX, IDC_LIST_BLACKLIST, m_listCtrl);
    if(m_btnAdd)
        DDX_Control(pDX, IDC_BUTTON_ADD, m_btnAdd);
    if(m_btnRemove)
        DDX_Control(pDX, IDC_BUTTON_REMOVE, m_btnRemove);
    if(m_btnImport)
        DDX_Control(pDX, IDC_BUTTON_IMPORT, m_btnImport);
    if(m_btnExport)
        DDX_Control(pDX, IDC_BUTTON_EXPORT, m_btnExport);
    if(m_editEntry)
        DDX_Control(pDX, IDC_EDIT_BLACKLIST_ENTRY, m_editEntry);
}

BOOL CBlacklistForm::DoCreateDialog()
{
	BOOL ret = Create(IDD);
	if (!ret) return FALSE;

    // prepare titlebar
	m_title.ModifyStyle(0, WS_CLIPCHILDREN);
	m_title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

    /*CRect	rc;
	rc.SetRect(0, 0, 60, 23);
    m_btnSearch.Create(_T("Search"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &m_title, IDC_BUTTON_SEARCH);
    m_btnSearch.SetFont(GetFont());
    rc.SetRect(0, 0, 250, 23);
    m_editSearch.Create(ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, rc, &m_title, IDC_EDIT_SEARCH);
    m_editSearch.SetFont(GetFont());
    m_editSearch.SetFocus();
    */
    SetWindowPos(NULL, 0, 0, 600, 400, SWP_NOMOVE);

	OnInitialize();

	return TRUE;
};

void CBlacklistForm::OnInitialize()
{
    m_listCtrl.InsertColumn(0, _T("Entry"), LVCFMT_LEFT, 120);
    m_listCtrl.InsertColumn(1, _T("Filter Name"), LVCFMT_LEFT, 250);

    //int i = 0;
    //while(GraphStudio::InsertHresultLookup(i, &m_listCtrl)) i++;

    m_listCtrl.SetExtendedStyle( m_listCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
}


BEGIN_MESSAGE_MAP(CBlacklistForm, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_ADD, &CBlacklistForm::OnBnClickedButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CBlacklistForm::OnBnClickedButtonRemove)
    ON_BN_CLICKED(IDC_BUTTON_ADD, &CBlacklistForm::OnBnClickedButtonImport)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CBlacklistForm::OnBnClickedButtonExport)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CLookupForm-Meldungshandler

void CBlacklistForm::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    // resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(m_listCtrl)) {
		m_title.GetClientRect(&rc2);

		//m_editSearch.SetWindowPos(NULL, cx - 1*(60+6) - 250, 6, 250, 21, SWP_SHOWWINDOW | SWP_NOZORDER);
		//m_btnSearch.SetWindowPos(NULL, cx - 1*(60+6), 4, 60, 25, SWP_SHOWWINDOW | SWP_NOZORDER);

		//m_listCtrl.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		m_title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
		m_title.Invalidate();
		/*m_editSearch.Invalidate();
		m_btnSearch.Invalidate();
        m_listCtrl.Invalidate();*/
	}
}

void CBlacklistForm::OnBnClickedButtonAdd()
{

}

void CBlacklistForm::OnBnClickedButtonRemove()
{

}

void CBlacklistForm::OnBnClickedButtonImport()
{

}

void CBlacklistForm::OnBnClickedButtonExport()
{

}

//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "LookupForm.h"

int CALLBACK CompareFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamData)
{
    if(!lParam1 || !lParam2) return 0;
    SortStruct* sort=(SortStruct*)lParamData;
    CStringArray* arData1=(CStringArray*)(sort->Ascending ? lParam1 : lParam2);
    CStringArray* arData2=(CStringArray*)(sort->Ascending ? lParam2 : lParam1);

    CString val1 = arData1->GetAt(sort->SortCol);
    CString val2 = arData2->GetAt(sort->SortCol);

    return val1.CompareNoCase(val2);
}

//-----------------------------------------------------------------------------
//
//	CLookupForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CLookupForm, CDialog)

CLookupForm::CLookupForm(CWnd* pParent /*=NULL*/, BOOL isHR /*=FALSE*/)
	: CDialog(CLookupForm::IDD, pParent), m_isHR(isHR)
{

}

CLookupForm::~CLookupForm()
{
}

void CLookupForm::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, m_title);
    DDX_Control(pDX, IDC_LIST_LOOKUP, m_listCtrl);
    if(m_btnSearch)
        DDX_Control(pDX, IDC_BUTTON_SEARCH, m_btnSearch);
    if(m_editSearch)
        DDX_Control(pDX, IDC_EDIT_SEARCH, m_editSearch);
}

BOOL CLookupForm::DoCreateDialog()
{
	BOOL ret = Create(IDD);
	if (!ret) return FALSE;

    // prepare titlebar
	m_title.ModifyStyle(0, WS_CLIPCHILDREN);
	m_title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

    CRect	rc;
	rc.SetRect(0, 0, 60, 23);
    m_btnSearch.Create(_T("Search"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &m_title, IDC_BUTTON_SEARCH);
    m_btnSearch.SetFont(GetFont());
    rc.SetRect(0, 0, 250, 23);
    m_editSearch.Create(ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, rc, &m_title, IDC_EDIT_SEARCH);
    m_editSearch.SetFont(GetFont());
    m_editSearch.SetFocus();

    SetWindowPos(NULL, 0, 0, 600, 400, SWP_NOMOVE);

	OnInitialize();

	return TRUE;
};

void CLookupForm::OnInitialize()
{
    if(m_isHR)
    {
        SetWindowText(_T("Lookup HRESULT values"));

        m_listCtrl.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 120);
        m_listCtrl.InsertColumn(1, _T("Hex value"), LVCFMT_RIGHT, 80);
        m_listCtrl.InsertColumn(2, _T("Dec value"), LVCFMT_RIGHT, 80);
        m_listCtrl.InsertColumn(3, _T("Message"), LVCFMT_LEFT, 250);

        int i = 0;
        while(GraphStudio::InsertHresultLookup(i, &m_listCtrl)) i++;
    }
    else
    {
        SetWindowText(_T("Lookup GUID values"));

        m_listCtrl.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 160);
        m_listCtrl.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 242);
        m_listCtrl.InsertColumn(2, _T("From"), LVCFMT_LEFT, 80);
        m_listCtrl.InsertColumn(3, _T("Info"), LVCFMT_LEFT, 300);

        int i = 0;
        while(GraphStudio::InsertGuidLookup(i, &m_listCtrl)) i++;
    }

    // Make Item-Data lower for better search (more results)
    int count = m_listCtrl.GetItemCount();
    for(int i=0;i<count;i++)
    {
        CStringArray* arData = (CStringArray*)m_listCtrl.GetItemData(i);
        for(int j=0;j<arData->GetCount();j++)
        {
            CString str = arData->GetAt(j);
            str.MakeLower();
            arData->SetAt(j, str);
        }
    }

    m_listCtrl.SortItems(CompareFunction, (DWORD_PTR)&sortData);

    m_listCtrl.SetExtendedStyle( m_listCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
}


BEGIN_MESSAGE_MAP(CLookupForm, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CLookupForm::OnBnClickedButtonSearch)
    ON_WM_SIZE()
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_LOOKUP, &CLookupForm::OnLvnColumnclickListLookup)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CLookupForm-Meldungshandler

void CLookupForm::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    // resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(m_listCtrl)) {
		m_title.GetClientRect(&rc2);

		m_editSearch.SetWindowPos(NULL, cx - 1*(60+6) - 250, 6, 250, 21, SWP_SHOWWINDOW);
		m_btnSearch.SetWindowPos(NULL, cx - 1*(60+6), 4, 60, 25, SWP_SHOWWINDOW);

		m_listCtrl.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW);

		m_title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW);
		m_title.Invalidate();
		m_editSearch.Invalidate();
		m_btnSearch.Invalidate();
        m_listCtrl.Invalidate();
	}
}

void CLookupForm::OnBnClickedButtonSearch()
{
    CString searchText;
    m_editSearch.GetWindowText(searchText);

    if(searchText.GetLength() <= 0) return;
    searchText.MakeLower(); // itemData is also lower

    int selItem = -1, selCol = -1;
    int start = m_listCtrl.GetSelectionMark();
    start++;

    int count = m_listCtrl.GetItemCount();
    for(int i=start;i<count;i++)
    {
        CStringArray* arData = (CStringArray*)m_listCtrl.GetItemData(i);
        for(int j=0; j<arData->GetCount(); j++)
        {
            int find = arData->GetAt(j).Find(searchText);
            if(find >= 0)
            {
                selItem = i;
                selCol = j;
                i=count;
                break;
            }
        }
    }

    if(selItem == -1 && start > 0)
    {
        for(int i=0;i<start;i++)
        {
            CStringArray* arData = (CStringArray*)m_listCtrl.GetItemData(i);
            for(int j=0; j<arData->GetCount(); j++)
            {
                int find = arData->GetAt(j).Find(searchText);
                if(find >= 0)
                {
                    selItem = i;
                    selCol = j;
                    i=count;
                    break;
                }
            }
        }
    }

    if(selItem != -1)
    {
        m_listCtrl.SetItemState(selItem, LVIS_SELECTED, LVIS_SELECTED);
        m_listCtrl.SetSelectionMark(selItem);
        m_listCtrl.EnsureVisible(selItem, FALSE);
        m_listCtrl.SetFocus();
    }
}

void CLookupForm::OnDestroy()
{
    int count = m_listCtrl.GetItemCount();
    for(int i=0;i<count;i++)
    {
        CStringArray* arData = (CStringArray*)m_listCtrl.GetItemData(i);
        delete arData;
    }
}

void CLookupForm::OnLvnColumnclickListLookup(NMHDR *pNMHDR, LRESULT *pResult)
{
    *pResult=0; // Not used
    int col=((NM_LISTVIEW*)pNMHDR)->iSubItem;
    sortData.SetCol(col);
    m_listCtrl.SortItems(CompareFunction, (DWORD_PTR)&sortData);

    int selItem = m_listCtrl.GetSelectionMark();
    if(selItem >= 0)
        m_listCtrl.EnsureVisible(selItem, FALSE);
}


void CLookupForm::OnOK()
{
    OnBnClickedButtonSearch();
}

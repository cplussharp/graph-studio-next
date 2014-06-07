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

IMPLEMENT_DYNAMIC(CLookupForm, CGraphStudioModelessDialog)

CLookupForm::CLookupForm(CWnd* pParent /*=NULL*/, BOOL isHR /*=FALSE*/)
	: CGraphStudioModelessDialog(CLookupForm::IDD, pParent), m_isHR(isHR)
{

}

CLookupForm::~CLookupForm()
{
}

void CLookupForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, m_title);
    DDX_Control(pDX, IDC_LIST_LOOKUP, m_listCtrl);
    if(m_btnSearch)
        DDX_Control(pDX, IDC_BUTTON_SEARCH, m_btnSearch);
    if(m_editSearch)
        DDX_Control(pDX, IDC_EDIT_SEARCH, m_editSearch);
}

BOOL CLookupForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);
	if (!ret) return FALSE;

    // prepare titlebar
	m_title.ModifyStyle(0, WS_CLIPCHILDREN);
	m_title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

    CRect	rc;
	rc.SetRect(0, 0, 60, 23);
    m_btnSearch.Create(_T("&Search"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &m_title, IDC_BUTTON_SEARCH);
    m_btnSearch.SetFont(GetFont());
    rc.SetRect(0, 0, 250, 23);
    m_editSearch.Create(ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, rc, &m_title, IDC_EDIT_SEARCH);
    m_editSearch.SetFont(GetFont());
    m_editSearch.SetFocus();

	OnInitialize();

	return TRUE;
};

CString CLookupForm::GetSettingsName() const
{
	// Distinguish saved positions of different window types
	return __super::GetSettingsName() + (m_isHR ? _T("_HResult") : _T("_Guid"));
}

CRect CLookupForm::GetDefaultRect() const 
{
	return CRect(50, 200, 650, 600);
}

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

    m_listCtrl.SetExtendedStyle( m_listCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP );
}


BEGIN_MESSAGE_MAP(CLookupForm, CGraphStudioModelessDialog)
    ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CLookupForm::OnBnClickedButtonSearch)
    ON_WM_SIZE()
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_LOOKUP, &CLookupForm::OnLvnColumnclickListLookup)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CLookupForm-Meldungshandler

void CLookupForm::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);

    // resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(m_listCtrl)) {
		m_title.GetClientRect(&rc2);

		m_editSearch.SetWindowPos(NULL, cx - 1*(60+6) - 250, 6, 250, 21, SWP_SHOWWINDOW | SWP_NOZORDER);
		m_btnSearch.SetWindowPos(NULL, cx - 1*(60+6), 4, 60, 25, SWP_SHOWWINDOW | SWP_NOZORDER);

		m_listCtrl.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		m_title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
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

    if(searchText.GetLength() <= 0) 
		return;
    searchText.MakeLower(); // itemData is also lower
	CString guidSearchText;

	if (!m_isHR) {
		guidSearchText = searchText;
		guidSearchText.Replace(_T("0x"), _T(""));
		const TCHAR delimiters[] = _T(" \t-,{}()=/;ul");			// remove commonly used GUID delimiters and match by remaining hex digits
		for (int n=0; n<sizeof(delimiters)/sizeof(delimiters[0]) - 1; n++) 
			guidSearchText.Remove(delimiters[n]);
	}

    int selItem = -1, selCol = -1;
    const int start = m_listCtrl.GetSelectionMark() + 1;
	CString item_text;

    const int count = m_listCtrl.GetItemCount();
	bool allowWrap = true;
    for(int i=start; ;i++)
    {
		if (i >= count) {			//  beyond end of list
			if (start > 0 && allowWrap) {
				allowWrap = false;
				i = 0;				// restart at beginning
			}
			else
				break;				// started at beginning or already wrapped so stop looping now
		}

        const CStringArray * const arData = (const CStringArray*)m_listCtrl.GetItemData(i);
        for(int j=0; j<arData->GetCount(); j++)
        {
			int find;
			if (!m_isHR && j==1)	// GUID column - strip out hyphens for easier hex matching
			{
				item_text = arData->GetAt(j);
				item_text.Remove(_T('-'));
				find = item_text.Find(guidSearchText);
			} else 
			{
				find = arData->GetAt(j).Find(searchText);
			}
            if(find >= 0)
            {
                selItem = i;
                selCol = j;
                i=count;			// terminate looping without wrapping
				allowWrap = false;
                break;
            }
        }

		if (i == start-1)		// about to hit start for second time round - stop looping
			break;
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

//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CStatisticForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CStatisticForm, CGraphStudioModelessDialog)
BEGIN_MESSAGE_MAP(CStatisticForm, CGraphStudioModelessDialog)
	ON_WM_SIZE()
	ON_WM_TIMER()
    ON_COMMAND(IDC_BUTTON_CLEAR, &CStatisticForm::OnResetClick)
	ON_COMMAND(IDC_BUTTON_EXPORT, &CStatisticForm::OnExportClick)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_DATA, &CStatisticForm::OnLvnGetdispinfoListData)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CStatisticForm class
//
//-----------------------------------------------------------------------------

CStatisticForm::CStatisticForm(CWnd* pParent)	: 
	CGraphStudioModelessDialog(CStatisticForm::IDD, pParent)
{
    ZeroMemory(&m_cachedEntry,sizeof(GraphStatisticEntry));
}

CStatisticForm::~CStatisticForm()
{
    FreeCachedStatisticEntry();
}

void CStatisticForm::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
    DDX_Control(pDX, IDC_LIST_DATA, m_listCtrl);
}

BOOL CStatisticForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

    CRect	rc;
	rc.SetRect(0, 0, 60, 23);
    btn_reset.Create(_T("&Reset"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_CLEAR);
    btn_reset.SetWindowPos(NULL, 4, 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
    btn_reset.SetFont(GetFont());

    btn_export.Create(_T("&Export"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_EXPORT);
    btn_export.SetWindowPos(NULL, 8 + rc.Width(), 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
    btn_export.SetFont(GetFont());

	m_listCtrl.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 300);
    m_listCtrl.InsertColumn(1, _T("Count"), LVCFMT_RIGHT, 60);
    m_listCtrl.InsertColumn(2, _T("Last"), LVCFMT_RIGHT, 60);
    m_listCtrl.InsertColumn(3, _T("Average"), LVCFMT_RIGHT, 60);
    m_listCtrl.InsertColumn(4, _T("StdDev"), LVCFMT_RIGHT, 60);
    m_listCtrl.InsertColumn(5, _T("Min"), LVCFMT_RIGHT, 60);
    m_listCtrl.InsertColumn(6, _T("Max"), LVCFMT_RIGHT, 60);

    m_listCtrl.SetExtendedStyle( m_listCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP );

    SetTimer(0, 1000, NULL);

	return TRUE;
}

CRect CStatisticForm::GetDefaultRect() const 
{
	return CRect(50, 200, 750, 600);
}

void CStatisticForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(title)) {
		title.GetClientRect(&rc2);
		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

        m_listCtrl.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

        title.Invalidate();
        m_listCtrl.Invalidate();

		title.Invalidate();
	}
}

void CStatisticForm::OnTimer(UINT_PTR id)
{
	switch (id) {
	case 0:
		{
            FreeCachedStatisticEntry();

            LONG entryCount;
            CComQIPtr<IAMStats> pStats = view->graph.gb;
            pStats->get_Count(&entryCount);
            m_listCtrl.SetItemCountEx(entryCount, LVSICF_NOSCROLL);
		}
		break;
	}
}

void CStatisticForm::OnResetClick()
{
    FreeCachedStatisticEntry();
	LONG entryCount;
    CComQIPtr<IAMStats> pStats = view->graph.gb;
    pStats->Reset();
    pStats->get_Count(&entryCount);
    m_listCtrl.SetItemCountEx(entryCount, LVSICF_NOSCROLL);
}

// from http://www.codeproject.com/Articles/7891/Using-virtual-lists
void CStatisticForm::OnLvnGetdispinfoListData(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
    
    //Create a pointer to the item
    LV_ITEM* pItem= &(pDispInfo)->item;

    //Do the list need text information?
    if (pItem->mask & LVIF_TEXT)
    {
        CString val = GetEntryString(pItem->iItem, pItem->iSubItem);

        //Copy the text to the LV_ITEM structure
        //Maximum number of characters is in pItem->cchTextMax
        lstrcpyn(pItem->pszText, val, pItem->cchTextMax);
    }

    *pResult = 0;
}

void CStatisticForm::OnExportClick()
{
	CString	fileFilter;
	CString	filename;

	fileFilter = _T("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*|");

	CFileDialog dlg(FALSE,_T("statistics"),NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,fileFilter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK)
    {
		CPath path(filename);
		if (path.GetExtension() == _T(""))
        {
			path.AddExtension(_T(".csv"));
			filename = CString(path);
		}

        CFile file(filename, CFile::modeCreate|CFile::modeWrite);

        // Output header
        CString row = _T("");
        for(int field=0; field < 7; field++)
        {
            LVCOLUMN column = { 0 };
            column.mask = LVCF_TEXT;
            column.cchTextMax = 100;
            column.pszText = new TCHAR[column.cchTextMax];
            m_listCtrl.GetColumn(field, &column);

            if (field > 0)
                row.Append(_T(";"));
            row.Append(column.pszText);

            delete [] column.pszText;
        }
        row.Append(_T("\n"));
        CT2CA outputHeaderText(row, CP_UTF8);
        file.Write(outputHeaderText, ::strlen(outputHeaderText));

        // Output data
        LONG entryCount;
        {
            CComQIPtr<IAMStats> pStats = view->graph.gb;
            pStats->get_Count(&entryCount);
        }
        
        for(__int64 i = 0; i<entryCount; i++)
        {
            row = _T("");
            for(int field=0; field<7; field++)
            {
                if (field > 0)
                    row.Append(_T(";"));
                row.Append(GetEntryString(i,field));
            }
            row.Append(_T("\n"));

            CT2CA outputText(row, CP_UTF8);
            file.Write(outputText, ::strlen(outputText));
        }
    }
}

const CString CStatisticForm::GetEntryString(LONG entryNr, int field)
{
    CString val;
    GraphStatisticEntry* entry = GetEntry(entryNr);
    if (entry)
    {
        //Which column?
        switch(field)
        {
            case 0:
                val = entry->szName;
                break;

            case 1:
                val.Format(_T("%d"),entry->lCount);
                break;

            case 2:
                val.Format(_T("%.2f"),entry->dLast);
                break;

            case 3:
                val.Format(_T("%.2f"),entry->dAverage);
                break;

            case 4:
                val.Format(_T("%.2f"),entry->dStdDev);
                break;

            case 5:
                val.Format(_T("%.2f"),entry->dMin);
                break;

            case 6:
                val.Format(_T("%.2f"),entry->dMax);
                break;
        }
    }

    return val;
}

GraphStatisticEntry* CStatisticForm::GetEntry(LONG entryNr)
{
    if (m_cachedEntry.lIndex == entryNr && m_cachedEntry.szName)
        return &m_cachedEntry;

    FreeCachedStatisticEntry();

    m_cachedEntry.lIndex = entryNr;
    CComQIPtr<IAMStats> pStats = view->graph.gb;
    HRESULT hr = pStats->GetValueByIndex(entryNr, &m_cachedEntry.szName, &m_cachedEntry.lCount,
                                                  &m_cachedEntry.dLast, &m_cachedEntry.dAverage,
                                                  &m_cachedEntry.dStdDev, &m_cachedEntry.dMin, &m_cachedEntry.dMax);

    if (hr == S_OK)
        return &m_cachedEntry;

    return NULL;
}

void CStatisticForm::FreeCachedStatisticEntry()
{
    if (m_cachedEntry.szName)
        SysFreeString(m_cachedEntry.szName);

    ZeroMemory(&m_cachedEntry,sizeof(GraphStatisticEntry));
}

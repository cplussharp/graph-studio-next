//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : cplussharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FileTypesForm.h"



//-----------------------------------------------------------------------------
//
//	CPageFileTypes class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPageFileTypes, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CPageFileTypes class
//
//-----------------------------------------------------------------------------
CPageFileTypes::CPageFileTypes(CString strTitle)
    : info(_T("root")), title(strTitle)
{
}

CPageFileTypes::~CPageFileTypes()
{
}


// overriden
BOOL CPageFileTypes::OnInitDialog()
{
	// create the tree
	CRect	rc;
	GetClientRect(&rc);

	BOOL ok = tree.Create(NULL, WS_CHILD | WS_VISIBLE, rc, this, IDC_TREE);
	if (!ok) return FALSE;

	tree.left_width = 90;		// make first column narrower

	info.Clear();

	tree.Initialize();
	UpdateTree();

	return TRUE;
}

void CPageFileTypes::UpdateTree()
{
    tree.BuildPropertyTree(&info);
}

void CPageFileTypes::OnSize(UINT nType, int cx, int cy)
{
	if (IsWindow(tree)) tree.MoveWindow(0, 0, cx, cy);
}



//-----------------------------------------------------------------------------
//
//	CFileTypesForm class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CFileTypesForm, CGraphStudioModelessDialog)

BEGIN_MESSAGE_MAP(CFileTypesForm, CGraphStudioModelessDialog)
	ON_WM_SIZE()
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_FILETYPES, &CFileTypesForm::OnTabChanged)
    ON_NOTIFY(TCN_SELCHANGING, IDC_TAB_FILETYPES, &CFileTypesForm::OnTabChanging)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CFileTypesForm::OnBnClickedButtonReload)
	ON_BN_CLICKED(IDC_BUTTON_COPY, &CFileTypesForm::OnBnClickedButtonCopy)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFileTypesForm class
//
//-----------------------------------------------------------------------------

CFileTypesForm::CFileTypesForm(CWnd* pParent)	: 
	CGraphStudioModelessDialog(CFileTypesForm::IDD, pParent),
    page_protocols(NULL),
    page_extensions(NULL),
    page_bytes(NULL),
    pageCount(3)
{
    for (DWORD i=0; i<pageCount; i++)
        pages[i] = NULL;
}

CFileTypesForm::~CFileTypesForm()
{
    for (DWORD i=0; i<pageCount; i++)
    {
        if (pages[i])
        {
            pages[i]->DestroyWindow();
            delete pages[i];
            pages[i] = NULL;
        }
    }
}

void CFileTypesForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
    DDX_Control(pDX, IDC_TAB_FILETYPES, tab);
}

BOOL CFileTypesForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	// create buttons
	CRect	rc;
	rc.SetRect(0, 0, 60, 23);
    btn_copy.Create(_T("&Copy"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_COPY);
	btn_copy.SetFont(GetFont());
    btn_copy.SetWindowPos(NULL, 4, 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	btn_reload.Create(_T("&Reload"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_CLEAR);
    btn_reload.SetWindowPos(NULL, 8 + rc.Width(), 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_reload.SetFont(GetFont());

    pages[0] = page_protocols   = new CPageFileTypes(_T("Protocols"));
    pages[1] = page_extensions  = new CPageFileTypes(_T("Extensions"));
    pages[2] = page_bytes       = new CPageFileTypes(_T("Bytes"));

    // we need to set WS_EX_CONTROLPARENT for the tabs
	tab.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

    TCITEM tci;
    tci.mask = TCIF_TEXT;
    tci.iImage = -1;

    for (DWORD i=0; i<pageCount; i++)
    {
        if (pages[i])
        {
            pages[i]->Create(CPageFileTypes::IDD, &tab);
            tci.pszText =  (LPTSTR)(LPCTSTR)CString(pages[i]->title);
            tab.InsertItem(i, &tci);
        }
    }

    OnTabChanged(NULL, NULL);

    OnInitialize();

	return TRUE;
}

BOOL CFileTypesForm::PreTranslateMessage(MSG *pMsg)
{
	if (pMsg && pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_TAB) {
			if ((0x8000 & GetKeyState(VK_CONTROL)) && !(0x8000 & GetKeyState(VK_MENU))) {	// control-tab to change page
				const int numPages = tab.GetItemCount();
				if (numPages > 1) {
					const int increment = (0x8000 & GetKeyState(VK_SHIFT)) ? -1 : 1;		// added shift iterates backwards
					int newPage = tab.GetCurSel() + increment;

					if (newPage < 0)
						newPage = numPages - 1;
					else if (newPage >= numPages)
						newPage = 0;
					HideCurrentPage();
					tab.SetCurSel(newPage);
					ShowPage(newPage);
				}
				return TRUE;
			}
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

CRect CFileTypesForm::GetDefaultRect() const 
{
	return CRect(50, 200, 450, 450);
}

void CFileTypesForm::OnInitialize()
{
	OnBnClickedButtonReload();
}

void CFileTypesForm::ShowPage(int nIndex)
{
    CRect rc;
    tab.GetClientRect(&rc);
    tab.AdjustRect(FALSE, &rc);
    //tab.GetItemRect(0,&rc);

    if (nIndex >= 0 && pages[nIndex])
    {
        pages[nIndex]->SetWindowPos( NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
        pages[nIndex]->SetFocus();
    }
}

void CFileTypesForm::OnTabChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShowPage(tab.GetCurSel());
    if (pResult)
		*pResult = 0;
}

void CFileTypesForm::HideCurrentPage()
{
    const int nIndex = tab.GetCurSel();
    if (nIndex >= 0 && pages[nIndex])
        pages[nIndex]->ShowWindow(SW_HIDE);
}

void CFileTypesForm::OnTabChanging(NMHDR *pNMHDR, LRESULT *pResult)
{
	HideCurrentPage();
    if (pResult)
		*pResult = 0;
}

void CFileTypesForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(title)) {
		title.GetClientRect(&rc2);

        tab.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
        OnTabChanged(NULL, NULL);

		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
		title.Invalidate();
	}
}

CString ByteFormatString(CString& strValue)
{
    CString strResult;
    for (int i=0; i<strValue.GetLength(); i+=2)
    {
        if (i>0) strResult.Append(_T(" "));
        strResult.Append(strValue.Mid(i,2).MakeUpper());
    }
    return strResult;
}

void CFileTypesForm::OnBnClickedButtonReload()
{
    for (DWORD i=0; i<pageCount; i++)
    {
        if (pages[i])
        {
            pages[i]->info.Clear();
            pages[i]->UpdateTree();
        }
    }

    DSUtil::FilterTemplates filters;
    filters.EnumerateAllRegisteredFilters();

    // search for registered protocols
    if (page_protocols)
    {
        ATL::CRegKey rkRoot(HKEY_CLASSES_ROOT);
        // only real protocols => not something like "WMP11.AssocProtocol.MMS"
        // faster, because i don't need to search in every entry for "Source Filter"
        TCHAR szName[10] = {0};
        DWORD szNameLength = 10;
        DWORD i = 0;
        long ret = 0;
        while (ERROR_NO_MORE_ITEMS != (ret = rkRoot.EnumKey(i++, szName, &szNameLength)))
        {
            if (ret != ERROR_SUCCESS)
                continue;

            CRegKey rkKey;
            if(ERROR_SUCCESS == rkKey.Open(HKEY_CLASSES_ROOT, szName, KEY_READ))
            {
                TCHAR szSourceFilterGuid[40] = {0};
                DWORD szLength = 40;
                if (ERROR_SUCCESS == rkKey.QueryStringValue(_T("Source Filter"), szSourceFilterGuid, &szLength))
                {
                    GraphStudio::PropItem* group = new GraphStudio::PropItem(CString(szName));

                    CString strClsid = szSourceFilterGuid;
                    GUID clsid = {0};
                    CLSIDFromString((LPOLESTR)strClsid.GetBuffer(), &clsid);

                    group->AddItem(new GraphStudio::PropItem(_T("CLSID"), CString(szSourceFilterGuid), false));

                    DSUtil::FilterTemplate ft;
		            if (filters.FindTemplateByCLSID(clsid, &ft))
                    {
                        group->AddItem(new GraphStudio::PropItem(_T("Name"), CString(ft.name), false));
                        group->AddItem(new GraphStudio::PropItem(_T("File"), CString(ft.file), false));
                    }

                    // last Change of this key
                    FILETIME timeMod = {0};
                    if (ERROR_SUCCESS == RegQueryInfoKey(rkKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &timeMod))
                        group->AddItem(new GraphStudio::PropItem(_T("Modified"), CTime(timeMod)));

                    page_protocols->info.AddItem(group);
                }
            }
            rkKey.Close();
            szNameLength = 10;
        }
        page_protocols->UpdateTree();
        rkRoot.Close();
    }

    // search for registered extensions
    if (page_extensions)
    {
        ATL::CRegKey rkRoot;
        CString strRoot = _T("Media Type\\Extensions");
        if (ERROR_SUCCESS == rkRoot.Open(HKEY_CLASSES_ROOT, strRoot, KEY_READ))
        {
            // {7DF62B50-6843-11D2-9EEB-006008039E37}
            static const GUID CLSID_StillVideo = {0x7DF62B50, 0x6843, 0x11D2, { 0x9E, 0xEB, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37} };

            TCHAR szName[50] = {0};
            DWORD szNameLength = 50;
            DWORD i = 0;
            while (ERROR_NO_MORE_ITEMS != rkRoot.EnumKey(i++, szName, &szNameLength))
            {
                CString strKey = strRoot;
                strKey.Append(_T("\\"));
                strKey.Append(szName);
                CRegKey rkKey;
                if(ERROR_SUCCESS == rkKey.Open(HKEY_CLASSES_ROOT, strKey, KEY_READ))
                {
                    GraphStudio::PropItem* group = new GraphStudio::PropItem(CString(szName));

                    TCHAR szGuid[40] = {0};
                    DWORD szLength = 40;
                    if (ERROR_SUCCESS == rkKey.QueryStringValue(_T("Source Filter"), szGuid, &szLength))
                    {
                        CString strClsid = szGuid;
                        GUID clsid = {0};
                        CLSIDFromString((LPOLESTR)strClsid.GetBuffer(), &clsid);
                        group->AddItem(new GraphStudio::PropItem(_T("CLSID"), CString(szGuid), false));

                        DSUtil::FilterTemplate ft;
		                if (filters.FindTemplateByCLSID(clsid, &ft))
                        {
                            group->AddItem(new GraphStudio::PropItem(_T("Name"), CString(ft.name), false));
                            group->AddItem(new GraphStudio::PropItem(_T("File"), CString(ft.file), false));
                        }
                        else if (clsid == CLSID_StillVideo)
                        {
                            group->AddItem(new GraphStudio::PropItem(_T("Name"), _T("Generate Still Video"), false));
                        }
                    }

                    szLength = 40;
                    if (ERROR_SUCCESS == rkKey.QueryStringValue(_T("Media Type"), szGuid, &szLength))
                    {
                        CString strMT;
                        GUID clsidMT = {0};
                        CLSIDFromString((LPOLESTR)strMT.GetBuffer(), &clsidMT);
                        GraphStudio::NameGuid(clsidMT,strMT,CgraphstudioApp::g_showGuidsOfKnownTypes);
                        group->AddItem(new GraphStudio::PropItem(_T("MediaType"), strMT, false));
                    }

                    szLength = 40;
                    if (ERROR_SUCCESS == rkKey.QueryStringValue(_T("SubType"), szGuid, &szLength))
                    {
                        CString strST = szGuid;
                        GUID clsidST = {0};
                        CLSIDFromString((LPOLESTR)strST.GetBuffer(), &clsidST);
                        GraphStudio::NameGuid(clsidST,strST,CgraphstudioApp::g_showGuidsOfKnownTypes);
                        group->AddItem(new GraphStudio::PropItem(_T("SubType"), strST, false));
                    }

                    // last Change of this key
                    FILETIME timeMod = {0};
                    if (ERROR_SUCCESS == RegQueryInfoKey(rkKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &timeMod))
                        group->AddItem(new GraphStudio::PropItem(_T("Modified"), CTime(timeMod)));

                    page_extensions->info.AddItem(group);
                }
                rkKey.Close();
                szNameLength = 50;
            }
            page_extensions->UpdateTree();
        }
        rkRoot.Close();
    }

    // search for registered byte pattern
    if (page_bytes)
    {
        ATL::CRegKey rkRoot;
        CString strRoot = _T("Media Type");
        if (ERROR_SUCCESS == rkRoot.Open(HKEY_CLASSES_ROOT, strRoot, KEY_READ))
        {
            TCHAR szMTName[40] = {0};
            DWORD szMTNameLength = 40;
            DWORD i = 0;
            while (ERROR_NO_MORE_ITEMS != rkRoot.EnumKey(i++, szMTName, &szMTNameLength))
            {
                CString strMT = szMTName;
                GUID clsidMT = {0};
                if(NOERROR == CLSIDFromString((LPOLESTR)strMT.GetBuffer(), &clsidMT))
                {
                    GraphStudio::NameGuid(clsidMT,strMT,false);

                    CString strKeyMT = strRoot;
                    strKeyMT.Append(_T("\\"));
                    strKeyMT.Append(szMTName);
                    CRegKey rkKeyMT;
                    if(ERROR_SUCCESS == rkKeyMT.Open(HKEY_CLASSES_ROOT, strKeyMT, KEY_READ))
                    {
                        TCHAR szSTName[40] = {0};
                        DWORD szSTNameLength = 40;
                        DWORD j = 0;
                        while (ERROR_NO_MORE_ITEMS != rkKeyMT.EnumKey(j++, szSTName, &szSTNameLength))
                        {
                            CString strST = szSTName;
                            GUID clsidST = {0};
                            if(NOERROR == CLSIDFromString((LPOLESTR)strST.GetBuffer(), &clsidST))
                            {
                                GraphStudio::NameGuid(clsidST,strST,false);

                                CString strKeyST = strKeyMT;
                                strKeyST.Append(_T("\\"));
                                strKeyST.Append(szSTName);
                                CRegKey rkKeyST;

                                if(ERROR_SUCCESS == rkKeyST.Open(HKEY_CLASSES_ROOT, strKeyST, KEY_READ))
                                {
                                    TCHAR szGuid[40] = {0};
                                    DWORD szLength = 40;
                                    if (ERROR_SUCCESS == rkKeyST.QueryStringValue(_T("Source Filter"), szGuid, &szLength))
                                    {
                                        CString groupName = strMT;
                                        groupName.Append(_T("\\"));
                                        groupName.Append(strST);
                                        GraphStudio::PropItem* group = new GraphStudio::PropItem(groupName);

                                        CString strClsid = szGuid;
                                        GUID clsid = {0};
                                        CLSIDFromString((LPOLESTR)strClsid.GetBuffer(), &clsid);
                                        group->AddItem(new GraphStudio::PropItem(_T("CLSID"), CString(szGuid), false));

                                        DSUtil::FilterTemplate ft;
		                                if (filters.FindTemplateByCLSID(clsid, &ft))
                                        {
                                            group->AddItem(new GraphStudio::PropItem(_T("Name"), CString(ft.name), false));
                                            group->AddItem(new GraphStudio::PropItem(_T("File"), CString(ft.file), false));
                                        }

                                        // Enumerate the values
                                        TCHAR szValueName[5] = {0};
                                        DWORD szValueNameLength = 5;
                                        DWORD dwRegType;
                                        DWORD k = 0;
                                        long ret = 0;
                                        while (ERROR_NO_MORE_ITEMS != (ret = RegEnumValue(rkKeyST, k++, szValueName, &szValueNameLength, NULL, &dwRegType, NULL, NULL)))
                                        {
                                            if (dwRegType == REG_SZ && ret == ERROR_SUCCESS)
                                            {
                                                TCHAR szValue[255] = {0};
                                                DWORD szValueLength = 255;
                                                if (ERROR_SUCCESS == rkKeyST.QueryStringValue(szValueName, szValue, &szValueLength))
                                                {
                                                    CString strValue = szValue;
                                                    CStringArray arValues;
                                                    DSUtil::Tokenizer(strValue, _T(","), arValues);
                                                    
                                                    CString strResultValue;
                                                    bool lastTokenWasEmpty = false;
                                                    for (int i=0; i<arValues.GetCount(); i++)
                                                    {
                                                        CString strToken = arValues.GetAt(i);
                                                        strToken = strToken.Trim();

                                                        switch (i % 4)
                                                        {
                                                            case 0:
                                                                if (i > 0) strResultValue.Append(_T(" && ["));
                                                                else strResultValue.Append(_T("["));
                                                                strResultValue.Append(strToken);
                                                                break;

                                                            case 1:
                                                                strResultValue.Append(_T(","));
                                                                strResultValue.Append(strToken);
                                                                break;

                                                            case 2:
                                                                strResultValue.Append(_T("] => ("));
                                                                strResultValue.Append(ByteFormatString(strToken));
                                                                break;

                                                            case 3:
                                                                if (!lastTokenWasEmpty) strResultValue.Append(_T(" = "));
                                                                strResultValue.Append(ByteFormatString(strToken));
                                                                strResultValue.Append(_T(")"));
                                                                break;
                                                        }

                                                        lastTokenWasEmpty = strToken.IsEmpty();
                                                    }

                                                    group->AddItem(new GraphStudio::PropItem(CString(szValueName), strResultValue, false));
                                                    // TODO maybe better format of the byte string
                                                }
                                            }
                                            szValueNameLength = 5;
                                        }

                                        // last Change of this key
                                        FILETIME timeMod = {0};
                                        if (ERROR_SUCCESS == RegQueryInfoKey(rkKeyST, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &timeMod))
                                            group->AddItem(new GraphStudio::PropItem(_T("Modified"), CTime(timeMod)));

                                        page_bytes->info.AddItem(group);
                                    }
                                }
                                rkKeyST.Close();
                            }
                            szSTNameLength = 40;
                        }
                    }
                    rkKeyMT.Close();
                }
                szMTNameLength = 40;
            }
            page_bytes->UpdateTree();
        }
        rkRoot.Close();
    }
}

void CFileTypesForm::OnBnClickedButtonCopy()
{
	// copy everything into clipboard
	CString	text = _T("");

    if (page_protocols)
    {
        text.Append(_T("Protocols:\n"));
        text.Append(_T("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"));
        for (int i=0; i<page_protocols->info.GetCount(); i++)
        {
            GraphStudio::PropItem* group = page_protocols->info.GetItem(i);
            if (!group) continue;

            text.Append(group->name);
            text.Append(_T(" => "));
            text.Append(group->GetItem(0)->value);

            GraphStudio::PropItem* item = group->GetItemByName(_T("Name"));
			if (item) text.AppendFormat(_T(" %s"), (LPCTSTR)item->value);

            item = group->GetItemByName(_T("File"));
			if (item) text.AppendFormat(_T(" (%s)"), (LPCTSTR)item->value);

			text.AppendFormat(_T(" [%s]"), (LPCTSTR)group->GetItem(group->GetCount() - 1)->value);

            text.Append(_T("\n"));
        }
    }

    if (page_extensions)
    {
        if (!text.IsEmpty()) text.Append(_T("\n"));

        text.Append(_T("Extensions:\n"));
        text.Append(_T("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"));
        for (int i=0; i<page_extensions->info.GetCount(); i++)
        {
            GraphStudio::PropItem* group = page_extensions->info.GetItem(i);
            if (!group) continue;

            text.Append(group->name);
            text.Append(_T(" => "));
            text.Append(group->GetItem(0)->value);

            GraphStudio::PropItem* item = group->GetItemByName(_T("Name"));
			if (item) text.AppendFormat(_T(" %s"), (LPCTSTR)item->value);

            item = group->GetItemByName(_T("File"));
			if (item) text.AppendFormat(_T(" (%s)"), (LPCTSTR)item->value);

			text.AppendFormat(_T(" [%s]"), (LPCTSTR)group->GetItem(group->GetCount() - 1)->value);

            text.Append(_T("\n"));
        }
    }

    if (page_bytes)
    {
        if (!text.IsEmpty()) text.Append(_T("\n"));

        text.Append(_T("Byte Pattern:\n"));
        text.Append(_T("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"));
        for (int i=0; i<page_bytes->info.GetCount(); i++)
        {
            GraphStudio::PropItem* group = page_bytes->info.GetItem(i);
            if (!group) continue;

            text.Append(group->name);
            text.Append(_T("\n"));

            text.Append(_T("=> "));
            text.Append(group->GetItem(0)->value);

            GraphStudio::PropItem* item = group->GetItemByName(_T("Name"));
            int bytePatternIndex = 1;
            if (item) 
            {
				text.AppendFormat(_T(" %s"), (LPCTSTR)item->value);
                bytePatternIndex++;
            }

            item = group->GetItemByName(_T("File"));
            if (item)
            {
				text.AppendFormat(_T(" (%s)"), (LPCTSTR)item->value);
                bytePatternIndex++;
            }
            text.Append(_T("\n"));

            for (;bytePatternIndex < group->GetCount()-1; bytePatternIndex++)
            {
                item = group->GetItem(bytePatternIndex);
				text.AppendFormat(_T("\t%s: %s\n"), (LPCTSTR)item->name, (LPCTSTR)item->value);
            }

			text.AppendFormat(_T("[%s]\n\n"), (LPCTSTR)group->GetItem(group->GetCount() - 1)->value);
        }
    }


    DSUtil::SetClipboardText(this->GetSafeHwnd(),text);
}

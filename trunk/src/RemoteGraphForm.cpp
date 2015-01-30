//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "RemoteGraphForm.h"
#include <Psapi.h>


// CRemoteGraphForm dialog

IMPLEMENT_DYNAMIC(CRemoteGraphForm, CDialog)

BEGIN_MESSAGE_MAP(CRemoteGraphForm, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(IDC_BUTTON_REFRESH, &CRemoteGraphForm::OnRefreshClick)
	ON_COMMAND(IDC_BUTTON_CONNECT, &CRemoteGraphForm::OnConnectClick)
	ON_COMMAND(IDC_BUTTON_SPY_PROPERTYFRAME, &CRemoteGraphForm::OnPropertiesClick)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_GRAPHS, &CRemoteGraphForm::OnItemchangedListGraphs)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_GRAPHS, &CRemoteGraphForm::OnDblclkListGraphs)
END_MESSAGE_MAP()

CRemoteGraphForm::CRemoteGraphForm(CWnd* pParent /*=NULL*/) : 
	CDialog(CRemoteGraphForm::IDD, pParent)
{
}

CRemoteGraphForm::~CRemoteGraphForm()
{
}

void CRemoteGraphForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_GRAPHS, list_graphs);
	DDX_Control(pDX, IDC_TITLEBAR, title);
}

BOOL CRemoteGraphForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

    // prepare ListCtrl
    list_graphs.InsertColumn(0, _T("Process ID"), LVCFMT_RIGHT, 120);
    list_graphs.InsertColumn(1, _T("Porcess Name"), LVCFMT_LEFT, 150);
    list_graphs.InsertColumn(2, _T("Instance"), LVCFMT_LEFT, 80);
    list_graphs.InsertColumn(3, _T("Creation Time"), LVCFMT_LEFT, 80);
    list_graphs.InsertColumn(4, _T("Process Image File"), LVCFMT_LEFT, 350);
    list_graphs.SetExtendedStyle(list_graphs.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP );

	// create buttons
	CRect	rc;
	rc.SetRect(0, 0, 80, 25);
	btn_refresh.Create(_T("&Refresh"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_REFRESH);
	btn_refresh.SetFont(GetFont());
	btn_connect.Create(_T("&Connect"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_CONNECT);
	btn_connect.SetFont(GetFont());
	if(CanCreateSpyFilterGraphHelperInstance())
	{
		btn_properties.Create(_T("&Properties..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_SPY_PROPERTYFRAME);
		btn_properties.SetFont(GetFont());
	}

	SetWindowPos(NULL, 0, 0, 600, 250, SWP_NOMOVE);

	OnRefreshClick();

	return TRUE;
}

void CRemoteGraphForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(list_graphs)) {
		title.GetClientRect(&rc2);
		
		INT nButtonIndex = 0;
		static const INT g_nButtonWidth = 90;

		if(btn_properties.GetSafeHwnd())
			btn_properties.SetWindowPos(NULL, cx - ++nButtonIndex * (g_nButtonWidth + 6), 4, g_nButtonWidth, 25, SWP_SHOWWINDOW | SWP_NOZORDER);
		btn_connect.SetWindowPos(NULL, cx - ++nButtonIndex * (g_nButtonWidth + 6), 4, g_nButtonWidth, 25, SWP_SHOWWINDOW | SWP_NOZORDER);
		btn_refresh.SetWindowPos(NULL, cx - ++nButtonIndex * (g_nButtonWidth + 6), 4, g_nButtonWidth, 25, SWP_SHOWWINDOW | SWP_NOZORDER);

		list_graphs.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
		title.Invalidate();
		btn_refresh.Invalidate();
		btn_connect.Invalidate();
		if(btn_properties.GetSafeHwnd())
			btn_properties.Invalidate();
	}
}

void CRemoteGraphForm::OnRefreshClick()
{
	// let's load objects from ROT
	CComPtr<IRunningObjectTable>	rot;
	HRESULT							hr;

	graphs.RemoveAll();
    list_graphs.DeleteAllItems();
    sel_graph = RemoteGraph();

	hr = GetRunningObjectTable(0, &rot);
	if (FAILED(hr)) return ;

	// scan through running objects
	CComPtr<IEnumMoniker>			emon;
	CComPtr<IMoniker>				moniker;
	CComPtr<IBindCtx>				bindctx;
	ULONG							f;

	hr = CreateBindCtx(0, &bindctx);
	if (FAILED(hr)) {
		return ;
	}

    CAtlRegExp<> regex;
    REParseError status = regex.Parse(_T("^\\!FilterGraph {[0-9A-F]+} pid {[0-9A-F]+}(; process\\: {.+?}, time\\: {[0-9]+\\-[0-9]+\\-[0-9]+})?"), FALSE);

	rot->EnumRunning(&emon);
	emon->Reset();
	while (emon->Next(1, &moniker, &f) == NOERROR) {
		
		// is this a graph object ?
		LPOLESTR	displayname;
		moniker->GetDisplayName(bindctx, NULL, &displayname);

		CString		name(displayname);
		if (name.Find(_T("!FilterGraph")) == 0 && !GraphStudio::DisplayGraph::IsOwnRotGraph(name)) {
            RemoteGraph	gr = {0};

            CAtlREMatchContext<> mc;
            gr.name = name;
			gr.moniker = moniker;
            gr.pid = 0;
            gr.instance = 0;
            gr.processIsWOW64 = FALSE;

            if (regex.Match(name, &mc))
            {
                const CAtlREMatchContext<>::RECHAR* szStart = 0;
                const CAtlREMatchContext<>::RECHAR* szEnd = 0;
                mc.GetMatch(0, &szStart, &szEnd);
                int nLength = (int) (szEnd - szStart);
                CString textInstance(szStart, nLength);
                StrToInt64ExW(CStringW(L"0x") + textInstance, STIF_SUPPORT_HEX, &reinterpret_cast<LONGLONG&>(gr.instance));

                mc.GetMatch(1, &szStart, &szEnd);
                nLength = (int) (szEnd - szStart);
                CString textPID(szStart, nLength);
			    if (StrToIntExW(CStringW(L"0x") + textPID, STIF_SUPPORT_HEX, &reinterpret_cast<INT&>(gr.pid)))
                {
                    CHandle process;
					process.Attach(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, gr.pid));
					if (process)
                    {
                        TCHAR pszPath[MAX_PATH] = { 0 };
					    if (GetModuleFileNameEx(process, NULL, pszPath, sizeof(pszPath)))
                        {
                            gr.processImagePath = pszPath;

                            // Extract filename
                            int fileNamePos = gr.processImagePath.FindFileName();
                            if (fileNamePos >= 0)
                                gr.processImageFileName = CString(gr.processImagePath).Mid(fileNamePos);
                        }
                        else
                        {
                            // a 32Bit process can't list the modules of a 64Bit process, so try to get the processImageFileName from the ROT-Name (works only for FilterGraphSpy-Entries)
                            mc.GetMatch(2, &szStart, &szEnd);
                            nLength = (int) (szEnd - szStart);
                            if (nLength > 0)
                            {
                                CString textFileName(szStart, nLength);
                                gr.processImageFileName = textFileName;
                            }
                        }

                        IsWow64Process(process, &gr.processIsWOW64);
                    }
                }

                mc.GetMatch(3, &szStart, &szEnd);
                nLength = (int) (szEnd - szStart);
                if (nLength > 0)
                {
                    CString textTime(szStart, nLength);
                    textTime.Replace(_T("-"), _T(":"));
                    gr.time = textTime;
                }
            }
            graphs.Add(gr);

            CString entryName = gr.name;
            if (gr.pid > 0)
                entryName.Format(_T("%d (0x%08lX)"), gr.pid, gr.pid);
			int nIndex = list_graphs.InsertItem(list_graphs.GetItemCount(), entryName);
            
            if (gr.processIsWOW64)
            {
                CString val = gr.processImageFileName;
                val.Append(_T(" *32"));
                list_graphs.SetItemText(nIndex, 1, val);
            }
            else
                list_graphs.SetItemText(nIndex, 1, gr.processImageFileName);

            if (gr.instance > 0)
            {
                CString val;
                val.Format(_T("0x%I64d"), gr.instance);
                list_graphs.SetItemText(nIndex, 2, val);
            }

            list_graphs.SetItemText(nIndex, 3, gr.time);
            list_graphs.SetItemText(nIndex, 4, gr.processImagePath);

			if (graphs.GetCount() == 1) {
                list_graphs.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
                list_graphs.SetSelectionMark(0);
			}
		}

		if (displayname) {
			CComPtr<IMalloc>	alloc;
			if (SUCCEEDED(CoGetMalloc(0, &alloc))) {
				alloc->Free(displayname);
			}
		}
		moniker = NULL;
	}
}

void CRemoteGraphForm::OnConnectClick()
{
	OnOK();
}

void CRemoteGraphForm::OnPropertiesClick()
{
	if(sel_graph.moniker)
		SpyDoPropertyFrameModal(sel_graph.moniker);
}

void CRemoteGraphForm::OnItemchangedListGraphs(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVNI_SELECTED))
    {
        const int sel = pNMListView->iItem;
        sel_graph = RemoteGraph();
	    if (sel >= 0 && sel < graphs.GetCount()) {
		    sel_graph = graphs[sel];
	    }
		// WARN: We don't seem to clear sel_graph when no items focused/selected
		btn_connect.EnableWindow(sel_graph.moniker != NULL);
		if(btn_properties.GetSafeHwnd())
			btn_properties.EnableWindow(sel_graph.moniker != NULL);
    }
}

void CRemoteGraphForm::OnDblclkListGraphs(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pItem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    *pResult = 0;

	if(!pItem || pItem->iItem < 0)
		return;
	if(GetKeyState(VK_CONTROL) < 0) // Ctrl + DblClk
	{
		ATLASSERT(sel_graph.moniker);
		SpyDoPropertyFrameModal(sel_graph.moniker);
		return;
	}
	OnOK();	
}

#include <afxole.h>

bool CRemoteGraphForm::CanCreateSpyFilterGraphHelperInstance()
{
	// WARN: This is not accurate since we are interested in installed spy with bitness that matches remote process
	class __declspec(uuid("5A9A684C-A891-4032-8D31-FF6EAB5A0C1E")) FilterGraphHelper;
	CComPtr<IUnknown> pUnknown;
	return SUCCEEDED(pUnknown.CoCreateInstance(__uuidof(FilterGraphHelper)));
}

bool CRemoteGraphForm::SpyDoPropertyFrameModal(IMoniker* pMoniker)
{
	ATLASSERT(pMoniker);
	_ATLTRY
	{
		CComPtr<IRunningObjectTable> pRunningObjectTable;
		ATLENSURE_SUCCEEDED(GetRunningObjectTable(0, &pRunningObjectTable));
		CComPtr<IBindCtx> pBindCtx;
		ATLENSURE_SUCCEEDED(CreateBindCtx(0, &pBindCtx));
		CComPtr<IUnknown> pUnknown;
		ATLENSURE_SUCCEEDED(pRunningObjectTable->GetObject(sel_graph.moniker, &pUnknown));
		// NOTE: Doing IDispatch::Invoke to not reference/import external TLB and be dependent from it while building
		class __declspec(uuid("6945711B-FE0F-4C54-965F-5B67969C28B7")) ISpy;
		const CComQIPtr<IDispatch, &__uuidof(ISpy)> pSpyDispatch = pUnknown;
		if(!pSpyDispatch)
			return false;
		ATLASSERT(pSpyDispatch);
		// [id(4)] HRESULT DoPropertyFrameModal([in] LONG nParentWindowHandle);
		CComVariant vParentWindowHandle((LONG) (LONG_PTR) m_hWnd);
		DISPPARAMS pParameters[1];
		ZeroMemory(pParameters, sizeof pParameters);
		pParameters[0].cArgs = 1;
		pParameters[0].rgvarg = &vParentWindowHandle;
		CComVariant vResult;
		// NOTE: https://support.microsoft.com/kb/248019/en?wa=wsignin1.0
		AfxOleGetMessageFilter()->EnableNotRespondingDialog(FALSE);
		const HRESULT nResult = pSpyDispatch->Invoke(4, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, pParameters, &vResult, NULL, NULL);
		AfxOleGetMessageFilter()->EnableNotRespondingDialog(TRUE);
		ATLENSURE_SUCCEEDED(nResult);
	}
	_ATLCATCHALL()
	{
		MessageBeep(MB_ICONERROR);
		return false;
	}
	return true;
}


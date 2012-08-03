//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "RemoteGraphForm.h"


// CRemoteGraphForm dialog

IMPLEMENT_DYNAMIC(CRemoteGraphForm, CDialog)

BEGIN_MESSAGE_MAP(CRemoteGraphForm, CDialog)
	ON_WM_SIZE()
	ON_COMMAND(IDC_BUTTON_REFRESH, &CRemoteGraphForm::OnRefreshClick)
	ON_COMMAND(IDC_BUTTON_CONNECT, &CRemoteGraphForm::OnConnectClick)
	ON_LBN_SELCHANGE(IDC_LIST_GRAPHS, &CRemoteGraphForm::OnLbnSelchangeListGraphs)
END_MESSAGE_MAP()

CRemoteGraphForm::CRemoteGraphForm(CWnd* pParent /*=NULL*/) : 
	CDialog(CRemoteGraphForm::IDD, pParent)
{
	sel_graph = NULL;
    isOwnGraph = false;
}

CRemoteGraphForm::~CRemoteGraphForm()
{
	graphs.RemoveAll();
	sel_graph = NULL;
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

	// create buttons
	CRect	rc;
	rc.SetRect(0, 0, 80, 25);
	btn_refresh.Create(_T("Refresh"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_REFRESH);
	btn_refresh.SetFont(GetFont());
	btn_connect.Create(_T("Connect"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_CONNECT);
	btn_connect.SetFont(GetFont());

	SetWindowPos(NULL, 0, 0, 400, 250, SWP_NOMOVE);

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

		btn_refresh.SetWindowPos(NULL, cx - 2*(60+6), 4, 60, 25, SWP_SHOWWINDOW);
		btn_connect.SetWindowPos(NULL, cx - 1*(60+6), 4, 60, 25, SWP_SHOWWINDOW);

		list_graphs.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW);

		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW);
		title.Invalidate();
		btn_refresh.Invalidate();
		btn_connect.Invalidate();
	}
}

void CRemoteGraphForm::OnRefreshClick()
{
	// let's load objects from ROT
	CComPtr<IRunningObjectTable>	rot;
	HRESULT							hr;

	graphs.RemoveAll();
	list_graphs.ResetContent();
	sel_graph = NULL;

	hr = GetRunningObjectTable(0, &rot);
	if (FAILED(hr)) return ;

	// scan through running objects
	CComPtr<IEnumMoniker>			emon;
	CComPtr<IMoniker>				moniker;
	CComPtr<IBindCtx>				bindctx;
	ULONG							f;

	hr = CreateBindCtx(0, &bindctx);
	if (FAILED(hr)) {
		rot = NULL;
		return ;
	}


	rot->EnumRunning(&emon);
	emon->Reset();
	while (emon->Next(1, &moniker, &f) == NOERROR) {
		
		// is this a graph object ?
		LPOLESTR	displayname;
		moniker->GetDisplayName(bindctx, NULL, &displayname);

		CString		name(displayname);
		if (name.Find(_T("!FilterGraph")) == 0) {
			RemoteGraph	gr;
			gr.name = name;
			gr.moniker = moniker;
			gr.moniker->AddRef();

			list_graphs.AddString(name);
			graphs.Add(gr);

			if (graphs.GetCount() == 1) {
				list_graphs.SetCurSel(0);
				OnLbnSelchangeListGraphs();
			}
		}

		if (displayname) {
			CComPtr<IMalloc>	alloc;
			if (SUCCEEDED(CoGetMalloc(0, &alloc))) {
				alloc->Free(displayname);
			}
			alloc = NULL;
		}
		moniker = NULL;
	}
	emon = NULL;
	bindctx = NULL;

	rot = NULL;
}

void CRemoteGraphForm::OnConnectClick()
{
    if(isOwnGraph)
    {
        MessageBox(_T("Can't connect to own graph!"),_T("Can't connect"), MB_OK|MB_ICONHAND);
    }
    else
	    OnOK();
}

void CRemoteGraphForm::OnLbnSelchangeListGraphs()
{
	// TODO: Add your control notification handler code here
	int sel = list_graphs.GetCurSel();

	sel_graph = NULL;
	if (sel >= 0 && sel < graphs.GetCount()) {
		sel_graph = graphs[sel].moniker;

        CString strPid;
        strPid.Format(_T("%08x"), GetCurrentProcessId());
        isOwnGraph = graphs[sel].name.Find(strPid) != -1;
	}
}

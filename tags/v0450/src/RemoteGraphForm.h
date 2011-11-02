//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


class RemoteGraph
{
public:
	IMoniker	*moniker;
	CString		name;
public:
	RemoteGraph() : moniker(NULL), name(_T("")) { }
	RemoteGraph(const RemoteGraph &g) : name(g.name), moniker(NULL) {
		moniker = g.moniker;
		if (moniker) moniker->AddRef();
	}
	RemoteGraph &operator =(const RemoteGraph &g) {
		if (moniker) moniker->Release();
		moniker = g.moniker;
		if (moniker) moniker->AddRef();
		name = g.name;
		return *this;
	}
	virtual ~RemoteGraph() {
		if (moniker) moniker->Release();
		moniker = NULL;
	}
};

//-----------------------------------------------------------------------------
//
//	CRemoteGraphForm class
//
//-----------------------------------------------------------------------------

class CRemoteGraphForm : public CDialog
{
protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CRemoteGraphForm)

	virtual void DoDataExchange(CDataExchange* pDX);

public:
	GraphStudio::TitleBar	title;
	CListBox				list_graphs;
	CButton					btn_refresh;
	CButton					btn_connect;
	CArray<RemoteGraph>		graphs;

	CComPtr<IMoniker>		sel_graph;

public:
	CRemoteGraphForm(CWnd* pParent = NULL);
	virtual ~CRemoteGraphForm();

	enum { IDD = IDD_DIALOG_ROT };

	// initialization
	virtual BOOL OnInitDialog();
	void OnSize(UINT nType, int cx, int cy);

	void OnRefreshClick();
	void OnConnectClick();

	afx_msg void OnLbnSelchangeListGraphs();
};

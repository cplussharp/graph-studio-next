//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


struct RemoteGraph
{
public:
	CComPtr<IMoniker>	moniker;
	CString				name;
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

	GraphStudio::TitleBar	title;
	CListBox				list_graphs;
	CButton					btn_refresh;
	CButton					btn_connect;
	CArray<RemoteGraph>		graphs;

public:
	RemoteGraph				sel_graph;

public:
	CRemoteGraphForm(CWnd* pParent = NULL);
	virtual ~CRemoteGraphForm();

protected:
	enum { IDD = IDD_DIALOG_ROT };

	// initialization
	virtual BOOL OnInitDialog();
	void OnSize(UINT nType, int cx, int cy);

	void OnRefreshClick();
	void OnConnectClick();

	afx_msg void OnLbnSelchangeListGraphs();
	afx_msg void OnDblclkListGraphs();
};

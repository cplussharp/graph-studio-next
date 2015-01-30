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
    INT                 pid;
    LONGLONG            instance;
    CString             time;
    CString             processImageFileName;
    CPath               processImagePath;
    BOOL                processIsWOW64;
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
	CListCtrl				list_graphs;
	CButton					btn_refresh;
	CButton					btn_connect;
	CButton					btn_properties;
	CArray<RemoteGraph>		graphs;

public:
	RemoteGraph				sel_graph;

public:
	CRemoteGraphForm(CWnd* pParent = NULL);
	virtual ~CRemoteGraphForm();

	static bool CanCreateSpyFilterGraphHelperInstance();
	bool SpyDoPropertyFrameModal(IMoniker* pMoniker);

protected:
	enum { IDD = IDD_DIALOG_ROT };

	// initialization
	virtual BOOL OnInitDialog();
	void OnSize(UINT nType, int cx, int cy);

	void OnRefreshClick();
	void OnConnectClick();
	void OnPropertiesClick();

	afx_msg void OnItemchangedListGraphs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListGraphs(NMHDR *pNMHDR, LRESULT *pResult);
};

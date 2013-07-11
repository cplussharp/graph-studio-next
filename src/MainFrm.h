//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
//
//	CMainFrame class
//
//-----------------------------------------------------------------------------

class CMainFrame : public CFrameWnd
{
public:
	CGraphView		*view;
	CToolBar		m_wndToolBar;
	CSeekingBar		m_wndSeekingBar;

	bool			TranslateKeyboardAccelerator(MSG *pMSG);	// used for processing keystrokes in modeless dialogs

protected:
	CStatusBar		m_wndStatusBar;
	CReBar			m_wndReBar;
    CComPtr<ITaskbarList3> m_pTaskbarList;
    CComboBox       m_comboRate;
    int             m_comboRate_defaultSel;

    static const UINT m_uTaskbarBtnCreatedMsg;

protected: 
	CMainFrame();
	virtual ~CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
	DECLARE_MESSAGE_MAP()

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    LRESULT OnTaskbarBtnCreated ( UINT uMsg, WPARAM wParam, LPARAM lParam );

	// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 
							CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
    void OnComboRateChanged();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

    void OnUpdatePlayRate();

	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnFileClose();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
};

// for ToolTips
extern CGraphView* GetActiveView();
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
protected: 
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
	DECLARE_MESSAGE_MAP()

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    LRESULT OnTaskbarBtnCreated ( UINT uMsg, WPARAM wParam, LPARAM lParam );

public:  // control bar embedded members
	CStatusBar		m_wndStatusBar;
	CToolBar		m_wndToolBar;
	CToolBar		m_wndPlaybackBar;
	CSeekingBar		m_wndSeekingBar;
	CReBar			m_wndReBar;
    CComPtr<ITaskbarList3> m_pTaskbarList;
    CComboBox       m_comboRate;
    int             m_comboRate_defaultSel;

	// view
	CGraphView		*view;

	// Overrides
public:
	virtual ~CMainFrame();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    void OnComboRateChanged();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

    void OnUpdatePlayRate();

    static const UINT m_uTaskbarBtnCreatedMsg;
};



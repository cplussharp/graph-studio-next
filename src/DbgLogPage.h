//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
//
//	CDbgLogPage class
//
//-----------------------------------------------------------------------------
class CDbgLogPage : public CDSPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

public:
	GraphStudio::TitleBar	title;
	CButton					btn_refresh;
	CButton					btn_settings;
	CButton					btn_locate;
	CButton					btn_clear;
	CEdit					edit_log;
	CFont			        font_log;
	CEdit					edit_filter;

	const CString			filterFile;
	const CString			logFile;
	int						lastFileSize;
	DWORD					fileStartOffset;	// the offset to display from in the log file (log files > 2GB not supported)

	CString					filterString;
	bool					filterRegexValid;
	CAtlRegExp<>			filterRegex;
	bool					restoreSelectionOnRefresh;
	bool					refreshOnTimer;

    enum { IDD = IDD_PROPPAGE_DBGLOG };
public:
	CDbgLogPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle, const CString& filterFile, const CString& logFile);
	virtual ~CDbgLogPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnSize(UINT nType, int cx, int cy);
	void OnTimer(UINT_PTR id);

	virtual HRESULT OnActivate();
	virtual HRESULT OnDeactivate();
    afx_msg void OnBnClickedRefresh();
	afx_msg void OnBnClickedSettings();
	afx_msg void OnLocateClick();
	afx_msg void OnClearLogClick();
	afx_msg void OnUpdateFilterString();

	void RefreshLog();

private:
	CRect GetButtonRect() const;
};


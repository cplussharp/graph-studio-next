//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#pragma once

class CComDllAnalyzerForm : public CGraphStudioModelessDialog
{
protected:
	DECLARE_DYNAMIC(CComDllAnalyzerForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);

public:
	GraphStudio::TitleBar	title;
	CButton         btn_open;
	CButton         btn_copy;
	CButton         btn_save;
	CEdit			edit_report;
	CFont			font_report;

	CComDllAnalyzerForm(CWnd* pParent = NULL);
	virtual ~CComDllAnalyzerForm();
	virtual CRect GetDefaultRect() const;

	// Dialog Data
	enum { IDD = IDD_DIALOG_COMDLLANALYZER };
	BOOL DoCreateDialog(CWnd* parent);

	void OnSize(UINT nType, int cx, int cy);
	void OnInitialize();

	afx_msg void OnClickedButtonOpen();
	afx_msg void OnClickedButtonCopytext();
	afx_msg void OnClickedButtonSave();
};


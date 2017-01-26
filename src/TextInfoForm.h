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
//	CTextInfoForm dialog
//
//-----------------------------------------------------------------------------

class CTextInfoForm : public CGraphStudioModelessDialog
{
protected:
	DECLARE_DYNAMIC(CTextInfoForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);

public:

    GraphStudio::TitleBar	title;
    CButton         btn_copy;
    CButton         btn_save;
	CEdit			edit_report;
	CComboBox		combo_reporttype;
	CFont			font_report;

public:

	CTextInfoForm(CWnd* pParent = NULL); 
	virtual ~CTextInfoForm();
	virtual CRect GetDefaultRect() const;

	// Dialog Data
	enum { IDD = IDD_DIALOG_TEXTVIEW };
    BOOL DoCreateDialog(CWnd* parent);

	void OnSize(UINT nType, int cx, int cy);
	BOOL PreTranslateMessage(MSG *pmsg);
	void OnInitialize();
	void OnBnClickedButtonRefresh();

	afx_msg void OnBnClickedButtonCopytext();
    afx_msg void OnClickedButtonSave();
};

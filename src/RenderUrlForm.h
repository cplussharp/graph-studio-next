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
//	CRenderUrlForm class
//
//-----------------------------------------------------------------------------
class CRenderUrlForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CRenderUrlForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);
public:
	GraphStudio::TitleBar	title;
	CButton					radio_url;
	CComboBox				combo_url;
	CString					result_file;

	// list of recent URLs/Files
	GraphStudio::FilenameList		url_list;

public:
	CRenderUrlForm(CWnd* pParent = NULL);   
	virtual ~CRenderUrlForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_RENDERURL };

	afx_msg void OnBnClickedRadioUrl();

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonClear();
};

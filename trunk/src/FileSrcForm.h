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
//	CFileSrcForm class
//
//-----------------------------------------------------------------------------
class CFileSrcForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CFileSrcForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);
public:
	GraphStudio::TitleBar	title;
	CButton					radio_file;
	CButton					radio_url;
	CButton					button_browse;
	CComboBox				combo_file;
	CComboBox				combo_url;

	CString					result_file;
    CString                 filter_name;

	// list of recent URLs/Files
	GraphStudio::FilenameList		file_list;
	GraphStudio::FilenameList		url_list;

public:
	CFileSrcForm(CWnd* pParent = NULL);  
    CFileSrcForm(const CString& filterName, CWnd* pParent = NULL); 
	virtual ~CFileSrcForm();

	static HRESULT ChooseSourceFile(IFileSourceFilter* fs, const CString& filterName);

	// Dialog Data
	enum { IDD = IDD_DIALOG_ASYNCOPEN };

	afx_msg void OnBnClickedRadioFile();
	afx_msg void OnBnClickedRadioUrl();

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonClear();
};

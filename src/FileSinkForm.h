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
//	CFileSinkForm class
//
//-----------------------------------------------------------------------------
class CFileSinkForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CFileSinkForm)
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
	CFileSinkForm(CWnd* pParent = NULL);
    CFileSinkForm(const CString& filtername, CWnd* pParent = NULL); 
	virtual ~CFileSinkForm();

	HRESULT ChooseSinkFile(IFileSinkFilter* fs);		// wrapper to do it in one call
	
	// Dialog Data
	enum { IDD = IDD_DIALOG_FILESINK };

	afx_msg void OnBnClickedRadioFile();
	afx_msg void OnBnClickedRadioUrl();

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonClear();


};

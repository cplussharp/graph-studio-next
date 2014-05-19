#pragma once


// CFilterFromFile-Dialogfeld

class CFilterFromFile : public CDialog
{
	DECLARE_DYNAMIC(CFilterFromFile)

public:
	CFilterFromFile(CWnd* pParent = NULL);   // Standardkonstruktor
	virtual ~CFilterFromFile();

// Dialogfelddaten
	enum { IDD = IDD_DIALOG_FILTERFROMFILE };

public:
	GraphStudio::TitleBar	title;
	CButton					button_browse;
	CComboBox				combo_file;
	CComboBox				combo_clsid;

	CString					result_file;
    CLSID                   result_clsid;
    IClassFactory*          filterFactory;

	// list of recent URLs/Files
	GraphStudio::FilenameList		file_list;
	GraphStudio::FilenameList		clsid_list;

	HRESULT					hr;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()
    virtual void OnOK();
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedButtonBrowse();
    afx_msg void OnClickedButtonClear();
    afx_msg void OnChangeComboFile();
};

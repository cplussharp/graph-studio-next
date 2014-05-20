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
    CListCtrl               list_clsid;

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
	static HRESULT GetClassFactoryEntryPoint(LPCOLESTR dll_file, LPFNGETCLASSOBJECT & entry_point);

    virtual BOOL OnInitDialog();

    afx_msg void OnClickedButtonBrowse();
    afx_msg void OnClickedButtonClear();
    afx_msg void OnChangeComboFile();
	afx_msg void OnBnClickedButtonScanClsids();
	afx_msg void OnLvnColumnclickListData(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListData(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListData(NMHDR *pNMHDR, LRESULT *pResult);
};

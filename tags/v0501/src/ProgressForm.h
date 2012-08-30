//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

class CGraphView;

//-----------------------------------------------------------------------------
//
//	CProgressForm class
//
//-----------------------------------------------------------------------------
class CProgressForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CProgressForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);
public:
	GraphStudio::TitleBar	title;

	CGraphView				*view;
	CButton					button_close;
	CButton					check_close;
	CStatic					label_caption;
	CStatic					label_time;
	CProgressCtrl			progress;

public:
	CProgressForm(CWnd* pParent = NULL);   
	virtual ~CProgressForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_PROGRESS };

	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	void OnClose();

	void UpdateCaption(CString text);
	void UpdateTimeLabel(CString text);
	void UpdateProgress(double pos);

	void OnBnClickedButtonClose();

	void OnGraphStopped();
};

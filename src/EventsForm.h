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
//	CEventsForm class
//
//-----------------------------------------------------------------------------
class CEventsForm : public CGraphStudioModelessDialog
{
protected:
	DECLARE_DYNAMIC(CEventsForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	GraphStudio::TitleBar	title;
	CListBox		list_events;
	CButton			btn_clear;
	CButton			btn_copy;

public:
	CEventsForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEventsForm();

	enum { IDD = IDD_DIALOG_EVENTS };

	// initialization
	BOOL DoCreateDialog(CWnd* parent);

	void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	void OnGraphEvent(long evcode, LONG_PTR param1, LONG_PTR param2);
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonCopy();
};

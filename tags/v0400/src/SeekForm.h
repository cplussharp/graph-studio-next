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
//	CSeekForm class
//
//-----------------------------------------------------------------------------
class CSeekForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CSeekForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	GraphStudio::TitleBar	title;
	CGraphView				*view;
	CStatic					label_duration;
	CStatic					label_position;
	CStatic					label_fps;
	CButton					radio_time;
	CButton					radio_frame;
	CButton					check_keyframe;
	CEdit					edit_time;
	CEdit					edit_frame;
	CCheckListBox			list_caps;

	__int64					caps;

public:
	CSeekForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSeekForm();

	enum { IDD = IDD_DIALOG_SEEK };

	// initialization
	BOOL DoCreateDialog();
	void OnSize(UINT nType, int cx, int cy);

	void OnTimer(UINT_PTR id);
	void UpdateGraphPosition();

	void OnTimeClick();
	void OnFrameClick();
	void GetCurrentCaps(__int64 &c);

	virtual void OnOK();
};

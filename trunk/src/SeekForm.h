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
class CSeekForm : public CGraphStudioModelessDialog
{
public:
	CSeekForm(CGraphView* graph_view, CWnd* pParent = NULL);
	virtual ~CSeekForm();

	BOOL DoCreateDialog();
	void UpdateGraphPosition();

protected:
	DECLARE_DYNAMIC(CSeekForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	GraphStudio::TitleBar	title;
	CCheckListBox			list_caps;

	GUID					time_format;

	// Cache these values to avoid excessive updating
	__int64					cached_caps;
	LONGLONG				cached_cur_pos;
	LONGLONG				cached_stop;
	LONGLONG				cached_duration;
	LONGLONG				cached_preroll;
	LONGLONG				cached_available_start;
	LONGLONG				cached_available_end;
	double					cached_fps;
	double					cached_rate;

protected:
	enum { IDD = IDD_DIALOG_SEEK };

	// initialization
	void OnSize(UINT nType, int cx, int cy);

	void OnTimer(UINT_PTR id);
	virtual void OnOK();

	CString FormatTimeString(LONGLONG time);
	bool ParseTimeString(const CString& time_str, LONGLONG& time);
	void SetTimeFormat(const GUID& new_time_format);
	void ResetCachedValues();

	int GetCurrentCaps();
	void EnableControls();

	afx_msg void OnFormatTimeClick();
	afx_msg void OnFormatFrameClick();
	afx_msg void OnFormatFieldClick();
	afx_msg void OnFormatSampleClick();
	afx_msg void OnFormatByteClick();
	afx_msg void OnCheckSetCurrentPosition();
	afx_msg void OnCheckSetStopPosition();
	afx_msg void OnCheckStopRelativeToCurrent();
	afx_msg void OnCheckStopRelativeToPrevious();
	afx_msg void OnClickedButtonSetPreroll();
	afx_msg void OnClickedButtonSetRate();
};

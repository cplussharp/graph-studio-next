//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

class CGraphView;

// No need to make this a CGraphStudioModelessDialog as it's not really a floating window
class CVolumeBarForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CVolumeBarForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	CSliderCtrl				slider_volume;
	CSliderCtrl				slider_balance;
	CStatic					label_volume;
	CStatic					label_balance;

	// we deal with this filter
	CComPtr<IBasicAudio>	basic_audio;

public:
	CVolumeBarForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVolumeBarForm();

	enum { IDD = IDD_DIALOG_VOLUMEBAR };

	// initialization
	BOOL DoCreateDialog();
	void DoHide();
	void DisplayVolume(IBaseFilter *filter);

	void OnKillFocus(CWnd *pNewWnd);
	void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);

	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	void UpdateLevels();
	void RefreshLevels();
};

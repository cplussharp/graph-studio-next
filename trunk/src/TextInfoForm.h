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
//	CTextInfoForm dialog
//
//-----------------------------------------------------------------------------

class CTextInfoForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CTextInfoForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);

public:

	CEdit			edit_report;
	CComboBox		combo_reporttype;
	CFont			font_report;

	CArray<CString>	lines;

	CGraphView		*view;

public:

	CTextInfoForm(CWnd* pParent = NULL); 
	virtual ~CTextInfoForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_TEXTVIEW };

	void OnSize(UINT nType, int cx, int cy);
	void OnInitialize();
	void OnBnClickedButtonRefresh();

	// report parts
	void DoFilterList();
	void DoConnectionDetails(int level, int offset);
	void DoPinDetails(GraphStudio::Pin *pin, int level, int offset);
	void DoMediaTypeDetails(AM_MEDIA_TYPE *pmt, int level, int offset);

	// formats
	void DoWaveFormatEx(AM_MEDIA_TYPE *pmt, int level, int offset);
	void DoVideoInfo(AM_MEDIA_TYPE *pmt, int level, int offset);
	void DoVideoInfo2(AM_MEDIA_TYPE *pmt, int level, int offset);
	void DoMPEG2VideoInfo(AM_MEDIA_TYPE *pmt, int level, int offset);
	void DoBitmapInfoHeader(BITMAPINFOHEADER *bmi, int offset);


	void DoDumpRawBuffer(void *buf, int len, int offset);

	void DoSimpleReport();
	void Echo(CString t);
	void DisplayReport();
	afx_msg void OnBnClickedButtonCopytext();
    afx_msg void OnClickedButtonSave();
};

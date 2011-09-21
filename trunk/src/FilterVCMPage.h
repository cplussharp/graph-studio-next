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
//	CFilterVCMPage class
//
//-----------------------------------------------------------------------------
class CFilterVCMPage : public CDSPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

public:

	GraphStudio::TitleBar	title;
	CButton					btn_config;
	CButton					btn_about;

	CComPtr<IAMVfwCompressDialogs>		vfwdialogs;

	enum { IDD = IDD_DIALOG_VCM_DIALOGS };
public:
	CFilterVCMPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CFilterVCMPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnSize(UINT nType, int cx, int cy);

    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();

	void OnConfigClick();
	void OnAboutClick();
};

//-----------------------------------------------------------------------------
//
//	CVideoCompressionPage class
//
//-----------------------------------------------------------------------------
class CVideoCompressionPage : public CDSPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

public:

	GraphStudio::TitleBar	title;

	CStatic					label_desc;
	CStatic					label_version;

	CStatic					label_irate;
	CSliderCtrl				tb_irate;
	CStatic					label_irate_val;
	CButton					btn_def_irate;

	CStatic					label_prate;
	CSliderCtrl				tb_prate;
	CStatic					label_prate_val;
	CButton					btn_def_prate;

	CStatic					label_quality;
	CSliderCtrl				tb_quality;
	CStatic					label_quality_val;
	CButton					btn_def_quality;

	CStatic					label_window;
	CSliderCtrl				tb_window;
	CStatic					label_window_val;
	CButton					btn_def_window;

	// Default values
	long					def_irate;
	long					def_prate;
	double					def_quality;
	long					def_window;
	long					flags;

	CString					desc;
	CString					version;

	CComPtr<IAMVideoCompression>		comp;

	enum { IDD = IDD_DIALOG_VCM };
public:
	CVideoCompressionPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CVideoCompressionPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();

	void UpdateIRate();
	void UpdatePRate();
	void UpdateQuality();
	void UpdateWindowSize();

	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);

	void OnIRateDefault();
	void OnPRateDefault();
	void OnQualityDefault();
	void OnWindowDefault();
};

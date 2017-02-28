//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once
#include "afxwin.h"

//-----------------------------------------------------------------------------
//
//	CSbeSinkPage class
//
//-----------------------------------------------------------------------------
class CSbeSinkPage : public CDSPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

private:

	GraphStudio::TitleBar				title;
	CComboBox				            combo_profile;
	CComboBox				            combo_recording;
	GraphStudio::FilenameList		    profile_list;
	GraphStudio::FilenameList		    recording_list;
	GraphStudio::Filter *				filter;
	CComPtr<IStreamBufferSink2>			sink;

    enum { IDD = IDD_PROPPAGE_STREAMBUFFERSINK };

	void RefreshControls();

public:
	CSbeSinkPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle, GraphStudio::Filter * filter);
	virtual ~CSbeSinkPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnTimer(UINT_PTR id);

    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();
	virtual HRESULT OnActivate();
	virtual HRESULT OnDeactivate();
	virtual HRESULT OnApplyChanges();
    afx_msg void OnBnClickedButtonBrowse();
    afx_msg void OnBnClickedButtonLock();
	afx_msg void OnBnClickedButtonRecordingBrowse();
	afx_msg void OnBnClickedButtonCreateRecording();
	afx_msg void OnBnClickedButtonStartRecording();
	afx_msg void OnBnClickedButtonStopRecording();
	afx_msg void OnBnClickedButtonCloseRecording();
	afx_msg void OnBnClickedButtonUnlock();
};


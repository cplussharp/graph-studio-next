//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "WMResizerPage.h"
#include "SbeSinkPage.h"


//-----------------------------------------------------------------------------
//
//	CSbeSinkPage class
//
//-----------------------------------------------------------------------------

static const TCHAR * const profile_cache = _T("SBE-ProfileCache");
static const TCHAR * const recording_cache = _T("SBE-RecordingCache");


BEGIN_MESSAGE_MAP(CSbeSinkPage, CDSPropertyPage)
	ON_WM_SIZE()
	ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CSbeSinkPage::OnBnClickedButtonBrowse)
    ON_BN_CLICKED(IDC_BUTTON_LOCK, &CSbeSinkPage::OnBnClickedButtonLock)
	ON_BN_CLICKED(IDC_BUTTON_RECORDING_BROWSE, &CSbeSinkPage::OnBnClickedButtonRecordingBrowse)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_RECORDING, &CSbeSinkPage::OnBnClickedButtonCreateRecording)
	ON_BN_CLICKED(IDC_BUTTON_START_RECORDING, &CSbeSinkPage::OnBnClickedButtonStartRecording)
	ON_BN_CLICKED(IDC_BUTTON_STOP_RECORDING, &CSbeSinkPage::OnBnClickedButtonStopRecording)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE_RECORDING, &CSbeSinkPage::OnBnClickedButtonCloseRecording)
	ON_BN_CLICKED(IDC_BUTTON_UNLOCK, &CSbeSinkPage::OnBnClickedButtonUnlock)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CSbeSinkPage class
//
//-----------------------------------------------------------------------------
CSbeSinkPage::CSbeSinkPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle, GraphStudio::Filter * filter) :
	CDSPropertyPage(_T("SbeSinkPage"), pUnk, IDD, strTitle),
	filter(filter)
{
	// retval
	if (phr) *phr = NOERROR;
}

CSbeSinkPage::~CSbeSinkPage()
{
}


BOOL CSbeSinkPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	return TRUE;
}

void CSbeSinkPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_COMBO_FILE, combo_profile);
	DDX_Control(pDX, IDC_COMBO_RECORDING_FILE, combo_recording);
}

HRESULT CSbeSinkPage::OnConnect(IUnknown *pUnknown)
{
    HRESULT hr = pUnknown->QueryInterface(__uuidof(sink), (void**)&sink);
	if (FAILED(hr)) return E_FAIL;

	if (!filter)
		return E_POINTER;

	return NOERROR;
}

HRESULT CSbeSinkPage::OnActivate()
{
	RefreshControls();

    // load saved lists
	profile_list.LoadList(profile_cache);
    for (int i=0; i<profile_list.GetCount(); i++) combo_profile.AddString(profile_list[i]);

	recording_list.LoadList(recording_cache);
	for (int i=0; i<recording_list.GetCount(); i++) combo_recording.AddString(recording_list[i]);

	SetTimer(0, 200, NULL);

	return NOERROR;
}

HRESULT CSbeSinkPage::OnDeactivate()
{
	KillTimer(0);
	return NOERROR;
}

HRESULT CSbeSinkPage::OnDisconnect()
{
	sink = NULL;

	// leave any recording object open as it's part of the filter

	return NOERROR;
}

HRESULT CSbeSinkPage::OnApplyChanges()
{
	return NOERROR;
}

void CSbeSinkPage::OnTimer(UINT_PTR id)
{
	ASSERT(id==0);
	RefreshControls();
}

void CSbeSinkPage::RefreshControls()
{
	const bool locked = sink && S_OK == sink->IsProfileLocked() ? TRUE : FALSE;

	BOOL started = FALSE, stopped = FALSE;
	HRESULT previous_status = S_OK;
	if (filter->recording) {
		filter->recording->GetRecordingStatus(&previous_status, &started, &stopped);
	}

	// can't lock or set profile if already locked
	const BOOL can_lock = locked ? FALSE : TRUE;
	GetDlgItem(IDC_COMBO_FILE)->EnableWindow(can_lock);
	GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(can_lock);
	GetDlgItem(IDC_BUTTON_LOCK)->EnableWindow(can_lock);

	const BOOL can_unlock = locked ? TRUE : FALSE;
	GetDlgItem(IDC_BUTTON_UNLOCK)->EnableWindow(can_unlock);

	// can only set recording file if locked and not already recording
	const BOOL can_create = locked && !filter->recording? TRUE : FALSE;
	GetDlgItem(IDC_COMBO_RECORDING_FILE)->EnableWindow(can_create);
	GetDlgItem(IDC_BUTTON_RECORDING_BROWSE)->EnableWindow(can_create);
	GetDlgItem(IDC_CHECK_REFERENCE_RECORDING)->EnableWindow(can_create);
	GetDlgItem(IDC_BUTTON_CREATE_RECORDING)->EnableWindow(can_create);

	const BOOL can_close = filter->recording? TRUE : FALSE;
	GetDlgItem(IDC_BUTTON_CLOSE_RECORDING)->EnableWindow(can_close);

	const BOOL can_start = locked && filter->recording && started==FALSE ? TRUE : FALSE;
	GetDlgItem(IDC_BUTTON_START_RECORDING)->EnableWindow(can_start);

	const BOOL can_stop = locked && filter->recording && stopped==FALSE ? TRUE : FALSE;
	GetDlgItem(IDC_BUTTON_STOP_RECORDING)->EnableWindow(can_stop);

	// stop time and start time edit controls enabled for convenient editing but enable/disable stop and start recording buttons
}

void CSbeSinkPage::OnBnClickedButtonBrowse()
{
	CString		filter;
	CString		filename;

	filter = _T("All Files|*.*|");

	CFileDialog dlg(FALSE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,filter);
    INT_PTR ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		combo_profile.SetWindowText(filename);
	}
}

void CSbeSinkPage::OnBnClickedButtonLock()
{
    CString result_file;
    combo_profile.GetWindowText(result_file);
    HRESULT hr = sink->LockProfile(result_file.GetLength() > 0 ? result_file : NULL);
    if(FAILED(hr))
    {
        DSUtil::ShowError(hr, _T("Can't lock profile"));
    } else {
		profile_list.UpdateList(result_file);
		profile_list.SaveList(profile_cache);
	}

	RefreshControls();
}

void CSbeSinkPage::OnBnClickedButtonUnlock()
{
	HRESULT hr = sink->UnlockProfile();
	DSUtil::ShowError(hr, _T("Can't unlock profile"));

	RefreshControls();
}

void CSbeSinkPage::OnBnClickedButtonRecordingBrowse()
{
	CString		filter;
	CString		filename;

	filter = _T("All Files|*.*|");

	CFileDialog dlg(FALSE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,filter);
	INT_PTR ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		combo_recording.SetWindowText(filename);
	}
}

void CSbeSinkPage::OnBnClickedButtonCreateRecording()
{
	ASSERT(!filter->recording);
	if (filter->recording)
		return;

	CComPtr<IUnknown> recorder;

	CString recording_file;
	combo_recording.GetWindowText(recording_file);
	const UINT reference_recording = IsDlgButtonChecked(IDC_CHECK_REFERENCE_RECORDING);

	HRESULT hr = sink->CreateRecorder(recording_file, reference_recording ? RECORDING_TYPE_REFERENCE : RECORDING_TYPE_CONTENT, &recorder);

	if(FAILED(hr))
	{
		DSUtil::ShowError(hr, _T("Can't create recording object"));
	} 
	else 
	{
		recording_list.UpdateList(recording_file);
		recording_list.SaveList(recording_cache);
	}

	if (recorder)
		recorder.QueryInterface<IStreamBufferRecordControl>(&filter->recording);

	RefreshControls();
}

void CSbeSinkPage::OnBnClickedButtonCloseRecording()
{
	ASSERT(filter->recording);

	filter->recording = NULL;

	RefreshControls();
}

void CSbeSinkPage::OnBnClickedButtonStartRecording()
{
	ASSERT(filter->recording);
	if (!filter->recording)
		return;

	CString start_time_string;
	GetDlgItem(IDC_EDIT_START_TIME)->GetWindowText(start_time_string);

	REFERENCE_TIME start_time = (REFERENCE_TIME)((_tstof(start_time_string)*UNITS) + 0.5);

	HRESULT hr = filter->recording->Start(&start_time);
	if(FAILED(hr))
	{
		DSUtil::ShowError(hr, _T("Can't start recording"));
	} 
	else 
	{
		double new_start_time = (start_time + 0.5) / UNITS;
		start_time_string.Format(_T("%f"), new_start_time);
		GetDlgItem(IDC_EDIT_START_TIME)->SetWindowText(start_time_string);
	}

	RefreshControls();
}

void CSbeSinkPage::OnBnClickedButtonStopRecording()
{
	ASSERT(filter->recording);
	if (!filter->recording)
		return;

	CString stop_time_string;
	GetDlgItem(IDC_EDIT_STOP_TIME)->GetWindowText(stop_time_string);

	const REFERENCE_TIME stop_time = (REFERENCE_TIME)((_tstof(stop_time_string)*UNITS) + 0.5);

	HRESULT hr = filter->recording->Stop(stop_time);
	if(FAILED(hr))
	{
		DSUtil::ShowError(hr, _T("Can't stop recording"));
	}

	RefreshControls();
}


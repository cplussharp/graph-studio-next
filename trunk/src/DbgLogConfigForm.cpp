//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

//-----------------------------------------------------------------------------
//
//	CDbgLogConfigForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CDbgLogConfigForm, CDialog)
BEGIN_MESSAGE_MAP(CDbgLogConfigForm, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CDbgLogConfigForm::OnBnClickedButtonBrowse)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CDbgLogConfigForm class
//
//-----------------------------------------------------------------------------

CDbgLogConfigForm::CDbgLogConfigForm(const CString& strFileName, CWnd* pParent) :
	CDialog(CDbgLogConfigForm::IDD, pParent),
        m_strLogFile(_T("")),
		m_nTrace(0),
		m_nError(0),
		m_nMemory(0),
		m_nLocking(0),
		m_nTiming(0),
		m_nTimeout(-1),
		m_nCustom1(0),
		m_nCustom2(0),
		m_nCustom3(0),
		m_nCustom4(0),
		m_nCustom5(0),
		m_strFileName(strFileName),
		m_strRegKey(_T(""))
{

}

CDbgLogConfigForm::~CDbgLogConfigForm()
{
}

void CDbgLogConfigForm::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);

    DDX_Text(pDX, IDC_EDIT_LOGFILE, m_strLogFile);
    DDX_Text(pDX, IDC_EDIT_TRACE, m_nTrace);
    DDX_Text(pDX, IDC_EDIT_ERROR, m_nError);
    DDX_Text(pDX, IDC_EDIT_MEMORY, m_nMemory);
	DDX_Text(pDX, IDC_EDIT_LOCKING, m_nLocking);
	DDX_Text(pDX, IDC_EDIT_TIMING, m_nTiming);
	DDX_Text(pDX, IDC_EDIT_TIMEOUT, m_nTimeout);
	DDX_Text(pDX, IDC_EDIT_CUSTOM1, m_nCustom1);
	DDX_Text(pDX, IDC_EDIT_CUSTOM2, m_nCustom2);
	DDX_Text(pDX, IDC_EDIT_CUSTOM3, m_nCustom3);
	DDX_Text(pDX, IDC_EDIT_CUSTOM4, m_nCustom4);
	DDX_Text(pDX, IDC_EDIT_CUSTOM5, m_nCustom5);
	
	DDX_Control(pDX, IDC_SPIN_TRACE, m_spinTrace);
	DDX_Control(pDX, IDC_SPIN_ERROR, m_spinError);
	DDX_Control(pDX, IDC_SPIN_MEMORY, m_spinMemory);
	DDX_Control(pDX, IDC_SPIN_LOCKING, m_spinLocking);
	DDX_Control(pDX, IDC_SPIN_TIMING, m_spinTiming);
	DDX_Control(pDX, IDC_SPIN_TIMEOUT, m_spinTimeout);
	DDX_Control(pDX, IDC_SPIN_CUSTOM1, m_spinCustom1);
	DDX_Control(pDX, IDC_SPIN_CUSTOM2, m_spinCustom2);
	DDX_Control(pDX, IDC_SPIN_CUSTOM3, m_spinCustom3);
	DDX_Control(pDX, IDC_SPIN_CUSTOM4, m_spinCustom4);
	DDX_Control(pDX, IDC_SPIN_CUSTOM5, m_spinCustom5);

	DDX_Control(pDX, IDOK, m_btnOK);
}

BOOL CDbgLogConfigForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	// set filename in title
	CString strTitle;
	GetWindowText(strTitle);
	strTitle.AppendFormat(_T(" of '%s'"), m_strFileName);
	SetWindowText(strTitle);

	// open reg key with the current values
	CRegKey	regKey;
	m_strRegKey.Format(_T("SOFTWARE\\Microsoft\\DirectShow\\Debug\\%s"), m_strFileName);
	if (regKey.Open(HKEY_LOCAL_MACHINE, m_strRegKey, KEY_READ) != ERROR_SUCCESS)
	{
		// maybe it is an older version
		m_strRegKey.Format(_T("SOFTWARE\\Debug\\%s"), m_strFileName);
		if (regKey.Open(HKEY_LOCAL_MACHINE, m_strRegKey, KEY_READ) != ERROR_SUCCESS)
		{
			m_strRegKey.Empty();
			return FALSE;
		}
	}

	// read the log-level
	regKey.QueryDWORDValue(_T("TRACE"), m_nTrace);
	regKey.QueryDWORDValue(_T("ERROR"), m_nError);
	regKey.QueryDWORDValue(_T("MEMORY"), m_nMemory);
	regKey.QueryDWORDValue(_T("LOCKING"), m_nLocking);
	regKey.QueryDWORDValue(_T("TIMING"), m_nTiming);

	DWORD timeout;
	regKey.QueryDWORDValue(_T("TIMEOUT"), timeout);
	if (timeout > INT32_MAX)
		m_nTimeout = -1;
	else
		m_nTimeout = timeout;

	regKey.QueryDWORDValue(_T("CUSTOM1"), m_nCustom1);
	regKey.QueryDWORDValue(_T("CUSTOM2"), m_nCustom2);
	regKey.QueryDWORDValue(_T("CUSTOM3"), m_nCustom3);
	regKey.QueryDWORDValue(_T("CUSTOM4"), m_nCustom4);
	regKey.QueryDWORDValue(_T("CUSTOM5"), m_nCustom5);

	TCHAR val[MAX_PATH];
	DWORD len = MAX_PATH;
	if (regKey.QueryStringValue(_T("LogToFile"), val, &len) == ERROR_SUCCESS)
		m_strLogFile = val;

    UpdateData(FALSE);

	// configure the UI
	int min = 0; int max = 9;
	m_spinTrace.SetRange(min, max);
	m_spinError.SetRange(min, max);
	m_spinMemory.SetRange(min, max);
	m_spinLocking.SetRange(min, max);
	m_spinTiming.SetRange(min, max);
	m_spinTimeout.SetRange32(-1, INT32_MAX);
	m_spinCustom1.SetRange(min, max);
	m_spinCustom2.SetRange(min, max);
	m_spinCustom3.SetRange(min, max);
	m_spinCustom4.SetRange(min, max);
	m_spinCustom5.SetRange(min, max);
	m_spinCustom5.GetRange(min, max);

	m_btnOK.SetShield(TRUE);
	m_btnOK.EnableWindow(TRUE);

	return TRUE;
}

void CDbgLogConfigForm::OnBnClickedButtonBrowse()
{
	UpdateData(TRUE);

	CString strFilter = _T("All Files|*.*|");

	CFileDialog dlg(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT | OFN_ENABLESIZING, strFilter);
	INT_PTR ret = dlg.DoModal();

	CString filename = dlg.GetPathName();
	if (ret == IDOK)
	{
		m_strLogFile = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void CDbgLogConfigForm::OnOK()
{
    UpdateData(TRUE);

	if (!m_strRegKey.IsEmpty())
	{
		DWORD timeout = INFINITE;
		if (m_nTimeout >= 0)
			timeout = m_nTimeout;

		if (DSUtil::IsUserAdmin())
		{
			CRegKey regKey;
			if (regKey.Open(HKEY_LOCAL_MACHINE, m_strRegKey, KEY_WRITE) == ERROR_SUCCESS)
			{
				regKey.SetDWORDValue(_T("TRACE"), m_nTrace);
				regKey.SetDWORDValue(_T("ERROR"), m_nError);
				regKey.SetDWORDValue(_T("MEMORY"), m_nMemory);
				regKey.SetDWORDValue(_T("LOCKING"), m_nLocking);
				regKey.SetDWORDValue(_T("TIMING"), m_nTiming);
				regKey.SetDWORDValue(_T("TIMEOUT"), timeout);
				regKey.SetDWORDValue(_T("CUSTOM1"), m_nCustom1);
				regKey.SetDWORDValue(_T("CUSTOM2"), m_nCustom2);
				regKey.SetDWORDValue(_T("CUSTOM3"), m_nCustom3);
				regKey.SetDWORDValue(_T("CUSTOM4"), m_nCustom4);
				regKey.SetDWORDValue(_T("CUSTOM5"), m_nCustom5);
				regKey.SetStringValue(_T("LogToFile"), m_strLogFile);

				DSUtil::ShowInfo(_T("DbgLog settings changed.\nPlease restart the application."));

				regKey.Close();
			}
			else
			{
				DSUtil::ShowError(_T("Can't write the new settings in the registry."));
			}
		}
		else
		{
			CString strRegText;
			strRegText.Format(_T("[HKEY_LOCAL_MACHINE\\%s]\n"), m_strRegKey);
			strRegText.AppendFormat(_T("\"TRACE\"=dword:%08x\n"), m_nTrace);
			strRegText.AppendFormat(_T("\"ERROR\"=dword:%08x\n"), m_nError);
			strRegText.AppendFormat(_T("\"MEMORY\"=dword:%08x\n"), m_nMemory);
			strRegText.AppendFormat(_T("\"LOCKING\"=dword:%08x\n"), m_nLocking);
			strRegText.AppendFormat(_T("\"TIMING\"=dword:%08x\n"), m_nTiming);
			strRegText.AppendFormat(_T("\"TIMEOUT\"=dword:%08x\n"), timeout);
			strRegText.AppendFormat(_T("\"CUSTOM1\"=dword:%08x\n"), m_nCustom1);
			strRegText.AppendFormat(_T("\"CUSTOM2\"=dword:%08x\n"), m_nCustom2);
			strRegText.AppendFormat(_T("\"CUSTOM3\"=dword:%08x\n"), m_nCustom3);
			strRegText.AppendFormat(_T("\"CUSTOM4\"=dword:%08x\n"), m_nCustom4);
			strRegText.AppendFormat(_T("\"CUSTOM5\"=dword:%08x\n"), m_nCustom5);
			strRegText.AppendFormat(_T("\"LogToFile\"=\"%s\"\n\n"), m_strLogFile);
			DWORD ret = DSUtil::WriteToRegistryAsAdmin(strRegText);
			if (!ret)
				DSUtil::ShowInfo(_T("DbgLog settings changed.\nPlease restart the application."));
			else
				DSUtil::ShowError(_T("Can't write the new settings in the registry."));
		}
	}

	EndDialog(IDOK);
}

//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "DbgLogPage.h"

//-----------------------------------------------------------------------------
//
//	CDbgLogPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CDbgLogPage, CDSPropertyPage)
	ON_WM_SIZE()
    ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CDbgLogPage::OnBnClickedRefresh)
    ON_BN_CLICKED(IDC_BUTTON_DBGLOGSETTINGS, &CDbgLogPage::OnBnClickedSettings)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CDbgLogPage class
//
//-----------------------------------------------------------------------------
CDbgLogPage::CDbgLogPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle, const CString& filterFile, const CString& logFile) :
	CDSPropertyPage(_T("DbgLogPage"), pUnk, IDD, strTitle), isActiv(false), filterFile(filterFile), logFile(logFile)
{
	if (phr) *phr = NOERROR;

	if (GraphStudio::HasFont(_T("Consolas")))
		GraphStudio::MakeFont(font_log, _T("Consolas"), 10, false, false);
	else
		GraphStudio::MakeFont(font_log, _T("Courier New"), 10, false, false);
}

CDbgLogPage::~CDbgLogPage()
{
}


BOOL CDbgLogPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	CRect	rc;
	rc.SetRect(0, 0, 60, 23);
	btn_refresh.Create(_T("&Refresh"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_REFRESH);
	btn_refresh.SetFont(GetFont());
	btn_refresh.SetWindowPos(NULL, 4, 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	btn_settings.Create(_T("&Settings"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_DBGLOGSETTINGS);
	btn_settings.SetWindowPos(NULL, 8 + rc.Width(), 4, rc.Width() + 20, rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_settings.SetFont(GetFont());
	btn_settings.SetShield(TRUE);

	edit_log.SetFont(&font_log);

	return TRUE;
}

void CDbgLogPage::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(edit_log)) {
		title.GetClientRect(&rc2);
		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		edit_log.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		title.Invalidate();
		edit_log.Invalidate();
	}
}

void CDbgLogPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_EDIT_LOGFILE, edit_log);
}

HRESULT CDbgLogPage::OnConnect(IUnknown *pUnknown)
{
	return NOERROR;
}

HRESULT CDbgLogPage::OnActivate()
{
    isActiv = true;

	OnBnClickedRefresh();

	return NOERROR;
}

HRESULT CDbgLogPage::OnDisconnect()
{
	isActiv = false;

	// clear log

	return NOERROR;
}

HRESULT CDbgLogPage::OnApplyChanges()
{
	return NOERROR;
}

void CDbgLogPage::OnBnClickedRefresh()
{
	CString strLines;
	CString strLine;

	if (PathFileExists(logFile))
	{
		CStdioFile file(logFile, CFile::modeRead | CFile::shareDenyNone);
		while (file.ReadString(strLine))
		{
			strLines.Append(strLine);
			strLines.Append(_T("\r\n"));
		}
	}

	edit_log.SetWindowText(strLines);
	edit_log.LineScroll(edit_log.GetLineCount());
}


void CDbgLogPage::OnBnClickedSettings()
{
	if (!filterFile.IsEmpty())
	{
		CString strFileName = PathFindFileName(filterFile);
		CDbgLogConfigForm dlg(strFileName, this);
		dlg.DoModal();
	}
}

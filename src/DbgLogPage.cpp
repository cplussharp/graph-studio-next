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
	ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CDbgLogPage::OnBnClickedRefresh)
    ON_BN_CLICKED(IDC_BUTTON_DBGLOGSETTINGS, &CDbgLogPage::OnBnClickedSettings)
	ON_BN_CLICKED(IDC_BUTTON_LOCATE, &CDbgLogPage::OnLocateClick)
	ON_EN_UPDATE(IDC_FILTER_STRING, &CDbgLogPage::OnUpdateFilterString)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CDbgLogPage class
//
//-----------------------------------------------------------------------------
CDbgLogPage::CDbgLogPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle, const CString& filterFile, const CString& logFile) :
CDSPropertyPage(_T("DbgLogPage"), pUnk, IDD, strTitle), isActiv(false),
filterFile(filterFile), logFile(logFile), restoreSelectionOnRefresh(true)
{
	logLastChanged.dwLowDateTime = 0;		// vs2010 doesn't like structs in initializer list for some reason
	logLastChanged.dwHighDateTime = 0;

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
	btn_refresh.Create(_T("&Refresh"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, rc, &title, IDC_BUTTON_REFRESH);
	btn_refresh.SetFont(GetFont());
	btn_refresh.SetWindowPos(NULL, 4, 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_refresh.SetCheck(BST_CHECKED);

	btn_locate.Create(_T("&Locate"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_LOCATE);
	btn_locate.SetFont(GetFont());
	btn_locate.SetWindowPos(NULL, 8 + rc.Width(), 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	btn_settings.Create(_T("&Settings"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_DBGLOGSETTINGS);
	btn_settings.SetWindowPos(NULL, 12 + 2*rc.Width(), 4, rc.Width() + 20, rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_settings.SetFont(GetFont());
	btn_settings.SetShield(TRUE);

	title.GetClientRect(&rc);
	CRect edit_rect(0, 0, 350, 18);		// recommended edit control height is 14 but add a bit as 14 looks cramped
	edit_rect.MoveToXY(rc.Width() - 354, (rc.Height() - 18) / 2);
	edit_filter.Create(WS_BORDER | WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LOWERCASE, edit_rect, &title, IDC_SEARCH_STRING);
	edit_filter.SetFont(GetFont());

	edit_log.SetFont(&font_log);

	REParseError status = filterRegex.Parse(_T(""), FALSE);

	return TRUE;
}

void CDbgLogPage::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc;
	GetClientRect(&rc);

	if (IsWindow(edit_log)) {
		CRect rc2;
		title.GetClientRect(&rc2);
		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		edit_log.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		CRect rc3;
		edit_filter.GetWindowRect(&rc3);
		edit_filter.SetWindowPos(NULL, rc.Width() - 4 - rc3.Width(), (rc2.Height() - rc3.Height()) / 2, rc3.Width(), rc3.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

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

HRESULT CDbgLogPage::OnActivate()
{
    isActiv = true;

	OnBnClickedRefresh();	// set timer
	if (!btn_refresh.GetCheck())
		RefreshLog();

	return NOERROR;
}

HRESULT CDbgLogPage::OnDeactivate()
{
	isActiv = false;

	KillTimer(1);

	return NOERROR;
}

void CDbgLogPage::OnBnClickedRefresh()
{
	bool isChecked = btn_refresh.GetCheck();

	if (isChecked)
	{
		RefreshLog();
		SetTimer(1, 2000, NULL);
	}
	else
	{
		KillTimer(1);
	}
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

void CDbgLogPage::OnLocateClick()
{
	if (!logFile.IsEmpty())
	{
		CString logFileWithPath = logFile;
		if (PathIsRelative(logFileWithPath))
		{
			TCHAR buf[MAX_PATH] = {};
			int ret = GetFullPathName(logFileWithPath, MAX_PATH, buf, NULL);
			if (ret == 0) return;
			else if (ret > MAX_PATH)
			{
				// Buffer to small
				TCHAR* buf2 = new TCHAR[ret];
				buf[0] = 0;
				ret = GetFullPathName(logFileWithPath, ret, buf, NULL);
				logFileWithPath = buf2;
				delete[] buf2;
			}
			else
				logFileWithPath = buf;
		}

		// open the explorer with the location
		CString		param;
		param = _T("/select, \"");
		param += logFileWithPath;
		param += _T("\"");

		ShellExecute(NULL, _T("open"), _T("explorer.exe"), param, NULL, SW_NORMAL);
	}
}

void CDbgLogPage::OnUpdateFilterString()
{
	CString filter_string;
	edit_filter.GetWindowText(filter_string);
	REParseError status = filterRegex.Parse(filter_string, FALSE);

	restoreSelectionOnRefresh = false;
	RefreshLog();
	restoreSelectionOnRefresh = true;
}

void CDbgLogPage::OnTimer(UINT_PTR id)
{
	if (isActiv)
		RefreshLog();
}

void CDbgLogPage::RefreshLog()
{
	CString strLines;
	CString strLine;

	if (PathFileExists(logFile))
	{
		CStdioFile file(logFile, CFile::modeRead | CFile::shareDenyNone);
		FILETIME lastChanged = { 0, 0 };
		if (GetFileTime(file, NULL, NULL, &lastChanged))
		{
			if (logLastChanged.dwHighDateTime == lastChanged.dwHighDateTime &&
				logLastChanged.dwLowDateTime == lastChanged.dwLowDateTime)
				return;

			logLastChanged = lastChanged;
		}

		while (file.ReadString(strLine))
		{
			CAtlREMatchContext<> mc;
			if (filterRegex.Match(strLine, &mc))
			{
				strLines.Append(strLine);
				strLines.Append(_T("\r\n"));
			}
		}
	}

	// current selection and scroll
	int selStart, selEnd;
	edit_log.GetSel(selStart, selEnd);
	SCROLLINFO scrollV = {};
	edit_log.GetScrollInfo(SB_VERT, &scrollV, SIF_POS);

	// set the new text
	edit_log.SetWindowText(strLines);

	// restore selection or scroll to last
	if (selStart != selEnd)
	{
		edit_log.SetSel(selStart, selEnd, TRUE);
		// edit_log.SetScrollInfo just scrolls the scrollbars and not the content!?
		edit_log.LineScroll(scrollV.nPos);
	}
	else
		edit_log.LineScroll(edit_log.GetLineCount());
}

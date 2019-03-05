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
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_LOG, &CDbgLogPage::OnClearLogClick)
	ON_EN_UPDATE(IDC_FILTER_STRING, &CDbgLogPage::OnUpdateFilterString)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CDbgLogPage class
//
//-----------------------------------------------------------------------------
CDbgLogPage::CDbgLogPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle, const CString& filterFile, const CString& logFile) :
CDSPropertyPage(_T("DbgLogPage"), pUnk, IDD, strTitle), refreshOnTimer(true),
filterFile(filterFile), logFile(logFile), lastFileSize(-1), fileStartOffset(0), restoreSelectionOnRefresh(true)
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

CRect CDbgLogPage::GetButtonRect() const
{
	CRect rc_button(0, 0, 40, 12);
	MapDialogRect(rc_button);
	return rc_button;
}

BOOL CDbgLogPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) 
		return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	const CRect rc_button = GetButtonRect();
	const int gap = rc_button.Height() / 2;

	CRect rc_title;
	title.GetClientRect(&rc_title);
	const int button_top = (rc_title.Height() - rc_button.Height()) / 2;

	btn_refresh.Create(_T("&Refresh"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, rc_button, &title, IDC_BUTTON_REFRESH);
	btn_refresh.SetFont(GetFont());
	btn_refresh.SetWindowPos(NULL, gap, button_top, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
	btn_refresh.SetCheck(refreshOnTimer ? BST_CHECKED : BST_UNCHECKED);

	btn_locate.Create(_T("&Locate"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc_button, &title, IDC_BUTTON_LOCATE);
	btn_locate.SetFont(GetFont());
	btn_locate.SetWindowPos(NULL, 2*gap + rc_button.Width(), button_top, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);

	btn_settings.Create(_T("&Settings"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc_button, &title, IDC_BUTTON_DBGLOGSETTINGS);
	btn_settings.SetWindowPos(NULL, 3*gap + 2*rc_button.Width(), button_top, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
	btn_settings.SetFont(GetFont());
	btn_settings.SetShield(TRUE);

	btn_clear.Create(_T("&Clear"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc_button, &title, IDC_BUTTON_CLEAR_LOG);
	btn_clear.SetWindowPos(NULL, 4*gap + 3*rc_button.Width(), button_top, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
	btn_clear.SetFont(GetFont());

	edit_filter.Create(WS_BORDER | WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LOWERCASE, rc_button, &title, IDC_SEARCH_STRING);
	edit_filter.SetWindowPos(NULL, 5*gap + 4*rc_button.Width(), button_top, rc_title.Width() - 6*gap - 4*rc_button.Width(), rc_button.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	edit_filter.SetFont(GetFont());
	edit_filter.SetWindowText(filterString);

	edit_log.SetFont(&font_log);

	OnUpdateFilterString();

	return TRUE;
}

void CDbgLogPage::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc_client;
	GetClientRect(&rc_client);

	if (IsWindow(edit_log)) {
		CRect rc_title;
		title.GetClientRect(&rc_title);
		title.SetWindowPos(NULL, 0, 0, rc_client.Width(), rc_title.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		const CRect rc_button = GetButtonRect();
		const int gap = rc_button.Height() / 2;
		const int button_top = (rc_title.Height() - rc_button.Height()) / 2;

		edit_log.SetWindowPos(NULL, 0, rc_title.Height(), rc_client.Width(), rc_client.Height() - rc_title.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		edit_filter.SetWindowPos(NULL, 5*gap + 4*rc_button.Width(), button_top, rc_title.Width() - 6*gap - 4*rc_button.Width(), rc_button.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

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
	RefreshLog();
	SetTimer(1, 2000, NULL);
	return NOERROR;
}

HRESULT CDbgLogPage::OnDeactivate()
{
	KillTimer(1);
	refreshOnTimer = btn_refresh.GetCheck() != 0;		// save dialog state
	edit_filter.GetWindowText(filterString);
	return NOERROR;
}

void CDbgLogPage::OnBnClickedRefresh()
{
	refreshOnTimer = btn_refresh.GetCheck() != 0;
	if (refreshOnTimer)
		RefreshLog();
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

void CDbgLogPage::OnClearLogClick()
{
	fileStartOffset = 0;

	if (!PathFileExists(logFile))
		return;

	CStdioFile file(logFile, CFile::modeRead | CFile::shareDenyNone | (filterRegexValid ? CFile::typeText : CFile::typeBinary) );
	DWORD sizeHigh = 0;
	const DWORD sizeLow = GetFileSize(file, &sizeHigh);
	if (sizeHigh != 0 || sizeLow > INT_MAX) {			// don't support log files > 2GB!! Don't refresh if file length the same as before
		return;
	}
	fileStartOffset = sizeLow;

	lastFileSize = -1;			// force refresh
	restoreSelectionOnRefresh = false;
	RefreshLog();
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
	filterRegexValid = filter_string.GetLength() > 0 && filterRegex.Parse(filter_string, FALSE) == REPARSE_ERROR_OK;

	lastFileSize = -1;			// force refresh
	restoreSelectionOnRefresh = false;
	refreshOnTimer = true;		// force single udpate on next timer message
}

void CDbgLogPage::OnTimer(UINT_PTR id)
{
	if (refreshOnTimer) {
		RefreshLog();
		refreshOnTimer = btn_refresh.GetCheck() != 0;
	}
}

void CDbgLogPage::RefreshLog()
{
	CString strLines;

	if (PathFileExists(logFile))
	{
		// DbgLog output already contains windows CR-LF sequences so we can open in binary mode
		CStdioFile file(logFile, CFile::modeRead | CFile::shareDenyNone | (filterRegexValid ? CFile::typeText : CFile::typeBinary) );

		DWORD sizeHigh = 0;
		DWORD sizeLow = GetFileSize(file, &sizeHigh);
		if (sizeHigh != 0 || sizeLow > INT_MAX || sizeLow == lastFileSize ) {			// don't support log files > 2GB!! Don't refresh if file length the same as before
			return;
		}
		lastFileSize = sizeLow;

		if (fileStartOffset > sizeLow) {		// sanity check if file offset is beyond end of file
			fileStartOffset = 0;				// reset offset to zero
		} else {
			sizeLow -= fileStartOffset;
			file.Seek(fileStartOffset, CFile::begin);
		}


		if (filterRegexValid) {
			// preallocate string buffer
			TCHAR * const string_buf = strLines.GetBufferSetLength(sizeLow / sizeof(TCHAR));
			strLines.ReleaseBuffer(0);
			CString strLine;
			CAtlREMatchContext<> mc;
			while (file.ReadString(strLine))
			{
				if (filterRegex.Match(strLine, &mc)) {
					strLines.Append(strLine);
					strLines.Append(_T("\r\n"));
				}
			}
		} else {
			CStringA strLinesAscii;
			CHAR * const ascii_buf = strLinesAscii.GetBufferSetLength(sizeLow + 1);
			if (ascii_buf) {
				const UINT bytes_read = file.Read(ascii_buf, sizeLow);
				strLinesAscii.ReleaseBuffer(min(bytes_read, sizeLow));		// force null termination after bytes read
				strLines = strLinesAscii;
			}
		}
	}

	// current selection and scroll
	int selStart = -1, selEnd = -1;
	SCROLLINFO scrollV = {0};
	if (restoreSelectionOnRefresh) {
		edit_log.GetSel(selStart, selEnd);
		if (selStart == selEnd) {							// no selected text
			int length = edit_log.GetWindowTextLength();	// if at end of text
			if (selStart >= length) {						
				selStart = -1;								// move to end of text after refresh
			}
		}
		edit_log.GetScrollInfo(SB_VERT, &scrollV, SIF_POS);
	}
	restoreSelectionOnRefresh = true;

	edit_log.SetWindowText(strLines);

	if (selStart >= 0)		// restore position
	{			
		edit_log.SetSel(selStart, selEnd, TRUE);
		// edit_log.SetScrollInfo just scrolls the scrollbars and not the content!?
		edit_log.LineScroll(scrollV.nPos);
	} 
	else					// scroll to end of log
	{
		edit_log.LineScroll(edit_log.GetLineCount());
		const int len = edit_log.GetWindowTextLength();
		edit_log.SetSel(len, len, TRUE);
	}
}

//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ComDllAnalyzerForm.h"

IMPLEMENT_DYNAMIC(CComDllAnalyzerForm, CGraphStudioModelessDialog)
BEGIN_MESSAGE_MAP(CComDllAnalyzerForm, CGraphStudioModelessDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CComDllAnalyzerForm::OnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_COPYTEXT, &CComDllAnalyzerForm::OnClickedButtonCopytext)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CComDllAnalyzerForm::OnClickedButtonSave)
END_MESSAGE_MAP()

CComDllAnalyzerForm::CComDllAnalyzerForm(CWnd* pParent) :
	CGraphStudioModelessDialog(CTextInfoForm::IDD, pParent)
{
}


CComDllAnalyzerForm::~CComDllAnalyzerForm()
{
}

BOOL CComDllAnalyzerForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);

	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	CRect	rc;
	rc.SetRect(0, 0, 60, 23);
	btn_open.Create(_T("&Analyze"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_BROWSE);
	btn_open.SetWindowPos(NULL, 12 + 2 * rc.Width(), 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_open.SetFont(GetFont());

	btn_copy.Create(_T("&Copy"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_COPYTEXT);
	btn_copy.SetWindowPos(NULL, 4, 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_copy.SetFont(GetFont());

	btn_save.Create(_T("&Save"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_SAVE);
	btn_save.SetWindowPos(NULL, 8 + rc.Width(), 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	btn_save.SetFont(GetFont());

	OnInitialize();

	return TRUE;
};

CRect CComDllAnalyzerForm::GetDefaultRect() const
{
	return CRect(50, 200, 850, 600);
}

void CComDllAnalyzerForm::OnInitialize()
{
	if (GraphStudio::HasFont(_T("Consolas")))
		GraphStudio::MakeFont(font_report, _T("Consolas"), 10, false, false);
	else
		GraphStudio::MakeFont(font_report, _T("Courier New"), 10, false, false);
	edit_report.SetFont(&font_report);
}

void CComDllAnalyzerForm::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_EDIT_DETAILS, edit_report);
}

void CComDllAnalyzerForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	if (IsWindow(edit_report)) {
		title.GetClientRect(&rc2);
		title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		edit_report.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		btn_open.GetWindowRect(&rc2);
		btn_open.SetWindowPos(NULL, rc.Width() - 4 - rc2.Width(), 5, rc2.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		title.Invalidate();
		edit_report.Invalidate();
	}
}


void CComDllAnalyzerForm::OnClickedButtonOpen()
{
	CString		filter;
	CString		filename;

	filter = _T("Filter-File (*.dll,*.ax)|*.dll;*.ax|All Files|*.*|");

	CFileDialog dlg(TRUE, NULL, NULL, OFN_OVERWRITEPROMPT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST, filter);
	INT_PTR ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		edit_report.SetWindowText(_T(""));

		CComDllAnalyzer comDllAnalyzer(filename);

		if (FAILED(comDllAnalyzer.errorHr))
		{
			DSUtil::ShowError(comDllAnalyzer.errorHr, comDllAnalyzer.errorMsg);
			return;
		}

		const CString str = comDllAnalyzer.registry.ToString();
		edit_report.SetWindowText(str);
	}
}

void CComDllAnalyzerForm::OnClickedButtonCopytext()
{
	// copy the content to the clipboard
	CString		text;

	edit_report.GetWindowText(text);

	DSUtil::SetClipboardText(this->GetSafeHwnd(), text);
}


void CComDllAnalyzerForm::OnClickedButtonSave()
{
	CString	filter;
	CString	filename;

	filter = _T("Reg Files (*.reg)|*.reg|All Files (*.*)|*.*|");

	CFileDialog dlg(FALSE, _T("reg"), NULL, OFN_OVERWRITEPROMPT | OFN_ENABLESIZING | OFN_PATHMUSTEXIST, filter);
	INT_PTR ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK)
	{
		CPath path(filename);
		if (path.GetExtension() == _T(""))
		{
			path.AddExtension(_T(".reg"));
			filename = CString(path);
		}

		CFile file(filename, CFile::modeCreate | CFile::modeWrite);

		CString	text;
		edit_report.GetWindowText(text);
		CT2CA outputText(text, CP_UTF8);
		file.Write(outputText, (DWORD) ::strlen(outputText));
	}
}
//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "TextInfoForm.h"


//-----------------------------------------------------------------------------
//
//	CTextInfoForm dialog
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CTextInfoForm, CGraphStudioModelessDialog)
BEGIN_MESSAGE_MAP(CTextInfoForm, CGraphStudioModelessDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_COPYTEXT, &CTextInfoForm::OnBnClickedButtonCopytext)
    ON_CBN_SELCHANGE(IDC_COMBO_REPORTTYPE, &CTextInfoForm::OnBnClickedButtonRefresh)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CTextInfoForm::OnClickedButtonSave)
END_MESSAGE_MAP()


LPCTSTR	ReportNames[] =
{
	_T("Graph Report (Level 1)"),
	_T("Graph Report (Level 2)"),
	_T("Graph Report (Level 3)"),
	_T("Graph Report (Level 4)"),
	_T("Graph Report (Level 5)"),
    _T("Graph Report (Level 6)")
};
int ReportNamesCount = sizeof(ReportNames)/sizeof(ReportNames[0]);



CTextInfoForm::CTextInfoForm(CWnd* pParent) : 
	CGraphStudioModelessDialog(CTextInfoForm::IDD, pParent)
{

}

CTextInfoForm::~CTextInfoForm()
{
}

BOOL CTextInfoForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);

	if (!ret) return FALSE;

    // prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

    CRect	rc;
	rc.SetRect(0, 0, 60, 23);
    btn_copy.Create(_T("&Copy"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_COPYTEXT);
    btn_copy.SetWindowPos(NULL, 4, 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
    btn_copy.SetFont(GetFont());

    btn_save.Create(_T("&Save"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_SAVE);
    btn_save.SetWindowPos(NULL, 8 + rc.Width(), 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
    btn_save.SetFont(GetFont());

    rc.SetRect(0, 0, 150, 23);
    combo_reporttype.Create(WS_TABSTOP | WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, rc, &title, IDC_COMBO_REPORTTYPE);
    combo_reporttype.SetFont(GetFont());

	// Force a second resize to give the combo box a chance to position itself once it's fully created
	// Would not be an issue if the setup above was done in OnInitDialog...
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE);		// resize down to zero
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER);		// resize down to zero
	RestorePosition();								// then restore position or set default position

	OnInitialize();

	return TRUE;
};

CRect CTextInfoForm::GetDefaultRect() const 
{
	return CRect(50, 200, 650, 600);
}

void CTextInfoForm::OnInitialize()
{
	if(GraphStudio::HasFont(_T("Consolas")))
        GraphStudio::MakeFont(font_report, _T("Consolas"), 10, false, false);
    else
        GraphStudio::MakeFont(font_report, _T("Courier New"), 10, false, false);
	edit_report.SetFont(&font_report);

	combo_reporttype.ResetContent();
	for (int i=0; i<ReportNamesCount; i++) {
		combo_reporttype.AddString(ReportNames[i]);
	}
	combo_reporttype.SetCurSel(ReportNamesCount - 1);
}

void CTextInfoForm::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_EDIT_DETAILS, edit_report);
}

void CTextInfoForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);
	
	if (IsWindow(edit_report)) {
        title.GetClientRect(&rc2);
        title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

        edit_report.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		combo_reporttype.GetWindowRect(&rc2);
        combo_reporttype.SetWindowPos(NULL, rc.Width()-4-rc2.Width(), 5, rc2.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

        title.Invalidate();
        edit_report.Invalidate();
	}
}


// CTextInfoForm message handlers

void CTextInfoForm::OnBnClickedButtonRefresh()
{
	// get the report level
	int level = combo_reporttype.GetCurSel();

	// gnerate the report
	CGraphReportGenerator graphReportGenerator(&view->graph, view->render_params.use_media_info);
	CString report = graphReportGenerator.GetReport(level);

	edit_report.SetWindowText(report);
}

void CTextInfoForm::OnBnClickedButtonCopytext()
{
	// copy the content to the clipboard
	CString		text;

	edit_report.GetWindowText(text);

    DSUtil::SetClipboardText(this->GetSafeHwnd(), text);
}


void CTextInfoForm::OnClickedButtonSave()
{
	CString	filter;
	CString	filename;

	filter = _T("Log Files (*.log,*.txt)|*.log;*.txt|All Files (*.*)|*.*|");

	CFileDialog dlg(FALSE,_T("log"),NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,filter);
    INT_PTR ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK)
    {
		CPath path(filename);
		if (path.GetExtension() == _T(""))
        {
			path.AddExtension(_T(".log"));
			filename = CString(path);
		}

        CFile file(filename, CFile::modeCreate|CFile::modeWrite);
        
        CString	text;
        edit_report.GetWindowText(text);
        CT2CA outputText(text, CP_UTF8);
        file.Write(outputText, (DWORD) ::strlen(outputText));
    }
}

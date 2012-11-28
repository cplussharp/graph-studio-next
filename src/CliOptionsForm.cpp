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
//	CCliOptionsForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CCliOptionsForm, CDialog)
BEGIN_MESSAGE_MAP(CCliOptionsForm, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CCliOptionsForm class
//
//-----------------------------------------------------------------------------

CCliOptionsForm::CCliOptionsForm(CWnd* pParent)	: 
	CDialog(CCliOptionsForm::IDD, pParent)
{

}

CCliOptionsForm::~CCliOptionsForm()
{
}

void CCliOptionsForm::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
    DDX_Control(pDX, IDC_EDIT_OPTIONS, edit_options);
}

BOOL CCliOptionsForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

    if(GraphStudio::HasFont(_T("Consolas")))
        GraphStudio::MakeFont(font_options, _T("Consolas"), 10, false, false);
    else
        GraphStudio::MakeFont(font_options, _T("Courier New"), 10, false, false);
	edit_options.SetFont(&font_options);

    CString strTxt;
    strTxt =  _T("Command Line Options for the programm:\r\n");
    strTxt += _T("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n\r\n");
    strTxt += _T("graphstudionext.exe [file] [options]\r\n");
    strTxt += _T("-> file (optional)      -> a graph file or a video-file to render\r\n");
    strTxt += _T("-> options (optional)   -> one or more of the following switches:\r\n");
    strTxt += _T("    -run                    -> runs the graph if a file was rendered\r\n");
    strTxt += _T("    -exitafterrun           -> closes the application after the graph was stopped\r\n");
    strTxt += _T("    -noclock                -> disables the clock if a file was rendered\r\n");
    strTxt += _T("    -progressview           -> opens the application in the progress view\r\n");
    strTxt += _T("    -exitonerror            -> closes the application on an error (no error dialog)\r\n");
    strTxt += _T("                               and sets the error code as the exit code\r\n");
    strTxt += _T("    -filters                -> opens the filters dialog on startup\r\n");
    strTxt += _T("    -?                      -> opens this window on startup\r\n");
    strTxt += _T("\r\n");
    strTxt += _T("Example:\r\n");
    strTxt += _T("graphstudionext.exe convert.grf -run -exitonerror -exitafterrun -noclock -progressview\r\n");

    edit_options.SetWindowText(strTxt);

    UpdateData(FALSE);

	return TRUE;
}

void CCliOptionsForm::OnOK()
{
    UpdateData(TRUE);
	EndDialog(IDOK);
}

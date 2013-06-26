//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CProgressForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CProgressForm, CGraphStudioModelessDialog)
BEGIN_MESSAGE_MAP(CProgressForm, CGraphStudioModelessDialog)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CProgressForm::OnBnClickedButtonClose)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
//
//	CProgressForm class
//
//-----------------------------------------------------------------------------

CProgressForm::CProgressForm(CWnd* pParent)	: 
	CGraphStudioModelessDialog(CProgressForm::IDD, pParent)
{

}

CProgressForm::~CProgressForm()
{
}

void CProgressForm::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, button_close);
	DDX_Control(pDX, IDC_CHECK_AUTOSWITCH, check_close);
	DDX_Control(pDX, IDC_STATIC_CAPTION, label_caption);
	DDX_Control(pDX, IDC_STATIC_TIME, label_time);
	DDX_Control(pDX, IDC_PROGRESS_POSITION, progress);
}

BOOL CProgressForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	check_close.SetCheck(TRUE);
	progress.SetRange(0, 500);
	return TRUE;
}

void CProgressForm::OnBnClickedButtonClose()
{
	OnClose();
}

void CProgressForm::OnCancel()
{
	// nerobime nic.. ziadny exit
}

void CProgressForm::OnClose()
{
	ShowWindow(SW_HIDE);

	AfxGetMainWnd()->ShowWindow(SW_SHOW);
	AfxGetMainWnd()->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void CProgressForm::UpdateCaption(CString text)
{
	label_caption.SetWindowText(text);
}

void CProgressForm::UpdateTimeLabel(CString text)
{
	label_time.SetWindowText(text);
}

void CProgressForm::UpdateProgress(double pos)
{
	int ipos = (int)(pos * 500);
	if (ipos < 0) ipos = 0;
	if (ipos > 500) ipos = 500;

	progress.SetPos(ipos);
}

void CProgressForm::OnGraphStopped()
{
	BOOL	auto_close = check_close.GetCheck();
	if (auto_close) {
		OnClose();
	}
}

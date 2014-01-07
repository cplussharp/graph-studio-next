//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

struct MERITVAL
{
	DWORD		merit;
	LPCTSTR		name;
};

const MERITVAL		TableMerits[] =
{
	{ MERIT_HW_COMPRESSOR,		_T("MERIT_HW_COMPRESSOR") },
	{ MERIT_SW_COMPRESSOR,		_T("MERIT_SW_COMPRESSOR") },
	{ MERIT_DO_NOT_USE,			_T("MERIT_DO_NOT_USE") },
	{ MERIT_DO_NOT_USE+1,		_T("MERIT_DO_NOT_USE + 1") },
	{ MERIT_UNLIKELY,			_T("MERIT_UNLIKELY") },
	{ MERIT_UNLIKELY+1,			_T("MERIT_UNLIKELY + 1") },
	{ MERIT_NORMAL-1,			_T("MERIT_NORMAL - 1") },
	{ MERIT_NORMAL,				_T("MERIT_NORMAL") },
	{ MERIT_NORMAL+1,			_T("MERIT_NORMAL + 1") },
	{ MERIT_PREFERRED-1,		_T("MERIT_PREFERRED - 1") },
	{ MERIT_PREFERRED,			_T("MERIT_PREFERRED") },
	{ MERIT_PREFERRED+1,		_T("MERIT_PREFERRED + 1") },
	{ MERIT_PREFERRED+2,		_T("MERIT_PREFERRED + 2") }
};
const int			TableMeritCount = sizeof(TableMerits) / sizeof(TableMerits[0]);

//-----------------------------------------------------------------------------
//
//	CMeritChangeDialog class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CMeritChangeDialog, CDialog)
BEGIN_MESSAGE_MAP(CMeritChangeDialog, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CMeritChangeDialog class
//
//-----------------------------------------------------------------------------

CMeritChangeDialog::CMeritChangeDialog(CWnd* pParent)	: 
	CDialog(CMeritChangeDialog::IDD, pParent)
{

}

CMeritChangeDialog::~CMeritChangeDialog()
{
}

void CMeritChangeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FILTER, label_filter);
	DDX_Control(pDX, IDC_EDIT_ORIGINAL, edit_original);
	DDX_Control(pDX, IDC_COMBO_NEW, cb_newmerit);
}

BOOL CMeritChangeDialog::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	// create a nice big font
	GraphStudio::MakeFont(font_filter, _T("Arial"), 12, true, false);
	label_filter.SetFont(&font_filter);
	label_filter.SetWindowText(filter_name);

	// original value
	int		idx = -1;
	int		i;

	for (i=0; i<TableMeritCount; i++) {
		if (old_merit == TableMerits[i].merit) {
			idx = i;
			break;
		}
	}

	CString		str;
	if (idx >= 0) {
		str.Format(_T("%08x (%s)"), old_merit, TableMerits[idx].name);
	} else {
		// simply format the hexadecimal code
		str.Format(_T("%08x"), old_merit);
	}
	str = str.MakeUpper();
	str = _T("0x") + str;
	edit_original.SetWindowText(str);

	// combobox
	idx = -1;
	cb_newmerit.ResetContent();
	for (i=0; i<TableMeritCount; i++) {
		str.Format(_T("%08x (%s)"), TableMerits[i].merit, TableMerits[i].name);
		str = str.MakeUpper();
		str = _T("0x") + str;
		cb_newmerit.AddString(str);
		if (TableMerits[i].merit == old_merit) {
			idx = i;
		}
	}

	if (idx >= 0) {
		cb_newmerit.SetCurSel(idx);
	} else {
		str.Format(_T("%08x"), old_merit);
		str = str.MakeUpper();
		str = _T("0x") + str;
		cb_newmerit.SetWindowText(str);
	}
	
	return TRUE;
}

void CMeritChangeDialog::OnOK()
{
	/*
		Take the first word of the text entered in the combobox.
		Try to understand "0x" as hexadecimal.
	*/

	CString		value;
	cb_newmerit.GetWindowText(value);

	bool		hexa = false;
	int			pos;

	value = value.MakeLower();
	value = value.Trim();

	pos = value.Find(_T("0x"));
	if (pos == 0) {
		hexa = true;
		value.Delete(0, 2);
	} else {
		hexa = false;
	}

	DWORD		meritval;
	int			cnt;

	if (hexa) {
		cnt = _stscanf_s(value.GetBuffer(), _T("%x"), &meritval);
	} else {
		cnt = _stscanf_s(value.GetBuffer(), _T("%d"), &meritval);
	}

	if (cnt <= 0) {
		DSUtil::ShowError(_T("The value must be either a number\nor a hexadecimal value with the \"0x\" prefix."));
		return ;
	}

	new_merit = meritval;
	__super::OnOK();
}


bool ChangeMeritDialog(CString name, DWORD original_merit, DWORD &new_merit)
{
	CMeritChangeDialog	dlg;
	dlg.filter_name = name;
	dlg.old_merit = original_merit;
	dlg.new_merit = original_merit;

	INT_PTR ret = dlg.DoModal();
	if (ret == IDOK) {
		new_merit = dlg.new_merit;
		return true;
	}
	return false;
}


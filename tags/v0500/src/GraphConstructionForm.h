//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

class CGraphView;

//-----------------------------------------------------------------------------
//
//	CGraphConstructionForm class
//
//-----------------------------------------------------------------------------
class CGraphConstructionForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CGraphConstructionForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

public:
	GraphStudio::TitleBar	title;
	CGraphView				*view;
	CBrowserControl			browser_window;

public:
	CGraphConstructionForm(CWnd *pParent = NULL);   // standard constructor
	virtual ~CGraphConstructionForm();

	enum { IDD = IDD_DIALOG_CONSTRUCTION };

	// initialization
	BOOL DoCreateDialog();

	void OnSize(UINT nType, int cx, int cy);
	void Reload(GraphStudio::RenderParameters *params);

	void GenerateHTML(CString &text, GraphStudio::RenderParameters *params);
};

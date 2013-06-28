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
class CGraphConstructionForm : public CGraphStudioModelessDialog
{
protected:
	DECLARE_DYNAMIC(CGraphConstructionForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

public:
	GraphStudio::TitleBar	title;
	CBrowserControl			browser_window;

public:
	CGraphConstructionForm(CWnd *pParent = NULL);   // standard constructor
	virtual ~CGraphConstructionForm();
	virtual CRect GetDefaultRect() const;

	enum { IDD = IDD_DIALOG_CONSTRUCTION };

	// initialization
	BOOL DoCreateDialog(CWnd* parent);

	void OnSize(UINT nType, int cx, int cy);
	void Reload(GraphStudio::RenderParameters *params);

	void GenerateHTML(CString &text, GraphStudio::RenderParameters *params);
};

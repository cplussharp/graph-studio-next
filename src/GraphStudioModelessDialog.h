#pragma once

class CGraphView;


// Base class for modeless dialogs that allow keyboard accelerators
class CGraphStudioModelessDialog : public CDialog
{
	DECLARE_DYNAMIC(CGraphStudioModelessDialog)

public:
	explicit CGraphStudioModelessDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL)
		: CDialog(lpszTemplateName, pParentWnd), view(NULL) {}

	explicit CGraphStudioModelessDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL)
		: CDialog(nIDTemplate, pParentWnd), view(NULL) {}

public:
	CGraphView* view;

protected:
	BOOL PreTranslateMessage(MSG *pmsg);
};
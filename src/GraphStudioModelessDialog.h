#pragma once

class CGraphView;


// 
/*	
	Base class for modeless dialogs that allow keyboard accelerators and store and restore size and position
 	
	Size and position settings are stored relative under the class name so it is very important 
	that classes deriving from this use the correct macros to define MFC runtime class information for the new class.
	If not then registry settings will overwrite each other.
	
	DECLARE_DYNAMIC(classname) inside the class declaration
	IMPLEMENT_DYNAMIC(classname, CGraphStudioModelessDialog)
*/
class CGraphStudioModelessDialog : public CDialog
{
protected:
	DECLARE_DYNAMIC(CGraphStudioModelessDialog)
	DECLARE_MESSAGE_MAP()

public:
	explicit CGraphStudioModelessDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	explicit CGraphStudioModelessDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);

	BOOL PreTranslateMessage(MSG *pmsg);

public:
	CGraphView* view;

protected:
	virtual CString GetSettingsName() const;
	virtual CRect GetDefaultRect() const;							// Get default window rect relative to parent window
	virtual bool ShouldRestorePosition() const { return true; }
	virtual bool ShouldRestoreSize() const { return true; }

	void SavePosition();
	void RestorePosition();

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	afx_msg void OnDestroy();

protected:
	CRect	position;
	bool	position_saved;
};
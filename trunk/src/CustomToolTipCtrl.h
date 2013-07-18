#pragma once

class CustomToolTipCallback
{
public:
	// cursorScreenPos may need converting from screen coordinates to client coordinates
    virtual void GetToolTipLabelText(POINT cursorScreenPos, CString& labelText, CString& descriptionText) const = 0;
};

/////////////////////////////////////////////////////////////////////////////
// CCustomToolTipCtrl window
class CCustomToolTipCtrl : public CMFCToolTipCtrl
{
public:
	CCustomToolTipCtrl(CustomToolTipCallback *parent);
    virtual ~CCustomToolTipCtrl();

protected:
	CustomToolTipCallback* m_callback;
	int	m_nCurrID;

	// Data to display - set in OnShow and cache until used by OnDrawLabel
	CString label;

	// CMFCToolTipCtrl overrides
	CSize OnDrawLabel(CDC* pDC,CRect rect,BOOL bCalcOnly);

	afx_msg void OnShow(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

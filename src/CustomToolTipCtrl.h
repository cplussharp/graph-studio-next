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

	CSize OnDrawLabel(CDC* pDC,CRect rect,BOOL bCalcOnly);
};

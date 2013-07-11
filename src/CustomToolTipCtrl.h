#pragma once

/////////////////////////////////////////////////////////////////////////////
// CCustomToolTipCtrl window
class CCustomToolTipCtrl : public CMFCToolTipCtrl
{
public:
	CCustomToolTipCtrl();
    virtual ~CCustomToolTipCtrl();

	int	m_nCurrID;

protected:
	CSize OnDrawLabel(CDC* pDC,CRect rect,BOOL bCalcOnly);
};

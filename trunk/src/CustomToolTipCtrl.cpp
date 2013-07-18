#include "stdafx.h"
#include "CustomToolTipCtrl.h"
#include "MainFrm.h"


BEGIN_MESSAGE_MAP(CCustomToolTipCtrl, CMFCToolTipCtrl)
	//{{AFX_MSG_MAP(CMFCToolTipCtrl)
	ON_NOTIFY_REFLECT(TTN_SHOW, &CCustomToolTipCtrl::OnShow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomToolTipCtrl
CCustomToolTipCtrl::CCustomToolTipCtrl(CustomToolTipCallback* callback)
	: m_callback(callback)
{
	m_nCurrID = 0;
}

CCustomToolTipCtrl::~CCustomToolTipCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CCustomToolTipCtrl message handlers
CSize CCustomToolTipCtrl::OnDrawLabel(CDC* pDC, CRect rect, BOOL bCalcOnly)
{
	ASSERT_VALID(pDC);

	POINT cursor;
	GetCursorPos(&cursor);

	CSize sizeText(0, 0);
	if (label.GetLength() > 0 || m_strDescription.GetLength() > 0) {
		const BOOL bDrawDescr = m_Params.m_bDrawDescription && !m_strDescription.IsEmpty();
		CFont* const pOldFont = (CFont*) pDC->SelectObject(m_Params.m_bBoldLabel && bDrawDescr ? &afxGlobalData.fontBold : &afxGlobalData.fontTooltip);

		if (label.Find(_T('\n')) >= 0) // Multi-line text
		{
			UINT nFormat = DT_NOPREFIX;
			if (bCalcOnly)
				nFormat |= DT_CALCRECT;

			if (m_pRibbonButton != NULL)
				nFormat |= DT_NOPREFIX;

			const int nHeight = pDC->DrawText(label, rect, nFormat);
			sizeText = CSize(rect.Width(), nHeight);
		}
		else
		{
			if (bCalcOnly)
				sizeText = pDC->GetTextExtent(label);
			else
			{
				UINT nFormat = DT_LEFT | DT_NOCLIP | DT_SINGLELINE;

				if (!bDrawDescr)
				{
					nFormat |= DT_VCENTER;
				}

				if (m_pRibbonButton != NULL)
				{
					nFormat |= DT_NOPREFIX;
				}

				sizeText.cy = pDC->DrawText(label, rect, nFormat);
				sizeText.cx = rect.Width();
			}
		}
		pDC->SelectObject(pOldFont);
	}
	return sizeText;
}

afx_msg void CCustomToolTipCtrl::OnShow(NMHDR* pNMHDR, LRESULT* pResult)
{
	POINT cursor;
	GetCursorPos(&cursor);

	if (m_callback) {
		label = _T("");
		CString description;
		m_callback->GetToolTipLabelText(cursor, label, description);
		SetDescription(description);

		// if text to display call base class to continue normal display
		if (label.GetLength() > 0 || m_strDescription.GetLength() > 0) {
			__super::OnShow(pNMHDR, pResult);
			return;
		}
	}

	// nothing to display, hide tooltip window, resize to zero and return TRUE
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);
	*pResult = TRUE;
}

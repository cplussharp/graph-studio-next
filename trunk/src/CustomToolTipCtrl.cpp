#include "stdafx.h"
#include "CustomToolTipCtrl.h"
#include "MainFrm.h"

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

	CString labelText, descriptionText;
	if (m_callback) {
		m_callback->GetToolTipLabelText(cursor, labelText, descriptionText);
	}

	CSize sizeText(0, 0);
	if (labelText.GetLength() == 0 && descriptionText.GetLength() == 0) {
		SetDescription(CString());
	} else {
		const BOOL bDrawDescr = m_Params.m_bDrawDescription && !m_strDescription.IsEmpty();
		CFont* const pOldFont = (CFont*) pDC->SelectObject(m_Params.m_bBoldLabel && bDrawDescr ? &afxGlobalData.fontBold : &afxGlobalData.fontTooltip);

		if (labelText.Find(_T('\n')) >= 0) // Multi-line text
		{
			UINT nFormat = DT_NOPREFIX;
			if (bCalcOnly)
				nFormat |= DT_CALCRECT;

			if (m_pRibbonButton != NULL)
				nFormat |= DT_NOPREFIX;

			const int nHeight = pDC->DrawText(labelText, rect, nFormat);
			sizeText = CSize(rect.Width(), nHeight);
		}
		else
		{
			if (bCalcOnly)
				sizeText = pDC->GetTextExtent(labelText);
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

				sizeText.cy = pDC->DrawText(labelText, rect, nFormat);
				sizeText.cx = rect.Width();
			}
		}
		pDC->SelectObject(pOldFont);
		SetDescription(descriptionText);
	}

	return sizeText;
}

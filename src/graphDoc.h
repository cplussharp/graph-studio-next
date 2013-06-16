//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


class CGraphDoc : public CDocument
{
protected: // create from serialization only
	CGraphDoc();
	virtual ~CGraphDoc();
	DECLARE_DYNCREATE(CGraphDoc)

	virtual BOOL OnNewDocument();
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	DECLARE_MESSAGE_MAP()

	afx_msg void OnFileNewwindow();
};



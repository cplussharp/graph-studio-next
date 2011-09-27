// graphDoc.cpp : implementation of the CGraphDoc class
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGraphDoc

IMPLEMENT_DYNCREATE(CGraphDoc, CDocument)

BEGIN_MESSAGE_MAP(CGraphDoc, CDocument)
END_MESSAGE_MAP()


// CGraphDoc construction/destruction

CGraphDoc::CGraphDoc()
{
	// TODO: add one-time construction code here

}

CGraphDoc::~CGraphDoc()
{
}

BOOL CGraphDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CGraphDoc serialization

void CGraphDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CGraphDoc diagnostics

#ifdef _DEBUG
void CGraphDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGraphDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CGraphDoc commands

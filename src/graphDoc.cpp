// graphDoc.cpp : implementation of the CGraphDoc class
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGraphDoc

IMPLEMENT_DYNCREATE(CGraphDoc, CDocument)

BEGIN_MESSAGE_MAP(CGraphDoc, CDocument)
	ON_COMMAND(ID_FILE_NEWWINDOW, &CGraphDoc::OnFileNewwindow)
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


void CGraphDoc::OnFileNewwindow() 
{
	CDocTemplate * const doc_template = this->GetDocTemplate();
	ASSERT(doc_template);

	CDocument* pDoc = NULL;
	CFrameWnd* pFrame = NULL;

	// Create a new instance of the document referenced
	// by the doc_template member.
	if (doc_template)
		pDoc = doc_template->CreateNewDocument();
	ASSERT(pDoc);

	if (pDoc)
	{
		POSITION first_view_pos = GetFirstViewPosition();
		CView* const first_view = first_view_pos ? GetNextView(first_view_pos) : NULL;
		CFrameWnd * const current_frame = first_view ? first_view->GetParentFrame() : NULL;
		ASSERT(current_frame);

		// If creation worked, use create a new frame for
		// that document.
		pFrame = doc_template->CreateNewFrame(pDoc, current_frame);
		ASSERT(pFrame);
		if (pFrame)
		{
			// Set the title, and initialize the document.
			// If document initialization fails, clean-up
			// the frame window and document.

			doc_template->SetDefaultTitle(pDoc);
			if (!pDoc->OnNewDocument())
			{
				ASSERT(FALSE);
				pFrame->DestroyWindow();
				pFrame = NULL;
			}
			else
			{
				CRect r(100, 100, 400, 250);	// backup values in case of no current frame window
				if (current_frame) {
					current_frame->GetWindowRect(&r);
					r.OffsetRect(50, 50);
				}
				pFrame->SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), SWP_SHOWWINDOW);

				// Otherwise, update the frame
				doc_template->InitialUpdateFrame(pFrame, pDoc, TRUE);
			}
		}
	}

	// If we failed, clean up the document and show a
	// message to the user.

	if (!pFrame || !pDoc)
	{
		delete pDoc;
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
	}
}

// CGraphDoc commands

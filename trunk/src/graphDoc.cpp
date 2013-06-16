// graphDoc.cpp : implementation of the CGraphDoc class
//

#include "stdafx.h"
#include "graphDoc.h"

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

static CFrameWnd* GetParentFrameForDocument(CDocument* doc)
{
	if (doc) {
		POSITION view_position = doc->GetFirstViewPosition();
		CView * const view = doc->GetNextView(view_position);
		if (view) {
			CFrameWnd * const frame = view->GetParentFrame();
			if (frame) {
				return frame;
			}
		}
	}
	return NULL;
}

BOOL CGraphDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	BOOL can_close = __super::CanCloseFrame(pFrame);

	if (can_close) {
		CWinApp * const app = AfxGetApp();
		CFrameWnd * const our_frame = GetParentFrameForDocument(this);
		ASSERT(app);
		ASSERT(our_frame);

		// If we're closing and our frame window is m_pMainWnd then set m_pMainWnd to one of our siblings (if one exists) 
		// before we're destroyed. If we don't do this then CWnd::OnNcDestroy will terminate the application
		if (app && app->m_pMainWnd == our_frame) {
			CDocTemplate * const our_doc_template = GetDocTemplate();
			ASSERT(our_doc_template);
			if (our_doc_template) {
				POSITION other_doc_position = our_doc_template->GetFirstDocPosition();
				ASSERT(other_doc_position);
				CDocument* other_doc = NULL;

				// Iterate through sibling documents looking for one that isn't this
				while (other_doc_position && NULL != (other_doc = our_doc_template->GetNextDoc(other_doc_position))) {
					if (other_doc != static_cast<CDocument*>(this)) {
						CFrameWnd * const other_frame = GetParentFrameForDocument(other_doc);
						ASSERT(other_frame);
						if (other_frame) {
							app->m_pMainWnd = other_frame;		// found sibling, set m_pMainWnd and finish
							break;
						}
					}
				}
			}
		}


	}
	return can_close;
}


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

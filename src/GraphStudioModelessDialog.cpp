
#include "stdafx.h"

#include "GraphStudioModelessDialog.h"
#include "MainFrm.h"


/* Logic for restoring dialogs to saved positions

   If saved position
	   get saved rectangle
	   if all corners are on the same monitor
		  success
	   try stored pos + default size
	   if all corners are on the same monitor
		  success
	   try default pos + stored size
	   if all corners are on the same monitor
		  success
   default to default pos + default size
   else if still not visible just position near top left of nearest monitor at default size
*/

static bool IsRectOnSingleMonitor(const CRect& rect)
{
	const HMONITOR monitor = MonitorFromPoint(rect.TopLeft(), MONITOR_DEFAULTTONULL);	// Find monitor from top left of rect
	MONITORINFO info;
	info.cbSize = sizeof(info);

	if (!monitor || !GetMonitorInfo(monitor, &info))		// Get monitor's info
		return false;										// fail if not on monitor or no info

	CRect intersection;
	intersection.IntersectRect(&info.rcMonitor, &rect);		// Calculate intersection between rect and monitor
	return intersection == rect;							// rect is fully on monitor iff intersection of rect and monitor is rect
}

// Ensure that rect_saved is on a single monitor or adjust until it is (last resort use default size at top left of nearest monitor)
static void EnsureRectVisible(CRect& rect_saved, const CRect& rect_default)
{
	if (IsRectOnSingleMonitor(rect_saved))
		return;

	// Otherwise try saved size at default position
	CRect rect = rect_saved;
	rect.MoveToXY(rect_default.TopLeft());

	if (IsRectOnSingleMonitor(rect)) {
		rect_saved = rect;
		return;
	}

	// Otherwise try default size at saved position
	rect = rect_saved;
	rect.right  = rect.left + rect_default.Width();
	rect.bottom = rect.top  + rect_default.Height();

	if (IsRectOnSingleMonitor(rect)) {
		rect_saved = rect;
		return;
	}

	// otherwise use default position and size
	rect_saved = rect_default;
	if (IsRectOnSingleMonitor(rect_default)) {
		return;
	}

	// As last resort put it near top left of nearest monitor with default size
	const HMONITOR monitor = MonitorFromRect(&rect_default, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO info;
	info.cbSize = sizeof(info);
	if (GetMonitorInfo(monitor, &info)) {
		rect_saved.MoveToXY(info.rcWork.left + 100, info.rcWork.top + 100);
	}
}

IMPLEMENT_DYNAMIC(CGraphStudioModelessDialog, CDialog)

BEGIN_MESSAGE_MAP(CGraphStudioModelessDialog, CDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CGraphStudioModelessDialog::CGraphStudioModelessDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
	: CDialog(lpszTemplateName, pParentWnd)
	, view(NULL)
	, position_saved(false)
{
	ASSERT(pParentWnd);		// this should at least be the main frame window
}

CGraphStudioModelessDialog::CGraphStudioModelessDialog(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)
	, view(NULL) 
	, position_saved(false)
{
	ASSERT(pParentWnd);		// this should at least be the main frame window
}

BOOL CGraphStudioModelessDialog::PreTranslateMessage(MSG *pmsg)
{
	switch (pmsg->message) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			// Only forward key combinations containing control or alt to prevent interfering with edit boxes etc
			if ((GetKeyState(VK_CONTROL)&0x8000) || (GetKeyState(VK_MENU)&0x8000)) {
				CMainFrame * const frame = view ? view->GetParentFrame() : NULL;
				if (frame && frame->TranslateKeyboardAccelerator(pmsg)) {
					return TRUE;
				}
			}
	}
	return __super::PreTranslateMessage(pmsg);
}

void CGraphStudioModelessDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	ASSERT(view);

	if (bShow) 
		RestorePosition();
}

void CGraphStudioModelessDialog::OnClose()
{
	if (this->IsWindowVisible()) {
		SavePosition();
	}
	__super::OnClose();
}

void CGraphStudioModelessDialog::OnDestroy()
{
	if (!position_saved) {
		SavePosition();
	}
	__super::OnDestroy();
}

CString CGraphStudioModelessDialog::GetSettingsName() const
{
	const CRuntimeClass * const class_info = this->GetRuntimeClass();

	// If this assertion fires a derived class has not implemented MFC runtime class information and won't have uniquely named registry settings
	ASSERT(class_info && class_info->m_lpszClassName && strcmp(class_info->m_lpszClassName, "CGraphStudioModelessDialog"));

	class_info ? class_info->m_lpszClassName : NULL;

	CString name(_T("Settings\\"));

	return name + (class_info && class_info->m_lpszClassName ? CString(class_info->m_lpszClassName) : CString(_T("UnknownClass")));
}

CRect CGraphStudioModelessDialog::GetDefaultRect() const 
{
	return CRect(50, 200, 250, 400);
}

void CGraphStudioModelessDialog::RestorePosition()
{
	if (!view || !ShouldRestorePosition())
		return;

	position_saved = false;

	CRect rect_current;
	this->GetWindowRect(&rect_current);

	// if any settings fail to read then defaults will be unusable
	CRect rect_touse;
	const CString name(GetSettingsName());
	rect_touse.left	=	AfxGetApp()->GetProfileInt(name, _T("left"),	INT_MIN);
	rect_touse.top	=	AfxGetApp()->GetProfileInt(name, _T("top"),		INT_MIN);
	if (ShouldRestoreSize()) {
		rect_touse.right	= rect_touse.left +	AfxGetApp()->GetProfileInt(name, _T("width"),	INT_MAX/2);
		rect_touse.bottom	= rect_touse.top +	AfxGetApp()->GetProfileInt(name, _T("height"),	INT_MAX/2);
	} else {
		rect_touse.right	= rect_touse.left	+	rect_current.Width();
		rect_touse.bottom	= rect_touse.top	+	rect_current.Height();
	}

	CRect rect_default(GetDefaultRect());
	if (!ShouldRestoreSize()) {
		rect_default.right	= rect_default.left + rect_current.Width();
		rect_default.bottom = rect_default.top	+ rect_current.Height();
	}

	CRect rect_frame;
	view->GetParentFrame()->GetWindowRect(&rect_frame);
	rect_touse.OffsetRect(rect_frame.left, rect_frame.top);			// adjust rects by position of frame window
	rect_default.OffsetRect(rect_frame.left, rect_frame.top);

	EnsureRectVisible(rect_touse, rect_default);

	const int flags = ShouldRestoreSize() ? 0 : SWP_NOSIZE;
	this->SetWindowPos(NULL, rect_touse.left, rect_touse.top, rect_touse.Width(), rect_touse.Height(), flags);
}

void CGraphStudioModelessDialog::SavePosition()
{
	if (!ShouldRestorePosition())
		return;

	ASSERT(view);
	const CFrameWnd * const frame = view ? view->GetParentFrame() : NULL;
	ASSERT(frame);

	if (AfxGetApp()->GetMainWnd() == frame) {			// Only save position if we're the main window
		position_saved = true;

		CRect rect_frame;
		frame->GetWindowRect(&rect_frame);

		CRect rect_this;
		this->GetWindowRect(&rect_this);

		rect_this.OffsetRect(-rect_frame.left, -rect_frame.top);	// calculate offset of our window relative to top left of frame

		CString name(GetSettingsName());

		AfxGetApp()->WriteProfileInt(name, _T("left"),		rect_this.left);
		AfxGetApp()->WriteProfileInt(name, _T("top"),		rect_this.top);
		if (ShouldRestoreSize()) {
			AfxGetApp()->WriteProfileInt(name, _T("width"),		rect_this.Width());
			AfxGetApp()->WriteProfileInt(name, _T("height"),	rect_this.Height());
		}
	}
}


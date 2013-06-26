
#include "stdafx.h"

#include "GraphStudioModelessDialog.h"
#include "MainFrm.h"

IMPLEMENT_DYNAMIC(CGraphStudioModelessDialog, CDialog)

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

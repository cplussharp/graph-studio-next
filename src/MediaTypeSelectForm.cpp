// MediaTypeSelectForm.cpp : implementation file
//

#include "stdafx.h"
#include "MediaTypeSelectForm.h"

#include "display_graph.h"


// CMediaTypeSelectForm dialog

IMPLEMENT_DYNAMIC(CMediaTypeSelectForm, CDialog)

CMediaTypeSelectForm::CMediaTypeSelectForm(CWnd* pParent /*=NULL*/)
	: CDialog(CMediaTypeSelectForm::IDD, pParent)
	, selected_media_type_index(-1)
{

}

CMediaTypeSelectForm::~CMediaTypeSelectForm()
{
}

BOOL CMediaTypeSelectForm::OnInitDialog()
{
    CDialog::OnInitDialog();

    // prepare titlebar
	m_title.ModifyStyle(0, WS_CLIPCHILDREN);
	m_title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	enum Columns			// Column order in list control
	{
		SUB_TYPE = 0,
		FORMAT_TYPE,
		FORMAT_DETAILS,
        MAJOR_TYPE,
	};

	CRect clientRect;
	media_types_list.GetClientRect(&clientRect);
	const int columnWidth = (clientRect.Width() - GetSystemMetrics(SM_CXVSCROLL)) / 4;
	media_types_list.SetView(LV_VIEW_DETAILS);
    media_types_list.SetExtendedStyle( media_types_list.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
    media_types_list.InsertColumn(SUB_TYPE,		_T("Sub Type"),		LVCFMT_LEFT, columnWidth, SUB_TYPE); 
	media_types_list.InsertColumn(FORMAT_TYPE,	_T("Format Type"),	LVCFMT_LEFT, columnWidth, FORMAT_TYPE); 
    media_types_list.InsertColumn(FORMAT_DETAILS,	_T("Format Details"),	LVCFMT_LEFT, columnWidth, FORMAT_DETAILS); 
    media_types_list.InsertColumn(MAJOR_TYPE,	_T("Major Type"),	LVCFMT_LEFT, columnWidth, MAJOR_TYPE); 

	// First entry is <Any> Media Type
	LPCTSTR any = _T("<Any>");
	media_types_list.InsertItem(0, any);
	media_types_list.SetItemText(0, FORMAT_TYPE, any);
    media_types_list.SetItemText(0, FORMAT_DETAILS, any);
	media_types_list.SetItemText(0, MAJOR_TYPE, any);
	media_types_list.SetItemData(0, -1);

	for (size_t index=0; index<media_types.GetCount(); index++) {
		const CMediaType& mediaType = media_types[index];
		CString majorType, subType, formatType, formatDetails;

		GraphStudio::NameGuid(mediaType.majortype,	majorType,	CgraphstudioApp::g_showGuidsOfKnownTypes);
		GraphStudio::NameGuid(mediaType.subtype,	subType,	CgraphstudioApp::g_showGuidsOfKnownTypes);
		GraphStudio::NameGuid(mediaType.formattype, formatType, CgraphstudioApp::g_showGuidsOfKnownTypes);

        // get formatDetails (like '640x480' or '2 channels 44khz')
        if (mediaType.pbFormat != NULL)
        {
            BITMAPINFOHEADER* bmi = NULL;
            if(mediaType.formattype == FORMAT_VideoInfo)
                bmi = &((VIDEOINFOHEADER*)mediaType.pbFormat)->bmiHeader;
            else if(mediaType.formattype == FORMAT_VideoInfo2 || mediaType.formattype == FORMAT_MPEG2_VIDEO)
                bmi = &((VIDEOINFOHEADER2*)mediaType.pbFormat)->bmiHeader;

            if(bmi != NULL)
                formatDetails.Format(_T("%4d x %4d"), bmi->biWidth, bmi->biHeight);
            else if(mediaType.formattype == FORMAT_WaveFormatEx)
            {
                WAVEFORMATEX* wfx = (WAVEFORMATEX*)mediaType.pbFormat;
                formatDetails.Format(_T("%dx %dHz with %dBits"), wfx->nChannels, wfx->nSamplesPerSec, wfx->wBitsPerSample);
            }
        }

		media_types_list.InsertItem(index+1, subType);
		media_types_list.SetItemText(index+1, FORMAT_TYPE, formatType);
        media_types_list.SetItemText(index+1, FORMAT_DETAILS, formatDetails);
		media_types_list.SetItemText(index+1, MAJOR_TYPE, majorType);
		media_types_list.SetItemData(index+1, index);
	}

    media_types_list.SetFocus();

    return FALSE;
}

int CMediaTypeSelectForm::GetSelectedMediaType()
{
	int selectedType = -1;		// default to no media type selected

	if (media_types_list.GetSelectedCount() == 1) {
		const int selected = media_types_list.GetNextItem(-1, LVNI_SELECTED);
		if (selected >= 0) {
			selectedType = media_types_list.GetItemData(selected);
		}
	}
	return selectedType;
}

void CMediaTypeSelectForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, m_title);
	DDX_Control(pDX, IDC_LIST_MEDIATYPES, media_types_list);
	DDX_Control(pDX, IDOK, ok_button);
	DDX_Control(pDX, IDCANCEL, cancel_button);

	//{{AFX_DATA_MAP(CMediaTypeSelectForm)
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		selected_media_type_index = GetSelectedMediaType();
    }
}

BEGIN_MESSAGE_MAP(CMediaTypeSelectForm, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MEDIATYPES, &CMediaTypeSelectForm::OnDblclkListMediatypes)
	ON_WM_SIZE()
	ON_WM_SIZING()
END_MESSAGE_MAP()


// CMediaTypeSelectForm message handlers


void CMediaTypeSelectForm::OnDblclkListMediatypes(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (pNMItemActivate && pNMItemActivate->iItem >= 0) {
		OnOK();
	}
	*pResult = 0;
}


void CMediaTypeSelectForm::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

    CRect		rc, rc2;
	GetClientRect(&rc);
    m_title.GetClientRect(&rc2);

	CRect okRect;
	ok_button.GetWindowRect(&okRect);
	const int buttonXSpacing = okRect.Width() / 4;
	const int buttonYSpacing = okRect.Height() / 2;

	media_types_list.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height() - 2*okRect.Height(), SWP_SHOWWINDOW);

	const int buttonY = cy - okRect.Height() - buttonYSpacing;
	int buttonX = cx - buttonXSpacing - okRect.Width();
	cancel_button.SetWindowPos(NULL, buttonX, buttonY, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);

	buttonX -= buttonXSpacing + okRect.Width();
	ok_button.SetWindowPos(NULL, buttonX, buttonY, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);

    m_title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW);
	m_title.Invalidate();
}


void CMediaTypeSelectForm::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);

	CRect* dlgRect = static_cast<CRect*>(pRect);
	if (dlgRect) {
		CRect okRect;
		ok_button.GetWindowRect(&okRect);

		// Prevent dialog being resized below minimum height and width
		const int minHeight = okRect.Height() * 8;
		const int minWidth = okRect.Width() * 3;

		if (dlgRect->Width() < minWidth)
			dlgRect->right = dlgRect->left + minWidth;

		if (dlgRect->Height() < minHeight)
			dlgRect->bottom = dlgRect->top + minHeight;
	}
}

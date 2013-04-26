// MediaTypeSelectForm.cpp : implementation file
//

#include "stdafx.h"
#include "MediaTypeSelectForm.h"

#include "display_graph.h"


// CMediaTypeSelectForm dialog

BOOL CMediaTypeSelectForm::s_use_major_type		= TRUE;
BOOL CMediaTypeSelectForm::s_use_sub_type		= TRUE;
BOOL CMediaTypeSelectForm::s_use_sample_size		= TRUE;
BOOL CMediaTypeSelectForm::s_use_format_block	= TRUE;


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
		INDEX = 0,
		SUB_TYPE,
		FORMAT_TYPE,
		FORMAT_DETAILS,
        MAJOR_TYPE,
	};

	CRect clientRect;
	media_types_list.GetClientRect(&clientRect);
	const int columnWidth = (clientRect.Width() - GetSystemMetrics(SM_CXVSCROLL)) / 4;
	media_types_list.SetView(LV_VIEW_DETAILS);
    media_types_list.SetExtendedStyle( media_types_list.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP );
    media_types_list.InsertColumn(SUB_TYPE,			_T("Index"),			LVCFMT_LEFT, columnWidth/4, INDEX); 
    media_types_list.InsertColumn(SUB_TYPE,			_T("Sub Type"),			LVCFMT_LEFT, columnWidth, SUB_TYPE); 
	media_types_list.InsertColumn(FORMAT_TYPE,		_T("Format Type"),		LVCFMT_LEFT, columnWidth, FORMAT_TYPE); 
    media_types_list.InsertColumn(FORMAT_DETAILS,	_T("Format Details"),	LVCFMT_LEFT, columnWidth, FORMAT_DETAILS); 
    media_types_list.InsertColumn(MAJOR_TYPE,		_T("Major Type"),		LVCFMT_LEFT, 3*columnWidth/4, MAJOR_TYPE); 

	// First entry is <Default> Media Type
	const LPCTSTR any = _T("<Default>");
	media_types_list.InsertItem(0, NULL);

	media_types_list.SetItemText(0, SUB_TYPE,		any);
	media_types_list.SetItemText(0, FORMAT_TYPE,	any);
    media_types_list.SetItemText(0, FORMAT_DETAILS, any);
	media_types_list.SetItemText(0, MAJOR_TYPE,		any);
	media_types_list.SetItemData(0,					-1);

	for (size_t index=0; index<media_types.GetCount(); index++) {
		const CMediaType& mediaType = media_types[index];
		CString indexStr, majorType, subType, formatType, formatDetails;

		indexStr.Format(_T("%u"), index);
		GraphStudio::NameGuid(mediaType.majortype,	majorType,	CgraphstudioApp::g_showGuidsOfKnownTypes);
		GraphStudio::NameGuid(mediaType.subtype,	subType,	CgraphstudioApp::g_showGuidsOfKnownTypes);
		GraphStudio::NameGuid(mediaType.formattype, formatType, CgraphstudioApp::g_showGuidsOfKnownTypes);

        // get formatDetails (like '640x480' or '2 channels 44khz')
        if (mediaType.pbFormat)
        {
            const BITMAPINFOHEADER* bmi = NULL;
			if(mediaType.formattype == FORMAT_VideoInfo && mediaType.cbFormat >= sizeof(VIDEOINFOHEADER))
                bmi = &((const VIDEOINFOHEADER*)mediaType.pbFormat)->bmiHeader;
            else if( (mediaType.formattype == FORMAT_VideoInfo2 || mediaType.formattype == FORMAT_MPEG2_VIDEO)
					&& mediaType.cbFormat >= sizeof(VIDEOINFOHEADER) )
                bmi = &((const VIDEOINFOHEADER2*)mediaType.pbFormat)->bmiHeader;

            if(bmi != NULL) 
			{
				const int pixels = bmi->biWidth * bmi->biHeight;
				const float averageBPP = pixels ? (8.0*bmi->biSizeImage)/pixels : 0;
				formatDetails.Format(_T("%4d x %4d, %3d bpp, %6.3f av"), 
							bmi->biWidth, bmi->biHeight, bmi->biBitCount, averageBPP);
			} 
			else if(mediaType.formattype == FORMAT_WaveFormatEx && mediaType.cbFormat >= sizeof(WAVEFORMATEX))
            {
                const WAVEFORMATEX* const wfx = (WAVEFORMATEX*)mediaType.pbFormat;
                formatDetails.Format(_T("%dx %dHz with %dBits"), wfx->nChannels, wfx->nSamplesPerSec, wfx->wBitsPerSample);
            }
        }

		media_types_list.InsertItem(index+1, NULL);

		media_types_list.SetItemText(index+1, INDEX,			indexStr);
		media_types_list.SetItemText(index+1, SUB_TYPE,			subType);
		media_types_list.SetItemText(index+1, FORMAT_TYPE,		formatType);
        media_types_list.SetItemText(index+1, FORMAT_DETAILS,	formatDetails);
		media_types_list.SetItemText(index+1, MAJOR_TYPE,		majorType);
		media_types_list.SetItemData(index+1,					index);
	}

    // Select default entry
    LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask	= LVIF_STATE;
	lvi.state	= LVIS_SELECTED;
	lvi.stateMask   = LVIS_SELECTED;
    media_types_list.SetItemState(0, &lvi);

    // focus to the list, to navigate in it
    media_types_list.SetFocus();
    //SendMessage(WM_NEXTDLGCTL, (WPARAM)media_types_list.GetSafeHwnd(), TRUE);

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
	DDX_Check(pDX, IDC_USE_MAJOR_TYPE,		s_use_major_type);
	DDX_Check(pDX, IDC_USE_SUB_TYPE,		s_use_sub_type);
	DDX_Check(pDX, IDC_USE_SAMPLE_SIZE,		s_use_sample_size);
	DDX_Check(pDX, IDC_USE_FORMAT_BLOCK,	s_use_format_block);
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

	media_types_list.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height() - 2*okRect.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	const int buttonY = cy - okRect.Height() - buttonYSpacing;
	int buttonX = cx - buttonXSpacing - okRect.Width();
	cancel_button.SetWindowPos(NULL, buttonX, buttonY, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);

	buttonX -= buttonXSpacing + okRect.Width();
	ok_button.SetWindowPos(NULL, buttonX, buttonY, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);

    m_title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
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

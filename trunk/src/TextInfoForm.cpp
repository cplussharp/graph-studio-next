//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "TextInfoForm.h"


//-----------------------------------------------------------------------------
//
//	CTextInfoForm dialog
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CTextInfoForm, CGraphStudioModelessDialog)
BEGIN_MESSAGE_MAP(CTextInfoForm, CGraphStudioModelessDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_COPYTEXT, &CTextInfoForm::OnBnClickedButtonCopytext)
    ON_CBN_SELCHANGE(IDC_COMBO_REPORTTYPE, &CTextInfoForm::OnBnClickedButtonRefresh)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CTextInfoForm::OnClickedButtonSave)
END_MESSAGE_MAP()


LPCTSTR	ReportNames[] =
{
	_T("Graph Report (Level 1)"),
	_T("Graph Report (Level 2)"),
	_T("Graph Report (Level 3)"),
	_T("Graph Report (Level 4)"),
	_T("Graph Report (Level 5)"),
    _T("Graph Report (Level 6)")
};
int ReportNamesCount = sizeof(ReportNames)/sizeof(ReportNames[0]);



CTextInfoForm::CTextInfoForm(CWnd* pParent) : 
	CGraphStudioModelessDialog(CTextInfoForm::IDD, pParent)
{

}

CTextInfoForm::~CTextInfoForm()
{
}

BOOL CTextInfoForm::DoCreateDialog(CWnd* parent)
{
	BOOL ret = Create(IDD, parent);

	if (!ret) return FALSE;

    // prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

    CRect	rc;
	rc.SetRect(0, 0, 60, 23);
    btn_copy.Create(_T("&Copy"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_COPYTEXT);
    btn_copy.SetWindowPos(NULL, 4, 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
    btn_copy.SetFont(GetFont());

    btn_save.Create(_T("&Save"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rc, &title, IDC_BUTTON_SAVE);
    btn_save.SetWindowPos(NULL, 8 + rc.Width(), 4, rc.Width(), rc.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
    btn_save.SetFont(GetFont());

    rc.SetRect(0, 0, 150, 23);
    combo_reporttype.Create(WS_TABSTOP | WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, rc, &title, IDC_COMBO_REPORTTYPE);
    combo_reporttype.SetFont(GetFont());

	// Force a second resize to give the combo box a chance to position itself once it's fully created
	// Would not be an issue if the setup above was done in OnInitDialog...
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE);		// resize down to zero
	RestorePosition();								// then restore position or set default position

	OnInitialize();

	return TRUE;
};

CRect CTextInfoForm::GetDefaultRect() const 
{
	return CRect(50, 200, 650, 600);
}

void CTextInfoForm::OnInitialize()
{
	if(GraphStudio::HasFont(_T("Consolas")))
        GraphStudio::MakeFont(font_report, _T("Consolas"), 10, false, false);
    else
        GraphStudio::MakeFont(font_report, _T("Courier New"), 10, false, false);
	edit_report.SetFont(&font_report);

	combo_reporttype.ResetContent();
	for (int i=0; i<ReportNamesCount; i++) {
		combo_reporttype.AddString(ReportNames[i]);
	}
	combo_reporttype.SetCurSel(0);
}

void CTextInfoForm::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_EDIT_DETAILS, edit_report);
}

void CTextInfoForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);
	
	if (IsWindow(edit_report)) {
        title.GetClientRect(&rc2);
        title.SetWindowPos(NULL, 0, 0, rc.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

        edit_report.SetWindowPos(NULL, 0, rc2.Height(), rc.Width(), rc.Height() - rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

		combo_reporttype.GetWindowRect(&rc2);
        combo_reporttype.SetWindowPos(NULL, rc.Width()-4-rc2.Width(), 5, rc2.Width(), rc2.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

        title.Invalidate();
        edit_report.Invalidate();
	}
}


// CTextInfoForm message handlers

void CTextInfoForm::OnBnClickedButtonRefresh()
{
	edit_report.SetWindowText(_T(""));
	DoSimpleReport();
}

void CTextInfoForm::DoSimpleReport()
{
	// just enumerate the filters
	int level = combo_reporttype.GetCurSel();

	lines.RemoveAll();
	DoFilterList(level);
	DoConnectionDetails(level, 0);
    DoFileInfos(level);

	DisplayReport();
}

void CTextInfoForm::Echo(CString t)
{
	lines.Add(t);
}

void CTextInfoForm::DoFilterList(int level)
{
	CString t;
	Echo(_T("--------------------------------------------------"));
	Echo(_T("  Filters"));
	Echo(_T("--------------------------------------------------"));
	for (int i=0; i<view->graph.filters.GetCount(); i++) {
		GraphStudio::Filter	*filter = view->graph.filters[i];
		t.Format(_T("%3d. %s"), (i+1), filter->name);
		Echo(t);

		if (filter->file_name != _T("")) {
			CString	short_fn = PathFindFileName(filter->file_name);
			t = _T("      File: ") + short_fn;
            Echo(t);
		}

        if(level >= 0)
        {
		    CString	type;
		    switch (filter->filter_type) {
		    case GraphStudio::Filter::FILTER_DMO:		type = _T("DMO"); break;
		    case GraphStudio::Filter::FILTER_WDM:		type = _T("WDM"); break;
		    case GraphStudio::Filter::FILTER_STANDARD:	type = _T("Standard"); break;
		    case GraphStudio::Filter::FILTER_UNKNOWN:	type = _T("Unknown"); break;
		    }

            if(type != _T(""))
            {
		        t = _T("      Type:    ") + type;
                Echo(t);
            }

            if(filter->clsid_str != _T(""))
            {
                t = _T("      CLSID:   ") + filter->clsid_str;
                Echo(t);
            }

            if(level > 3)
            {
                GraphStudio::PropItem group(_T("Filter Details"));
				GraphStudio::GetFilterDetails(filter->clsid, &group);

                for(int i=0;i<group.GetCount();i++)
                {
                    if(group.GetItem(i)->name == _T("File"))
                    {
                        for(int j=0;j<group.GetItem(i)->GetCount();j++)
                        {
                            if(group.GetItem(i)->GetItem(j)->name == _T("File Name"))
                            {
                                t = _T("      File:    ") + group.GetItem(i)->GetItem(j)->value;
                                Echo(t);
                            }
                            if(group.GetItem(i)->GetItem(j)->name == _T("File Path"))
                            {
                                t = _T("      Path:    ") + group.GetItem(i)->GetItem(j)->value;
                                Echo(t);
                            }
                            else if(group.GetItem(i)->GetItem(j)->name == _T("Version Info"))
                            {
                                if(group.GetItem(i)->GetItem(j)->GetCount() > 0)
                                {
                                    t = _T("      Version: ") + group.GetItem(i)->GetItem(j)->GetItem(0)->value;
                                    Echo(t);
                                }
                            }
                        }
                    }
                }
            }
        }
	}
	Echo(_T(""));
}

void CTextInfoForm::DoFileInfos(int level)
{
    if(level < 5 || !view->render_params.use_media_info) return;
	CString t;
	Echo(_T("--------------------------------------------------"));
	Echo(_T("  Media Files"));
	Echo(_T("--------------------------------------------------"));
	for (int i=0; i<view->graph.filters.GetCount(); i++) {
		GraphStudio::Filter	*filter = view->graph.filters[i];
		if (filter->file_name != _T(""))
        {
            CMediaInfo* mi = CMediaInfo::GetInfoForFile(filter->file_name);
            if(mi != NULL)
            {
                Echo(mi->GetText());
                Echo(_T(""));
            }
		}
	}
	Echo(_T(""));
}

void CTextInfoForm::DoConnectionDetails(int level, int offset)
{
	CString		ofs, t;
	for (int i=0; i<offset; i++) ofs += _T(" ");

	if (level > 0)
	{
		Echo(ofs + _T("--------------------------------------------------"));
		Echo(ofs + _T("  Connections"));
		Echo(ofs + _T("--------------------------------------------------"));
	
		int index=0;
		for (int i=0; i<view->graph.filters.GetCount(); i++) {
			GraphStudio::Filter	*filter = view->graph.filters[i];

			for (int j=0; j<filter->output_pins.GetCount(); j++) {
				GraphStudio::Pin *opin = filter->output_pins[j];
				GraphStudio::Pin *peer = opin->peer;
				if (opin->connected && peer) {
					t.Format(_T("%3d. [%s]/(%s) -> [%s]/(%s)"), (index+1), filter->name, opin->name, peer->filter->name, peer->name);
					Echo(ofs + t);
					if (level > 0) { 
						DoPinDetails(opin, level, offset+6);
					}
					index++;
				}
			}
		}
	}
	Echo(_T(""));
}

void CTextInfoForm::DoPinDetails(GraphStudio::Pin *pin, int level, int offset)
{
	CString		ofs, t, f;
	for (int i=0; i<offset; i++) ofs += _T(" ");

	AM_MEDIA_TYPE	mt;
	HRESULT hr = pin->pin->ConnectionMediaType(&mt);
	if (SUCCEEDED(hr)) {
        GraphStudio::NameGuid(mt.majortype,  f, CgraphstudioApp::g_showGuidsOfKnownTypes);	
		t = _T("Major:   ") + f;		Echo(ofs+t);

		GraphStudio::NameGuid(mt.subtype,    f, CgraphstudioApp::g_showGuidsOfKnownTypes);	
		t = _T("Subtype: ") + f;		Echo(ofs+t);

		// parse one level deeper
		if (level > 1) {
			DoMediaTypeDetails(&mt, level, offset+4);
		}

		GraphStudio::NameGuid(mt.formattype, f, CgraphstudioApp::g_showGuidsOfKnownTypes);	
		t = _T("Format:  ") + f;		Echo(ofs+t);
		if (level > 2) {
			// parse format
			if (mt.formattype == FORMAT_WaveFormatEx) DoWaveFormatEx(&mt, level, offset); else
			if (mt.formattype == FORMAT_VideoInfo2) DoVideoInfo2(&mt, level, offset); else
			if (mt.formattype == FORMAT_VideoInfo) DoVideoInfo(&mt, level, offset); else
			if (mt.formattype == FORMAT_MPEG2_VIDEO) DoMPEG2VideoInfo(&mt, level, offset);
		}

		Echo(_T(""));
		FreeMediaType(mt);
	}
}

void CTextInfoForm::DoMediaTypeDetails(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString		ofs, t;
	for (int i=0; i<offset; i++) ofs += _T(" ");

	t = _T("bFixedSizeSamples:    ");
	if (pmt->bFixedSizeSamples) t += _T("TRUE"); else t += _T("FALSE");
	Echo(ofs +t);

	t = _T("bTemporalCompression: ");
	if (pmt->bTemporalCompression) t += _T("TRUE"); else t += _T("FALSE");
	Echo(ofs +t);
	t.Format(_T("lSampleSize:          %d"), pmt->lSampleSize);		Echo(ofs +t);
	t.Format(_T("cbFormat:             %d"), pmt->cbFormat);		Echo(ofs +t);
}

void CTextInfoForm::DoWaveFormatEx(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString			t, f, ofs;
	for (int i=0; i<offset; i++) ofs += _T(" ");

	if (!pmt->pbFormat || pmt->cbFormat < sizeof(WAVEFORMATEX))
		return;
	const WAVEFORMATEX	* const wfx = (WAVEFORMATEX*)pmt->pbFormat;

	Echo(ofs+_T("WAVEFORMATEX:"));
	t.Format(_T("    wFormatTag:           0x%04x (%d)"), wfx->wFormatTag, wfx->wFormatTag);	Echo(ofs+t);
	t.Format(_T("    nChannels:            %d"), wfx->nChannels);				Echo(ofs+t);
	t.Format(_T("    nSamplesPerSec:       %d"), wfx->nSamplesPerSec);			Echo(ofs+t);
	t.Format(_T("    nAvgBytesPerSec:      %d"), wfx->nAvgBytesPerSec);			Echo(ofs+t);
	t.Format(_T("    nBlockAlign:          %d"), wfx->nBlockAlign);				Echo(ofs+t);
	t.Format(_T("    wBitsPersample:       %d"), wfx->wBitsPerSample);			Echo(ofs+t);
	t.Format(_T("    cbSize:               %d"), wfx->cbSize);					Echo(ofs+t);

	if (wfx->cbSize > 0 && level > 3) {
		BYTE *raw = ((BYTE*)wfx) + sizeof(WAVEFORMATEX);
		Echo(ofs+_T("Extradata:"));
		DoDumpRawBuffer(raw, wfx->cbSize, offset+4);
	}
	
}

void CTextInfoForm::DoMPEG2VideoInfo(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString				t, f, ofs;
	for (int i=0; i<offset; i++) ofs += _T(" ");

	DoVideoInfo2(pmt, level, offset);

	if (!pmt->pbFormat || pmt->cbFormat < sizeof(MPEG2VIDEOINFO))
		return;
	const MPEG2VIDEOINFO * const mv = (MPEG2VIDEOINFO*)pmt->pbFormat;

	Echo(ofs+_T("MPEG2VIDEOINFO:"));
	t.Format(_T("    dwStartTimeCode:      %d"), mv->dwStartTimeCode);			Echo(ofs+t);
	t.Format(_T("    cbSequenceHeader:     %d"), mv->cbSequenceHeader);			Echo(ofs+t);
	switch (mv->dwProfile) {
	case AM_MPEG2Profile_Simple:			f = _T("AM_MPEG2Profile_Simple"); break;
	case AM_MPEG2Profile_Main:				f = _T("AM_MPEG2Profile_Main"); break;
	case AM_MPEG2Profile_SNRScalable:		f = _T("AM_MPEG2Profile_SNRScalable"); break;
	case AM_MPEG2Profile_SpatiallyScalable:	f = _T("AM_MPEG2Profile_SpatiallyScalable"); break;
	case AM_MPEG2Profile_High:				f = _T("AM_MPEG2Profile_High"); break;
	default:
		f.Format(_T("%d"), mv->dwProfile);
		break;
	}
	t.Format(_T("    dwProfile:            %s"), f);			Echo(ofs+t);

	switch (mv->dwLevel) {
	case AM_MPEG2Level_Low:			f = _T("AM_MPEG2Level_Low"); break;
	case AM_MPEG2Level_Main:		f = _T("AM_MPEG2Level_Main"); break;
	case AM_MPEG2Level_High1440:	f = _T("AM_MPEG2Level_High1440"); break;
	case AM_MPEG2Level_High:		f = _T("AM_MPEG2Level_High"); break;
	default:
		f.Format(_T("%d"), mv->dwLevel);
		break;
	}
	t.Format(_T("    dwLevel:              %s"), f);			Echo(ofs+t);
	if (mv->cbSequenceHeader > 0 && level > 3) {
		BYTE *raw = (BYTE*)mv->dwSequenceHeader;
		Echo(ofs+_T("Sequence Header:"));
		DoDumpRawBuffer(raw, mv->cbSequenceHeader, offset+4);
	}
}

void CTextInfoForm::DoVideoInfo2(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString				t, f, ofs;
	for (int i=0; i<offset; i++) ofs += _T(" ");

	if (!pmt->pbFormat || pmt->cbFormat < sizeof(VIDEOINFOHEADER2))
		return;
	const VIDEOINFOHEADER2 * const vih = (VIDEOINFOHEADER2*)pmt->pbFormat;

	Echo(ofs+_T("VIDEOINFOHEADER2:"));
	t.Format(_T("    rcSource:             (%d,%d,%d,%d)"), vih->rcSource.left,vih->rcSource.top, vih->rcSource.right, vih->rcSource.bottom);
	Echo(ofs+t);
	t.Format(_T("    rcTarget:             (%d,%d,%d,%d)"), vih->rcTarget.left,vih->rcTarget.top, vih->rcTarget.right, vih->rcTarget.bottom);
	Echo(ofs+t);
	t.Format(_T("    dwBitRate:            %d"), vih->dwBitRate);				Echo(ofs+t);
	t.Format(_T("    dwBitErrorRate:       %d"), vih->dwBitErrorRate);			Echo(ofs+t);
	t.Format(_T("    AvgTimePerFrame:      %I64d"), vih->AvgTimePerFrame);		Echo(ofs+t);
	t.Format(_T("    dwInterlaceFlags:     %d"), vih->dwInterlaceFlags);		Echo(ofs+t);
	t.Format(_T("    dwCopyProtectFlags:   %d"), vih->dwCopyProtectFlags);		Echo(ofs+t);
	t.Format(_T("    dwPictAspectRatioX:   %d"), vih->dwPictAspectRatioX);		Echo(ofs+t);
	t.Format(_T("    dwPictAspectRatioY:   %d"), vih->dwPictAspectRatioY);		Echo(ofs+t);
	t.Format(_T("    dwControlFlags:       %d"), vih->dwControlFlags);			Echo(ofs+t);
	DoBitmapInfoHeader(&vih->bmiHeader, offset);

	/*
	int left = sizeof(VIDEOINFOHEADER2) - pmt->cbFormat;
	if (left > 0 && level > 3) {
		BYTE *raw = ((BYTE*)vih) + sizeof(VIDEOINFOHEADER2);
		Echo(ofs+_T("Extradata:"));
		DoDumpRawBuffer(raw, left, offset+4);
	}

    VIDEOINFOHEADER2    hdr;

	*/
}

void CTextInfoForm::DoVideoInfo(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString				t, f, ofs;
	for (int i=0; i<offset; i++) ofs += _T(" ");

	if (!pmt->pbFormat || pmt->cbFormat < sizeof(VIDEOINFOHEADER))
		return;
	const VIDEOINFOHEADER * const vih = (VIDEOINFOHEADER*)pmt->pbFormat;

	Echo(ofs+_T("VIDEOINFOHEADER:"));
	t.Format(_T("    rcSource:             (%d,%d,%d,%d)"), vih->rcSource.left,vih->rcSource.top, vih->rcSource.right, vih->rcSource.bottom);
	Echo(ofs+t);
	t.Format(_T("    rcTarget:             (%d,%d,%d,%d)"), vih->rcTarget.left,vih->rcTarget.top, vih->rcTarget.right, vih->rcTarget.bottom);
	Echo(ofs+t);
	t.Format(_T("    dwBitRate:            %d"), vih->dwBitRate);				Echo(ofs+t);
	t.Format(_T("    dwBitErrorRate:       %d"), vih->dwBitErrorRate);			Echo(ofs+t);
	t.Format(_T("    AvgTimePerFrame:      %I64d"), vih->AvgTimePerFrame);		Echo(ofs+t);
	DoBitmapInfoHeader(&vih->bmiHeader, offset);

	/*
	int left = sizeof(VIDEOINFOHEADER) - pmt->cbFormat;
	if (left > 0 && level > 3) {
		BYTE *raw = ((BYTE*)vih) + sizeof(VIDEOINFOHEADER);
		Echo(ofs+_T("Extradata:"));
		DoDumpRawBuffer(raw, left, offset+4);
	}
	*/
}

void CTextInfoForm::DoBitmapInfoHeader(const BITMAPINFOHEADER *bmi, int offset)
{
	CString				t, f, ofs;
	for (int i=0; i<offset; i++) ofs += _T(" ");

	Echo(ofs+_T("BITMAPINFOHEADER:"));
	t.Format(_T("    biSize:               %d"), bmi->biSize);				Echo(ofs+t);
	t.Format(_T("    biWidth:              %d"), bmi->biWidth);				Echo(ofs+t);
	t.Format(_T("    biHeight:             %d"), bmi->biHeight);			Echo(ofs+t);
	t.Format(_T("    biPlanes:             %d"), bmi->biPlanes);			Echo(ofs+t);
	t.Format(_T("    biBitCount:           %d"), bmi->biBitCount);			Echo(ofs+t);
	t.Format(_T("    biCompression:        0x%08X"), bmi->biCompression);	Echo(ofs+t);
	t.Format(_T("    biSizeImage:          %d"), bmi->biSizeImage);			Echo(ofs+t);
	t.Format(_T("    biXPelsPerMeter:      %d"), bmi->biXPelsPerMeter);		Echo(ofs+t);
	t.Format(_T("    biYPelsPerMeter:      %d"), bmi->biYPelsPerMeter);		Echo(ofs+t);
	t.Format(_T("    biClrUsed:            %d"), bmi->biClrUsed);			Echo(ofs+t);
	t.Format(_T("    biClrImportant:       %d"), bmi->biClrImportant);		Echo(ofs+t);
}

void CTextInfoForm::DoDumpRawBuffer(void *buf, int len, int offset)
{
	CString		ofs, line, t;
	for (int i=0; i<offset; i++) ofs += _T(" ");

	BYTE *ptr = (BYTE*)buf;
	int	left = len;
	while (left > 0) {
		line = ofs;
		for (int i=0; i<16 && left>0; i++) {
			t.Format(_T("%02x "), ptr[0]);
			ptr++;
			left--;
			line = line + t;
			if (i == 7) line += _T(" ");
		}
		Echo(line);
	}
}


void CTextInfoForm::DisplayReport()
{
	CString		t, t2;
	for (int i=0; i<lines.GetCount(); i++) {
		t = t + lines[i] + _T("\r\n");
	}
	edit_report.GetWindowText(t2);
	t = t + t2;
	edit_report.SetWindowText(t);
}

void CTextInfoForm::OnBnClickedButtonCopytext()
{
	// copy the content to the clipboard
	CString		text;

	edit_report.GetWindowText(text);

    DSUtil::SetClipboardText(this->GetSafeHwnd(),text);
}


void CTextInfoForm::OnClickedButtonSave()
{
	CString	filter;
	CString	filename;

	filter = _T("Log Files (*.log,*.txt)|*.log;*.txt|All Files (*.*)|*.*|");

	CFileDialog dlg(FALSE,_T("log"),NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK)
    {
		CPath path(filename);
		if (path.GetExtension() == _T(""))
        {
			path.AddExtension(_T(".log"));
			filename = CString(path);
		}

        CFile file(filename, CFile::modeCreate|CFile::modeWrite);
        
        CString	text;
        edit_report.GetWindowText(text);
        CT2CA outputText(text, CP_UTF8);
        file.Write(outputText, ::strlen(outputText));
    }
}

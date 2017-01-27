//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GraphReportGenerator.h"

CGraphReportGenerator::CGraphReportGenerator(GraphStudio::DisplayGraph* graph, bool use_media_info)
	: m_graph(graph), m_use_media_info(use_media_info), m_level(0)
{
}


CGraphReportGenerator::~CGraphReportGenerator()
{
}

CString CGraphReportGenerator::GetReport(int level)
{
	if (m_lines.IsEmpty() || m_level != level)
		GenerateReport(level);

	CString	report;
	for (int i = 0; i < m_lines.GetCount(); i++)
	{
		report.Append(m_lines[i]);
		report.Append(_T("\r\n"));
	}

	return report;
}

void CGraphReportGenerator::SaveReport(int level, CString fn)
{
	if (m_lines.IsEmpty() || m_level != level)
		GenerateReport(level);

	CString	report;
	for (int i = 0; i < m_lines.GetCount(); i++)
	{
		report.Append(m_lines[i]);
		report.Append(_T("\r\n"));
	}

	FILE		*f = NULL;
	if (_tfopen_s(&f, fn, _T("wb")) != NOERROR || !f)
		return;

	CStringA	xa = UTF16toUTF8(report);
	fwrite(xa.GetBuffer(), 1, xa.GetLength(), f);
	fclose(f);
}

CStringA CGraphReportGenerator::UTF16toUTF8(const CStringW &utf16)
{
	CStringA utf8;
	int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, 0, 0);
	if (len>1) {
		char *ptr = utf8.GetBuffer(len - 1);
		if (ptr) WideCharToMultiByte(CP_UTF8, 0, utf16, -1, ptr, len, 0, 0);
		utf8.ReleaseBuffer();
	}
	return utf8;
}

void CGraphReportGenerator::GenerateReport(int level)
{
	m_lines.RemoveAll();
	m_level = level;

	DoFilterList(level);
	DoConnectionDetails(level);
	DoFileInfos(level);
}

void CGraphReportGenerator::DoFilterList(int level)
{
	CString t;
	Line(_T("--------------------------------------------------"));
	Line(_T("  Filters"));
	Line(_T("--------------------------------------------------"));
	for (int i = 0; i<m_graph->filters.GetCount(); i++) {
		GraphStudio::Filter	*filter = m_graph->filters[i];
		t.Format(_T("%3d. %s"), (i + 1), (LPCTSTR)filter->name);
		Line(t);

		if (filter->file_name != _T("")) {
			CString	short_fn = PathFindFileName(filter->file_name);
			t = _T("      File: ") + short_fn;
			Line(t);
		}

		if (level >= 0)
		{
			CString	type;
			switch (filter->filter_type) {
			case GraphStudio::Filter::FILTER_DMO:		type = _T("DMO"); break;
			case GraphStudio::Filter::FILTER_WDM:		type = _T("WDM"); break;
			case GraphStudio::Filter::FILTER_STANDARD:	type = _T("Standard"); break;
			case GraphStudio::Filter::FILTER_UNKNOWN:	type = _T("Unknown"); break;
			}

			if (type != _T(""))
			{
				t = _T("      Type:    ") + type;
				Line(t);
			}

			if (filter->clsid_str != _T(""))
			{
				t = _T("      CLSID:   ") + filter->clsid_str;
				Line(t);
			}

			if (level > 3)
			{
				GraphStudio::PropItem group(_T("Filter Details"));
				GraphStudio::GetFilterDetails(*filter, &group);

				for (int i = 0; i<group.GetCount(); i++)
				{
					if (group.GetItem(i)->name == _T("File"))
					{
						for (int j = 0; j<group.GetItem(i)->GetCount(); j++)
						{
							if (group.GetItem(i)->GetItem(j)->name == _T("File Name"))
							{
								t = _T("      File:    ") + group.GetItem(i)->GetItem(j)->value;
								Line(t);
							}
							if (group.GetItem(i)->GetItem(j)->name == _T("File Path"))
							{
								t = _T("      Path:    ") + group.GetItem(i)->GetItem(j)->value;
								Line(t);
							}
							else if (group.GetItem(i)->GetItem(j)->name == _T("Version Info"))
							{
								if (group.GetItem(i)->GetItem(j)->GetCount() > 0)
								{
									t = _T("      Version: ") + group.GetItem(i)->GetItem(j)->GetItem(0)->value;
									Line(t);
								}
							}
						}
					}
				}
			}
		}
	}
	Line();
}

void CGraphReportGenerator::DoFileInfos(int level)
{
	if (level < 5 || !m_use_media_info) return;
	CString t;
	Line(_T("--------------------------------------------------"));
	Line(_T("  Media Files"));
	Line(_T("--------------------------------------------------"));
	for (int i = 0; i<m_graph->filters.GetCount(); i++) {
		GraphStudio::Filter	*filter = m_graph->filters[i];
		if (filter->file_name != _T(""))
		{
			bool getFileInfo = filter->IsSource();

			// Don't check in running state because the default writer filter will block the access to the file
			// and the file will mostly have no valid content!
			if (!getFileInfo && filter->IsRenderer())
			{
				FILTER_STATE graph_state = State_Stopped;
				m_graph->GetState(graph_state, 10);
				getFileInfo = graph_state != State_Running;
			}
			
			if (getFileInfo)
			{
				CMediaInfo* mi = CMediaInfo::GetInfoForFile(filter->file_name, false);
				if (mi != NULL)
				{
					Line(mi->GetText());
					Line();
				}
			}
		}
	}
	Line();
}

void CGraphReportGenerator::DoConnectionDetails(int level)
{
	CString	t;

	if (level > 0)
	{
		Line(_T("--------------------------------------------------"));
		Line(_T("  Connections"));
		Line(_T("--------------------------------------------------"));

		int index = 0;
		for (int i = 0; i<m_graph->filters.GetCount(); i++) {
			GraphStudio::Filter	*filter = m_graph->filters[i];

			for (int j = 0; j<filter->output_pins.GetCount(); j++) {
				GraphStudio::Pin *opin = filter->output_pins[j];
				GraphStudio::Pin *peer = opin->peer;
				if (opin->connected && peer) {
					t.Format(_T("%3d. [%s]/(%s) -> [%s]/(%s)"), (index + 1), (LPCTSTR)filter->name, (LPCTSTR)opin->name, (LPCTSTR)peer->filter->name, (LPCTSTR)peer->name);
					Line(t);
					if (level > 0) {
						DoPinDetails(opin, level, 6);
					}
					index++;
				}
			}
		}
	}
	Line();
}

void CGraphReportGenerator::DoPinDetails(GraphStudio::Pin *pin, int level, int offset)
{
	CString		ofs, t, f;
	for (int i = 0; i<offset; i++) ofs += _T(" ");

	AM_MEDIA_TYPE	mt;
	HRESULT hr = pin->ipin->ConnectionMediaType(&mt);
	if (SUCCEEDED(hr)) {
		GraphStudio::NameGuid(mt.majortype, f, CgraphstudioApp::g_showGuidsOfKnownTypes);
		t = _T("Major:   ") + f;		Line(ofs + t);

		GraphStudio::NameGuid(mt.subtype, f, CgraphstudioApp::g_showGuidsOfKnownTypes);
		t = _T("Subtype: ") + f;		Line(ofs + t);

		// parse one level deeper
		if (level > 1) {
			DoMediaTypeDetails(&mt, level, offset + 4);
		}

		GraphStudio::NameGuid(mt.formattype, f, CgraphstudioApp::g_showGuidsOfKnownTypes);
		t = _T("Format:  ") + f;		Line(ofs + t);
		if (level > 2) {
			// parse format
			if (mt.formattype == FORMAT_WaveFormatEx) DoWaveFormatEx(&mt, level, offset); else
				if (mt.formattype == FORMAT_VideoInfo2) DoVideoInfo2(&mt, level, offset); else
					if (mt.formattype == FORMAT_VideoInfo) DoVideoInfo(&mt, level, offset); else
						if (mt.formattype == FORMAT_MPEG2_VIDEO) DoMPEG2VideoInfo(&mt, level, offset);
		}

		Line();
		FreeMediaType(mt);
	}
}

void CGraphReportGenerator::DoMediaTypeDetails(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString		ofs, t;
	for (int i = 0; i<offset; i++) ofs += _T(" ");

	t = _T("bFixedSizeSamples:    ");
	if (pmt->bFixedSizeSamples) t += _T("TRUE"); else t += _T("FALSE");
	Line(ofs + t);

	t = _T("bTemporalCompression: ");
	if (pmt->bTemporalCompression) t += _T("TRUE"); else t += _T("FALSE");
	Line(ofs + t);
	t.Format(_T("lSampleSize:          %d"), pmt->lSampleSize);		Line(ofs + t);
	t.Format(_T("cbFormat:             %d"), pmt->cbFormat);		Line(ofs + t);
}

void CGraphReportGenerator::DoWaveFormatEx(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString			t, f, ofs;
	for (int i = 0; i<offset; i++) ofs += _T(" ");

	if (!pmt->pbFormat || pmt->cbFormat < sizeof(WAVEFORMATEX))
		return;
	const WAVEFORMATEX	* const wfx = (WAVEFORMATEX*)pmt->pbFormat;

	Line(ofs + _T("WAVEFORMATEX:"));
	t.Format(_T("    wFormatTag:           0x%04x (%d)"), wfx->wFormatTag, wfx->wFormatTag);	Line(ofs + t);
	t.Format(_T("    nChannels:            %d"), wfx->nChannels);				Line(ofs + t);
	t.Format(_T("    nSamplesPerSec:       %d"), wfx->nSamplesPerSec);			Line(ofs + t);
	t.Format(_T("    nAvgBytesPerSec:      %d"), wfx->nAvgBytesPerSec);			Line(ofs + t);
	t.Format(_T("    nBlockAlign:          %d"), wfx->nBlockAlign);				Line(ofs + t);
	t.Format(_T("    wBitsPersample:       %d"), wfx->wBitsPerSample);			Line(ofs + t);
	t.Format(_T("    cbSize:               %d"), wfx->cbSize);					Line(ofs + t);

	if (wfx->cbSize > 0 && level > 3) {
		BYTE *raw = ((BYTE*)wfx) + sizeof(WAVEFORMATEX);
		Line(ofs + _T("Extradata:"));
		DoDumpRawBuffer(raw, wfx->cbSize, offset + 4);
	}

}

void CGraphReportGenerator::DoMPEG2VideoInfo(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString				t, f, ofs;
	for (int i = 0; i<offset; i++) ofs += _T(" ");

	DoVideoInfo2(pmt, level, offset);

	if (!pmt->pbFormat || pmt->cbFormat < sizeof(MPEG2VIDEOINFO))
		return;
	const MPEG2VIDEOINFO * const mv = (MPEG2VIDEOINFO*)pmt->pbFormat;

	Line(ofs + _T("MPEG2VIDEOINFO:"));
	t.Format(_T("    dwStartTimeCode:      %d"), mv->dwStartTimeCode);			Line(ofs + t);
	t.Format(_T("    cbSequenceHeader:     %d"), mv->cbSequenceHeader);			Line(ofs + t);
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
	t.Format(_T("    dwProfile:            %s"), (LPCTSTR)f);			Line(ofs + t);

	switch (mv->dwLevel) {
	case AM_MPEG2Level_Low:			f = _T("AM_MPEG2Level_Low"); break;
	case AM_MPEG2Level_Main:		f = _T("AM_MPEG2Level_Main"); break;
	case AM_MPEG2Level_High1440:	f = _T("AM_MPEG2Level_High1440"); break;
	case AM_MPEG2Level_High:		f = _T("AM_MPEG2Level_High"); break;
	default:
		f.Format(_T("%d"), mv->dwLevel);
		break;
	}
	t.Format(_T("    dwLevel:              %s"), (LPCTSTR)f);			Line(ofs + t);
	if (mv->cbSequenceHeader > 0 && level > 3) {
		BYTE *raw = (BYTE*)mv->dwSequenceHeader;
		Line(ofs + _T("Sequence Header:"));
		DoDumpRawBuffer(raw, mv->cbSequenceHeader, offset + 4);
	}
}

void CGraphReportGenerator::DoVideoInfo2(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString				t, f, ofs;
	for (int i = 0; i<offset; i++) ofs += _T(" ");

	if (!pmt->pbFormat || pmt->cbFormat < sizeof(VIDEOINFOHEADER2))
		return;
	const VIDEOINFOHEADER2 * const vih = (VIDEOINFOHEADER2*)pmt->pbFormat;

	Line(ofs + _T("VIDEOINFOHEADER2:"));
	t.Format(_T("    rcSource:             (%d,%d,%d,%d)"), vih->rcSource.left, vih->rcSource.top, vih->rcSource.right, vih->rcSource.bottom);
	Line(ofs + t);
	t.Format(_T("    rcTarget:             (%d,%d,%d,%d)"), vih->rcTarget.left, vih->rcTarget.top, vih->rcTarget.right, vih->rcTarget.bottom);
	Line(ofs + t);
	t.Format(_T("    dwBitRate:            %d"), vih->dwBitRate);				Line(ofs + t);
	t.Format(_T("    dwBitErrorRate:       %d"), vih->dwBitErrorRate);			Line(ofs + t);
	t.Format(_T("    AvgTimePerFrame:      %I64d"), vih->AvgTimePerFrame);		Line(ofs + t);
	t.Format(_T("    dwInterlaceFlags:     %d"), vih->dwInterlaceFlags);		Line(ofs + t);
	t.Format(_T("    dwCopyProtectFlags:   %d"), vih->dwCopyProtectFlags);		Line(ofs + t);
	t.Format(_T("    dwPictAspectRatioX:   %d"), vih->dwPictAspectRatioX);		Line(ofs + t);
	t.Format(_T("    dwPictAspectRatioY:   %d"), vih->dwPictAspectRatioY);		Line(ofs + t);
	t.Format(_T("    dwControlFlags:       %d"), vih->dwControlFlags);			Line(ofs + t);
	DoBitmapInfoHeader(&vih->bmiHeader, offset);

	/*
	int left = sizeof(VIDEOINFOHEADER2) - pmt->cbFormat;
	if (left > 0 && level > 3) {
	BYTE *raw = ((BYTE*)vih) + sizeof(VIDEOINFOHEADER2);
	Line(ofs+_T("Extradata:"));
	DoDumpRawBuffer(raw, left, offset+4);
	}

	VIDEOINFOHEADER2    hdr;

	*/
}

void CGraphReportGenerator::DoVideoInfo(AM_MEDIA_TYPE *pmt, int level, int offset)
{
	CString				t, f, ofs;
	for (int i = 0; i<offset; i++) ofs += _T(" ");

	if (!pmt->pbFormat || pmt->cbFormat < sizeof(VIDEOINFOHEADER))
		return;
	const VIDEOINFOHEADER * const vih = (VIDEOINFOHEADER*)pmt->pbFormat;

	Line(ofs + _T("VIDEOINFOHEADER:"));
	t.Format(_T("    rcSource:             (%d,%d,%d,%d)"), vih->rcSource.left, vih->rcSource.top, vih->rcSource.right, vih->rcSource.bottom);
	Line(ofs + t);
	t.Format(_T("    rcTarget:             (%d,%d,%d,%d)"), vih->rcTarget.left, vih->rcTarget.top, vih->rcTarget.right, vih->rcTarget.bottom);
	Line(ofs + t);
	t.Format(_T("    dwBitRate:            %d"), vih->dwBitRate);				Line(ofs + t);
	t.Format(_T("    dwBitErrorRate:       %d"), vih->dwBitErrorRate);			Line(ofs + t);
	t.Format(_T("    AvgTimePerFrame:      %I64d"), vih->AvgTimePerFrame);		Line(ofs + t);
	DoBitmapInfoHeader(&vih->bmiHeader, offset);

	/*
	int left = sizeof(VIDEOINFOHEADER) - pmt->cbFormat;
	if (left > 0 && level > 3) {
	BYTE *raw = ((BYTE*)vih) + sizeof(VIDEOINFOHEADER);
	Line(ofs+_T("Extradata:"));
	DoDumpRawBuffer(raw, left, offset+4);
	}
	*/
}

void CGraphReportGenerator::DoBitmapInfoHeader(const BITMAPINFOHEADER *bmi, int offset)
{
	CString				t, f, ofs;
	for (int i = 0; i<offset; i++) ofs += _T(" ");

	Line(ofs + _T("BITMAPINFOHEADER:"));
	t.Format(_T("    biSize:               %d"), bmi->biSize);				Line(ofs + t);
	t.Format(_T("    biWidth:              %d"), bmi->biWidth);				Line(ofs + t);
	t.Format(_T("    biHeight:             %d"), bmi->biHeight);			Line(ofs + t);
	t.Format(_T("    biPlanes:             %d"), bmi->biPlanes);			Line(ofs + t);
	t.Format(_T("    biBitCount:           %d"), bmi->biBitCount);			Line(ofs + t);
	t.Format(_T("    biCompression:        0x%08X"), bmi->biCompression);	Line(ofs + t);
	t.Format(_T("    biSizeImage:          %d"), bmi->biSizeImage);			Line(ofs + t);
	t.Format(_T("    biXPelsPerMeter:      %d"), bmi->biXPelsPerMeter);		Line(ofs + t);
	t.Format(_T("    biYPelsPerMeter:      %d"), bmi->biYPelsPerMeter);		Line(ofs + t);
	t.Format(_T("    biClrUsed:            %d"), bmi->biClrUsed);			Line(ofs + t);
	t.Format(_T("    biClrImportant:       %d"), bmi->biClrImportant);		Line(ofs + t);
}

void CGraphReportGenerator::DoDumpRawBuffer(void *buf, int len, int offset)
{
	CString		ofs, line, t;
	for (int i = 0; i<offset; i++) ofs += _T(" ");

	BYTE *ptr = (BYTE*)buf;
	int	left = len;
	while (left > 0) {
		line = ofs;
		for (int i = 0; i<16 && left>0; i++) {
			t.Format(_T("%02x "), ptr[0]);
			ptr++;
			left--;
			line = line + t;
			if (i == 7) line += _T(" ");
		}
		Line(line);
	}
}

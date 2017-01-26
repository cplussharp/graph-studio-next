#pragma once
//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

class CGraphReportGenerator
{
public:
	CGraphReportGenerator(GraphStudio::DisplayGraph* graph, bool use_media_info);
	virtual ~CGraphReportGenerator();

	CString GetReport(int level);
	void SaveReport(int level, CString fn);

protected:
	GraphStudio::DisplayGraph* m_graph;
	bool m_use_media_info;

	void GenerateReport(int level);
	
	int m_level;
	CArray<CString> m_lines;

	static CStringA UTF16toUTF8(const CStringW &utf16);
	void Line(CString line = _T("")) { m_lines.Add(line); }

	// report parts
	void DoFilterList(int level);
	void DoFileInfos(int level);
	void DoConnectionDetails(int level);

	void DoPinDetails(GraphStudio::Pin *pin, int level, int offset);
	void DoMediaTypeDetails(AM_MEDIA_TYPE *pmt, int level, int offset);

	// formats
	void DoWaveFormatEx(AM_MEDIA_TYPE *pmt, int level, int offset);
	void DoVideoInfo(AM_MEDIA_TYPE *pmt, int level, int offset);
	void DoVideoInfo2(AM_MEDIA_TYPE *pmt, int level, int offset);
	void DoMPEG2VideoInfo(AM_MEDIA_TYPE *pmt, int level, int offset);
	void DoBitmapInfoHeader(const BITMAPINFOHEADER *bmi, int offset);

	void DoDumpRawBuffer(void *buf, int len, int offset);
};

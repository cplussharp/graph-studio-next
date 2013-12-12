//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	//-------------------------------------------------------------------------
	//
	//	Details
	//
	//-------------------------------------------------------------------------

	int GetVersionInfo(CString filename, PropItem *info);
	int GetFileDetails(CString filename, PropItem *filedetails);
	int GetFilterDetails(const CLSID & clsid, PropItem *info);
	int GetFilterInformationFromTemplate(const DSUtil::FilterTemplate &, PropItem *info);
	int GetFilterInformationFromCLSID(const DSUtil::FilterCategories & categories, const GUID & clsid, PropItem * info);

	void CLSIDToString(const CLSID& clsid, CString &str);

	int GetObjectName(GUID clsid, CString &name);
	int GetObjectFile(GUID clsid, CString &filename);

	int GetPinDetails(IPin *pin, PropItem *info);
	int GetPinTemplateDetails(const DSUtil::PinTemplate *pin, PropItem *info);
	int GetMediaTypeDetails(const CMediaType *pmt, PropItem *mtinfo);

	int GetAllocatorDetails(const ALLOCATOR_PROPERTIES *prop, PropItem *apinfo);
	int GetWaveFormatExDetails(const WAVEFORMATEX *wfx, PropItem *wfxinfo);
	int GetWaveFormatExtensibleDetails(const WAVEFORMATEXTENSIBLE *wfx, PropItem *wfxinfo);
	int GetVideoInfoDetails(const VIDEOINFOHEADER *vih, PropItem *vihinfo);
	int GetVideoInfo2Details(const VIDEOINFOHEADER2 *vih, PropItem *vihinfo);
	int GetBitmapInfoDetails(const BITMAPINFOHEADER *bih, PropItem *bihinfo);

	int GetMpeg1VideoInfoDetails(const MPEG1VIDEOINFO *mvi, PropItem *mviinfo);
	int GetMpeg2VideoInfoDetails(const MPEG2VIDEOINFO *mvi, PropItem *mviinfo);
	int GetMpegLayer3InfoDetails(const MPEGLAYER3WAVEFORMAT *mp3, PropItem *mp3info);
	int GetMpeg1WaveFormatDetails(const MPEG1WAVEFORMAT *wfx, PropItem *mpinfo);

	// Four-CC
	int GetFourCC(DWORD fcc, CString &str);

	// Decoder Specific Information
	int GetExtradata_AAC(const CMediaType *pmt, PropItem *mtinfo);
    int GetExtradata_H264(const CMediaType *pmt, PropItem *mtinfo);
    int GetExtradata_MPEGVideo(const CMediaType *pmt, PropItem *mtinfo);

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

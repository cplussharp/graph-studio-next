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
	int GetFilterDetails(CLSID clsid, PropItem *info);

	void CLSIDToString(const CLSID& clsid, CString &str);

	int GetObjectName(GUID clsid, CString &name);
	int GetObjectFile(GUID clsid, CString &filename);

	int GetPinDetails(IPin *pin, PropItem *info);
	int GetPinTemplateDetails(DSUtil::PinTemplate *pin, PropItem *info);
	int GetMediaTypeDetails(CMediaType *pmt, PropItem *mtinfo);

	int GetAllocatorDetails(ALLOCATOR_PROPERTIES *prop, PropItem *apinfo);
	int GetWaveFormatExDetails(WAVEFORMATEX *wfx, PropItem *wfxinfo);
	int GetWaveFormatExtensibleDetails(WAVEFORMATEXTENSIBLE *wfx, PropItem *wfxinfo);
	int GetVideoInfoDetails(VIDEOINFOHEADER *vih, PropItem *vihinfo);
	int GetVideoInfo2Details(VIDEOINFOHEADER2 *vih, PropItem *vihinfo);
	int GetBitmapInfoDetails(BITMAPINFOHEADER *bih, PropItem *bihinfo);

	int GetMpeg1VideoInfoDetails(MPEG1VIDEOINFO *mvi, PropItem *mviinfo);
	int GetMpeg2VideoInfoDetails(MPEG2VIDEOINFO *mvi, PropItem *mviinfo);
	int GetMpegLayer3InfoDetails(MPEGLAYER3WAVEFORMAT *mp3, PropItem *mp3info);
	int GetMpeg1WaveFormatDetails(MPEG1WAVEFORMAT *wfx, PropItem *mpinfo);

	// Four-CC
	int GetFourCC(DWORD fcc, CString &str);

	// Decoder Specific Information
	int GetExtradata_AAC(CMediaType *pmt, PropItem *mtinfo);
    int GetExtradata_H264(CMediaType *pmt, PropItem *mtinfo);
    int GetExtradata_MPEGVideo(CMediaType *pmt, PropItem *mtinfo);

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

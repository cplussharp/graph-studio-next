//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

namespace GraphStudio
{

#define	KNOWN(x)			{ x, _T( # x ) }
	struct GuidPair {
		GUID	guid; 
		LPCTSTR	name;
	};

	const GuidPair	KnownGuidList[] = 
	{
		// filters
		KNOWN(CLSID_AsyncReader),
		KNOWN(GUID_NULL),

		// Media Types
		KNOWN(MEDIATYPE_AnalogAudio),
		KNOWN(MEDIATYPE_AnalogVideo),
		KNOWN(MEDIATYPE_Audio),
		KNOWN(MEDIATYPE_AUXLine21Data),
		KNOWN(MEDIATYPE_DTVCCData),
		KNOWN(MEDIATYPE_DVD_ENCRYPTED_PACK),
		KNOWN(MEDIATYPE_DVD_NAVIGATION),
		KNOWN(MEDIATYPE_File),
		KNOWN(MEDIATYPE_Interleaved),
		KNOWN(MEDIATYPE_LMRT),
		KNOWN(MEDIATYPE_Midi),
		KNOWN(MEDIATYPE_MPEG1SystemStream),
		KNOWN(MEDIATYPE_MPEG2_PACK),
		KNOWN(MEDIATYPE_MPEG2_PES),
		KNOWN(MEDIATYPE_MSTVCaption),
		KNOWN(MEDIATYPE_NULL),
		KNOWN(MEDIATYPE_ScriptCommand),
		KNOWN(MEDIATYPE_Stream),
		KNOWN(MEDIATYPE_Text),
		KNOWN(MEDIATYPE_Timecode),
		KNOWN(MEDIATYPE_URL_STREAM),
		KNOWN(MEDIATYPE_VBI),
		KNOWN(MEDIATYPE_Video),

		// SubTypes
		KNOWN(MEDIASUBTYPE_A2B10G10R10),
		KNOWN(MEDIASUBTYPE_A2R10G10B10),
		KNOWN(MEDIASUBTYPE_AI44),
		KNOWN(MEDIASUBTYPE_AIFF),
		KNOWN(MEDIASUBTYPE_AnalogVideo_NTSC_M),
		KNOWN(MEDIASUBTYPE_AnalogVideo_PAL_B),
		KNOWN(MEDIASUBTYPE_AnalogVideo_PAL_D),
		KNOWN(MEDIASUBTYPE_AnalogVideo_PAL_G),
		KNOWN(MEDIASUBTYPE_AnalogVideo_PAL_H),
		KNOWN(MEDIASUBTYPE_AnalogVideo_PAL_I),
		KNOWN(MEDIASUBTYPE_AnalogVideo_PAL_M),
		KNOWN(MEDIASUBTYPE_AnalogVideo_PAL_N),
		KNOWN(MEDIASUBTYPE_AnalogVideo_PAL_N_COMBO),
		KNOWN(MEDIASUBTYPE_AnalogVideo_SECAM_B),
		KNOWN(MEDIASUBTYPE_AnalogVideo_SECAM_D),
		KNOWN(MEDIASUBTYPE_AnalogVideo_SECAM_G),
		KNOWN(MEDIASUBTYPE_AnalogVideo_SECAM_H),
		KNOWN(MEDIASUBTYPE_AnalogVideo_SECAM_K),
		KNOWN(MEDIASUBTYPE_AnalogVideo_SECAM_K1),
		KNOWN(MEDIASUBTYPE_AnalogVideo_SECAM_L),
		KNOWN(MEDIASUBTYPE_ARGB1555),
		KNOWN(MEDIASUBTYPE_ARGB1555_D3D_DX7_RT),
		KNOWN(MEDIASUBTYPE_ARGB1555_D3D_DX9_RT),
		KNOWN(MEDIASUBTYPE_ARGB32),
		KNOWN(MEDIASUBTYPE_ARGB32_D3D_DX7_RT),
		KNOWN(MEDIASUBTYPE_ARGB32_D3D_DX9_RT),
		KNOWN(MEDIASUBTYPE_ARGB4444),
		KNOWN(MEDIASUBTYPE_ARGB4444_D3D_DX7_RT),
		KNOWN(MEDIASUBTYPE_ARGB4444_D3D_DX9_RT),
		KNOWN(MEDIASUBTYPE_Asf),
		KNOWN(MEDIASUBTYPE_AU),
		KNOWN(MEDIASUBTYPE_Avi),
		KNOWN(MEDIASUBTYPE_AYUV),
		KNOWN(MEDIASUBTYPE_CFCC),
		KNOWN(MEDIASUBTYPE_CLJR),
		KNOWN(MEDIASUBTYPE_CLPL),
		KNOWN(MEDIASUBTYPE_CPLA),
		KNOWN(MEDIASUBTYPE_DOLBY_AC3),
		KNOWN(MEDIASUBTYPE_DOLBY_AC3_SPDIF),
		KNOWN(MEDIASUBTYPE_DRM_Audio),
		KNOWN(MEDIASUBTYPE_DssAudio),
		KNOWN(MEDIASUBTYPE_DssVideo),
		KNOWN(MEDIASUBTYPE_DTS),
		KNOWN(MEDIASUBTYPE_DtvCcData),
		KNOWN(MEDIASUBTYPE_dv25),
		KNOWN(MEDIASUBTYPE_dv50),
		KNOWN(MEDIASUBTYPE_DVCS),
		KNOWN(MEDIASUBTYPE_DVD_LPCM_AUDIO),
		KNOWN(MEDIASUBTYPE_DVD_NAVIGATION_DSI),
		KNOWN(MEDIASUBTYPE_DVD_NAVIGATION_PCI),
		KNOWN(MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER),
		KNOWN(MEDIASUBTYPE_DVD_SUBPICTURE),
		KNOWN(MEDIASUBTYPE_dvh1),
		KNOWN(MEDIASUBTYPE_dvhd),
		KNOWN(MEDIASUBTYPE_DVSD),
		KNOWN(MEDIASUBTYPE_dvsd),
		KNOWN(MEDIASUBTYPE_dvsl),
		KNOWN(MEDIASUBTYPE_H264),
		KNOWN(MEDIASUBTYPE_IA44),
		KNOWN(MEDIASUBTYPE_IEEE_FLOAT),
		KNOWN(MEDIASUBTYPE_IF09),
		KNOWN(MEDIASUBTYPE_IJPG),
		KNOWN(MEDIASUBTYPE_IMC1),
		KNOWN(MEDIASUBTYPE_IMC2),
		KNOWN(MEDIASUBTYPE_IMC3),
		KNOWN(MEDIASUBTYPE_IMC4),
		KNOWN(MEDIASUBTYPE_IYUV),
		KNOWN(MEDIASUBTYPE_Line21_BytePair),
		KNOWN(MEDIASUBTYPE_Line21_GOPPacket),
		KNOWN(MEDIASUBTYPE_Line21_VBIRawData),
		KNOWN(MEDIASUBTYPE_MDVF),
		KNOWN(MEDIASUBTYPE_MJPG),
		KNOWN(MEDIASUBTYPE_MPEG1Audio),
		KNOWN(MEDIASUBTYPE_MPEG1AudioPayload),
		KNOWN(MEDIASUBTYPE_MPEG1Packet),
		KNOWN(MEDIASUBTYPE_MPEG1Payload),
		KNOWN(MEDIASUBTYPE_MPEG1System),
		KNOWN(MEDIASUBTYPE_MPEG1Video),
		KNOWN(MEDIASUBTYPE_MPEG1VideoCD),
		KNOWN(MEDIASUBTYPE_MPEG2_AUDIO),
		KNOWN(MEDIASUBTYPE_MPEG2_PROGRAM),
		KNOWN(MEDIASUBTYPE_MPEG2_TRANSPORT),
		KNOWN(MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE),
		KNOWN(MEDIASUBTYPE_MPEG2_UDCR_TRANSPORT),
		KNOWN(MEDIASUBTYPE_MPEG2_VIDEO),
		KNOWN(MEDIASUBTYPE_MPEG2_WMDRM_TRANSPORT),
		KNOWN(MEDIASUBTYPE_None),
		KNOWN(MEDIASUBTYPE_NULL),
		KNOWN(MEDIASUBTYPE_NV12),
		KNOWN(MEDIASUBTYPE_NV24),
		KNOWN(MEDIASUBTYPE_Overlay),
		KNOWN(MEDIASUBTYPE_PCM),
		KNOWN(MEDIASUBTYPE_Plum),
		KNOWN(MEDIASUBTYPE_QTJpeg),
		KNOWN(MEDIASUBTYPE_QTMovie),
		KNOWN(MEDIASUBTYPE_QTRle),
		KNOWN(MEDIASUBTYPE_QTRpza),
		KNOWN(MEDIASUBTYPE_QTSmc),
		KNOWN(MEDIASUBTYPE_RAW_SPORT),
		KNOWN(MEDIASUBTYPE_RGB1),
		KNOWN(MEDIASUBTYPE_RGB24),
		KNOWN(MEDIASUBTYPE_RGB32),
		KNOWN(MEDIASUBTYPE_RGB4),
		KNOWN(MEDIASUBTYPE_RGB555),
		KNOWN(MEDIASUBTYPE_RGB565),
		KNOWN(MEDIASUBTYPE_RGB8),
		KNOWN(MEDIASUBTYPE_S340),
		KNOWN(MEDIASUBTYPE_S342),
		KNOWN(MEDIASUBTYPE_SDDS),
		KNOWN(MEDIASUBTYPE_SPDIF_TAG_241h),
		KNOWN(MEDIASUBTYPE_TELETEXT),
		KNOWN(MEDIASUBTYPE_TVMJ),
		KNOWN(MEDIASUBTYPE_UYVY),
		KNOWN(MEDIASUBTYPE_VPS),
		KNOWN(MEDIASUBTYPE_VPVBI),
		KNOWN(MEDIASUBTYPE_VPVideo),
		KNOWN(MEDIASUBTYPE_WAKE),
		KNOWN(MEDIASUBTYPE_WAVE),
		KNOWN(MEDIASUBTYPE_WSS),
		KNOWN(MEDIASUBTYPE_Y211),
		KNOWN(MEDIASUBTYPE_Y411),
		KNOWN(MEDIASUBTYPE_Y41P),
		KNOWN(MEDIASUBTYPE_YUY2),
		KNOWN(MEDIASUBTYPE_YUYV),
		KNOWN(MEDIASUBTYPE_YV12),
		KNOWN(MEDIASUBTYPE_YVU9),
		KNOWN(MEDIASUBTYPE_YVYU),
	
		// Formats
		KNOWN(FORMAT_525WSS),
		KNOWN(FORMAT_AnalogVideo),
		KNOWN(FORMAT_DolbyAC3),
		KNOWN(FORMAT_DVD_LPCMAudio),
		KNOWN(FORMAT_DvInfo),
		KNOWN(FORMAT_MPEG2_VIDEO),
		KNOWN(FORMAT_MPEG2Audio),
		KNOWN(FORMAT_MPEG2Video),
		KNOWN(FORMAT_MPEGStreams),
		KNOWN(FORMAT_MPEGVideo),
		KNOWN(FORMAT_None),
		KNOWN(FORMAT_VideoInfo),
		KNOWN(FORMAT_VIDEOINFO2),
		KNOWN(FORMAT_VideoInfo2),
		KNOWN(FORMAT_WaveFormatEx),

		// Wave formats
		KNOWN(KSDATAFORMAT_SUBTYPE_PCM),
		KNOWN(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT),
		KNOWN(KSDATAFORMAT_SUBTYPE_DRM),
		KNOWN(KSDATAFORMAT_SUBTYPE_ALAW),
		KNOWN(KSDATAFORMAT_SUBTYPE_MULAW),
		KNOWN(KSDATAFORMAT_SUBTYPE_ADPCM)
	};
	const int KnownGuidCount = sizeof(KnownGuidList) / sizeof(KnownGuidList[0]);


	bool NameGuid(GUID guid, CString &str)
	{
		for (int i=0; i<KnownGuidCount; i++) {
			if (KnownGuidList[i].guid == guid) {
				str = CString(KnownGuidList[i].name);
				return true;
			}
		}

		LPOLESTR	str2;
		StringFromCLSID(guid, &str2);
		str = CString(str2);
		CoTaskMemFree(str2);
		return false;
	}


	struct FormatTagPair {
		int		tag;
		LPCTSTR	name;
	};

	const FormatTagPair	KnownFormatsList[] = 
	{
		{ 0,		_T("Invalid Format") },
		{ 1,		_T("Raw PCM") },
		{ 2,		_T("ADPCM") },
		{ 3,		_T("IEEE Float") },
		{ 5,		_T("IBM CVSD") },
		{ 6,		_T("A-Law PCM") },
		{ 7,		_T("Mu-Law PCM") },
		{ 0x10,		_T("OKI ADPCM") },
		{ 0x11,		_T("DVI/IMA ADPCM") },
		{ 0x12,		_T("Mediaspace ADPCM") },
		{ 0x13,		_T("Sierra ADPCM") },
		{ 0x14,		_T("G.723 ADPCM") },
		{ 0x15,		_T("DigiSTD") },
		{ 0x16,		_T("DigiFIX") },
		{ 0x17,		_T("Dialogic OKI ADPCM") },
		{ 0x18,		_T("Media Vision ADPCM") },
		{ 0x20,		_T("YAMAHA ADPCM") },
		{ 0x21,		_T("Sonarc Speech") },
		{ 0x22,		_T("DSP Group TrueSpeech") },
		{ 0x23,		_T("Echo Speech") },
		{ 0x24,		_T("AudioFile AF36") },
		{ 0x25,		_T("APTX") },
		{ 0x26,		_T("AudioFile AF10") },
		{ 0x30,		_T("Dolby AC-2") },
		{ 0x31,		_T("GSM 6.10") },
		{ 0x32,		_T("MSN Audio") },
		{ 0x33,		_T("Antex ADPCME") },
		{ 0x34,		_T("Control Resources VQLPC") },
		{ 0x35,		_T("DigiReal") },
		{ 0x36,		_T("DigiADPCM") },
		{ 0x37,		_T("Control Resources CR10") },
		{ 0x38,		_T("NMS VBXADPCM") },
		{ 0x39,		_T("CS IMA ADPCM") },
		{ 0x3A,		_T("Echo Speech 3") },
		{ 0x3B,		_T("Rockwell ADPCM") },
		{ 0x3C,		_T("Rockwell Digitalk") },
		{ 0x3D,		_T("Xebec") },
		{ 0x40,		_T("G.721 ADPCM") },
		{ 0x41,		_T("G.728 CELP") },
		{ 0x50,		_T("MPEG Audio") },
		{ 0x55,		_T("MPEG Layer 3") },
		{ 0x60,		_T("Cirrus Logic") },
		{ 0x61,		_T("ESS PCM") },
		{ 0x62,		_T("Voxware") },
		{ 0x63,		_T("Canopus ATRAC") },
		{ 0x64,		_T("G.726 ADPCM") },
		{ 0x65,		_T("G.722 ADPCM") },
		{ 0x66,		_T("DSAT") },
		{ 0x67,		_T("DSAT Display") },
		{ 0x80,		_T("Softsound") },
		{ 0xFF,		_T("MPEG-2/4 AAC") },
		{ 0x100,	_T("Rhetorex ADPCM") },
		{ 0x200,	_T("Creative ADPCM") },
		{ 0x202,	_T("Creative Fastspeech 8") },
		{ 0x203,	_T("Creative Fastspeech 10") },
		{ 0x300,	_T("Quarterdeck") },
		{ 0x400,	_T("Brooktree Digital") },
		{ 0x1000,	_T("Olivetti GSM") },
		{ 0x1001,	_T("Olivetti ADPCM") },
		{ 0x1002,	_T("Olivetti CELP") },
		{ 0x1003,	_T("Olivetti SBC") },
		{ 0x1004,	_T("Olivetti OPR") },
		{ 0x1100,	_T("LH Codec") },
		{ 0x1400,	_T("Norris") },
		{ 0x2000,	_T("Dolby AC-3") }
	};
	const int KnownFormatsCount = sizeof(KnownFormatsList) / sizeof(KnownFormatsList[0]);

	int GetFormatName(int wFormatTag, CString &str)
	{
		for (int i=0; i<KnownFormatsCount; i++) {
			if (wFormatTag == KnownFormatsList[i].tag) {
				str = CString(KnownFormatsList[i].name);
				return 0;
			}
		}

		str.Format(_T("Unknown (%d)"), wFormatTag);
		return 1;
	}


};



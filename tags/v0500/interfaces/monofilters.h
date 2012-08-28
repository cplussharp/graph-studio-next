//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

namespace Monogram
{
	/*
		These are interfaces for the most of monogram filters
	*/

	typedef __int64			int64;
	
	// {30323449-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_I420 = 
	{ 0x30323449, 0x0000, 0x000, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };

	// {A3F41BB6-19DF-416c-941A-89A2D84F4B9F}
	static const GUID CLSIDI_MonogramMultigraphSink = 
	{ 0xa3f41bb6, 0x19df, 0x416c, { 0x94, 0x1a, 0x89, 0xa2, 0xd8, 0x4f, 0x4b, 0x9f } };

	// {D3E5C632-3DC9-489b-8E80-7F436CE0F120}
	static const GUID IID_IMonogramMultigraphSink = 
	{ 0xd3e5c632, 0x3dc9, 0x489b, { 0x8e, 0x80, 0x7f, 0x43, 0x6c, 0xe0, 0xf1, 0x20 } };

	// {AF2F9B96-DE7C-4ae4-A955-15201E47AF6F}
	static const GUID CLSID_MonogramMultigraphSource = 
	{ 0xaf2f9b96, 0xde7c, 0x4ae4, { 0xa9, 0x55, 0x15, 0x20, 0x1e, 0x47, 0xaf, 0x6f } };

	// {9016FD9E-03C5-42ce-8BE1-AA46AB736893}
	static const GUID IID_IMonogramMultigraphSource = 
	{ 0x9016fd9e, 0x3c5, 0x42ce, { 0x8b, 0xe1, 0xaa, 0x46, 0xab, 0x73, 0x68, 0x93 } };

	// {88F36DB6-D898-40b5-B409-466A0EECC26A}
	static const GUID CLSID_MonogramAACEncoder = 
	{ 0x88f36db6, 0xd898, 0x40b5, { 0xb4, 0x9, 0x46, 0x6a, 0xe, 0xec, 0xc2, 0x6a } };

	// {353F6225-5DF8-4016-8C82-E435FC33C9FF}
	static const GUID CLSID_MonogramAACEncoderPropertypage = 
	{ 0x353f6225, 0x5df8, 0x4016, { 0x8c, 0x82, 0xe4, 0x35, 0xfc, 0x33, 0xc9, 0xff } };

	// {000000FF-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_AAC = 
	{ 0x000000FF, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

	// {B8CDACAE-7D39-4673-A80D-FFDD1D1B3DAF}
	static const GUID IID_IMonogramAACEncoderPropertyPage = 
	{ 0xb8cdacae, 0x7d39, 0x4673, { 0xa8, 0xd, 0xff, 0xdd, 0x1d, 0x1b, 0x3d, 0xaf } };

	// {4289BDC9-571F-41dc-B508-86B512B9F90C}
	static const GUID IID_IMonogramAACEncoder = 
	{ 0x4289bdc9, 0x571f, 0x41dc, { 0xb5, 0x8, 0x86, 0xb5, 0x12, 0xb9, 0xf9, 0xc } };

	// {B51F2FA2-4998-4763-AA3D-6F3BB8F92F2D}
	static const GUID CLSID_MonogramAVCEncoder = 
	{ 0xb51f2fa2, 0x4998, 0x4763, { 0xaa, 0x3d, 0x6f, 0x3b, 0xb8, 0xf9, 0x2f, 0x2d } };

	// {0A7A6B99-7BAC-41e3-BCA1-25FAC2FF6764}
	static const GUID CLSID_MonogramAVCEncoderPropertyPage = 
	{ 0xa7a6b99, 0x7bac, 0x41e3, { 0xbc, 0xa1, 0x25, 0xfa, 0xc2, 0xff, 0x67, 0x64 } };

	// {5F5177CA-1797-4b4e-9214-A017A4117120}
	static const GUID CLSID_MonogramAVCStatPage = 
	{ 0x5f5177ca, 0x1797, 0x4b4e, { 0x92, 0x14, 0xa0, 0x17, 0xa4, 0x11, 0x71, 0x20 } };

	// {C0DB9FCD-AFF8-41ff-AA00-EEA2CBAC8B72}
	static const GUID IID_IMonogramAVCEncoder = 
	{ 0xc0db9fcd, 0xaff8, 0x41ff, { 0xaa, 0x0, 0xee, 0xa2, 0xcb, 0xac, 0x8b, 0x72 } };

	// {658A49AB-2930-4479-B651-37AD36E017F0}
	static const GUID CLSID_MonogramLiveMux = 
	{ 0x658a49ab, 0x2930, 0x4479, { 0xb6, 0x51, 0x37, 0xad, 0x36, 0xe0, 0x17, 0xf0 } };

	// {F13D8636-E66A-4c84-9BB7-D64CE058AD29}
	static const GUID IID_IMonogramLiveMux = 
	{ 0xf13d8636, 0xe66a, 0x4c84, { 0x9b, 0xb7, 0xd6, 0x4c, 0xe0, 0x58, 0xad, 0x29 } };

	// {F5488CD7-2287-440a-906A-83F71958D290}
	static const GUID CLSID_MonogramAudioProc = 
	{ 0xf5488cd7, 0x2287, 0x440a, { 0x90, 0x6a, 0x83, 0xf7, 0x19, 0x58, 0xd2, 0x90 } };

	// {45815C33-859A-4549-93A7-70724C51048D}
	static const GUID CLSID_MonogramAudioPropertyPage = 
	{ 0x45815c33, 0x859a, 0x4549, { 0x93, 0xa7, 0x70, 0x72, 0x4c, 0x51, 0x4, 0x8d } };

	// {1FB0F046-623C-40a7-B439-41E4BFCB8BAB}
	static const GUID CLSID_MonogramX264 = 
	{ 0x1fb0f046, 0x623c, 0x40a7, { 0xb4, 0x39, 0x41, 0xe4, 0xbf, 0xcb, 0x8b, 0xab } };

	// {A519B1EF-EB84-4a49-AE39-38DC4533EBC1}
	static const GUID IID_IMonogramX264 = 
	{ 0xa519b1ef, 0xeb84, 0x4a49, { 0xae, 0x39, 0x38, 0xdc, 0x45, 0x33, 0xeb, 0xc1 } };

	// {65DD8A3A-CDA8-4fdc-BAAB-E8748C285926}
	static const GUID IID_IMonogramAudioProc = 
	{ 0x65dd8a3a, 0xcda8, 0x4fdc, { 0xba, 0xab, 0xe8, 0x74, 0x8c, 0x28, 0x59, 0x26 } };

	// {39C1B412-7960-493a-91F5-370286B57DDE}
	static const GUID CLSID_MonogramFrameGrabber = 
	{ 0x39c1b412, 0x7960, 0x493a, { 0x91, 0xf5, 0x37, 0x2, 0x86, 0xb5, 0x7d, 0xde } };

	// {DFAA04D9-482F-45f8-89E9-ED7FB0B05DF5}
	static const GUID CLSID_MonogramFrameGrabberPropertyPage = 
	{ 0xdfaa04d9, 0x482f, 0x45f8, { 0x89, 0xe9, 0xed, 0x7f, 0xb0, 0xb0, 0x5d, 0xf5 } };

	// {287B8E7A-C70C-4a22-AAA8-1D81D7EB92CF}
	static const GUID IID_IMonogramFrameGrabber = 
	{ 0x287b8e7a, 0xc70c, 0x4a22, { 0xaa, 0xa8, 0x1d, 0x81, 0xd7, 0xeb, 0x92, 0xcf } };

	// {C4EC9F87-094D-48ad-81C3-BCC01F644174}
	static const GUID CLSID_MonogramPump = 
	{ 0xc4ec9f87, 0x94d, 0x48ad, { 0x81, 0xc3, 0xbc, 0xc0, 0x1f, 0x64, 0x41, 0x74 } };

	// {66241FE9-EB9C-4c8d-A604-B9823BDA1C6A}
	static const GUID CLSID_MonogramPumpPage = 
	{ 0x66241fe9, 0xeb9c, 0x4c8d, { 0xa6, 0x4, 0xb9, 0x82, 0x3b, 0xda, 0x1c, 0x6a } };

	// {69317ABC-93EF-4304-997A-541472722E79}
	static const GUID IID_IMonogramPump = 
	{ 0x69317abc, 0x93ef, 0x4304, { 0x99, 0x7a, 0x54, 0x14, 0x72, 0x72, 0x2e, 0x79 } };

	// {DCAE2617-0091-4ce0-9C1E-ECEF0942D669}
	static const GUID IID_IMonogramQueue = 
	{ 0xdcae2617, 0x91, 0x4ce0, { 0x9c, 0x1e, 0xec, 0xef, 0x9, 0x42, 0xd6, 0x69 } };

	// {067EFD5B-EFE2-42b3-B466-864F5027DBDA}
	static const GUID CLSID_MonogramQueue = 
	{ 0x67efd5b, 0xefe2, 0x42b3, { 0xb4, 0x66, 0x86, 0x4f, 0x50, 0x27, 0xdb, 0xda } };

	// {E739CB5C-3F3E-486b-9A9B-0C833C85AA86}
	static const GUID CLSID_MonogramHttpSource = 
	{ 0xe739cb5c, 0x3f3e, 0x486b, { 0x9a, 0x9b, 0xc, 0x83, 0x3c, 0x85, 0xaa, 0x86 } };

	// {AE3481E0-1F41-43ae-A7FE-007B426809E6}
	static const GUID CLSID_MonogramHttpSourcePropertyPage = 
	{ 0xae3481e0, 0x1f41, 0x43ae, { 0xa7, 0xfe, 0x0, 0x7b, 0x42, 0x68, 0x9, 0xe6 } };

	// {0B2D06A3-6091-4b9c-B2C4-F0141B918CA7}
	static const GUID CLSID_MonogramHttpSourceProxyPropertyPage = 
	{ 0xb2d06a3, 0x6091, 0x4b9c, { 0xb2, 0xc4, 0xf0, 0x14, 0x1b, 0x91, 0x8c, 0xa7 } };

	// {D98E730B-406F-42f6-A808-50D37B497652}
	static const GUID IID_IMonogramHttpSource = 
	{ 0xd98e730b, 0x406f, 0x42f6, { 0xa8, 0x8, 0x50, 0xd3, 0x7b, 0x49, 0x76, 0x52 } };

	// {3AF5D6A7-4F53-407f-B503-A8105631FD88}
	static const GUID IID_IMonogramHttpSourcePropertyPage = 
	{ 0x3af5d6a7, 0x4f53, 0x407f, { 0xb5, 0x3, 0xa8, 0x10, 0x56, 0x31, 0xfd, 0x88 } };

	// {29BEAFCF-C0B8-40d3-BCFD-4081AAA9F171}
	static const GUID CLSID_MonogramNetworkSource = 
	{ 0x29beafcf, 0xc0b8, 0x40d3, { 0xbc, 0xfd, 0x40, 0x81, 0xaa, 0xa9, 0xf1, 0x71 } };

	// {A2A01BF7-75F3-40a4-98C7-C887FF9D8334}
	static const GUID CLSID_MonogramNetworkSourcePropertyPage = 
	{ 0xa2a01bf7, 0x75f3, 0x40a4, { 0x98, 0xc7, 0xc8, 0x87, 0xff, 0x9d, 0x83, 0x34 } };

	// {89B6A9BE-596F-4e13-82E1-36BCB52170BC}
	static const GUID IID_IMonogramNetworkSource = 
	{ 0x89b6a9be, 0x596f, 0x4e13, { 0x82, 0xe1, 0x36, 0xbc, 0xb5, 0x21, 0x70, 0xbc } };

	// {69CC7614-8847-49d1-A8CA-830785C98D72}
	static const GUID CLSID_MonogramNetworkSink = 
	{ 0x69cc7614, 0x8847, 0x49d1, { 0xa8, 0xca, 0x83, 0x7, 0x85, 0xc9, 0x8d, 0x72 } };

	// {82686CA5-F882-4e7c-85B6-E8EAA7FA39D6}
	static const GUID IID_IMonogramNetworkSink = 
	{ 0x82686ca5, 0xf882, 0x4e7c, { 0x85, 0xb6, 0xe8, 0xea, 0xa7, 0xfa, 0x39, 0xd6 } };

	// {21BD9BDB-6B69-4049-A76F-EC0234C41F42}
	static const GUID CLSID_MonogramNetworkSinkPropertyPage = 
	{ 0x21bd9bdb, 0x6b69, 0x4049, { 0xa7, 0x6f, 0xec, 0x2, 0x34, 0xc4, 0x1f, 0x42 } };

	// {EC8791C5-705F-45a3-B696-86330473E0E6}
	static const GUID CLSID_MonogramRTPSink = 
	{ 0xec8791c5, 0x705f, 0x45a3, { 0xb6, 0x96, 0x86, 0x33, 0x4, 0x73, 0xe0, 0xe6 } };

	// {99277A7D-8B13-4299-B736-E14055831B32}
	static const GUID CLSID_MonogramRTPPropertyPage = 
	{ 0x99277a7d, 0x8b13, 0x4299, { 0xb7, 0x36, 0xe1, 0x40, 0x55, 0x83, 0x1b, 0x32 } };

	// {99277A7D-8B13-4299-B736-E14055831B32}
	static const GUID IID_IMonogramRTPSink = 
	{ 0x99277a7d, 0x8b13, 0x4299, { 0xb7, 0x36, 0xe1, 0x40, 0x55, 0x83, 0x1b, 0x32 } };

	// {24146FB0-F2B2-43f6-9F10-0D795E30D059}
	static const GUID CLSID_MonogramMetaSource = 
	{ 0x24146fb0, 0xf2b2, 0x43f6, { 0x9f, 0x10, 0xd, 0x79, 0x5e, 0x30, 0xd0, 0x59 } };

	// {F18FBC02-7F0D-4800-8803-5E4FC18AEC01}
	static const GUID CLSID_MonogramMetaSourcePage = 
	{ 0xf18fbc02, 0x7f0d, 0x4800, { 0x88, 0x3, 0x5e, 0x4f, 0xc1, 0x8a, 0xec, 0x1 } };

	// {6C6A69A3-FCAC-40b8-B6F4-1ED74619B587}
	static const GUID IID_IMonogramMetaSource = 
	{ 0x6c6a69a3, 0xfcac, 0x40b8, { 0xb6, 0xf4, 0x1e, 0xd7, 0x46, 0x19, 0xb5, 0x87 } };

	// {87493084-DC79-4978-8B0B-C37714FFA909}
	static const GUID CLSID_MonogramVideoProc = 
	{ 0x87493084, 0xdc79, 0x4978, { 0x8b, 0xb, 0xc3, 0x77, 0x14, 0xff, 0xa9, 0x9 } };

	// {C8A8A229-63AB-4665-B237-2A421DE56F76}
	static const GUID CLSID_MonogramVideoPropertyPage = 
	{ 0xc8a8a229, 0x63ab, 0x4665, { 0xb2, 0x37, 0x2a, 0x42, 0x1d, 0xe5, 0x6f, 0x76 } };

	// {13DAE007-20DC-496e-9091-F922216B4D43}
	static const GUID IID_IMonogramVideoProc = 
	{ 0x13dae007, 0x20dc, 0x496e, { 0x90, 0x91, 0xf9, 0x22, 0x21, 0x6b, 0x4d, 0x43 } };

	//-------------------------------------------------------------------------
	//
	//	Enums and Structures
	//
	//-------------------------------------------------------------------------

	enum {
		AAC_VERSION_MPEG4 = 0,
		AAC_VERSION_MPEG2 = 1
	};

	enum {
		AAC_OBJECT_MAIN		= 1,
		AAC_OBJECT_LOW		= 2,
		AAC_OBJECT_SSR		= 3,
		AAC_OBJECT_LTP		= 4
	};

	enum {
		AAC_OUTPUT_RAW		= 0,
		AAC_OUTPUT_ADTS		= 1,
		AAC_OUTPUT_LATM		= 2
	};

	struct AACConfig
	{
		int			version;
		int			object_type;
		int			output_type;
		int			bitrate;
	};

	struct AACInfo
	{
		int			samplerate;
		int			channels;
		int			frame_size;
		__int64		frames_done;
	};

	struct AudioProcConfig
	{
		int			volume_enabled;
		int			mixing_enabled;
		int			resample_enabled;

		float		amplify_db;					// uroven amplifikacie v DeciBeloch
		int			out_mode;					// vystupny mod
		int			out_samplerate;				// vystupna frekvencia
	};

	enum {
		PERIOD_FRAME			= 0,
		PERIOD_MILLISECOND		= 1,
		PERIOD_SECOND			= 2,
		PERIOD_MINUTE			= 3,
		PERIOD_HOUR				= 4
	};

	enum {
		GRAB_FORMAT_BMP			= 0,
		GRAB_FORMAT_JPG			= 1,
		GRAB_FORMAT_PNG			= 2,
		GRAB_FORMAT_YUV_FRAME	= 3,
		GRAB_FORMAT_YUV_SEQ		= 4
	};

	enum {
		GRAB_MODE_SINGLE		= 0,
		GRAB_MODE_ALL			= 1,
		GRAB_MODE_RANGE			= 2,
		GRAB_MODE_PERIODICALLY	= 3,
		GRAB_MODE_NONE			= 4
	};

	struct FrameGrabberConfig
	{
		int64		range_lo;
		int64		range_hi;
		int64		period;
		int			period_mode;
		BSTR		dest_folder;
		BSTR		name_prefix;
		int			use_suffix;
		int			suffix_mode;
		int			quality;
		int			out_format;
		int			grab_mode;
	};

	enum {
		PUMP_OUTPUT_AUTO = 0,
		PUMP_OUTPUT_MONOLIVE = 1,
		PUMP_OUTPUT_MPEG_TS = 2,
		PUMP_OUTPUT_MPEG_PS = 3
	};

	enum {
		HTTP_STATE_DISCONNECTED		= 0,
		HTTP_STATE_CONNECTED		= 1,

		HTTP_STATE_DWORD			= 0xffffffff
	};

	enum {
		MODE_PULL = 0,
		MODE_PUSH = 1,
		MODE_UNK  = 2
	};

	enum {
		PROXY_DIRECT = 0,
		PROXY_AUTO = 1,
		PROXY_MANUAL = 2
	};

	struct HttpSourceInfo
	{
		BSTR			url;
		BSTR			content_type;
		__int64			content_length;
		__int64			range_start;
		__int64			range_size;
		__int64			total_size;
		__int64			received;
		int				mode;
		int				error_code;
		int				state;
	};

	enum {
		NET_SOURCE_MODE_TCP		= 0,
		NET_SOURCE_MODE_UDP		= 1,
		NET_SOURCE_MODE_RTP		= 2
	};

	enum {
		NET_FORMAT_TS			= 0,
		NET_FORMAT_MONO_LIVE	= 1,
		NET_FORMAT_RAW			= 2
	};

	enum {
		NET_MODE_TCP_SERVER		= 0,
		NET_MODE_TCP_CLIENT		= 1,
		NET_MODE_UDP			= 2
	};

	struct NetConfig {
		int		mode;
		int		server_port;
		BSTR	target;			// bud host alebo target
		BSTR	host;
		int		sap_announce;	// ma sa announcovat ?
		BSTR	sap_name;		// meno servicu
	};



	enum {
		AVC_PROFILE_BASELINE		= 0,
		AVC_PROFILE_MAIN			= 1,
		AVC_PROFILE_HIGH			= 2,
		AVC_PROFILE_HIGH_10			= 3,
		AVC_PROFILE_HIGH_422		= 4,
		AVC_PROFILE_HIGH_444		= 5
	};

	enum {
		AVC_LEVEL_1					= 0,
		AVC_LEVEL_1B				= 1,
		AVC_LEVEL_1_1				= 2,
		AVC_LEVEL_1_2				= 3,
		AVC_LEVEL_1_3				= 4,
		AVC_LEVEL_2					= 5,
		AVC_LEVEL_2_1				= 6,
		AVC_LEVEL_2_2				= 7,
		AVC_LEVEL_3					= 8,
		AVC_LEVEL_3_1				= 9,
		AVC_LEVEL_3_2				= 10,
		AVC_LEVEL_4					= 11,
		AVC_LEVEL_4_1				= 12,
		AVC_LEVEL_4_2				= 13,
		AVC_LEVEL_5					= 14,
		AVC_LEVEL_5_1				= 15
	};

	enum {
		AVC_RC_MODE_VBR				= 0,
		AVC_RC_MODE_CBR				= 1,
		AVC_RC_MODE_CQ				= 2,
		AVC_RC_MODE_REMOTE			= 3
	};

	enum {
		AVC_I16x16					= 1,
		AVC_I4x4					= 2,
		AVC_P16x16					= 4,
		AVC_P8x16					= 8,
		AVC_P8x8					= 16,
		AVC_P4x8					= 32,
		AVC_P4x4					= 64
	};

	enum {
		AVC_SUBPEL_DISABLED			= 0,
		AVC_SUBPEL_1_STEP			= 1,
		AVC_SUBPEL_2_STEP			= 2
	};

	enum {
		AVC_DIRECTMV_SPATIAL		= 0,
		AVC_DIRECTMV_TEMPORAL		= 1,
		AVC_DIRECTMV_AUTO			= 2
	};

	enum {
		AVC_SEARCH_DIAMOND			= 0,
		AVC_SEARCH_THREE_STEP		= 1,
		AVC_SEARCH_FULL				= 2
	};

	enum {
		AVC_COMPARE_SAD				= 0,
		AVC_COMPARE_SATD			= 1,
		AVC_COMPARE_SSD				= 2
	};
	struct AVCConfig 
	{
		int			profile;
		int			level;

		// rate control
		int			rc_mode;
		int			min_qp;
		int			max_qp;
		BSTR		remote_host;			// ak je rc_mode AVC_RC_MODE_REMOTE
		int			bitrate;				// ak je VBR / CBR
		int			quality;				// ak je CQ -> toto je QP

		// prediction
		int			mode_flags;				// I16x16, I4x4, P16x16.....
		int			hpel, qpel;				// half a quarter pixel precision
		int			max_search_range;		// maximalna velkost MV vo full pixloch
		int			direct_mv_prediction;
		int			search_method;
		int			compare_method;

		// common
		int			reference_frames;		// pocet referencnych framov
		int			b_frames;				// pocet b framov
		int			max_keyframe_interval;	
		int			cabac;
		int			annexb;
		int			scene_change;
		int			access_unit_delimiters;
		int			mbaff;
		int			two_pass_frame;
		int			try_realtime;
		int			interlaced;
		int			deblocking;
		int			transform_8x8;
		int			decimate;
		int			weighted_prediction;

	};

	struct AVCInfo
	{
		DWORD		input;
		int			width;
		int			height;
		double		qp;
		double		bitrate;
		__int64		frames;
	};

	struct RTP_SINK_CONFIG
	{
		int		cfg_version;				// inkrementovat pri kazdej zmene
		LPWSTR	session_name;				// meno sessionu
		LPWSTR	session_info;				// info k sessionu
		LPWSTR	ip_address;					// vystupna IP adresa
		int		base_port;					// base port
		LPWSTR	source_ip;					// IPcka, z ktorej sa vysiela
		bool	sap;						// vysielat SAP ?
		bool	blocking;					// blokovaci mod
	};

	struct RTP_STREAM
	{
		TCHAR	name[64];
		TCHAR	format[512];
		int		port;
	};

	struct RTP_STREAMS
	{
		int			count;
		RTP_STREAM	streams[64];
	};

	enum META_MODE {
		MM_NONE		 = 0,
		MM_NETWORK	 = 1,
		MM_MANUAL	 = 2,
	};

	struct META_CONFIG 
	{
		LPWSTR		host;
		int			port; 
		int			interval;	// ako casto sa ma posielat "stare" xml v milisekundach INFINITE = raz
		META_MODE	mode;		
	};

	struct VideoProcConfig
	{
		int		deinterlace_method;
		int		zoom;					// 0 - 100
		int		crop_vertical;			// * 8
		int		crop_horizontal;		// * 8
		int		aspect_ratio_mode;		// rezim aspect ratia
		int		out_w, out_h;			// output width & height
		int		in_w, in_h;				// input width & height
		int64	frames_done;			// pocitadlo framov

		GUID	subtype_in;				// typy spojeni
		GUID	subtype_out;
	};

	//-------------------------------------------------------------------------
	//
	//	Interfaces
	//
	//-------------------------------------------------------------------------

	DECLARE_INTERFACE_(IMonogramX264, IUnknown)
	{
		STDMETHOD(SetBitrate)(int bitrate_kbps) PURE;
	};

	DECLARE_INTERFACE_(IMonogramQueue, IUnknown)
	{
		STDMETHOD(SetBufferSize)(int max_samples);
		STDMETHOD(GetBufferSize)(int *max_samples);
		STDMETHOD(GetFullness)(float *fullness_perc);
	};

	DECLARE_INTERFACE_(IMonogramMultigraphSink, IUnknown)
	{
		STDMETHOD(SetName)(LPCWSTR name);
		STDMETHOD(SetBlocking)(BOOL blocking);
	};

	DECLARE_INTERFACE_(IMonogramMultigraphSource, IUnknown)
	{
		STDMETHOD(SetSourceName)(LPCWSTR name);
		STDMETHOD(GetSourceName)(BSTR *name);
		STDMETHOD(SetAudioOffset)(REFERENCE_TIME rtOffset);
		STDMETHOD(GetAudioOffset)(REFERENCE_TIME *rtOffset);
	};

	DECLARE_INTERFACE_(IMonogramAACEncoderPropertyPage, IUnknown)
	{
		STDMETHOD(Update)(int info_only) PURE;
	};

	DECLARE_INTERFACE_(IMonogramAACEncoder, IUnknown)
	{
		STDMETHOD(GetConfig)(AACConfig *config) PURE;
		STDMETHOD(SetConfig)(AACConfig *config) PURE;
		STDMETHOD(GetInfo)(AACInfo *info) PURE;
	};

	DECLARE_INTERFACE_(IMonogramAVCEncoder, IUnknown)
	{
		STDMETHOD(GetSettings)(AVCConfig *config);
		STDMETHOD(SetSettings)(AVCConfig *config);
		STDMETHOD(GetInfo)(AVCInfo *info);
		STDMETHOD(SetProcessor)(int i);
	};
	
	DECLARE_INTERFACE_(IMonogramLiveMux, IUnknown)
	{
		STDMETHOD(SetBlockingMode)(BOOL blocking) PURE;
	};

	DECLARE_INTERFACE_(IMonogramAudioProc, IUnknown)
	{
		STDMETHOD(GetConfig)(AudioProcConfig *config) PURE;
		STDMETHOD(SetVolumeEnabled)(BOOL enabled) PURE;
		STDMETHOD(SetVolumeAmp)(float amp_db) PURE;
		STDMETHOD(SetMixingEnabled)(BOOL enabled) PURE;
		STDMETHOD(SetMixingMode)(int mode) PURE;
		STDMETHOD(SetResampleEnabled)(BOOL enabled) PURE;
		STDMETHOD(SetResampleFreq)(int frequency) PURE;
	};

	DECLARE_INTERFACE_(IMonogramFrameGrabber, IUnknown)
	{
		STDMETHOD(GetConfig)(FrameGrabberConfig *config) PURE;
		STDMETHOD(SetRange)(int64 range_lo, int64 range_hi) PURE;
		STDMETHOD(SetPeriod)(int64 period, int mode) PURE;
		STDMETHOD(SetSuffix)(int use_suffix, int mode) PURE;
		STDMETHOD(SetFormat)(int format, int quality) PURE;
		STDMETHOD(SetGrabMode)(int mode) PURE;
		STDMETHOD(SetNames)(LPWSTR folder, LPWSTR prefix) PURE;
	};

	DECLARE_INTERFACE_(IMonogramPump, IUnknown)
	{
		STDMETHOD(GetOutputFormat)(int *format);
		STDMETHOD(SetOutputFormat)(int format);
	};

	DECLARE_INTERFACE_(IMonogramHttpSourcePropertyPage, IUnknown)
	{
		STDMETHOD(Update)() PURE;
	};

	DECLARE_INTERFACE_(IMonogramHttpSource, IUnknown)
	{
		STDMETHOD(GetURL)(BSTR *url) PURE;
		STDMETHOD(SetURL)(LPWSTR url) PURE;
		STDMETHOD(Disconnect)() PURE;
		STDMETHOD(GetHTTPState)(HttpSourceInfo *info) PURE;
		STDMETHOD(SetPropertyPage)(IUnknown *pUnk) PURE;
		STDMETHOD(SetProxyMode)(UINT type, LPCWSTR host, UINT port) PURE;
	};

	DECLARE_INTERFACE_(IMonogramNetworkSource, IUnknown) 
	{
		STDMETHOD(GetConfig)(BSTR *host, int *port, int *mode) PURE;
		STDMETHOD(SetConfig)(LPWSTR host, int port, int mode) PURE;
		STDMETHOD(GetFormat)(int *format) PURE;
		STDMETHOD(SetFormat)(int format) PURE;
	};

	DECLARE_INTERFACE_(IMonogramNetworkSink, IUnknown) 
	{
		STDMETHOD(GetConfig)(NetConfig *config) PURE;
		STDMETHOD(SetConfig)(NetConfig *config) PURE;
		STDMETHOD(GetBlocking)(BOOL *blocking) PURE;
		STDMETHOD(SetBlocking)(BOOL blocking) PURE;
	};

	DECLARE_INTERFACE_(IMonogramRTPSink, IUnknown)
	{
		STDMETHOD(SetConfig)(RTP_SINK_CONFIG *config);
		STDMETHOD(GetConfig)(RTP_SINK_CONFIG *config);
		STDMETHOD(GetSDP)(BSTR *sdp_text);
		STDMETHOD(GetStreams)(RTP_STREAMS *streams);
	};

	DECLARE_INTERFACE_(IMonogramMetaSource, IUnknown)
	{
		STDMETHOD(SetConfig)(META_CONFIG *cfg);
		STDMETHOD(GetConfig)(META_CONFIG *cfg);
		STDMETHOD(GetXML)(void **xml_root, int *version);			// XMLNode z MonoBase
		STDMETHOD(SetXML)(LPCTSTR lpctstrXML, int length);
	};

	DECLARE_INTERFACE_(IMonogramVideoProc, IUnknown)
	{
		STDMETHOD(GetConfig)(VideoProcConfig *conf);
		STDMETHOD(SetDeinterlace)(int mode);
		STDMETHOD(SetOutputSize)(int width, int height);
		STDMETHOD(SetCrop)(int crop_v, int crop_h);
		STDMETHOD(SetAspectRatioMode)(int mode);
		STDMETHOD(SetZoom)(int percent);
	};

};

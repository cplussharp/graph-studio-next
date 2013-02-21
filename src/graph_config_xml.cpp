//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <math.h>
#include <atlbase.h>
#include <atlpath.h>
#include <atlenc.h>

#pragma warning(disable: 4244)			// DWORD -> BYTE warning

namespace GraphStudio
{

#define PRESET_START()		if (0) { 
#define PRESET(x)			} else if (conf->name == _T(x)) {
#define PRESET_END()		}

	BOOL GetLocalAddresses(CArray<CString> &adrlist)
	{
		char	hostname[1024];
		if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) return FALSE;

		adrlist.RemoveAll();

		struct hostent *he = gethostbyname(hostname);
		if (!he) return FALSE;

		for (int i=0; he->h_addr_list[i] != NULL; i++) {
			struct in_addr	addr;
			memcpy(&addr, he->h_addr_list[i], sizeof(addr));
			CString			addr_str;
			addr_str = inet_ntoa(addr);
			adrlist.Add(addr_str);
		}

		return TRUE;
	}


	//-------------------------------------------------------------------------
	//
	//	DisplayGraph class
	//
	//-------------------------------------------------------------------------

	HRESULT DisplayGraph::LoadXML_ConfigInterface(XML::XMLNode *conf, IBaseFilter *filter)
	{
		HRESULT hr = S_OK;
		
		PRESET_START()		

		PRESET("ifilesourcefilter")
			//	<ifilesourcefilter source="d:\sga.avi"/>

			CComQIPtr<IFileSourceFilter> fsource(filter);
			if (fsource) {

				CString filename = conf->GetValue(_T("source"));

				hr = fsource->Load((LPCOLESTR)filename.GetBuffer(), NULL);
				if (SUCCEEDED(hr))
					return hr;

				while (FAILED(hr)) {
					CFileSrcForm form(_T("Missing source file"));
					form.result_file = filename;
					hr = form.ChooseSourceFile(fsource);
				}
			}

		PRESET("ifilesinkfilter")
			//	<ifilesinkfilter dest="d:\sga.avi"/>

			CComQIPtr<IFileSinkFilter> fsink(filter);
			if (fsink) {

				CString filename = conf->GetValue(_T("dest"));

				hr = fsink->SetFileName((LPCOLESTR)filename.GetBuffer(), NULL);
				if (SUCCEEDED(hr))
					return hr;

				CFileSinkForm form(_T("Missing destination file"));
				while (FAILED(hr)) {
					form.result_file = filename;
					hr = form.ChooseSinkFile(fsink);
				}
			}

		PRESET("ipersiststream")
			// <ipersiststream encoding="base64" data="MAAwADAAMAAwADAAMAAwADAAMAAwACAA="/>

			CComQIPtr<IPersistStream> persist_stream(filter);
			
			const CString base64_str = conf->GetValue(_T("data"));
			ASSERT(base64_str.GetLength() > 0);
			ASSERT(conf->GetValue(_T("encoding")) == _T("base64"));

			if (persist_stream && base64_str.GetLength() > 0) {

				const int stream_size = Base64DecodeGetRequiredLength(base64_str.GetLength());
				HGLOBAL stream_hglobal = GlobalAlloc(GHND, stream_size);
				BYTE * stream_data = (BYTE *)GlobalLock(stream_hglobal);

				const CStringA base64_mcbs(base64_str);
				int converted_length = stream_size;

				if (stream_hglobal 
						&& stream_data 
						&& base64_mcbs.GetLength() > 0
						&& Base64Decode(base64_mcbs, base64_mcbs.GetLength(), stream_data, &converted_length)
						&& converted_length > 0) {

					GlobalUnlock(stream_hglobal);
					stream_data = NULL;

					const HGLOBAL temp_hglobal = GlobalReAlloc(stream_hglobal, converted_length, GMEM_ZEROINIT);
					ASSERT(temp_hglobal);
					if (temp_hglobal) {
						stream_hglobal = temp_hglobal;
						ASSERT(GlobalSize(stream_hglobal) == converted_length);

						CComPtr<IStream> stream;
						CreateStreamOnHGlobal(stream_hglobal, FALSE, &stream);
						ASSERT(stream);

						if (stream)
							hr = persist_stream->Load(stream);
					}
				} else {
					ASSERT(!"Failed to decode base64");	// something went wrong in string wrangling
				}
				if (stream_data)
					GlobalUnlock(stream_hglobal);
				if (stream_hglobal)
					GlobalFree(stream_hglobal);
			}

		PRESET("imonogramgraphsink")
			// <imonogramgraphsink name="video" blocking="1"/>

			CComPtr<Monogram::IMonogramMultigraphSink>	sink;
			hr = filter->QueryInterface(Monogram::IID_IMonogramMultigraphSink, (void**)&sink);
			if (SUCCEEDED(hr)) {
				CString	sinkname = conf->GetValue(_T("name"));
				hr = sink->SetName(sinkname.GetBuffer());

				BOOL	blocking = (conf->GetValue(_T("blocking"), 1) == 1 ? TRUE : FALSE);
				sink->SetBlocking(blocking);

				if (FAILED(hr)) 
					return hr;
			}

		PRESET("imonogramgraphsource")
			//	<imonogramgraphsource name="video"/>

			CComPtr<Monogram::IMonogramMultigraphSource>	src;
			hr = filter->QueryInterface(Monogram::IID_IMonogramMultigraphSource, (void**)&src);
			if (SUCCEEDED(hr)) {
				CString	srcname = conf->GetValue(_T("name"));
				hr = src->SetSourceName(srcname.GetBuffer());
				if (FAILED(hr)) 
					return hr;
			}

		PRESET("imonogramqueue")
			//	<imonogramqueue samples="30"/>

			CComPtr<Monogram::IMonogramQueue>			queue;
			hr = filter->QueryInterface(Monogram::IID_IMonogramQueue, (void**)&queue);
			if (SUCCEEDED(hr)) {
				int samples = conf->GetValue(_T("samples"), 10);
				hr = queue->SetBufferSize(samples);
				if (FAILED(hr)) 
					return hr;
			}

		PRESET("imonogramaudioproc")
			//	<imonogramaudioproc samplerate="44100" channels="2"/>

			CComPtr<Monogram::IMonogramAudioProc>	proc;
			hr = filter->QueryInterface(Monogram::IID_IMonogramAudioProc, (void**)&proc);
			if (SUCCEEDED(hr)) {
				int samplerate = conf->GetValue(_T("samplerate"), 44100);
				int channels   = conf->GetValue(_T("channels"), 1);

				proc->SetResampleEnabled(TRUE);
				proc->SetResampleFreq(samplerate);

				proc->SetMixingEnabled(TRUE);
				int mode;
				switch (channels) {
				case 0:		mode = 1; break;
				case 1:		mode = 1; break;
				case 2:		mode = 2; break;
				case 3:		mode = 3; break;
				case 4:		mode = 4; break;
				case 5:		mode = 4; break;
				case 6:		mode = 5; break;
				default:	mode = 2; break;
				}
				proc->SetMixingMode(mode);
			}

		PRESET("imonogramvideoproc")
			// <imonogramvideoproc deinterlace="vfilter" vcrop="1" hcrop="1"
			//                     aspect="free" width="640" height="480"/>
			CComPtr<Monogram::IMonogramVideoProc>	proc;
			hr = filter->QueryInterface(Monogram::IID_IMonogramVideoProc, (void**)&proc);
			if (SUCCEEDED(hr)) {
				CString		method = conf->GetValue(_T("deinterlace"));
				int			deint_method = 0;
				if (method == _T("vfilter"))	deint_method = 9; else
				if (method == _T("linear"))		deint_method = 5; else
				if (method == _T("weave"))		deint_method = 1; else
				if (method == _T("none"))		deint_method = 0; else
				if (method == _T("bob"))		deint_method = 3;

				proc->SetDeinterlace(deint_method);

				int		crop_v	= conf->GetValue(_T("vcrop"), 0);
				int		crop_h	= conf->GetValue(_T("hcrop"), 0);
				proc->SetCrop(crop_v, crop_h);

				CString		aspect = conf->GetValue(_T("aspect"));
				int			aspect_method = 0;
				if (aspect == _T("free"))	aspect_method = 0; else
				if (aspect == _T("4:3"))	aspect_method = 1; else
				if (aspect == _T("16:9"))	aspect_method = 2; else
				if (aspect == _T("16:10"))	aspect_method = 3;
				proc->SetAspectRatioMode(aspect_method);

				int		outw	= conf->GetValue(_T("width"), -1);
				int		outh	= conf->GetValue(_T("height"), -1);
				proc->SetOutputSize(outw, outh);

				int		zoom	= conf->GetValue(_T("zoom"), 0);
				proc->SetZoom(zoom);
			}

		PRESET("imonogramaacencoder")
			// <imonogramaacencoder version="mpeg4" object_type="lc" 
			//                      output_type="raw" bitrate="128000"/>
			int	version = (conf->GetValue(_T("version")) == _T("mpeg2") ? 1 : 0);
			int	object_type = 2;		// low
			int output_type = 0;		// raw
			int bitrate = 128000;

			CString	ot = conf->GetValue(_T("object_type"));
			if (ot == _T("main")) object_type = 1; else
			if (ot == _T("lc")) object_type = 2; else
			if (ot == _T("ssr")) object_type = 3; else
			if (ot == _T("ltp")) object_type = 4;

			ot = conf->GetValue(_T("output_type"));
			if (ot == _T("raw")) output_type = 0; else
			if (ot == _T("adts")) output_type = 1; else
			if (ot == _T("latm")) output_type = 2;

			bitrate = conf->GetValue(_T("bitrate"), 128000);

			CComPtr<Monogram::IMonogramAACEncoder>	enc;
			hr = filter->QueryInterface(Monogram::IID_IMonogramAACEncoder, (void**)&enc);
			if (SUCCEEDED(hr)) {
				Monogram::AACConfig		config;
				config.version     = version;
				config.object_type = object_type;
				config.output_type = output_type;
				config.bitrate     = bitrate;
				enc->SetConfig(&config);	
			}

		PRESET("imonogramx264")
			// <imonogramx264
			//	  bitrate="250000"
			// />

			int	br = conf->GetValue(_T("bitrate"), 800000);
			CComPtr<Monogram::IMonogramX264>	enc;
			hr = filter->QueryInterface(Monogram::IID_IMonogramX264, (void**)&enc);
			if (SUCCEEDED(hr)) {
				enc->SetBitrate(br / 1000);
			}

		PRESET("imonogramavcencoder")
			// <imonogramavcencoder
			// 	  profile="baseline" level="51"
			// 	  rcmode="vbr" minqp="16" maxqp="32" bitrate="2500000"
			// 	  bframes="0" max_i_interval="125" deblock="1"
			// 	  pred="I16x16, I4x4, P16x16, P16x8, P8x8" subpel="2h, 2q"
			// />
			int		profile = 0;			// baseline
			int		level = 15;				// AVC_LEVEL_5_1
			int		minqp = 16;
			int		maxqp = 32;
			int		rcmode = 0;				// AVC_RC_MODE_VBR
			int		qp    = (minqp + maxqp) / 2;
			int		bitrate = 1000000;
			int		bframes = 0;
			int		ref_frames = 1;
			int		max_i_interval = 125;
			int		cabac = 0;
			int		deblock = 1;
		
			CString	pr = conf->GetValue(_T("profile"));
			if (pr == _T("baseline")) profile = Monogram::AVC_PROFILE_BASELINE; else
			if (pr == _T("main")) profile = Monogram::AVC_PROFILE_MAIN; else
			if (pr == _T("high")) profile = Monogram::AVC_PROFILE_HIGH;

			CString	lv = conf->GetValue(_T("level"));
			if (lv == _T("10")) level = Monogram::AVC_LEVEL_1; else
			if (lv == _T("1b")) level = Monogram::AVC_LEVEL_1B; else
			if (lv == _T("11")) level = Monogram::AVC_LEVEL_1_1; else
			if (lv == _T("12")) level = Monogram::AVC_LEVEL_1_2; else
			if (lv == _T("13")) level = Monogram::AVC_LEVEL_1_3; else
			if (lv == _T("20")) level = Monogram::AVC_LEVEL_2; else
			if (lv == _T("21")) level = Monogram::AVC_LEVEL_2_1; else
			if (lv == _T("22")) level = Monogram::AVC_LEVEL_2_2; else
			if (lv == _T("30")) level = Monogram::AVC_LEVEL_3; else
			if (lv == _T("31")) level = Monogram::AVC_LEVEL_3_1; else
			if (lv == _T("32")) level = Monogram::AVC_LEVEL_3_2; else
			if (lv == _T("40")) level = Monogram::AVC_LEVEL_4; else
			if (lv == _T("41")) level = Monogram::AVC_LEVEL_4_1; else
			if (lv == _T("42")) level = Monogram::AVC_LEVEL_4_2; else
			if (lv == _T("50")) level = Monogram::AVC_LEVEL_5; else
			if (lv == _T("51")) level = Monogram::AVC_LEVEL_5_1;

			minqp = conf->GetValue(_T("minqp"), 16);		
			maxqp = conf->GetValue(_T("maxqp"), 32);
			qp    = (minqp + maxqp) / 2;
			
			CString	rcm = conf->GetValue(_T("rcmode"));
			if (rcm == _T("vbr")) rcmode = Monogram::AVC_RC_MODE_VBR; else
			if (rcm == _T("cbr")) rcmode = Monogram::AVC_RC_MODE_CBR; else
			if (rcm == _T("cq")) rcmode = Monogram::AVC_RC_MODE_CQ;

			// quality ?
			if (rcmode == 2) {
				qp = conf->GetValue(_T("qp"), qp);
			} else 
			if (rcmode == 1 || rcmode == 0) {
				bitrate = conf->GetValue(_T("bitrate"), bitrate);
			}

			// bframes
			bframes			= conf->GetValue(_T("bframes"), bframes);
			max_i_interval	= conf->GetValue(_T("max_i_interval"), max_i_interval);
			ref_frames		= conf->GetValue(_T("ref_frames"), ref_frames);
			cabac			= conf->GetValue(_T("cabac"), cabac);
			deblock			= conf->GetValue(_T("deblock"), deblock);

			CComPtr<Monogram::IMonogramAVCEncoder>	enc;
			hr = filter->QueryInterface(Monogram::IID_IMonogramAVCEncoder, (void**)&enc);
			if (SUCCEEDED(hr)) {
				Monogram::AVCConfig		config;

				config.profile = profile;
				config.level = level;
				config.rc_mode = rcmode;
				config.min_qp = minqp;
				config.max_qp = maxqp;
				config.bitrate = bitrate;
				config.quality = qp;
				config.remote_host = NULL;

				// prediction
				config.mode_flags = Monogram::AVC_I16x16 | Monogram::AVC_I4x4 | 
									Monogram::AVC_P16x16 | Monogram::AVC_P8x16 | Monogram::AVC_P8x8;
				config.hpel = Monogram::AVC_SUBPEL_2_STEP;
				config.qpel = Monogram::AVC_SUBPEL_2_STEP;
				config.max_search_range = 64;
				config.direct_mv_prediction = Monogram::AVC_DIRECTMV_SPATIAL;
				config.search_method = Monogram::AVC_SEARCH_DIAMOND;
				config.compare_method = Monogram::AVC_COMPARE_SAD;

				// common
				config.reference_frames = ref_frames;
				config.b_frames = bframes;
				config.max_keyframe_interval = max_i_interval;
				config.cabac = cabac;
				config.annexb = 0;
				config.scene_change = 1;
				config.access_unit_delimiters = 0;
				config.mbaff = 0;
				config.two_pass_frame = 1;
				config.try_realtime = 1;
				config.interlaced = 0;
				config.deblocking = deblock;
				config.transform_8x8 = 0;
				config.decimate = 1;
				config.weighted_prediction = 0;

				enc->SetSettings(&config);	
			}


		PRESET("imonogramrtpsink")
			// <imonogramrtpsink
			//   	session="live-sga" info="Doplnujuce Informacie"
			//  	ip_address="224.0.0.1" baseport="6700" source_ip=""
			//  	sap="1" blocking="1"
			// />

			CComPtr<Monogram::IMonogramRTPSink>		sink;
			hr = filter->QueryInterface(Monogram::IID_IMonogramRTPSink, (void**)&sink);
			if (SUCCEEDED(hr)) {
				Monogram::RTP_SINK_CONFIG		config;
				CString				dest_ip = conf->GetValue(_T("ip_address"));
				CString				src_ip  = conf->GetValue(_T("source_ip"));
				CString				session = conf->GetValue(_T("session"));
				CString				info    = conf->GetValue(_T("info"));

				if (dest_ip == _T("")) dest_ip = _T("224.0.0.1");
				if (session == _T("")) session = _T("Session");

				config.base_port		= conf->GetValue(_T("baseport"), 6000);
				config.ip_address		= dest_ip.GetBuffer();

				CArray<CString>		adrlist;
				GetLocalAddresses(adrlist);
				if (adrlist.GetCount() == 0) adrlist.Add(_T("127.0.0.1"));
				if (src_ip == _T("")) src_ip = adrlist[0];

				config.source_ip		= src_ip.GetBuffer();
				config.session_name		= session.GetBuffer();
				config.session_info		= info.GetBuffer();
				config.sap				= (conf->GetValue(_T("sap"), 1) == 1 ? true : false);
				config.blocking			= (conf->GetValue(_T("blocking"), 1) == 1 ? true : false);
		
				sink->SetConfig(&config);
			}

		PRESET("imonogrammetasource")
			// <imonogrammetasource mode="network" host="224.5.6.7" port="5000" interval="100" />

			CComPtr<Monogram::IMonogramMetaSource>	metasrc;
			hr = filter->QueryInterface(Monogram::IID_IMonogramMetaSource, (void**)&metasrc);
			if (SUCCEEDED(hr)) {

				Monogram::META_CONFIG		config;

				CString	host = conf->GetValue(_T("host"));
				CString mode = conf->GetValue(_T("mode")); mode.MakeLower();
				CString sinterval = conf->GetValue(_T("interval")); sinterval.MakeLower();
				int interval = INFINITE;
				if (sinterval != _T("infinite")) {
					interval = conf->GetValue(_T("interval"), INFINITE);
				}

				config.host = host.GetBuffer();
				config.port = conf->GetValue(_T("port"), 0);
				config.interval =  interval;
				config.mode = Monogram::MM_NONE;
				if (mode == _T("network")) config.mode = Monogram::MM_NETWORK;
				if (mode == _T("manual")) config.mode = Monogram::MM_MANUAL;


				metasrc->SetConfig(&config);
			}

		PRESET("imonogramhttpsource")
			// <imonogramhttpsource url="http://n04.joj.sk/stream/id4.live"/>

			CComPtr<Monogram::IMonogramHttpSource>	httpsrc;
			hr = filter->QueryInterface(Monogram::IID_IMonogramHttpSource, (void**)&httpsrc);
			if (SUCCEEDED(hr)) {

				CString		url = conf->GetValue(_T("url"));
				hr = httpsrc->SetURL(url.GetBuffer());
			}

		PRESET("iambuffernegotiation")
			// <iambuffernegotiation pin="Capture" latency="40"/>
			hr = LoadXML_IAMBufferNegotiation(conf, filter);


		PRESET_END()


		return hr;
	}








};


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

#include "MediaTypeSelectForm.h"
#include "GRF_File.h"

#include <atlenc.h>
#include <set>

#pragma warning(disable: 4244)			// DWORD -> BYTE warning


GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation


	int DisplayGraph::g_filterXGap = 40;
	int DisplayGraph::g_filterYGap = 32;


    static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* /*lpelfe*/, NEWTEXTMETRICEX* /*lpntme*/, int /*FontType*/, LPARAM lParam)
    {
        LPARAM* l = (LPARAM*)lParam;
        *l = TRUE;
        return TRUE;
    }

    bool HasFont(CString fontName)
    {
        // Get the screen DC
        CDC dc;
        if (!dc.CreateCompatibleDC(NULL))
        {
    	    return false;
        }
        LOGFONT lf = { 0 };
        // Any character set will do
        lf.lfCharSet = DEFAULT_CHARSET;
        // Set the facename to check for
        _tcscpy(lf.lfFaceName, fontName);
        LPARAM lParam = 0;
        // Enumerate fonts
        ::EnumFontFamiliesEx(dc.GetSafeHdc(), &lf,  (FONTENUMPROC)EnumFontFamExProc, (LPARAM)&lParam, 0);
        return lParam ? true : false;
    }

	void MakeFont(CFont &f, CString name, int size, bool bold, bool italic)
	{
		HDC dc = CreateCompatibleDC(NULL);
		int nHeight    = -MulDiv(size, (int)(GetDeviceCaps(dc, LOGPIXELSY)), 72 );
		DeleteDC(dc);

		DWORD dwBold   = (bold ? FW_BOLD : 0);
		DWORD dwItalic = (italic ? TRUE : FALSE);

		f.CreateFont(nHeight, 0, 0, 0, dwBold, dwItalic, FALSE, FALSE, DEFAULT_CHARSET,
					  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, VARIABLE_PITCH, name);
	}

	// attempt to determine whether a filter is a source or renderer
	// based on methods used in dshowutil.h
	static Filter::FilterPurpose GetFilterPurpose(const Filter * filter)
	{
		if (!filter)
			return Filter::FILTER_OTHER;

		CComQIPtr<IAMFilterMiscFlags> flags(filter->filter);

		if (flags) {
			const ULONG filter_flags = flags->GetMiscFlags();

			if (filter_flags & AM_FILTER_MISC_FLAGS_IS_RENDERER)
				return Filter::FILTER_RENDERER;
			else if (filter_flags & AM_FILTER_MISC_FLAGS_IS_SOURCE)
				return Filter::FILTER_SOURCE;
			else
				return Filter::FILTER_OTHER;
		}

        // Look for the following conditions:

        // 1) Zero output pins AND at least 1 unmapped input pin
        // - or -
        // 2) At least 1 rendered input pin.

        // definitions:
        // unmapped input pin = IPin::QueryInternalConnections returns E_NOTIMPL
        // rendered input pin = IPin::QueryInternalConnections returns "0" slots

        // These cases are somewhat obscure and probably don't apply to many filters
        // that actually exist.

		for (int i=0; i<filter->input_pins.GetCount(); i++) {
			const Pin * const input_pin = filter->input_pins[i];

			// It's an input pin. Is it mapped to an output pin?
			ULONG nPin = 0;
			CONST HRESULT hr = input_pin->pin->QueryInternalConnections(NULL, &nPin);
			if (hr == S_OK) {
				// The count (nPin) was zero, and the method returned S_OK, so
				// this input pin is mapped to exactly zero ouput pins. 
				// Therefore, it is a rendered input pin.
				return Filter::FILTER_RENDERER;

			// The heuristics below are unreliable, probably because it matches the default
			// QueryInternalConnections implementation in the baseclasses 
			//} else if (hr == E_NOTIMPL && filter->output_pins.GetCount() == 0) {
			// This pin is not mapped to any particular output pin. 
			//	// and there are no output pins
			//	CComPtr<IUnknown> unk;
			//	if (S_OK == filter->filter->QueryInterface(__uuidof(IBasicAudio), (void**)&unk)
			//			|| filter->filter->QueryInterface(__uuidof(IBasicVideo), (void**)&unk))
			//		return Filter::FILTER_RENDERER;
			}
		}

		CComPtr<IUnknown> unk;

		// Last resort - some heuristics on which interfaces the filter supports, could be improved

		if (		S_OK == filter->filter->QueryInterface(__uuidof(IBasicAudio), (void**)&unk)
				|| S_OK == filter->filter->QueryInterface(__uuidof(IBasicVideo), (void**)&unk)
				|| S_OK == filter->filter->QueryInterface(__uuidof(IFileSinkFilter), (void**)&unk))
			return Filter::FILTER_RENDERER;
		else if (S_OK == filter->filter->QueryInterface(__uuidof(IFileSourceFilter), (void**)&unk))
			return Filter::FILTER_SOURCE;
		else
			return Filter::FILTER_OTHER;
	}


	//-------------------------------------------------------------------------
	//
	//	DisplayGraph class
	//
	//-------------------------------------------------------------------------

	DisplayGraph::DisplayGraph()
		: callback(NULL)
        , params(NULL)
		, dc(NULL)
		, is_remote(false)
		, is_frame_stepping(false)
		, uses_clock(true)
        , rotRegister(0)
		, dirty(true)
		, m_filter_graph_clsid(&CLSID_FilterGraph)
		, m_log_file(INVALID_HANDLE_VALUE)
	{
		HRESULT			hr = NOERROR;
		graph_callback = new GraphCallbackImpl(NULL, &hr, this);
		graph_callback->NonDelegatingAddRef();
		MakeNew();
	}

	DisplayGraph::~DisplayGraph()
	{
		CloseLogFile();
		if (graph_callback) {
			graph_callback->NonDelegatingRelease();
			graph_callback = NULL;
		}
		DeleteAllFilters();
	}

	// caller must clean up graph if error returned
	HRESULT DisplayGraph::ConnectToRemote(IFilterGraph *remote_graph)
	{
		int ret = MakeNew();
		if (ret < 0) 
			return E_FAIL;

		// release graph objects
        RemoveFromRot();
		mc = NULL;
		ms = NULL;
		fs = NULL;
		if (me) {
			// clear events...
			me->SetNotifyWindow(NULL, 0, NULL);
			me = NULL;
		}
		if (!is_remote && gb)
			gb->SetLogFile(NULL);
		gb = NULL;
		cgb = NULL;
		is_remote = false;
		is_frame_stepping = false;
		uses_clock = true;

		// attach remote graph
		HRESULT hr = remote_graph->QueryInterface(IID_IGraphBuilder, (void**)&gb);
		if (FAILED(hr)) 
			return hr;

		// get hold of interfaces
		hr = gb->QueryInterface(IID_IMediaControl, (void**)&mc);
		if (FAILED(hr)) 
			return hr;
		hr = gb->QueryInterface(IID_IMediaSeeking, (void**)&ms);
		if (FAILED(hr)) 
			return hr;
		hr = gb->QueryInterface(IID_IVideoFrameStep, (void**)&fs);
		if (FAILED(hr)) 
			return hr;

		// now we're a remote graph
		is_remote = true;
        if(params) 
			params->is_remote = true;
		return hr;
	}

	int DisplayGraph::AttachCaptureGraphBuilder()
	{
		if (cgb) return 1;

		HRESULT		hr = cgb.CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER);
		if (FAILED(hr)) return -1;

		cgb->SetFiltergraph(gb);
		return 0;
	}

	int DisplayGraph::MakeNew()
	{
		if (ms) ms = NULL;
		if (fs) fs = NULL;

        // release from ROT
        RemoveFromRot();

		// we only do this for our own graph so we don't mess up
		// the host application when connected to remote graph
		if (!is_remote) {
			if (mc) {
				mc->Stop();
				mc = NULL;
			}

			if (me) {
				// clear events...
				me->SetNotifyWindow(NULL, 0, NULL);
				me = NULL;
			}

			SelectAllFilters(true);
			RemoveSelectionFromGraph();
			DeleteAllFilters();
			columns.RemoveAll();
			if (gb)
				gb->SetLogFile(NULL);
			gb = NULL;
			cgb = NULL;
		} else {
			mc = NULL;
			me = NULL;
			DeleteAllFilters();
			columns.RemoveAll();
			gb = NULL;
			cgb = NULL;
		}

		is_remote = false;
        if(params) params->is_remote = false;
		is_frame_stepping = false;
		uses_clock = true;

		// create new instance of filter graph
		HRESULT hr;
		do {
			hr = gb.CoCreateInstance(*m_filter_graph_clsid, NULL, CLSCTX_INPROC_SERVER);
			if (FAILED(hr)) 
				break;

			// If log file already open use it with the new graph showing any errors
			if (m_log_file != INVALID_HANDLE_VALUE)
				DSUtil::ShowError(gb->SetLogFile((DWORD_PTR)m_log_file), _T("Set Log File"));

            AddToRot();

			gb->SetDefaultSyncSource();

			gb->QueryInterface(IID_IMediaControl, (void**)&mc);

			// setup event handler
			gb->QueryInterface(IID_IMediaEventEx, (void**)&me);
			me->SetNotifyWindow((OAHWND)wndEvents, WM_GRAPH_EVENT, (LONG_PTR)this);
			gb->QueryInterface(IID_IMediaSeeking, (void**)&ms);
			gb->QueryInterface(IID_IVideoFrameStep, (void**)&fs);

			AttachCaptureGraphBuilder();

			// attach graph callback
			CComPtr<IObjectWithSite>		obj_with_site;
			hr = gb->QueryInterface(IID_IObjectWithSite, (void**)&obj_with_site);
			if (SUCCEEDED(hr)) {
				CComPtr<IUnknown>	unk;
				graph_callback->NonDelegatingQueryInterface(IID_IUnknown, (void**)&unk);

				obj_with_site->SetSite(unk);

				unk = NULL;
				obj_with_site = NULL;
			}

		} while (0);

		if (FAILED(hr)) {
            RemoveFromRot();
			cgb = NULL;
			gb = NULL;
			mc = NULL;
			me = NULL;
			ms = NULL;
			fs = NULL;
			return -1;
		}

		return 0;
	}

	HRESULT DisplayGraph::OpenLogFile(LPCTSTR file_name)
	{
		if (!gb)
			return E_POINTER;

		if (is_remote)
			return E_FAIL;

		if (IsLogFileOpen())
			return E_HANDLE;

		gb->SetLogFile(NULL);

		m_log_file = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		const DWORD error = GetLastError();
		if(error != 0 && error != ERROR_ALREADY_EXISTS)
			return HRESULT_FROM_WIN32(error);

		return gb->SetLogFile((DWORD_PTR)m_log_file);
	}

	HRESULT DisplayGraph::CloseLogFile()
	{
		HRESULT hr = S_FALSE;

		if (gb && !is_remote)
			hr = gb->SetLogFile(NULL);

		if (m_log_file != INVALID_HANDLE_VALUE) {
			CloseHandle(m_log_file);
			m_log_file = INVALID_HANDLE_VALUE;
		}

		return hr;
	}

	bool DisplayGraph::IsOwnRotGraph(const CString& moniker_name)
	{
		CString strPid;
		// MAINTENANCE WARNING - this format string needs to be a substring of the format string in DisplayGraph::AddToRot
		strPid.Format(_T("pid %08x"), GetCurrentProcessId());
		return ( moniker_name.Find(strPid) != -1 );
	}

    void DisplayGraph::AddToRot()
    {
		// MAINTENANCE WARNING - this format string needs to include the format string in DisplayGraph::IsOwnGraph
		#ifdef _WIN64
			const WCHAR * const moniker_format_string = L"FilterGraph %08I64x pid %08x GraphStudioNextx64";
		#else
			const WCHAR * const moniker_format_string = L"FilterGraph %08x pid %08x GraphStudioNext";
		#endif

        IMoniker * pMoniker = NULL;
        IRunningObjectTable *pROT = NULL;

        if (FAILED(GetRunningObjectTable(0, &pROT))) 
            return;
    
        const size_t STRING_LENGTH = 256;

        WCHAR wsz[STRING_LENGTH];

        StringCchPrintfW(
            wsz, STRING_LENGTH, 
            moniker_format_string,
			(void*)(gb.p),
            GetCurrentProcessId()
            );
    
        HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
        if (SUCCEEDED(hr)) 
        {
            hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, gb, pMoniker, &rotRegister);
            pMoniker->Release();
        }
        pROT->Release();
    }

    void DisplayGraph::RemoveFromRot()
    {
        if (rotRegister != 0)
        {
            IRunningObjectTable *pROT;
            if (SUCCEEDED(GetRunningObjectTable(0, &pROT)))
            {
                pROT->Revoke(rotRegister);
                pROT->Release();
            }

            rotRegister = 0;
        }
    }

	void DisplayGraph::SelectAllFilters(bool select)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			filters[i]->Select(select);
		}
	}

	CSize DisplayGraph::GetGraphSize()
	{
		// find out the rectangle
		int maxx = 0;
		int maxy = 0;

		for (int i=0; i<filters.GetCount(); i++) {
			Filter	*filter = filters[i];
			if (filter->posx + filter->width > maxx) maxx = filter->posx+filter->width;
			if (filter->posy + filter->height > maxy) maxy = filter->posy+filter->height;
		}

		maxx = NextGridPos(maxx) + GRID_SIZE;
		maxy = NextGridPos(maxy) + GRID_SIZE;

		return CSize(maxx, maxy);
	}

	int DisplayGraph::GetState(FILTER_STATE &state, DWORD timeout)
	{
		if (!mc) {
			state = State_Stopped;
			return -1;
		}

		// pretend we're in paused state
		if (is_frame_stepping) {
			state = State_Paused;
			return NOERROR;
		}

		HRESULT hr = mc->GetState(timeout, (OAFilterState*)&state);
		if (FAILED(hr)) return hr;

		return hr;
	}

	void DisplayGraph::DoFrameStep()
	{
		if (fs) {
			fs->Step(1, NULL);
			is_frame_stepping = true;
		}
	}

	HRESULT DisplayGraph::DoPlay()
	{
		HRESULT hr = S_OK;

		// notify all EVR windows
		if (!is_remote) {
			for (int i=0; i<filters.GetCount(); i++) {
				Filter	*filter = filters[i];
				if (filter->videowindow) {
					hr = filter->videowindow->Start();
				}
			}
		}

		if (is_frame_stepping) {
			if (fs) 
				hr = fs->CancelStep();
			if (mc) 
				hr = mc->Run();

			// reset the frame stepping flag
			is_frame_stepping = false;
		} else {
			hr = mc ? mc->Run() : E_NOINTERFACE;
		}
		return hr;
	}

	HRESULT DisplayGraph::DoStop()
	{
		if (is_frame_stepping) {
			if (fs) fs->CancelStep();
			is_frame_stepping = false;
		}

		if (mc) {		
			mc->Stop();
			Seek(0);
			return NOERROR;
		}

		return E_NOINTERFACE;
	}

	HRESULT DisplayGraph::DoPause()
	{
		// send start notification to all EVR filters
		// set the new clock for all filters
		if (!is_remote) {
			for (int i=0; i<filters.GetCount(); i++) {
				Filter	*filter = filters[i];
				if (filter->videowindow) {
					filter->videowindow->Start();
				}
			}
		}

		if (mc) mc->Pause();
		return NOERROR;
	}

	int DisplayGraph::Seek(double time_ms, BOOL keyframe)
	{
		if (!ms) 
			return E_POINTER;

		REFERENCE_TIME	rtpos = time_ms * (UNITS/1000);
		DWORD			flags = AM_SEEKING_AbsolutePositioning;

		if (keyframe) {
			flags |= AM_SEEKING_SeekToKeyFrame;
		}

		ms->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);		// other format may be in use by CSeekForm
		return ms->SetPositions(&rtpos, flags, NULL, AM_SEEKING_NoPositioning);
	}

    int DisplayGraph::GetRate(double* rate)
    {
        if (!ms) return -1;

        return ms->GetRate(rate);
    }

    int DisplayGraph::SetRate(double rate)
    {
        if (!ms) return -1;

        return ms->SetRate(rate);
    }

	void DisplayGraph::RefreshClock()
	{
		// update clock status for all filters
		for (int i=0; i<filters.GetCount(); i++) {
			Filter	*filter = filters[i];
			filter->UpdateClock();
		}
	}

	void DisplayGraph::SetClock(bool default_clock, IReferenceClock *new_clock)
	{
		FILTER_STATE	state;
		int ret = GetState(state, 50);
		if (ret < 0) {
			MessageBeep(MB_ICONASTERISK);
			return ;
		}

		if (state != State_Stopped) {
			MessageBeep(MB_ICONASTERISK);
			return ;
		}

		if (default_clock) {
			if (gb)
				gb->SetDefaultSyncSource();

			uses_clock = true;

		} else {
			// Don't call SetSyncSource directly on filters, call IMediaFilter::SetSyncSource on filter graph manager instead
			// MSDN docs warn about this. Calling filters directly causes unreliable clock setting behaviour
			uses_clock = (new_clock == NULL ? false : true);
			CComQIPtr<IMediaFilter> sync_interface(gb);
			if (sync_interface)
				sync_interface->SetSyncSource(new_clock);

		}
		RefreshClock();
	}

	// seeking helpers
	int DisplayGraph::GetFPS(double &fps)	
	{
		fps = this->fps;
		return 0;
	}

	int DisplayGraph::RefreshFPS()
	{
		if (!ms) {
			fps = 0;
			return 0;
		}

		HRESULT	hr = NOERROR;
		do {
			REFERENCE_TIME		rtDur, rtFrames;

			rtDur = 0;
			rtFrames = 0;

			if (ms->IsFormatSupported(&TIME_FORMAT_FRAME) != NOERROR) {
				hr = E_FAIL;
				break;
			}
			hr = ms->SetTimeFormat(&TIME_FORMAT_FRAME);
			if (SUCCEEDED(hr))
				hr = ms->GetDuration(&rtFrames);
			hr = ms->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
			if (SUCCEEDED(hr))
				hr = ms->GetDuration(&rtDur);

			// special case
			if (rtFrames == 0 || rtDur == 0) {
				fps = 0.0;
				return 0;
			}

			// calculate the FPS
			fps = (double)rtFrames * (double)UNITS / (double)rtDur;
			hr = NOERROR;

		} while (0);

		if (FAILED(hr)) {
			fps = -1.0;
			return -1;
		}

		return 0;
	}

	int DisplayGraph::GetPositionAndDuration(double &current_ms, double &duration_ms)
	{
		if (!ms) {
			current_ms = 0;
			duration_ms = 0;
			return 0;
		}

		HRESULT hr = NOERROR;
		do {
			GUID			time_format;
			REFERENCE_TIME	rtDur, rtCur;
			hr = ms->GetTimeFormat(&time_format);
			if (FAILED(hr)) break;

			// get duration
			hr = ms->GetDuration(&rtDur);
			if (FAILED(hr)) {
				duration_ms = 0;
			} else {
				// do we need to convert the time ?
				if (time_format != TIME_FORMAT_MEDIA_TIME) {
					REFERENCE_TIME	temp;
					GUID			out_format = TIME_FORMAT_MEDIA_TIME;

					hr = ms->ConvertTimeFormat(&temp, &out_format, rtDur, &time_format);
					if (FAILED(hr)) {
						temp = 0;
					}
					rtDur = temp;
				}

				// in milliseconds
				duration_ms = (double)rtDur / (double)(UNITS/1000);
			}

			/*
				I had to enable exceptions for C++ because Gabest's Avi Splitter
				kept crashing after this call when not connected. Not sure why.
				But the splitter wasn't able to play any files.. so I guess
				it might be wrong.
			*/

			// get position
			try {
				hr = ms->GetCurrentPosition(&rtCur);
			}
			catch (...) {
				hr = E_FAIL;
			}

			if (FAILED(hr)) {
				current_ms = 0;
			} else {
				// do we need to convert the time ?
				if (time_format != TIME_FORMAT_MEDIA_TIME) {
					REFERENCE_TIME	temp;
					GUID			out_format = TIME_FORMAT_MEDIA_TIME;

					hr = ms->ConvertTimeFormat(&temp, &out_format, rtCur, &time_format);
					if (FAILED(hr)) {
						temp = 0;
					}
					rtCur = temp;
				}

				// in seconds
				current_ms = (double)rtCur / (double)(UNITS/1000);
			}

			hr = NOERROR;
		} while (0);

		if (FAILED(hr)) {
			current_ms = 0;
			duration_ms = 0;
			return hr;
		}

		return 0;
	}

	void DisplayGraph::RemoveSelectionFromGraph()
	{
		// first delete connections and then filters
		for (int i=0; i<filters.GetCount(); i++) {
			filters[i]->RemoveSelectedConnections();
		}
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i]->selected) {
				if (callback) 
					callback->OnFilterRemoved(this, filters[i]);
				filters[i]->RemoveFromGraph();
			}
		}
		RefreshFilters();
	}

	HRESULT DisplayGraph::AddFilter(IBaseFilter *filter, CString proposed_name)
	{
		if (!gb) return E_FAIL;

		// find the best name
		CComPtr<IBaseFilter>	temp;
		CString					name = proposed_name;

		HRESULT hr = gb->FindFilterByName(name, &temp);
		if (SUCCEEDED(hr)) {
			temp = NULL;
			int i=0;
			do {
				name.Format(_T("%s %.04d"), proposed_name, i);
				hr = gb->FindFilterByName(name, &temp);
				temp = NULL;
				i++;
			} while (hr == NOERROR);
		}

		// now we have unique name
		hr = gb->AddFilter(filter, name);
		if (FAILED(hr)) {
			// cannot add filter
			return hr;
		}

		// refresh our filters
		RefreshFilters();
		return NOERROR;
	}

	static CStringA UTF16toUTF8(const CStringW &utf16)
	{
	   CStringA utf8;
	   int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, 0, 0);
	   if (len>1) { 
		  char *ptr = utf8.GetBuffer(len-1);
		  if (ptr) WideCharToMultiByte(CP_UTF8, 0, utf16, -1, ptr, len, 0, 0);
		  utf8.ReleaseBuffer();
	   }
	   return utf8;
	} 

	static HRESULT SaveXML_IFileSourceFilter(IBaseFilter *filter, XML::XMLWriter &xml)
	{
		CComQIPtr<IFileSourceFilter> src(filter);
		if (src) {
			LPOLESTR		fn = NULL;
            CMediaType media_type;
			if (SUCCEEDED(src->GetCurFile(&fn, &media_type))) {
				//	<ifilesourcefilter source="d:\sga.avi"/>
				xml.BeginNode(_T("ifilesourcefilter"));
					xml.WriteValue(_T("source"), CString(fn));
				xml.EndNode();
				if (fn) 
					CoTaskMemFree(fn);
			}
		}
		return S_OK;
	}

	static HRESULT SaveXML_IFileSinkFilter(IBaseFilter *filter, XML::XMLWriter &xml)
	{
		CComQIPtr<IFileSinkFilter> sink(filter);
		if (sink) {
			LPOLESTR		fn = NULL;
            CMediaType media_type;
			if (SUCCEEDED(sink->GetCurFile(&fn, &media_type))) {
				//	<ifilesinkfilter dest="d:\sga.avi"/>
				xml.BeginNode(_T("ifilesinkfilter"));
					xml.WriteValue(_T("dest"), CString(fn));
				xml.EndNode();
				if (fn) 
					CoTaskMemFree(fn);
			}
		}
		return S_OK;
	}

	static HRESULT SaveXML_IPersistStream(IBaseFilter *filter, XML::XMLWriter &xml)
	{
		HRESULT hr = E_FAIL;
		CComQIPtr<IPersistStream> persist_stream(filter);
		if (persist_stream) {
			const HGLOBAL hglobal_stream = GlobalAlloc(GHND, 0);	// free by stream unless stream isn't created
			CComPtr<IStream> stream;
			CreateStreamOnHGlobal(hglobal_stream, FALSE, &stream);

			if (stream 
					&& hglobal_stream  
					&& SUCCEEDED(hr = persist_stream->Save(stream, TRUE))) {

				const SIZE_T binary_max_size = GlobalSize(hglobal_stream);
				BYTE * const binary_data = new BYTE[binary_max_size];	// create buffer big enough for all data
				LARGE_INTEGER start_offset;
				start_offset.QuadPart = 0LL;
				HRESULT hr2 = stream->Seek(start_offset, STREAM_SEEK_SET, NULL);
				ASSERT(SUCCEEDED(hr2));

				ULONG binary_size = 0;
				hr2 = stream->Read(binary_data, binary_max_size, &binary_size);	// read as much data as is available
				ASSERT(SUCCEEDED(hr2));

				if (binary_size > 0) {		// Some datas have zero bytes of IPersistStream data
					const DWORD base64_flags = ATL_BASE64_FLAG_NOCRLF;
					const int base64_size = Base64EncodeGetRequiredLength(binary_size, base64_flags);
					char* const base64_data = new char[base64_size];

					int converted_size = base64_size;
					if (Base64Encode((const BYTE*)binary_data, binary_size, base64_data, &converted_size, base64_flags)
							&& converted_size > 0) {

						// <ipersiststream encoding="base64" data="MAAwADAAMAAwADAAMAAwADAAMAAwACAA="/>
						xml.BeginNode(_T("ipersiststream"));
							xml.WriteValue(_T("encoding"), _T("base64"));
							xml.WriteValue(_T("data"), CString(base64_data, converted_size));
						xml.EndNode();
					} else {
						ASSERT(!"base64 conversion failed");
					}

					delete[] base64_data;
				}

				delete[] binary_data;
			} else {
				ASSERT(!"Failed to create IStream");
			}
			if (hglobal_stream)
				GlobalFree(hglobal_stream);		
		}
		return S_OK;
	}

	static void SaveXML_MediaType(XML::XMLWriter &xml, const CMediaType& mt)
	{
		xml.BeginNode(_T("mediaType"));

			CString type_name;
			CString guid_name;

			// name major and/or sub type if known for readability of XML file
			if (NameGuid(mt.majortype, guid_name, false)) {		
				type_name = guid_name;
			}
			if (NameGuid(mt.subtype, guid_name, false)) {
				type_name += _T(" / ");
				type_name += guid_name;
			}
			if (type_name.GetLength() > 0) {
				xml.WriteValue(_T("type"), type_name);
			}

			xml.WriteValue(_T("sampleSize"),			mt.lSampleSize);
			xml.WriteValue(_T("fixedSizeSamples"),		mt.bFixedSizeSamples ? _T("true") : _T("false"));
			xml.WriteValue(_T("temporalCompression"),	mt.bTemporalCompression ? _T("true") : _T("false"));

			LPOLESTR clsid_olestr = NULL;
			StringFromCLSID(mt.majortype, &clsid_olestr);
			if (clsid_olestr) {
				xml.WriteValue(_T("majorType"), clsid_olestr);
				CoTaskMemFree(clsid_olestr);
			}
			clsid_olestr = NULL;

			StringFromCLSID(mt.subtype, &clsid_olestr);
			if (clsid_olestr) {
				xml.WriteValue(_T("subType"), clsid_olestr);
				CoTaskMemFree(clsid_olestr);
			}
			clsid_olestr = NULL;

		xml.EndNode();
	}

	static void SaveXML_MediaTypeFormat(XML::XMLWriter &xml, const CMediaType& mt)
	{
		const BYTE* const format_data = mt.Format();
		const ULONG format_length = mt.FormatLength();

		if (format_data && format_length) {

			xml.BeginNode(_T("format"));
			{
				CString guid_name;
				if (NameGuid(mt.formattype, guid_name, false)) {		
					xml.WriteValue(_T("type"), guid_name);
				}

				LPOLESTR clsid_olestr = NULL;
				StringFromCLSID(mt.formattype, &clsid_olestr);
				if (clsid_olestr) {
					xml.WriteValue(_T("formatType"), clsid_olestr);
					CoTaskMemFree(clsid_olestr);
				}

				const DWORD base64_flags = ATL_BASE64_FLAG_NOCRLF;
				const int base64_size = Base64EncodeGetRequiredLength(format_length, base64_flags);
				char* const base64_data = new char[base64_size];

				int converted_size = base64_size;
				if (Base64Encode((const BYTE*)format_data, format_length, base64_data, &converted_size, base64_flags)
						&& converted_size > 0) {

						xml.WriteValue(_T("encoding"), _T("base64"));
						xml.WriteValue(_T("data"), CString(base64_data, converted_size));

				} else {
					ASSERT(!"base64 conversion failed");
				}
				delete [] base64_data;
			}
			xml.EndNode();
		}
	}

	// Save connection from given output pin
	static void SaveXML_Connection(XML::XMLWriter &xml, const CArray<Filter*>& filters, const Pin* const pin, int out_filter_index, int out_pin_index)
	{
		const Filter* const filter = pin->filter;

		// work out the filter index of the peer input pin
		int in_filter_index = -1;
		for (int f=0; f<filters.GetCount(); f++) {
			if (pin->peer->filter == filters[f]) {
				in_filter_index = f;
			}
		}
		ASSERT(in_filter_index >= 0);

		// work out the pin index of the peer input pin
		int in_pin_index = -1;
		CString in_duplicate_ids;

		for (int ip=0; ip<pin->peer->filter->input_pins.GetCount(); ip++) {
			const Pin * const sibling = pin->peer->filter->input_pins[ip];
			if (pin->peer == sibling) {
				in_pin_index = ip;
			} else if (pin->peer->id == sibling->id) {
				if (in_duplicate_ids.GetLength() > 0)
					in_duplicate_ids += ",";
				CString temp;
				temp.Format(_T("%d"), ip);
				in_duplicate_ids += temp;
			}
		}
		ASSERT(in_pin_index >= 0);

		// work out if any duplicate ids on output pins
		CString out_duplicate_ids;

		for (int op=0; op<filter->output_pins.GetCount(); op++) {
			const Pin * const sibling = filter->output_pins[op];
			if (pin != sibling  && pin->id == sibling->id) {
				if (out_duplicate_ids.GetLength() > 0)
					out_duplicate_ids += ",";
				CString temp;
				temp.Format(_T("%d"), op);
				out_duplicate_ids += temp;
			}
		}
		ASSERT(in_pin_index >= 0);

		xml.BeginNode(_T("connect"));
			// Write these first to make XML more 
			xml.WriteValue(_T("out"), filter->name + _T("/") + pin->name);
			xml.WriteValue(_T("in"), pin->peer->filter->name + _T("/") + pin->peer->name);

			xml.WriteValue(_T("outFilterIndex"), out_filter_index);
			xml.WriteValue(_T("outPinId"), pin->id);
			xml.WriteValue(_T("outPinName"), pin->name);
			if (out_duplicate_ids.GetLength() > 0)
				xml.WriteValue(_T("outPinIdConflicts"), out_duplicate_ids);
			xml.WriteValue(_T("outPinIndex"), out_pin_index);

			xml.WriteValue(_T("inFilterIndex"), in_filter_index);
			xml.WriteValue(_T("inPinId"), pin->peer->id);
			xml.WriteValue(_T("inPinName"), pin->peer->name);
			if (in_duplicate_ids.GetLength() > 0)
				xml.WriteValue(_T("inPinIdConflicts"), in_duplicate_ids);
			xml.WriteValue(_T("inPinIndex"), in_pin_index);

			xml.WriteValue(_T("direct"), _T("true"));

			CMediaType mt;
			if (SUCCEEDED(pin->pin->ConnectionMediaType(&mt))) {
				SaveXML_MediaType(xml, mt);
				SaveXML_MediaTypeFormat(xml, mt);
			}

		xml.EndNode();
	}

	static void SaveXML_GraphConnections(XML::XMLWriter& xml, const CArray<Filter*>& filters)
	{
		std::set<const Pin*> saved_input_pins;	// The input pins whose connections have already been saved
		bool all_inputs_saved = false;		// true when all filters have had their connected inputs saved so we can stop iterating
		int iterations = 0;

		// Now let's add all the connections
		// Loop over filters only saving connections from filters whose input connections have been saved already
		// until all connections have been saved
		while (!all_inputs_saved) {		
			if (iterations++ > 500)	{	// Sanity check to prevent pathological infinite looping
				ASSERT(false);
				break;
			}

			all_inputs_saved = true;	// test every filter for unsaved input connections in loop below

			for (int of=0; of<filters.GetCount(); of++) {
				
				const Filter * const filter = filters[of];
				bool inputs_saved = true;

				// Look for any unsaved connections on input pins
				for (int j=0; j<filter->input_pins.GetCount(); j++) {
					const Pin* const pin = filter->input_pins[j];
					if (pin->peer && saved_input_pins.find(pin) == saved_input_pins.end()) {		
						inputs_saved = false;		// found input pin with unsaved connection
						break;
					}
				}

				all_inputs_saved = all_inputs_saved && inputs_saved;

				// Only save a filter's output pin connections if all of its input pin connections are already saved
				if (inputs_saved) {
					for (int op=0; op<filter->output_pins.GetCount(); op++) {
						Pin * const pin = filter->output_pins[op];
						if (pin->peer && saved_input_pins.find(pin->peer) == saved_input_pins.end()) {	// if connection not saved yet
							saved_input_pins.insert(pin->peer);		// record that we've saved this input pin's connection

							SaveXML_Connection(xml, filters, pin, of, op);
						}
					}
				}
			}
		}

	}

	static void SaveXML_Filter(XML::XMLWriter &xml, const Filter* filter, int index)
	{
		xml.BeginNode(_T("filter"));
			// save the filter CLSID. If the filter is a wrapper it will initialize properly
			// after loading IPersistStream 
			xml.WriteValue(_T("name"), filter->name);
			xml.WriteValue(_T("index"), index);

			CString guid_name;
			if (NameGuid(filter->clsid, guid_name, false)) {		
				xml.WriteValue(_T("class"), guid_name);
			}

			LPOLESTR	strclsid = NULL;
			StringFromCLSID(filter->clsid, &strclsid);
			xml.WriteValue(_T("clsid"), strclsid);
			CoTaskMemFree(strclsid);

			// now check for interfaces
			SaveXML_IFileSourceFilter(filter->filter, xml);
			SaveXML_IFileSinkFilter(filter->filter, xml);
			SaveXML_IPersistStream(filter->filter, xml);
		xml.EndNode();
	}

	int DisplayGraph::SaveXML(CString fn)
	{
		XML::XMLWriter			xml;

		xml.BeginNode(_T("graph"));
			xml.WriteValue(_T("name"), _T("Unnamed Graph"));

			for (int i=0; i<filters.GetCount(); i++) {
				SaveXML_Filter(xml, filters[i], i);
			}

			SaveXML_GraphConnections(xml, filters);

		xml.EndNode();

		FILE		*f;
        if(_tfopen_s(&f,fn, _T("wb")) != NOERROR)
            return -1;

		CString		x = xml.XML();
		CStringA	xa = UTF16toUTF8(x);
		fwrite(xa.GetBuffer(), 1, xa.GetLength(), f);
		fclose(f);

		return 0;
	}

	HRESULT DisplayGraph::LoadXML(CString fn)
	{
		XML::XMLFile xml;
		
		HRESULT	hr = xml.LoadFromFile(fn);
		if (FAILED(hr)) {
			return hr;
		}

		// load graph
		XML::XMLNode * const root = xml.root;
		XML::XMLIterator it;
		if (root->Find(_T("graph"), &it) < 0) 
			return VFW_E_NOT_FOUND;

		XML::XMLNode * const gn = *it;

		// Filter list stored in the same order as stored in XML file for fixing up Filter index references
		// Only store for the duration of loading
		// The IBaseFilter* are only used to find matching Filters so will not crash if pointers are invalid
		CArray<IBaseFilter *>	filters_loaded_order;

		HRESULT connection_hresult = S_OK;
		HRESULT filter_hresult = S_OK;

		for (it = gn->nodes.begin(); it != gn->nodes.end(); it++) {
			XML::XMLNode * const node = *it;

			if (node->name == _T("filter"))	{
				CComPtr<IBaseFilter> created_filter;
				hr = LoadXML_Filter(node, created_filter);
				filters_loaded_order.Add(created_filter.p);		// Add NULL if filter failed to load
				if (FAILED(hr))
					filter_hresult = hr;

			} else if (node->name == _T("render")) 
				hr = LoadXML_Render(node); 
			else if (node->name == _T("connect")) {
				hr = LoadXML_Connect(node, filters_loaded_order); 
				if (FAILED(hr))
					connection_hresult = hr;

			} else if (node->name == _T("config")) 
				hr = LoadXML_Config(node); 
			else if (node->name == _T("iamgraphstreams")) 
				hr = LoadXML_IAMGraphStreams(node); 
			else if (node->name == _T("schedule")) 
				hr = LoadXML_Schedule(node); 
			else if (node->name == _T("command")) 
				hr = LoadXML_Command(node); 
		}

		// TODO report all errors - perhaps in graph construction window?

		if (FAILED(filter_hresult)) {
			SmartPlacement();			// arrange filters so user can see what's failed
			DSUtil::ShowError(filter_hresult, _T("Failed to create filter(s)"));
		}

		if (FAILED(connection_hresult)) {
			SmartPlacement();			// arrange filters so user can see what's failed
			DSUtil::ShowError(connection_hresult, _T("Connection(s) failed - try different XML options"));
		}

		return S_OK;
	}

	HRESULT DisplayGraph::LoadXML_Schedule(XML::XMLNode *node)
	{
		// <schedule pattern="*:*:*" action="restart"/>
		CMainFrame * const frame = (CMainFrame*)AfxGetMainWnd();
		if (frame) {
			CGraphView * const view = frame->view;

			const CString	pattern = node->GetValue(_T("pattern"));
			const CString	action  = node->GetValue(_T("action"));

			int act = 0;
			if (action == _T("start")) 
				act = ScheduleEvent::ACTION_START; 
			else if (action == _T("stop")) 
				act = ScheduleEvent::ACTION_STOP; 
			else if (action == _T("restart")) 
				act = ScheduleEvent::ACTION_RESTART;

			if (view->form_schedule) {
				// add a new schedule event
				view->form_schedule->AddEvent(pattern, act);
			}
		}

		return S_OK;
	}

	HRESULT DisplayGraph::LoadXML_IAMBufferNegotiation(XML::XMLNode *conf, IBaseFilter *filter)
	{
        // <iambuffernegotiation pin="Capture" latency="40"/>
		Filter		gf(this);

		gf.LoadFromFilter(filter);

		const CString pin_name = conf->GetValue(_T("pin"));
		Pin	* const pin = gf.FindPinByName(pin_name);
		if (!pin) 
			return VFW_E_NOT_FOUND;

		// let's query for IAMBufferNegotiation
		DSUtil::MediaTypes				mtlist;
		CComPtr<IAMBufferNegotiation>	buf_neg;

		HRESULT hr = pin->pin->QueryInterface(IID_IAMBufferNegotiation, (void**)&buf_neg);
		if (FAILED(hr)) 
			return hr;

		hr = DSUtil::EnumMediaTypes(pin->pin, mtlist);
		if (FAILED(hr))
			return hr;
		if (mtlist.GetCount() <= 0)
			return VFW_E_NOT_FOUND;

		// if it's an audio pin, we need to calculate the buffer size
		const CMediaType mt = mtlist[0];
		if (mt.majortype == MEDIATYPE_Audio && 
			mt.subtype == MEDIASUBTYPE_PCM &&
			mt.formattype == FORMAT_WaveFormatEx && 
			mt.cbFormat >= sizeof(WAVEFORMATEX)) {
			int		latency_ms = conf->GetValue(_T("latency"), -1);
			if (latency_ms > 0) {
			
				const WAVEFORMATEX * const wfx = (WAVEFORMATEX*)mt.pbFormat;

				// just like MSDN said: -1 = we don't care
				ALLOCATOR_PROPERTIES		alloc;
				alloc.cbAlign	= -1;
				alloc.cbBuffer	= (wfx->nAvgBytesPerSec * latency_ms) / 1000;
				alloc.cbPrefix	= -1;
				alloc.cBuffers	= 20;

				hr = buf_neg->SuggestAllocatorProperties(&alloc);
				DSUtil::ShowError(hr, _T("IAMBufferNegotiation::SuggestAllocatorProperties failed"));
			}
		} else {
			// we'll see
		}
		return S_OK;
	}

	HRESULT DisplayGraph::LoadXML_IAMGraphStreams(XML::XMLNode *node)
	{
		if (!gb) 
			return E_POINTER;

		// <iamgraphstreams sync="1"/>
		// <iamgraphstreams max_latency="800000"/>

		const int	sync    = node->GetValue(_T("sync"), -1);
		const int	latency = node->GetValue(_T("max_latency"), -1);

		CComPtr<IAMGraphStreams>	gs;
		HRESULT hr = gb->QueryInterface(IID_IAMGraphStreams, (void**)&gs);
		if (SUCCEEDED(hr)) {

			// enable sync
			if (sync >= 0) {
				hr = gs->SyncUsingStreamOffset((sync == 1 ? TRUE : FALSE));
				DSUtil::ShowError(hr, _T("Failed to set IAMGraphStreams::SyncUsingStreamOffset"));
			}

			// max latency
			if (sync == 1 && latency >= 0) {
				const REFERENCE_TIME rtMaxLatency = latency;
				hr = gs->SetMaxGraphLatency(rtMaxLatency);
				DSUtil::ShowError(hr, _T("Failed to set IAMGraphStreams::SetMaxGraphLatency"));
			}
			return S_OK;
		}
		return hr;
	}

	HRESULT DisplayGraph::LoadXML_Command(XML::XMLNode *node)
	{
		const CString msg = node->GetValue(_T("msg"));

		if (msg == _T("run")) {
			DoPlay();
		} else if (msg == _T("pause")) {
			DoPause();
		} else if (msg == _T("stop")) {
			DoStop();
		} else if (msg == _T("progress")) {
			
			CMainFrame * const frame = (CMainFrame*)AfxGetMainWnd();
			if (frame) {
				// run in the progress mode
				frame->view->OnViewProgressview();
			}
		}

		return S_OK;
	}

	HRESULT DisplayGraph::LoadXML_Render(XML::XMLNode *node)
	{
		const CString pin_path = node->GetValue(_T("pin"));
		Pin	* const pin = FindPinByPath(pin_path);
		if (!pin) 
			return VFW_E_NOT_FOUND;

		// try to render
		params->MarkRender(true);
		HRESULT	hr = gb->Render(pin->pin);
		params->MarkRender(false);
		if (callback) 
			callback->OnRenderFinished();
		if (FAILED(hr)) 
			return hr;

		// reload newly added filters
		RefreshFilters();
		return S_OK;
	}

	// Returns success HRESULT fif media type loaded
	static HRESULT LoadXML_MediaType(XML::XMLNode *node, CMediaType& mt)
	{
		HRESULT hr = E_FAIL;	// not loaded

		XML::XMLIterator it;
		if (node->Find(_T("mediaType"), &it) >= 0) {
			XML::XMLNode * const node = *it;
			if (node) {
				mt.bFixedSizeSamples			= node->GetValue(_T("fixedSizeSamples"))	== _T("true") ? TRUE : FALSE;
				mt.bTemporalCompression			= node->GetValue(_T("temporalCompression")) == _T("true") ? TRUE : FALSE;
				mt.lSampleSize					= node->GetValue(_T("sampleSize"), 0);

				CString clsid_str = node->GetValue(_T("majorType"));
				hr = CLSIDFromString((LPOLESTR)clsid_str.GetBuffer(), &mt.majortype);

				clsid_str = node->GetValue(_T("subType"));
				CLSIDFromString((LPOLESTR)clsid_str.GetBuffer(), &mt.subtype);
			}
		}
		return hr;
	}

	static HRESULT LoadXML_MediaTypeFormat(XML::XMLNode *node, CMediaType& mt)
	{
		HRESULT hr = E_FAIL;	// not loaded

		XML::XMLIterator it;
		if (node->Find(_T("format"), &it) >= 0) {
			XML::XMLNode * const node = *it;
			if (node) {

				CString clsid_str = node->GetValue(_T("formatType"));
				hr = CLSIDFromString((LPOLESTR)clsid_str.GetBuffer(), &mt.formattype);

				const CString base64_str = node->GetValue(_T("data"));
				ASSERT(base64_str.GetLength() > 0);
				ASSERT(node->GetValue(_T("encoding")) == _T("base64"));

				if (base64_str.GetLength() > 0) {

					const int stream_size = Base64DecodeGetRequiredLength(base64_str.GetLength());
					mt.AllocFormatBuffer(stream_size);

					const CStringA base64_mcbs(base64_str);
					int converted_length = mt.FormatLength();

					if (Base64Decode(base64_mcbs, base64_mcbs.GetLength(), mt.Format(), &converted_length)
							&& converted_length > 0
							&& mt.ReallocFormatBuffer(converted_length)) {
						hr = S_OK;
					}
				}
			}
		}
		return hr;
	}

	HRESULT DisplayGraph::LoadXML_Connect(XML::XMLNode *node, const CArray<IBaseFilter *> & indexed_filters)
	{
		Pin * opin = NULL;
		Pin * ipin = NULL;

		const int ofilter_index = node->GetValue(_T("outFilterIndex"), -1);
		const int ifilter_index = node->GetValue(_T("inFilterIndex"), -1);

		if (ofilter_index >= 0 && ofilter_index < indexed_filters.GetCount()
				&& ifilter_index >= 0 && ifilter_index < indexed_filters.GetCount()) {

			// Look filter up in out list as DirectShow may have rearranged order within the graph
			Filter * const ofilter = FindFilter(indexed_filters[ofilter_index]);
			Filter * const ifilter = FindFilter(indexed_filters[ifilter_index]);

			if (ofilter && ifilter) {
				switch (CgraphstudioApp::g_ResolvePins) {
					case CgraphstudioApp::BY_ID: {
					
						CString id = node->GetValue(_T("outPinId"));
						opin = ofilter->FindPinByID(id);
					
						id = node->GetValue(_T("inPinId"));
						ipin = ifilter->FindPinByID(id);
					
					}	break;

					case CgraphstudioApp::BY_INDEX: {
					
						int index = node->GetValue(_T("outPinIndex"), -1);
						if (index >= 0 && index < ofilter->output_pins.GetCount())
							opin = ofilter->output_pins[index];

						index = node->GetValue(_T("inPinIndex"), -1);
						if (index >= 0 && index < ifilter->input_pins.GetCount())
							ipin = ifilter->input_pins[index];

					}	break;

					case CgraphstudioApp::BY_NAME:
					default: {

						CString id = node->GetValue(_T("outPinName"));
						opin = ofilter->FindPinByName(id);
					
						id = node->GetValue(_T("inPinName"));
						ipin = ifilter->FindPinByName(id);

					}	break;
				}
			}
		}

		// As further fallback, connect by the old pin path fields
		if (!opin || !ipin) {
			opin = FindPinByPath(node->GetValue(_T("out")));
			ipin = FindPinByPath(node->GetValue(_T("in")));
		}

		if (!opin || !ipin) 
			return VFW_E_NOT_FOUND;

		const CString direct = node->GetValue(_T("direct"));

		HRESULT hr = S_OK;
		if (direct.IsEmpty() || direct == _T("false"))
			hr = gb->Connect(opin->pin, ipin->pin);
		else {
			CMediaType media_type;
			const bool use_media_type = SUCCEEDED(LoadXML_MediaType(node, media_type));
			if (use_media_type)
				LoadXML_MediaTypeFormat(node, media_type);
				
			// Only use media type for connection if we managed to load it
			hr = gb->ConnectDirect(opin->pin, ipin->pin, use_media_type ? &media_type : NULL);
			if (FAILED(hr) && use_media_type) {
				// If connection with media type failed reattempt connection without media type
				hr = gb->ConnectDirect(opin->pin, ipin->pin, NULL);
			}
		}

		// reload newly added filters
		RefreshFilters();
		return hr;
	}

	HRESULT DisplayGraph::LoadXML_Config(XML::XMLNode *node)
	{
		const CString name = node->GetValue(_T("name"));
		Filter * const filter = FindFilter(name);
		if (!filter) 
			return VFW_E_NOT_FOUND;

		for (XML::XMLIterator it = node->nodes.begin(); it != node->nodes.end(); it++) {
			XML::XMLNode * conf = *it;
			const HRESULT hr = LoadXML_ConfigInterface(conf, filter->filter);
			if (FAILED(hr)) 
				return hr;
		}
		return S_OK;
	}

	HRESULT DisplayGraph::LoadXML_Filter(XML::XMLNode *node, CComPtr<IBaseFilter>& instance)
	{
		const CString			name		= node->GetValue(_T("name"));
		CString					clsid_str	= node->GetValue(_T("clsid"));
		const CString			dn			= node->GetValue(_T("displayname"));
		GUID					clsid;
		HRESULT					hr = NOERROR;
		int						filter_id_type = -1;
		bool					is_configured = false;

		instance = NULL;

		// detect how the filter is described
		if (clsid_str != _T("")) {
			filter_id_type = 0;
            hr = CLSIDFromString((LPOLESTR)clsid_str.GetBuffer(), &clsid);
			if (FAILED(hr)) 
				return hr;
		} else
		if (dn != _T("")) {
			filter_id_type = 1;
		}

		// create the filter
		switch (filter_id_type) {
		case 0:
			{
				// create by CLSID
				hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
			}
			break;
		case 1:
			{
				// create by display name
				CComPtr<IMoniker>		moniker;
				CComPtr<IBindCtx>		bind;
				ULONG					eaten = 0;

				hr = CreateBindCtx(0, &bind);
				if (SUCCEEDED(hr)) {
					hr = MkParseDisplayName(bind, dn, &eaten, &moniker);
					if (SUCCEEDED(hr)) {
						hr = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&instance);
					}
				}

				if (FAILED(hr)) instance = NULL;
				bind = NULL;
				moniker = NULL;
			}
			break;
		default:
			{
				hr = E_FAIL;
			}
			break;
		}

		// add the filter instance
		if (SUCCEEDED(hr) && instance) {
			// add the filter to graph
			hr = AddFilter(instance, name);
			if (FAILED(hr)) {
				// display error message
			}
		}

		// check for known interfaces
		if (SUCCEEDED(hr)) {
			hr = LoadXML_Interfaces(node, instance);
			is_configured = SUCCEEDED(hr);
		}

		if (SUCCEEDED(hr) && instance != NULL && !is_configured) {

			// now check for a few interfaces
			int r = ConfigureInsertedFilter(instance, name);
			if (r < 0) {
				instance = NULL;
			}
		}

		RefreshFilters();
		return hr;
	}

	// Returns S_OK if configured and S_FALSE if untouched
	HRESULT DisplayGraph::LoadXML_Interfaces(XML::XMLNode *node, IBaseFilter *filter)
	{
		HRESULT hr = S_FALSE;

		for (XML::XMLIterator it = node->nodes.begin(); it != node->nodes.end(); it++) {
			XML::XMLNode * const iface = *it;
			hr = LoadXML_ConfigInterface(iface, filter);
			if (FAILED(hr)) 
				return hr;
		}

		return hr;
	}

	HRESULT DisplayGraph::ParseGRFFile(LPCWSTR filename)
	{
		HRESULT hr = S_OK;

		GRF_File grf;
		hr = grf.Load(filename);

		if (FAILED(hr))
			return hr;

		// The order of the following operations should be the same as DirectShow loading of GRF files
		// This seems to be the order of operations derived from stepping through filter code in GraphEdit
		// 1 Create filter
		// 2 Set source file
		// 3 Set sink file
		// 4 Load filter configuration
		// 5 Connect pins

		for (int i=0; i<grf.grf_filters.GetCount(); i++) {
			GRF_Filter& filter = grf.grf_filters[i];

			CString guidStr;
			NameGuid(filter.clsid, guidStr, true);
			_tprintf(_T("Creating filter %d %s, CLSID %s\n"), filter.index, (LPCTSTR)filter.name, (LPCTSTR)guidStr); 

			hr = filter.ibasefilter.CoCreateInstance(filter.clsid, NULL, CLSCTX_INPROC_SERVER);

			ASSERT(filter.ibasefilter);

			if (!filter.ibasefilter) {
				CString errorStr;
				errorStr.Format(_T("Cannot create filter %d %s, CLSID %s"), filter.index, (LPCTSTR)filter.name, (LPCTSTR)guidStr);
				DSUtil::ShowError(hr, errorStr);

			} else {

				// 1: Create filter
				AddFilter(filter.ibasefilter, filter.name);

				// 2 Set source file
				if (!filter.source_filename.IsEmpty()) {
					CComQIPtr<IFileSourceFilter> source(filter.ibasefilter);
					if (source) {
						_tprintf(_T("  Loading source file %s\n"), (LPCTSTR)filter.source_filename);

						const DWORD file_attributes = GetFileAttributes(filter.source_filename);

						// Give the user a chance to fix up an invalid filename but only check file name for the first time
						if (INVALID_FILE_ATTRIBUTES == file_attributes)
							hr = E_INVALIDARG;
						else 
							hr = source->Load(filter.source_filename, NULL);

						while (FAILED(hr)) {
							CFileSrcForm form(filter.name);
							form.result_file = filter.source_filename;
							hr = form.ChooseSourceFile(source);
						}
					}
				}

				// 3 Set sink file
				if (!filter.sink_filename.IsEmpty()) {
					CComQIPtr<IFileSinkFilter> sink(filter.ibasefilter);
					if (sink) {
						_tprintf(_T("  Setting sink file %s\n"), (LPCTSTR)filter.sink_filename); 

						const DWORD file_attributes = GetFileAttributes(filter.sink_filename);

						// Give the user a chance to fix up an invalid filename but only check file name for the first time
						if (INVALID_FILE_ATTRIBUTES == file_attributes)
							hr = E_INVALIDARG;
						else 
							hr = sink->SetFileName(filter.sink_filename, NULL);

						CFileSinkForm form(filter.name);
						while (FAILED(hr)) {
							form.result_file = filter.sink_filename;
							hr = form.ChooseSinkFile(sink);
						}
					}
				}

				// 4 Load filter configuration
				if (!filter.ipersiststream_data.IsEmpty()) {
					_tprintf(_T("  Loading state\n")); 

					bool success = false;

					CComPtr<IStream> stream;
					hr = CreateStreamOnHGlobal(NULL, TRUE, &stream);
					if (stream) {
						hr = stream->Write((const void*)(const char*)filter.ipersiststream_data, filter.ipersiststream_data.GetLength(), NULL);
						LARGE_INTEGER zero;
						zero.QuadPart = 0LL;
						hr = stream->Seek(zero, STREAM_SEEK_SET, NULL);
						CComQIPtr<IPersistStream> ps(filter.ibasefilter);
						if (ps) {
							hr = ps->Load(stream);
							success = SUCCEEDED(hr); 
						}
					}
					if (!success) {
						CString errorStr;
						errorStr.Format(_T("Cannot load state for filter %d %s, CLSID %s"), filter.index, (LPCTSTR)filter.name, (LPCTSTR)guidStr);
						DSUtil::ShowError(hr, errorStr);
					}
				}
			}
		}
		
		// 5 Connect pins
		for (int i=0; i<grf.grf_connections.GetCount(); i++) {
			const GRF_Connection& connection = grf.grf_connections[i];

			RefreshFilters();

			_tprintf(_T("Connecting filter %d pin %s to filter %d pin %s\n"), 
					connection.output_filter_index, (LPCTSTR)connection.output_pin_id, 
					connection.input_filter_index, (LPCTSTR)connection.input_pin_id); 

			Filter* out_filter = NULL;
			if (connection.output_filter_index > 0 && connection.output_filter_index <= grf.grf_filters.GetCount())	// 1-based indices
				out_filter = FindFilter(grf.grf_filters[connection.output_filter_index - 1].ibasefilter);

			Filter* in_filter = NULL;
			if (connection.input_filter_index > 0 && connection.input_filter_index <= grf.grf_filters.GetCount())	// 1-based indices
				in_filter = FindFilter(grf.grf_filters[connection.input_filter_index - 1].ibasefilter);

			ASSERT(out_filter);
			ASSERT(in_filter);

			Pin *out_pin = NULL;
			Pin *in_pin = NULL;
			if (out_filter && in_filter) {
				out_pin = out_filter->FindPinByID(connection.output_pin_id);
				in_pin = in_filter->FindPinByID(connection.input_pin_id);

				if (!out_pin) {
					CString errorStr;
					errorStr.Format(_T("Can't find output pin ID %s for filter %d %s"), 
							(LPCTSTR)connection.output_pin_id,
							connection.output_filter_index,
							(LPCTSTR)out_filter->display_name);
					DSUtil::ShowError(errorStr);
				}

				if (!in_pin) {
					CString errorStr;
					errorStr.Format(_T("Can't find input pin ID %s for filter %d %s"), 
							(LPCTSTR)connection.input_pin_id,
							connection.input_filter_index,
							(LPCTSTR)in_filter->display_name);
					DSUtil::ShowError(errorStr);
				}

				if (out_pin && in_pin) {
					hr = gb->ConnectDirect(out_pin->pin, in_pin->pin, &connection.media_type);

					if (FAILED(hr)) {
						CString errorStr;
						errorStr.Format(_T("Connecting %s/%s to %s/%s"), 
								(LPCTSTR)out_filter->display_name,
								(LPCTSTR)connection.output_pin_id,
								(LPCTSTR)in_filter->display_name,
								(LPCTSTR)connection.input_pin_id);
						DSUtil::ShowError(hr, errorStr);

						hr = gb->ConnectDirect(out_pin->pin, in_pin->pin, NULL);	// reattempt with no media type

						CString title(_T("No media type: "));
						DSUtil::ShowError(hr, title + errorStr);
					}
				}
			}
		}
		return S_OK;	// we report any loading errors above and load a partial graph
	}

	HRESULT DisplayGraph::LoadGRF(CString fn)
	{
		HRESULT hr = S_OK;

		if (CgraphstudioApp::g_useInternalGrfParser)
			return ParseGRFFile(fn);

		{
			hr = StgIsStorageFile(fn);
			if (hr != S_OK)
				return hr;

			CComPtr<IStorage> pStorage;
			hr = StgOpenStorage(fn, 0, STGM_TRANSACTED | STGM_READ | STGM_SHARE_DENY_WRITE, 0, 0, &pStorage);
			if (FAILED(hr)) 
				return hr;

			CComQIPtr<IPersistStream> pPersistStream(gb);
			if (!pPersistStream) {
				hr = E_NOINTERFACE;
			} else {
				CComPtr<IStream> pStream;
				hr = pStorage->OpenStream(L"ActiveMovieGraph", 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream);
				if (SUCCEEDED(hr)) {
					hr = pPersistStream->Load(pStream);
				}
			}
		}

		if (FAILED(hr) && DSUtil::ShowError(hr, _T("Load failed. Try internal GRF file parser?"))) {
			return ParseGRFFile(fn);
		}
		return S_FALSE;
	}

	int DisplayGraph::SaveGRF(CString fn)
	{
		const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
		HRESULT hr;
		    
		IStorage *pStorage = NULL;
		hr = StgCreateDocfile(fn, STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pStorage);
		if (FAILED(hr)) return hr;

		IStream *pStream;
		hr = pStorage->CreateStream(wszStreamName, STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,	0, 0, &pStream);
		if (FAILED(hr)) {
			pStorage->Release();    
			return hr;
		}

		IPersistStream *pPersist = NULL;
		gb->QueryInterface(IID_IPersistStream, (void**)&pPersist);
		hr = pPersist->Save(pStream, TRUE);
		pStream->Release();
		pPersist->Release();
		if (SUCCEEDED(hr)) {
			hr = pStorage->Commit(STGC_DEFAULT);
		}
		pStorage->Release();
		return hr;
	}

	// Caller must do any required clean up if this returns error code
	HRESULT DisplayGraph::RenderFile(CString fn)
	{
		HRESULT	hr = E_FAIL;

		do {
			params->MarkRender(true);
			if (!gb) {
				hr = E_POINTER;	
				break;
			}
			hr = gb->RenderFile(fn, NULL);
			params->MarkRender(false);
			if (callback) 
				callback->OnRenderFinished();

		} while (0);

		return hr;
	}

	Pin *DisplayGraph::FindPinByPath(CString pin_path)
	{
		// find the filter
		CString	filter_name = DSUtil::get_next_token(pin_path, _T("/"));
		Filter * const filter = FindFilter(filter_name);
		if (!filter) 
			return NULL;

		// try to find the pin
		return filter->FindPinByName(pin_path);
	}

	Filter *DisplayGraph::FindFilter(CString name)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i]->name == name) {
				return filters[i];
			}
		}
		return NULL;
	}

	// Will not crash if IBaseFilter* is invalid pointer - but may return wrongly matched filter
	Filter *DisplayGraph::FindFilter(IBaseFilter *filter)
	{
		// find by filter interface
		for (int i=0; i<filters.GetCount(); i++) {
			Filter *filt = filters[i];
			if (filt->filter == filter) {
				return filt;
			}
		}
		return NULL;		
	}

	void DisplayGraph::DeleteAllFilters()
	{
		for (int i=filters.GetCount()-1; i>=0; i--) {
			Filter *filt = filters[i];
			if (callback) 
				callback->OnFilterRemoved(this, filt);
			delete filt;
		}
		filters.RemoveAll();
	}

	HRESULT DisplayGraph::ConnectPins(Pin *p1, Pin *p2, bool chooseMediaType)
	{
		// verify a few conditions first
		if (!p1 || !p2) return -1;								// need 2 pins
		if (p1->connected || p2->connected) return -1;			// 2 free pins
		if (p1->filter == p2->filter) return -1;				// not on the same filter
		if (p1->dir == p2->dir) return -1;						// oposite directions

		HRESULT hr = S_OK;

		params->MarkRender(true);
		hr = DSUtil::ConnectPin(gb, p1->pin, p2->pin, params->connect_mode != 0, params->connect_mode == 2);
		params->MarkRender(false);
		if (callback)
			callback->OnRenderFinished();
		
        if (hr == S_OK) {
		    RefreshFilters();
		    SmartPlacement();
        }

		DSUtil::ShowError(hr, _T("Connecting Pins"));

		return hr;
	}

	Pin *DisplayGraph::FindPinByPos(CPoint pt)
	{
		Filter *filter = FindFilterByPos(pt);
		if (!filter) return NULL;
		return filter->FindPinByPos(pt);
	}

	Filter *DisplayGraph::FindFilterByPos(CPoint pt)
	{
		int border = 6;
		for (int i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			CRect	rc(filter->posx-border, filter->posy-2, 
					   filter->posx + filter->width+border, filter->posy + filter->height+2);
			if (rc.PtInRect(pt)) return filter;
		}
		return NULL;
	}

	void DisplayGraph::RefreshFilters()
	{
		HRESULT hr = S_OK;

		CComPtr<IEnumFilters> enum_filters;
		CComPtr<IBaseFilter> ifilter;

		// Make a backup of stale list and clear the main list
		CArray<Filter*> stale_filters;
		stale_filters.Copy(filters);
		filters.RemoveAll();

		if (gb) {
			hr = gb->EnumFilters(&enum_filters);
			if (SUCCEEDED(hr) && enum_filters) {
				enum_filters->Reset();
				ULONG ff = 0;
				while (enum_filters->Next(1, &ifilter, &ff) == NOERROR) {
		
					Filter	* filter = NULL;

					// Find matching existing Filter if any and delete from stale list
					for (int i=0; i<stale_filters.GetCount(); i++) {
						Filter * const f = stale_filters[i];
						if (f->filter == ifilter) {
							filter = f;
							stale_filters.RemoveAt(i);
							break;
						}
					}

					// Create new Filter if not found in stale list
					if (!filter) {
						filter = new Filter(this);
						filter->filter = ifilter;
					}

					// add Filter back to main list in same order as enumerated
					filters.Add(filter);
					ifilter.Release();
				}
			}
		}

		// Remaining Filters in stale list are not in graph so need to be deleted
		for (int i=0; i<stale_filters.GetCount(); i++) {
			Filter * const f = stale_filters[i];
			if (callback) 
				callback->OnFilterRemoved(this, f);
			delete f;
			// no need to clear list as it's not used below
		}

		// Refresh updated list of Filters in the main list
		for (int i=0; i<filters.GetCount(); i++) {
			filters[i]->Refresh();
		}

		LoadPeers();
		RefreshClock();
		RefreshFPS();
		Dirty();
	}

	void DisplayGraph::LoadPeers()
	{
		for (int i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			filter->LoadPeers();
		}
	}


	// kreslenie grafu
	void DisplayGraph::Draw(CDC *dc)
	{
		int i;
		for (i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			filter->Draw(dc);
		}

		// now draw all connections
		for (i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			filter->DrawConnections(dc);
		}
	}

	void DisplayGraph::DrawArrow(CDC *dc, CPoint p1, CPoint p2, DWORD color, int nPenStyle)
	{
		DoDrawArrow(dc, p1, p2, color, nPenStyle);
	}

	Pin *DisplayGraph::FindPin(IPin *pin)
	{
		Filter	*filter = FindParentFilter(pin);
		if (!filter) return NULL;
		Pin *ret = filter->FindPin(pin);
		return ret;
	}

	Filter *DisplayGraph::FindParentFilter(IPin *pin)
	{
		Filter	*ret = NULL;
		for (int i=0; i<filters.GetCount(); i++) {
			ret = filters[i];
			if (ret->HasPin(pin)) {
				return ret;
			}
		}
		return NULL;
	}

	void DisplayGraph::DoubleSelected()
	{
		CArray<CLSID>	clsid;
		CArray<CString>	names;
        CArray<LPSTREAM> filterPersistData;
		int				i;

		for (i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			if (filter->selected) {
				clsid.Add(filter->clsid);
				names.Add(filter->name);

                // get persist data to get a real clone
                CComQIPtr<IPersistStream> pI = filter->filter;
                if(!pI)
                {
                    filterPersistData.Add(NULL);
                    continue;
                }

                LPSTREAM lpStream;
                HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &lpStream);
                if(FAILED(hr))
                {
                    filterPersistData.Add(NULL);
                    continue;
                }

                hr = pI->Save(lpStream, TRUE);
                if(FAILED(hr))
                {
                    lpStream->Release();
                    filterPersistData.Add(NULL);
                    continue;
                }

                LARGE_INTEGER pos = {0};
                lpStream->Seek(pos, STREAM_SEEK_SET, NULL);

                filterPersistData.Add(lpStream);
			}
		}

		// now insert them
		for (i=0; i<clsid.GetCount(); i++) {

			// now create an instance of this filter
			CComPtr<IBaseFilter>	instance;
			HRESULT					hr;

			hr = CoCreateInstance(clsid[i], NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
			if (SUCCEEDED(hr)) {
				
				// now check for a few interfaces
				int ret = ConfigureInsertedFilter(instance, names[i]);
				if (ret < 0) {
					instance = NULL;
                    continue;
				}

				if (instance) {
					// add the filter to graph
					hr = AddFilter(instance, names[i]);
					if (FAILED(hr)) {
						// display error message
                        continue;
					}

                    if(filterPersistData[i] && ret == 0)
                    {
                        CComQIPtr<IPersistStream> pI = instance;
                        if(!pI) continue;

                        pI->Load(filterPersistData[i]);
                    }
				}
				instance = NULL;
			}

		}

        // cleanup IPersistStream-Data
        for(i=0;i<filterPersistData.GetCount();i++)
            if(filterPersistData[i])
                filterPersistData[i]->Release();
	}

	// Recurse upstream depth first using visited_filters to only process filters once
	// add leaf filters without connected input pins to inputs array
	static void FindUpstreamInputs(Filter* const filter, std::set<Filter*>& visited_filters, CArray<Filter*>& inputs)
	{
		if (visited_filters.find(filter) != visited_filters.end())		// already visited - nothing to do
			return;
		visited_filters.insert(filter);		// mark this one as visited

		bool connected_inputs = false;
		for (int i=0; i<filter->input_pins.GetCount(); i++) {
			const Pin* const pin = filter->input_pins[i];
			if (pin->connected) {
				connected_inputs = true;
				FindUpstreamInputs(pin->peer->filter, visited_filters, inputs);		// recurse upstream for each connected input
			}
		}
		if (!connected_inputs)
			inputs.Add(filter);		// No connected inputs - add this to the list of input filters
	}

	// Return a subset list of filters in the correct order to place first vertically so ensure
	// filters upstream from multiple connected inputs are in reasonable vertical order with minimal crossing
	static void FindInputFilters(const CArray<Filter*>& all_filters, CArray<Filter*>& input_filters)
	{
		std::set<Filter*> visited_filters;	// maintains record of filters we've already processed and don't have to process again

		//	Recurse upstream depth first from each filter with multiple connected inputs
		for (int i=0; i<all_filters.GetCount(); i++) {
			Filter * const filter = all_filters[i];
			if (filter->NumOfConnectedPins(PINDIR_INPUT) > 1) {
				FindUpstreamInputs(filter, visited_filters, input_filters);
			}
		}
		//	Then recurse upstream from each filter with multiple inputs with only 1 connected
		for (int i=0; i<all_filters.GetCount(); i++) {
			Filter * const filter = all_filters[i];
			if (filter->input_pins.GetCount() > 1
					&& filter->NumOfConnectedPins(PINDIR_INPUT) == 1) {
				FindUpstreamInputs(filter, visited_filters, input_filters);
			}
		}
	}

	// Iterate downstream depth first until we find a filter that's already been assigned a column
	static Filter* FindFirstPositionedDownstreamFilter(Filter* const start_filter)
	{
		if (start_filter->column >= 0)
			return start_filter;				// found

		for (int i=0; i<start_filter->output_pins.GetCount(); i++) {
			Pin * const out_pin = start_filter->output_pins[i];
			if (out_pin->peer) {
				Filter * const positioned_filter = FindFirstPositionedDownstreamFilter(out_pin->peer->filter);
				if (positioned_filter)
					return positioned_filter;	// found
			}
		}
		return NULL;							// not found
	}

	// Recurse depth first down the first connected output pin and find the depth of this chain
	static int DepthOfFirstConnectedOuputChain(Filter *start_filter, int current_depth)
	{
		for (int i=0; i<start_filter->output_pins.GetCount(); i++) {
			Pin * const out_pin = start_filter->output_pins[i];
			if (out_pin->peer)
				return DepthOfFirstConnectedOuputChain(out_pin->peer->filter, current_depth+1);	// found connected output
		}
		return current_depth;
	}

	int DisplayGraph::CalcDownstreamYPosition(Filter* const start_filter) const
	{
		int y = -1;
		int spanned_columns = -1;

		const Filter* const downstream_positioned_filter = FindFirstPositionedDownstreamFilter(start_filter);
		if (downstream_positioned_filter) {
			// Don't position any higher than the first downstream filter that's already positioned
			y = downstream_positioned_filter->posy;

			spanned_columns = downstream_positioned_filter->column - 1;
		} else {
			spanned_columns = DepthOfFirstConnectedOuputChain(start_filter, 0);
		}

		if (spanned_columns >= columns.GetSize())
			spanned_columns = columns.GetSize() - 1;

		// Don't position any higher than the end of the columns we cross
		for (int col=0; col<=spanned_columns; col++) {
			y = max(y, columns[col].y);
		}

		return y;
	}

	// do some nice automatic filter placement
	void DisplayGraph::SmartPlacement()
	{
		// Reset all columns and placement information cached in Filters
		columns.RemoveAll();

		for (int i=0; i<filters.GetCount(); i++) {
			Filter	* const filter = filters[i];
			filter->Refresh();

			// reset placement helpers
			filter->column = -1;	// flag not placed in column
			filter->posy = 0;
			filter->posx = 0;
		}

		// Load connections between filters
		for (int i=0; i<filters.GetCount(); i++) {
			filters[i]->LoadPeers();
		}

		// Deal with filters that have multiple inputs to prevent crossing input lines
		// Find upstream inputs from filters with multiple connected inputs in depth-first, first input pin order 
		CArray<Filter*> input_filters;
		FindInputFilters(filters, input_filters);

		// Position these filters first
		for (int i=0; i<input_filters.GetCount(); i++) {
			Filter	* const filter = input_filters[i];
			if (filter->column < 0) {
				filter->CalculatePlacementChain(0, GRID_SIZE, CalcDownstreamYPosition(filter));
			}
		}

		// Next position remaining filters that have no connected inputs, and some connected outputs
		for (int i=0; i<filters.GetCount(); i++) {
			Filter	* const filter = filters[i];
			if (filter->column < 0
					&& filter->NumOfConnectedPins(PINDIR_INPUT) == 0
					&& filter->NumOfConnectedPins(PINDIR_OUTPUT) > 0) {
				filter->CalculatePlacementChain(0, GRID_SIZE, CalcDownstreamYPosition(filter));
			}
		}

		const int NUM_GROUPS = 3;
		typedef CArray<Filter*> FilterList;
		FilterList groups[NUM_GROUPS];

		// Divide unconnected filters into groups
		for (int i=0; i<filters.GetCount(); i++) {
			Filter	* const filter = filters[i];
			if (filter->column < 0) {
				int group;
				switch (filter->filter_purpose) {
					case Filter::FILTER_SOURCE:		group = 0;	break;
					case Filter::FILTER_RENDERER:	group = 2;	break;
					default:
						group = filter->input_pins.GetCount() == 0 ? 0 : 1;
						break;
				}
				groups[group].Add(filter);
			}
		}

		// number of columns to layout unconnected filters
		const int MIN_ROW_LENGTH = 5;
		int row_length = columns.GetCount() - 1;
		if (row_length < MIN_ROW_LENGTH)
			row_length = MIN_ROW_LENGTH;

		// Position groups
		for (int i=0; i<NUM_GROUPS; i++) {
			PositionRowOfUnconnectedFilters(groups[i], row_length);
		}

		// then set final x values for every filter
		for (int i=0; i<filters.GetCount(); i++) {
			Filter	* const filter = filters[i];
			ASSERT(filter->column >= 0);
			filter->column = max(0, filter->column);		// sanity check on depth
			CPoint	&pt     = columns[filter->column];
			filter->posx = pt.x;
		}
	}

	void DisplayGraph::PositionRowOfUnconnectedFilters(const CArray<Filter*> & unconnected, int max_row_length)
	{
		while (max_row_length >= columns.GetCount()) {
			columns.Add(CPoint(GRID_SIZE, GRID_SIZE));	// add columns if needed
		}

		int filter_index = 0;
		while (true) {

			int num_columns = unconnected.GetCount() - filter_index;
			if (num_columns <= 0)
				break;
			else if (num_columns > max_row_length)			// divide into more than one row
				num_columns = max_row_length;

			int y_pos = GRID_SIZE;
			for (int col=0; col<num_columns; col++)		// find y position for row
				y_pos = max(y_pos, columns[col].y);

			for (int col=0; col<num_columns; col++) {
				Filter* const filter = unconnected[filter_index];

				filter->column = col;
				filter->posx = columns[col].x;
				filter->posy = y_pos;
				columns[col].y = NextGridPos(y_pos + filter->height + g_filterYGap);	// update y position of bottom of column

				if (col+1 < columns.GetCount()) {		// if current column too narrow reposition all following columns	
					const int next_filter_x = NextGridPos(columns[col].x + filter->width + g_filterXGap);

					if (next_filter_x > columns[col+1].x) {
						const int dif = next_filter_x - columns[col+1].x;
						for (int i=col+1; i<columns.GetCount(); i++) {
							columns[i].x += dif;
						}
					}
				}
				filter_index++;
			}
		}
	}

	//-------------------------------------------------------------------------
	//
	//	Filter class
	//
	//-------------------------------------------------------------------------

	Filter::Filter(DisplayGraph *parent)
		: column(-1)
	{
		graph = parent;
		params = (graph != NULL ? graph->params : NULL);
		clsid = CLSID_VideoMixingRenderer9;
		filter = NULL;
		posx = 0;
		posy = 0;
		selected = false;
		basic_audio = NULL;
		clock = NULL;
		videowindow = NULL;
		overlay_icon_active = -1;
	}

	Filter::~Filter()
	{
		Release();
	}

	void Filter::Release()
	{
		if (videowindow) {
			delete videowindow;
			videowindow = NULL;
		}

		RemovePins();
		ReleaseIcons();
		name.Empty();

		basic_audio = NULL;
		clock = NULL;
		filter = NULL;

		overlay_icon_active = -1;
	}

	void Filter::RemovePins()
	{
		int i=0;
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *pin = input_pins[i];
			delete pin;
		}
		input_pins.RemoveAll();
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			delete pin;
		}
		output_pins.RemoveAll();
	}

	int Filter::NumOfDisconnectedPins(PIN_DIRECTION dir)
	{
		int ret = 0;
		if (dir == PINDIR_INPUT) {
			for (int i=0; i<input_pins.GetCount(); i++) {
				Pin *pin = input_pins[i];
				if (!pin->IsConnected()) ret++;
			}
		} else
		if (dir == PINDIR_OUTPUT) {
			for (int i=0; i<output_pins.GetCount(); i++) {
				Pin *pin = output_pins[i];
				if (!pin->IsConnected()) ret++;
			}
		}
		return ret;
	}

	int Filter::NumOfConnectedPins(PIN_DIRECTION dir)
	{
		int ret = 0;
		if (dir == PINDIR_INPUT) {
			for (int i=0; i<input_pins.GetCount(); i++) {
				Pin *pin = input_pins[i];
				if (pin->IsConnected()) ret++;
			}
		} else
		if (dir == PINDIR_OUTPUT) {
			for (int i=0; i<output_pins.GetCount(); i++) {
				Pin *pin = output_pins[i];
				if (pin->IsConnected()) ret++;
			}
		}
		return ret;
	}

	void Filter::Refresh()
	{
		LoadFromFilter(filter);
	}

	void Filter::RemoveSelectedConnections()
	{
		int i;
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			if (pin->selected) {
				pin->Disconnect();
			}
		}
	}

	void Filter::RemoveFromGraph()
	{
		// now all our connections need to be broken
		int i;
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			if (pin->connected) pin->Disconnect();
		}
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *pin = input_pins[i];
			if (pin->connected) pin->Disconnect();
		}

		// we can now try to remove us from the graph
		graph->gb->RemoveFilter(filter);
		selected = false;
	}

	void Filter::UpdateClock()
	{
		bool	we_are_sync_source = false;

		if (filter && clock) {
			CComPtr<IReferenceClock>	syncclock;
			syncclock = NULL;

			HRESULT hr = filter->GetSyncSource(&syncclock);
			if (SUCCEEDED(hr)) {
				if (syncclock == clock) {
					we_are_sync_source = true;
				}
			}
		}

		// update icon
		for (int i=0; i<overlay_icons.GetCount(); i++) {
			OverlayIcon	*icon = overlay_icons[i];
			if (icon->id == OverlayIcon::ICON_CLOCK) {
				icon->state = (we_are_sync_source ? 1 : 0);
			}
		}
	}

	void Filter::LoadFromFilter(IBaseFilter *f)
	{
		if (f != filter) {
			Release();

			// keep a reference
			filter = f;

			if (params) {
				width = params->min_filter_width;
				height = params->min_filter_height;
			}
		}
		if (!filter || !f) return ;

		// get the filter CLSID
		f->GetClassID(&clsid);
        NameGuid(clsid, clsid_str, CgraphstudioApp::g_showGuidsOfKnownTypes);

		FILTER_INFO	info;
		memset(&info, 0, sizeof(info));
		filter->QueryFilterInfo(&info);

		// load name
		name = CString(info.achName);
		if (name == _T("")) name = _T("(Unnamed filter)");

		display_name = name;

		//---------------------------------------------------------------------
		//	Check FileSource & FileSink
		//---------------------------------------------------------------------
		CComPtr<IFileSourceFilter>	fs;
		CComPtr<IFileSinkFilter>	fsink;
		HRESULT						hr;
		CString						url_name = _T("");

		hr = f->QueryInterface(IID_IFileSourceFilter, (void**)&fs);
		if (SUCCEEDED(hr)) {
			LPOLESTR	url_name_ole = NULL;
            CMediaType media_type;
			hr = fs->GetCurFile(&url_name_ole, &media_type);
			if (SUCCEEDED(hr)) {
				url_name = CString(url_name_ole);
				CoTaskMemFree(url_name_ole);
			}
			fs = NULL;

			// get from the registry
			GetObjectName(clsid, display_name);

			// sometimes the FILTER_INFO contains 128-char long truncated file name instead of the proper filter name
			if (clsid == CLSID_AsyncReader) {
				name = _T("File Source (Async.)");
			}
		}
		hr = f->QueryInterface(IID_IFileSinkFilter, (void**)&fsink);
		if (SUCCEEDED(hr)) {
			LPOLESTR	url_name_ole = NULL;
            CMediaType media_type;
			hr = fsink->GetCurFile(&url_name_ole, &media_type);
			if (SUCCEEDED(hr)) {
				url_name = CString(url_name_ole);
				CoTaskMemFree(url_name_ole);
			}
			fsink = NULL;

			// get from the registry
			GetObjectName(clsid, display_name);
		}
		file_name = url_name;

		if (params) {
			if (params->display_file_name && url_name != _T("")) {
				CPath		path(url_name);
				int fstart = path.FindFileName();
				if (fstart >= 0) {
					url_name.Delete(0, fstart);
					if (url_name != ""){
						display_name = url_name;
					}
				}
			}
		}

		// automatically rename the video window so it would be easy to
		// recognize
		CComPtr<IVideoWindow>		vw;
		hr = f->QueryInterface(IID_IVideoWindow, (void**)&vw);
		if (SUCCEEDED(hr)) {
			CString		vw_name;
			vw_name = _T("ActiveMovie Window: ") + display_name;
			vw->put_Caption(vw_name.GetBuffer());
			vw = NULL;
		}

		// check for basic audio interface
		basic_audio = NULL;
		hr = f->QueryInterface(IID_IBasicAudio, (void**)&basic_audio);
		if (FAILED(hr)) {
			basic_audio = NULL;
		}

		clock = NULL;
		hr = f->QueryInterface(IID_IReferenceClock, (void**)&clock);
		if (FAILED(hr)) {
			clock = NULL;
		}

		// overlay icons
		ReleaseIcons();
		CreateIcons();
		overlay_icon_active = -1;
		UpdateClock();

		// now scan for pins
		IEnumPins	*epins;
		IPin		*pin;
		ULONG		ff;
		filter->EnumPins(&epins);
		epins->Reset();
		RemovePins();
		while (epins->Next(1, &pin, &ff) == NOERROR) {
			PIN_DIRECTION	dir;
			pin->QueryDirection(&dir);
			LoadPin(pin, dir);
			pin->Release();

#if 0
			CComPtr<IAMStreamConfig>	cfg;
			HRESULT						hhr;
			hhr = pin->QueryInterface(IID_IAMStreamConfig, (void**)&cfg);
			if (SUCCEEDED(hhr)) {

				int	count, size;
				cfg->GetNumberOfCapabilities(&count, &size);
				if (size == sizeof(VIDEO_STREAM_CONFIG_CAPS)) {
					VIDEO_STREAM_CONFIG_CAPS	caps;
					AM_MEDIA_TYPE				*pmt = NULL;

					cfg->GetStreamCaps(0, &pmt, (BYTE*)&caps);
					if (pmt) {
						DeleteMediaType(pmt);
					}

				}

			}

#endif
		}
		epins->Release();

		connected = false;
		int i;
		for (i=0; i<input_pins.GetCount(); i++) {
			if (input_pins[i]->connected) {
				connected = true;
				break;
			}
		}
		if (!connected) {
			for (i=0; i<output_pins.GetCount(); i++) {
				if (output_pins[i]->connected) {
					connected = true;
					break;
				}
			}
		}

		filter_purpose = GetFilterPurpose(this);

		// this will be okay for now...
		filter_type = Filter::FILTER_STANDARD;

		// if it's a DMO Wrapper we'll identify it as DMO
		if (clsid == CLSID_DMOWrapperFilter) {
			filter_type = Filter::FILTER_DMO;
		} else
		if (clsid == DSUtil::CLSID_KSProxy) {
			filter_type = Filter::FILTER_WDM;
		}

		// check for Enhanced Video Renderer
		if (graph && (graph->is_remote == false)) {
			if (clsid == CLSID_EnhancedVideoRenderer) {
				int ret;
				if (videowindow) { delete videowindow; videowindow = NULL; }

				// try to initialize the video window object
				videowindow = new EVR_VideoWindow();
				ret = videowindow->Open(this);
				if (ret < 0) {
					delete videowindow;
					videowindow = NULL;
				}
			}		
		}

		//---------------------------------------------------------------------
		// calculate size
		//---------------------------------------------------------------------		
		if (graph) {
			graph->dc->SelectObject(&params->font_filter);
			CSize	size = graph->dc->GetTextExtent(display_name);
			size.cx += 2 * 24;
			width = (size.cx + 15) &~ 0x0f;		if (width < params->min_filter_width) width = params->min_filter_width;
			height = (size.cy + 15) &~ 0x0f;	if (height < params->min_filter_height) height = params->min_filter_height;

			int		maxpins = max(input_pins.GetCount(), output_pins.GetCount());
			int		minsize = (((1 + maxpins)*params->pin_spacing) + (params->pin_spacing/2)) &~ 0x0f;
			if (height < minsize) height = minsize;
		}

		// we don't need it anymore
		if (info.pGraph) info.pGraph->Release();
	}

	bool Filter::HasPin(IPin *pin)
	{
		return (FindPin(pin) != NULL);
	}

	Pin *Filter::FindPin(IPin *pin)
	{
		int i;
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *p = input_pins[i];
			if (p->pin == pin) return p;
		}
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *p = output_pins[i];
			if (p->pin == pin) return p;
		}
		return NULL;
	}

	void Filter::LoadPin(IPin *pin, PIN_DIRECTION dir)
	{
		Pin	*npin = new Pin(this);
		if (npin->Load(pin) < 0) {
			delete npin;
			return ;
		}
	
		// to the proper pool
		if (dir == PINDIR_INPUT) input_pins.Add(npin); else 
		if (dir == PINDIR_OUTPUT) output_pins.Add(npin);
	}

	void Filter::LoadPeers()
	{
		for (int i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			pin->LoadPeer();
		}
	}

	// Helpers
	bool Filter::IsSource()
	{
		if (input_pins.GetCount() == 0 && output_pins.GetCount() > 0) return true;
		return false;
	}

	bool Filter::IsRenderer()
	{
		if (input_pins.GetCount() > 0 && output_pins.GetCount() == 0) return true;
		return false;
	}

	void DoDrawArrow(CDC *dc, CPoint p1, CPoint p2, DWORD color, int nPenStyle)
	{
		const	int connection_width = 2;
		CPen	pen(nPenStyle, connection_width, color);
        CPen    penArrow(PS_SOLID, connection_width, color);
		CBrush	brush(color);

		// direction vector
		double	vx, vy;
		if (p1 == p2) { vx = 0;	vy = 1.0; } else {
			vx = (p2.x - p1.x);
			vy = (p2.y - p1.y);
		}

		// vector length
		double	vs = sqrt(vx*vx + vy*vy); vx /= vs;	vy /= vs;
		double	arrow_size = 8;

		// find the midpoint
		double	mx, my;
		mx = p2.x - arrow_size * vx;
		my = p2.y - arrow_size * vy;

		// find the arrow points
		double	tx, ty;
		tx = vy;
		ty = -vx;

		CPoint	a1(mx + (tx*arrow_size/2.4), my + (ty*arrow_size/2.4));
		CPoint	a2(mx - (tx*arrow_size/2.4), my - (ty*arrow_size/2.4));

        dc->SetBkMode(TRANSPARENT);
        dc->SelectObject(&pen);
		dc->MoveTo(p1);
		dc->LineTo(p2);

        dc->SelectObject(&brush);
		dc->SelectObject(&penArrow);
        POINT	pts[3] = { a1, a2, p2 };
		dc->Polygon((const POINT*)&pts, 3);
	}

	void Filter::DrawConnections(CDC *dc)
	{		
		// render directed arrows for all connected output pins
		for (int i=0; i<output_pins.GetCount(); i++) {
			Pin		*pin = output_pins[i];
			Pin		*peer = pin->peer;

			if (pin && peer) {
                DWORD	color = RenderParameters::color_connection_type[pin->connectionType];
				CPoint	pt1, pt2;
				pin->GetCenterPoint(&pt1);
				peer->GetCenterPoint(&pt2);
				if (pin->selected) color = params->color_select;
			
				DoDrawArrow(dc, pt1, pt2, color);
			}
		}
	}

	void Filter::Draw(CDC *dc)
	{
		DWORD	back_color = params->color_filter_type[Filter::FILTER_UNKNOWN];
		if (connected) {
			back_color = params->color_filter_type[filter_type];
		}

		CPen	pen_light(PS_SOLID, 1, params->color_filter_border_light);
		CPen	pen_dark(PS_SOLID, 1, params->color_filter_border_dark);
		CPen	pen_back(PS_SOLID, 1, back_color);
		CBrush	brush_back(back_color);
		dc->SetBkMode(TRANSPARENT);

		CPen	*prev_pen   = dc->SelectObject(&pen_back);
		CBrush	*prev_brush = dc->SelectObject(&brush_back);
		CFont	*prev_font	= dc->SelectObject(&params->font_filter);

		//---------------------------------------------------------------------
		// draw the selection frame
		//---------------------------------------------------------------------
		if (selected) {
			CPen	sel_pen(PS_SOLID, 1, params->color_select);
			CBrush	sel_brush(params->color_select);
			dc->SelectObject(&sel_pen);
			dc->SelectObject(&sel_brush);
			dc->Rectangle(posx-2, posy-2, posx+width+2, posy+height+2);
		}

		//---------------------------------------------------------------------
		// draw the filter background
		//---------------------------------------------------------------------
		dc->SelectObject(&pen_back);
		dc->SelectObject(&brush_back);
		dc->Rectangle(posx, posy, posx+width, posy+height);

		//---------------------------------------------------------------------
		// draw the 3d edge
		//---------------------------------------------------------------------
		dc->SelectObject(&pen_light);
		dc->MoveTo(posx, posy+height-2);		dc->LineTo(posx, posy);		dc->LineTo(posx+width-1, posy);
		dc->MoveTo(posx+1, posy+height-3);	dc->LineTo(posx+1, posy+1);	dc->LineTo(posx+width-2, posy+1);
		dc->MoveTo(posx+width-4, posy+4);	
		dc->LineTo(posx+width-4, posy+height-4);	dc->LineTo(posx+3, posy+height-4);

		dc->SelectObject(&pen_dark);
		dc->MoveTo(posx+0, posy+height-1);	dc->LineTo(posx+width-1, posy+height-1);	dc->LineTo(posx+width-1, posy-1);
		dc->MoveTo(posx+1, posy+height-2);	dc->LineTo(posx+width-2, posy+height-2);	dc->LineTo(posx+width-2, posy);
		dc->MoveTo(posx+3, posy+height-4);	
		dc->LineTo(posx+3, posy+3);	dc->LineTo(posx+width-3, posy+3);

		//---------------------------------------------------------------------
		// draw the font
		//---------------------------------------------------------------------
		CRect	rc(posx, posy+8, posx+width, posy+height);
		dc->DrawText(display_name, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

		dc->SelectObject(prev_pen);
		dc->SelectObject(prev_brush);
		dc->SelectObject(prev_font);

		//---------------------------------------------------------------------
		// draw the pins
		//---------------------------------------------------------------------
		int i, x, y;
		x = posx-5;
		y = posy + params->pin_spacing;
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *pin = input_pins[i];
			pin->Draw(dc, true, x, y);
			y += params->pin_spacing;
		}
		x = posx+width-2;
		y = posy + params->pin_spacing;
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			pin->Draw(dc, false, x, y);
			y += params->pin_spacing;
		}

		//---------------------------------------------------------------------
		// draw overlay icons
		//---------------------------------------------------------------------
		int	ocx		 = 16;
		int ocy		 = 16;
		int	offset	 = 6;

		int	ov_count = overlay_icons.GetCount();

		if (ov_count > 0) {
			CDC		tmp_dc;
			tmp_dc.CreateCompatibleDC(NULL);
			for (i=0; i<ov_count; i++) {
				OverlayIcon	*icon = overlay_icons[i];

				int		ox = posx + width - (ov_count - i)*ocx - offset;
				int		oy = posy + offset;

				if (overlay_icon_active == i) {
					tmp_dc.SelectObject(icon->icon_hover[icon->state]);
				} else {
					tmp_dc.SelectObject(icon->icon_normal[icon->state]);
				}
	
				DWORD	transpColor = tmp_dc.GetPixel(0, 0);		
				dc->TransparentBlt(ox, oy, ocx, ocy, &tmp_dc, 0, 0, ocx, ocy, transpColor);
			}
			tmp_dc.DeleteDC();
		}
	}

	// new_columns is the column_index we're placing this filter in
	// x is the x position we've calculated for this filter
	// y is the y position we've calculated for this filter (or negative for none)
	void Filter::CalculatePlacementChain(int new_column, int x, int y)
	{
		if (new_column > graph->columns.GetCount()) {
			// this is an error case !!
			return ;
		}

		// Create column for this filter if required
		if (new_column == graph->columns.GetCount()) {
			// add one more
			CPoint	pt;
			pt.x = x;
			pt.y = DisplayGraph::GRID_SIZE;
			graph->columns.Add(pt);
		}

		// if not already added, add the next column beyond this one to leave enough space for this filter
		if (new_column == graph->columns.GetCount()-1) {
			CPoint	pt;
			pt.x = DisplayGraph::NextGridPos(x + width + DisplayGraph::g_filterXGap);
			pt.y = DisplayGraph::GRID_SIZE;
			graph->columns.Add(pt);
		}

		// we distribute new values, if they are larger
		// If we're adding to column zero, 
		// OR this filter has been added to column for first time or pushed into a later column than it was already
		// OR this filter's position is further right than the current column
		if (new_column == 0 || new_column > column || x > graph->columns[new_column].x) {
			column = max(column, new_column);

			CPoint& current_column = graph->columns[column];

			if (x > current_column.x) {		// required position of this filter is further right than current column x position
				const int dif = x - current_column.x;
				// move this and all following columns right to line up with this filter
				for (int i=column; i<graph->columns.GetCount(); i++) {
					graph->columns[i].x += dif;
				}
			}

			// Set y position of filter if not set before now
			// and update y position of column for filters added after ths one
			// Don't position ourselves any higher than the upstream filter that positioned us
			if (posy == 0)	{
				posy = DisplayGraph::NextGridPos(max(current_column.y, y));
				current_column.y = DisplayGraph::NextGridPos(posy + height + DisplayGraph::g_filterYGap);
			}

			// calculate position of next column that leaves enough space for this filter
			const int next_column_x = DisplayGraph::NextGridPos(current_column.x + width + DisplayGraph::g_filterXGap);

			if (column+1 < graph->columns.GetCount()
					&& next_column_x > graph->columns[column+1].x) {		// right of this filter is further right than next column x position
				const int dif = next_column_x - graph->columns[column+1].x;
				// move this and all following columns right to line up with this filter
				for (int i=column+1; i<graph->columns.GetCount(); i++) {
					graph->columns[i].x += dif;
				}
			}

			// position downstream filters recursively
			for (int i=0; i<output_pins.GetCount(); i++) {
				Pin *pin = output_pins[i];
				IPin *peer_pin = NULL;
				if (pin && pin->pin) 
					pin->pin->ConnectedTo(&peer_pin);

				if (peer_pin) {
					Filter	* const down_filter = graph->FindParentFilter(peer_pin);
					if (down_filter) {
						down_filter->CalculatePlacementChain(column + 1, next_column_x, posy);
					}
					peer_pin->Release();
				}
			}
		}
	}

	void Filter::SelectConnection(UINT flags, CPoint pt)
	{
		// we check our output pins and test for hit
		for (int i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			Pin *peer = pin->peer;
			if (!pin->connected) continue;

			if (pin && peer) {
				CPoint	pt1, pt2;
				pin->GetCenterPoint(&pt1);
				peer->GetCenterPoint(&pt2);

				bool hit = LineHit(pt1, pt2, pt);
				if (hit) {
					
					// testing ...
					pin->Select(true);
				} else {
					pin->Select(false);
				}
			}

		}
	}

	void Filter::BeginDrag()
	{
		start_drag_pos.x = posx;
		start_drag_pos.y = posy;
	}

	void Filter::VerifyDrag(int *deltax, int *deltay)
	{
		// adjust drag delta so we don't drag less than one grid space from 0
		if (start_drag_pos.x + (*deltax) < DisplayGraph::GRID_SIZE) {
			*deltax = DisplayGraph::GRID_SIZE - start_drag_pos.x;
		}
		if (start_drag_pos.y + (*deltay) < DisplayGraph::GRID_SIZE) {
			*deltay = DisplayGraph::GRID_SIZE - start_drag_pos.y;
		}
	}

	void Filter::Select(bool select)
	{
		selected = select;

		// select our connections as well
		int i;
		for (i=0; i<input_pins.GetCount(); i++) input_pins[i]->Select(select);
		for (i=0; i<output_pins.GetCount(); i++) output_pins[i]->Select(select);
	}

	Pin *Filter::FindPinByID(CString pin_id)
	{
		CArray<Pin*> *lists[] = {&output_pins, &input_pins};
		const size_t num_lists = sizeof(lists)/sizeof(lists[0]);

		CComPtr<IPin> ipin;
		HRESULT hr = filter->FindPin(pin_id, &ipin);

		for (CArray<Pin*> ** pins = lists; pins<lists+num_lists; pins++) {
			for (int p=0; p<(**pins).GetCount(); p++) {
				Pin * const pin = (**pins)[p];
				if (pin->pin == ipin) 
					return pin;
			}
		}

		// Alternative Pin ID search - don't ask filter
		// Work around for buggy filters where IBaseFilter::FindPin returns wrong pin
		//CArray<Pin*> *lists[] = {&output_pins, &input_pins};
		//const size_t num_lists = sizeof(lists)/sizeof(lists[0]);

		//for (CArray<Pin*> ** pins = lists; pins<lists+num_lists; pins++) {
		//	for (int p=0; p<(**pins).GetCount(); p++) {
		//		Pin * const pin = (**pins)[p];
		//		if (pin->id == pin_id) 
		//			return pin;
		//	}
		//}

		return NULL;
	}

	Pin *Filter::FindPinByName(CString name)
	{
		CArray<Pin*> *lists[] = {&output_pins, &input_pins};
		const size_t num_lists = sizeof(lists)/sizeof(lists[0]);

		for (CArray<Pin*> ** pins = lists; pins<lists+num_lists; pins++) {
			for (int p=0; p<(**pins).GetCount(); p++) {
				Pin * const pin = (**pins)[p];
				if (pin->name == name) 
					return pin;
			}
		}
		return NULL;
	}

	Pin *Filter::FindPinByPos(CPoint p, bool not_connected)
	{
		int i;
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			if (pin->connected && not_connected) continue;

			CPoint	cp;
			pin->GetCenterPoint(&cp);

			float	dist = (float)sqrt((float)((p.x-cp.x)*(p.x-cp.x) + (p.y-cp.y)*(p.y-cp.y)));
			if (dist < 8.0) return pin;
		}
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *pin = input_pins[i];
			if (pin->connected && not_connected) continue;

			CPoint	cp;
			pin->GetCenterPoint(&cp);

			float	dist = (float)sqrt((float)((p.x-cp.x)*(p.x-cp.x) + (p.y-cp.y)*(p.y-cp.y)));
			if (dist < 8.0) return pin;
		}
		return NULL;
	}

	// overlay icons
	void Filter::ReleaseIcons()
	{
		for (int i=0; i<overlay_icons.GetCount(); i++) {
			OverlayIcon *icon = overlay_icons[i];
			if (icon) delete icon;
		}
		overlay_icons.RemoveAll();
		overlay_icon_active = -1;
	}

	void Filter::CreateIcons()
	{
		ReleaseIcons();
		if (!params) return ;
		
		// reference clock ?
		if (clock) {
			OverlayIcon *icon = new OverlayIcon(this, OverlayIcon::ICON_CLOCK);
			icon->icon_normal[0] = &params->bmp_clock_inactive_lo;
			icon->icon_hover[0]  = &params->bmp_clock_inactive_hi;
			icon->icon_normal[1] = &params->bmp_clock_active_lo;
			icon->icon_hover[1]  = &params->bmp_clock_active_hi;

			overlay_icons.Add(icon);
		}

		// filter supports basic audio 
		if (basic_audio) {

			OverlayIcon	*icon = new OverlayIcon(this, OverlayIcon::ICON_VOLUME);
			icon->icon_normal[0] = &params->bmp_volume_lo;
			icon->icon_hover[0]  = &params->bmp_volume_hi;
			icon->icon_normal[1] = &params->bmp_volume_lo;
			icon->icon_hover[1]  = &params->bmp_volume_hi;

			overlay_icons.Add(icon);
		}
	}

	int Filter::CheckIcons(CPoint pt)
	{
		// check icon regions
		int i;
		int	offset	= 6;
		int	ocx		= 16;
		int ocy		= 16;
		int	oy		= posy + offset;

		int	ov_count = overlay_icons.GetCount();
		if (ov_count <= 0) {
			return -1;
		}

		// not in vertical range
		if (pt.y < oy || pt.y >= (oy+ocy)) {
			overlay_icon_active = -1;
			return -1;
		}

		for (i=0; i<ov_count; i++) {
			OverlayIcon	*icon = overlay_icons[i];

			int		ox = posx + width - (ov_count - i)*ocx - offset;

			if (pt.x >= ox && pt.x < ox+ocx) {
				overlay_icon_active = i;
				return i;
			}
		}

		overlay_icon_active = -1;
		return overlay_icon_active;
	}


	//-------------------------------------------------------------------------
	//
	//	Pin class
	//
	//-------------------------------------------------------------------------
	Pin::Pin(Filter *parent) :
		params(parent ? parent->params : NULL),
		filter(parent),
		peer(NULL),
		dir(PINDIR_INPUT),
		connected(false),
		selected(false)
	{
	}

	Pin::~Pin()
	{
		pin = NULL;
	}

	int Pin::Disconnect()
	{
		if (!connected || !peer) 
			return S_OK;

		// we need to disconnect both pins
		HRESULT hr = filter->graph->gb->Disconnect(peer->pin);
		if (FAILED(hr)) 
			return hr;

		hr = filter->graph->gb->Disconnect(pin);
		if (FAILED(hr)) 
			return hr;

		// clear variables
		peer->peer = NULL;
		peer->selected = false;
		peer->connected = false;
		peer = NULL;
		connected = false;
		selected = false;

		// we're okay
		return S_OK;
	}

	void Pin::Select(bool select)
	{
		selected = select;
		if (peer) {
			peer->selected = select;
		}
	}

	void Pin::GetCenterPoint(CPoint *pt)
	{
		// find out our index
		int	index = 0;
		if (dir == PINDIR_INPUT) {
			for (int i=0; i<filter->input_pins.GetCount(); i++) {
				if (filter->input_pins[i] == this) {
					index = i;
					break;
				}
			}
		} else {
			for (int i=0; i<filter->output_pins.GetCount(); i++) {
				if (filter->output_pins[i] == this) {
					index = i;
					break;
				}
			}
		}

		// calculate X and Y
		pt->y = filter->posy + (1+index)*params->pin_spacing + 4;
		if (dir == PINDIR_INPUT) {
			pt->x = filter->posx - 1;
		} else {
			pt->x = filter->posx + filter->width;
		}
	}

	int Pin::Load(IPin *pin)
	{
		if (this->pin) {
			this->pin = NULL;
		}
		if (!pin) return -1;

		// keep a reference
		pin->QueryInterface(IID_IPin, (void**)&this->pin);

		IPin *peerpin = NULL;
		pin->ConnectedTo(&peerpin);
		connected = (peerpin != NULL);
		if (peerpin) 
			peerpin->Release();

		PIN_INFO	info;
		memset(&info, 0, sizeof(info));
		pin->QueryPinInfo(&info);
		dir = info.dir;

		// find out name
		name = CString(info.achName);
		if (info.pFilter) 
			info.pFilter->Release();

		LPOLESTR idStr = NULL;
		if (SUCCEEDED(pin->QueryId(&idStr)) && idStr) {
			id = CString(idStr);
		}
		if (idStr)
			CoTaskMemFree(idStr);

        // Check Pin Media Type
        connectionType = PIN_CONNECTION_TYPE_OTHER;
        CMediaType pinMediaType;
        DSUtil::MediaTypes pinMtList;

        if (connected)
            pin->ConnectionMediaType(&pinMediaType);
        else if (SUCCEEDED(DSUtil::EnumMediaTypes(pin, pinMtList)))
        {
            for (int i=0;i<pinMtList.GetCount();i++)
                if (pinMtList[i].majortype != MEDIATYPE_NULL)
                {
                    pinMediaType = pinMtList[i];
                    break;
                }
        }

        if (pinMediaType.IsValid())
        {
            if (pinMediaType.majortype == MEDIATYPE_Stream)
                connectionType = PIN_CONNECTION_TYPE_STREAM;
            else if (pinMediaType.majortype == MEDIATYPE_Audio)
                connectionType = PIN_CONNECTION_TYPE_AUDIO;
            else if (pinMediaType.majortype == MEDIATYPE_Video)
                connectionType = PIN_CONNECTION_TYPE_VIDEO;
            else if (pinMediaType.majortype == MEDIATYPE_Subtitle)
                connectionType = PIN_CONNECTION_TYPE_SUBTITLE;
            else if (pinMediaType.majortype == MEDIATYPE_Interleaved)
                connectionType = PIN_CONNECTION_TYPE_MIXED;
        }

		return 0;
	}

	void Pin::LoadPeer()
	{
		IPin *p = NULL;
		pin->ConnectedTo(&p);
		if (p) {
			// find the peer pin
			peer = filter->graph->FindPin(p);

			// backward link
			if (peer) {
				peer->peer = this;
			}

			p->Release();
		} else {
			peer = NULL;
		}
	}

	bool Pin::IsConnected()
	{
		if (!pin) return false;
		IPin *peer = NULL;
		if (SUCCEEDED(pin->ConnectedTo(&peer))) {
			if (peer) {
				peer->Release();
				return true;
			}
		}
		return false;
	}

	void Pin::Draw(CDC *dc, bool input, int x, int y)
	{
		CPen	pen_light(PS_SOLID, 1, params->color_filter_border_light);
		CPen	pen_dark(PS_SOLID, 1, params->color_filter_border_dark);
		CPen	pen_back(PS_SOLID, 1, params->color_filter_type[0]);
		CBrush	brush_back(params->color_filter_type[0]);

		int		pinsize = 5;
		dc->SelectObject(&params->font_pin);
		CSize	size = dc->GetTextExtent(name);

		if (input) {
			dc->SelectObject(&pen_dark);
			dc->MoveTo(x+1, y+2+pinsize);		dc->LineTo(x+1+pinsize+1, y+2+pinsize);
			dc->MoveTo(x,   y+2+pinsize+1);		dc->LineTo(x+1+pinsize,	y+2+pinsize+1);

			dc->SelectObject(&pen_light);
			dc->MoveTo(x+1, y+2+pinsize-1);		dc->LineTo(x+1, y+1);	dc->LineTo(x+pinsize, y+1);
			dc->MoveTo(x,   y+2+pinsize);		dc->LineTo(x, y);		dc->LineTo(x+pinsize, y);

			dc->SelectObject(&pen_back);
			dc->SelectObject(&brush_back);
			dc->Rectangle(x+2, y+2, x+2+pinsize, y+2+pinsize);

			// dot in the middle
			dc->SelectObject(&pen_dark);
			dc->SetPixel(x+2+pinsize/2, y+pinsize/2 + 2, params->color_filter_border_dark);

			// pin name
			CRect	rc(x+pinsize+6, y - 10, x+pinsize+6+size.cx, y + 4+pinsize + 10);
			dc->DrawText(name, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		} else {
			dc->SelectObject(&pen_light);
			dc->MoveTo(x+1, y);		dc->LineTo(x+1 + pinsize, y);
			dc->MoveTo(x, y+1);		dc->LineTo(x+pinsize,	  y+1);
			dc->SelectObject(&pen_dark);
			dc->MoveTo(x, y+2+pinsize+1);		
			dc->LineTo(x+1+pinsize, y+2+pinsize+1);
			dc->LineTo(x+1+pinsize, y-1);
			dc->MoveTo(x, y+2+pinsize);			
			dc->LineTo(x+1+pinsize-1, y+2+pinsize);
			dc->LineTo(x+1+pinsize-1, y);

			dc->SelectObject(&pen_back);
			dc->SelectObject(&brush_back);
			dc->Rectangle(x, y+2, x+pinsize, y+2+pinsize);

			// dot in the middle
			dc->SelectObject(&pen_dark);
			dc->SetPixel(x+pinsize/2, y+pinsize/2 + 2, params->color_filter_border_dark);

			// pin name
			CRect	rc(x-4-size.cx, y - 10, x-4, y + 4+pinsize + 10);
			dc->DrawText(name, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

	}


	//-------------------------------------------------------------------------
	//
	//	OverlayIcon class
	//
	//-------------------------------------------------------------------------
	OverlayIcon::OverlayIcon(Filter *parent, int icon_id) :
		filter(parent),
		id(icon_id),
		state(0)
	{
		icon_normal[0] = NULL;
		icon_normal[1] = NULL;
		icon_hover[0] = NULL;
		icon_hover[1] = NULL;
	}

	OverlayIcon::~OverlayIcon()
	{
		// TODO:
	}



	typedef struct {
		float x;
		float y;
	} FlPoint;

	float Magnitude(FlPoint *p1, FlPoint *p2)
	{
		FlPoint v;
		v.x = (p2->x - p1->x);
		v.y = (p2->y - p1->y);
		return (float)sqrt(v.x*v.x + v.y*v.y);
	}

	int DistancePointLine(FlPoint *p1, FlPoint *p2, FlPoint *hit, float *distance)
	{
		float mag = Magnitude(p1, p2);
		float u;
		u = ( ( (hit->x - p1->x) * (p2->x - p1->x) ) +
			  ( (hit->y - p1->y) * (p2->y - p1->y) ) ) /
		    (mag*mag);
		if (u < 0.0 || u > 1.0) return -1;

		FlPoint intersect;
		intersect.x = p1->x + u*(p2->x - p1->x);
		intersect.y = p1->y + u*(p2->y - p1->y);
		*distance = Magnitude(hit, &intersect);
		return 0;
	}

	bool LineHit(CPoint p1, CPoint p2, CPoint hit)
	{
		// we calculate the distance of the "hit" point
		// from the line defined by p1,p2
		// if it's close enough we call it a hit
		if (p1 == p2) return false;

		FlPoint f1, f2, h;
		f1.x = p1.x;	f1.y = p1.y;
		f2.x = p2.x;	f2.y = p2.y;
		h.x = hit.x;	h.y = hit.y;

		float dist;
		if (DistancePointLine(&f1, &f2, &h, &dist) < 0) return false;
		if (dist < 3.5) return true;
		return false;
	}


	//-------------------------------------------------------------------------
	//
	//	DisplayGraph callback classes
	//
	//-------------------------------------------------------------------------

	GraphCallbackImpl::GraphCallbackImpl(LPUNKNOWN punk, HRESULT *phr, GraphStudio::DisplayGraph *parent) :
		CUnknown(_T("Graph Callback"), punk),
		graph(parent)
	{
		if (phr) *phr = NOERROR;
	}

	GraphCallbackImpl::~GraphCallbackImpl()
	{
		// nothing yet
	}

	STDMETHODIMP GraphCallbackImpl::NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_IAMGraphBuilderCallback) {
			return GetInterface((IAMGraphBuilderCallback*)this, ppv);
		} else
			return __super::NonDelegatingQueryInterface(riid, ppv);
	}

	STDMETHODIMP GraphCallbackImpl::SelectedFilter(IMoniker *pMon)
	{
		HRESULT	hr;

		if (!graph->params) return NOERROR;

		/**********************************************************************
			Check for never-ending render operation.
		***********************************************************************/

		// we've already stop accepting filters - graph builder will run out of options
		// and the operation will stop
		if (!graph->params->render_can_proceed) {
			return VFW_E_TIMEOUT;
		}

		bool timeout = false;
		
		if (graph->params->abort_timeout && graph->params->in_render) {

			DWORD		timenow = GetTickCount();

			if (graph->params->render_can_proceed &&
				(timenow > (graph->params->render_start_time + 10*1000))
				) {
	
				// TODO: perhaps display some error message
				graph->params->render_can_proceed = false;
				timeout = true;

			}
		}

		/**********************************************************************
			List of filters used in a render operation
		***********************************************************************/
		
		bool filter_blacklisted = false;
		RenderAction	ra;

		// moniker name
		LPOLESTR	moniker_name;
		hr = pMon->GetDisplayName(NULL, NULL, &moniker_name);
		if (SUCCEEDED(hr)) {
			ra.displ_name = CString(moniker_name);

			filter_blacklisted = CFavoritesForm::GetBlacklistedFilters()->ContainsMoniker(ra.displ_name);

			if (filter_blacklisted)
				ra.type	= RenderAction::ACTION_REJECT;
			else if (timeout)
				ra.type	= RenderAction::ACTION_TIMEOUT;
			else
				ra.type	= RenderAction::ACTION_SELECT;

			IMalloc *alloc = NULL;
			hr = CoGetMalloc(1, &alloc);
			if (SUCCEEDED(hr)) {
				alloc->Free(moniker_name);
				alloc->Release();
			}
			ra.time_ms = GetTickCount() - graph->params->render_start_time;

			if (ra.displ_name != _T("")) {
				graph->params->render_actions.push_back(ra);
			}
		}

		// check for preferred video filter
		/*
			Hm :( This does not work as I hoped. Until I find
			some other way I'll leave this disabled
		*/
	#if 0
		if (graph->params) {
			if (graph->params->preferred_video_renderer != _T("")) {

				// get the moniker name for the filter
				LPOLESTR	moniker_name;
				hr = pMon->GetDisplayName(NULL, NULL, &moniker_name);
				if (SUCCEEDED(hr)) {
					CString		name = CString(moniker_name);
					IMalloc		*alloc = NULL;
					hr = CoGetMalloc(1, &alloc);
					if (SUCCEEDED(hr)) { alloc->Free(moniker_name);	alloc->Release(); }

					// if the name matches the preferred, we're good to go
					if (name == graph->params->preferred_video_renderer) return NOERROR;

					// if it's in the list of renderers, we will ignore this filter
					if (graph->params->video_renderers) {						
						for (int i=0; i<graph->params->video_renderers->filters.GetCount(); i++) {
							DSUtil::FilterTemplate	&filter = graph->params->video_renderers->filters[i];
							if (filter.moniker_name == name) {
								return E_FAIL;
							}
						}
					}
				}
			}
		}
	#endif

		if (timeout)
			return VFW_E_TIMEOUT;
		else if (filter_blacklisted)
			return E_FAIL;
		else
			return S_OK;
	}

	STDMETHODIMP GraphCallbackImpl::CreatedFilter(IBaseFilter *pFilter)
	{

		/**********************************************************************
			List of filters used in a render operation
		***********************************************************************/
		if (graph->params->in_render) {
			RenderAction	ra;

			// moniker name
			CLSID		clsid;
			pFilter->GetClassID(&clsid);

			ra.time_ms = GetTickCount() - graph->params->render_start_time;
			ra.type = RenderAction::ACTION_CREATE;
			ra.clsid = clsid;

			graph->params->render_actions.push_back(ra);
		}

		return NOERROR;
	}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation


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

#include <set>

#pragma warning(disable: 4244)			// DWORD -> BYTE warning

namespace GraphStudio
{
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

	//-------------------------------------------------------------------------
	//
	//	DisplayGraph class
	//
	//-------------------------------------------------------------------------

	DisplayGraph::DisplayGraph()
	{
		callback = NULL;
        params = NULL;

		graph_name = _T("Graph");

		mc = NULL;
		me = NULL;
		ms = NULL;
		gb = NULL;
		cgb = NULL;
		dc = NULL;
		fs = NULL;
		is_remote = false;
		is_frame_stepping = false;
		uses_clock = true;
        rotRegister = 0;

		HRESULT			hr = NOERROR;
		graph_callback = new GraphCallbackImpl(NULL, &hr, this);
		graph_callback->NonDelegatingAddRef();

		MakeNew();

		dirty = true;
	}

	DisplayGraph::~DisplayGraph()
	{
		mc = NULL;
		me = NULL;
		fs = NULL;
		if (ms) ms = NULL;
		cgb = NULL;
		gb = NULL;

		if (graph_callback) {
			graph_callback->NonDelegatingRelease();
			graph_callback = NULL;
		}

		ZeroTags();
		RemoveUnusedFilters();
	}

	// caller must clean up graph if error returned
	int DisplayGraph::ConnectToRemote(IFilterGraph *remote_graph)
	{
		int ret = MakeNew();
		if (ret < 0) return -1;

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
		gb = NULL;
		cgb = NULL;
		is_remote = false;
		is_frame_stepping = false;
		uses_clock = true;

		// attach remote graph
		HRESULT hr;
		hr = remote_graph->QueryInterface(IID_IGraphBuilder, (void**)&gb);
		if (FAILED(hr)) return -1;

		// get hold of interfaces
		gb->QueryInterface(IID_IMediaControl, (void**)&mc);
		gb->QueryInterface(IID_IMediaSeeking, (void**)&ms);
		gb->QueryInterface(IID_IVideoFrameStep, (void**)&fs);

		// now we're a remote graph
		is_remote = true;
        if(params) params->is_remote = true;
		return 0;
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

			ZeroTags();
			RemoveAllFilters();
			RemoveUnusedFilters();
			bins.RemoveAll();
			gb = NULL;
			cgb = NULL;
		} else {
			mc = NULL;
			me = NULL;

			ZeroTags();
			RemoveUnusedFilters();
			bins.RemoveAll();
			gb = NULL;
			cgb = NULL;
		}

		is_remote = false;
        if(params) params->is_remote = false;
		is_frame_stepping = false;
		uses_clock = true;

		graph_name = _T("Graph");

		// create new instance of filter graph
		HRESULT hr;
		do {
			hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&gb);
			if (FAILED(hr)) break;

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

    void DisplayGraph::AddToRot()
    {
        IMoniker * pMoniker = NULL;
        IRunningObjectTable *pROT = NULL;

        if (FAILED(GetRunningObjectTable(0, &pROT))) 
            return;
    
        const size_t STRING_LENGTH = 256;

        WCHAR wsz[STRING_LENGTH];
 
        StringCchPrintfW(
            wsz, STRING_LENGTH, 
            L"FilterGraph GraphStudioNext pid %08x",  
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

	void DisplayGraph::RemoveAllFilters()
	{
		// we find all filters, disconnect them and remove from graph
		for (int i=0; i<filters.GetCount(); i++) {
			if (callback) callback->OnFilterRemoved(this, filters[i]);
			filters[i]->DeleteFilter();
		}
		RefreshFilters();
		Dirty();
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

		// 8-pixel alignment
		maxx = (maxx+7) &~ 0x07; maxx += 8;
		maxy = (maxy+7) &~ 0x07; maxy += 8;

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
		// notify all EVR windows
		if (!is_remote) {
			for (int i=0; i<filters.GetCount(); i++) {
				Filter	*filter = filters[i];
				if (filter->videowindow) {
					filter->videowindow->Start();
				}
			}
		}

		if (is_frame_stepping) {
			if (fs) fs->CancelStep();
			if (mc) mc->Run();

			// reset the frame stepping flag
			is_frame_stepping = false;
		} else {
			if (mc) {
				return mc->Run();
			} else {
				return E_NOINTERFACE;
			}
		}

		return NOERROR;
	}

	HRESULT DisplayGraph::DoStop()
	{
		if (is_frame_stepping) {
			if (fs) fs->CancelStep();
			is_frame_stepping = false;
		}

		if (mc) {		
			Seek(0);
			mc->Stop();
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
		if (!ms) return -1;

		REFERENCE_TIME	rtpos = time_ms * 10000;
		DWORD			flags = AM_SEEKING_AbsolutePositioning;

		if (keyframe) {
			flags |= AM_SEEKING_SeekToKeyFrame;
		}

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
		// set the new clock for all filters
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

			if (gb) {
				gb->SetDefaultSyncSource();
			}

			uses_clock = true;

			RefreshClock();

		} else {

			uses_clock = (new_clock == NULL ? false : true);

			// set the new clock for all filters
			for (int i=0; i<filters.GetCount(); i++) {
				Filter	*filter = filters[i];
				if (filter->filter) {
					filter->filter->SetSyncSource(new_clock);
				}
				filter->UpdateClock();
			}
		}
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
			//GUID				time_format;

			rtDur = 0;
			rtFrames = 0;

			if (ms->IsFormatSupported(&TIME_FORMAT_FRAME) != NOERROR) {
				hr = E_FAIL;
				break;
			}
			hr = ms->SetTimeFormat(&TIME_FORMAT_FRAME);
			if (FAILED(hr)) break;
			hr = ms->GetDuration(&rtFrames);
			if (FAILED(hr)) break;
			hr = ms->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
			if (FAILED(hr)) break;
			hr = ms->GetDuration(&rtDur);
			if (FAILED(hr)) break;

			// special case
			if (rtFrames == 0 || rtDur == 0) {
				fps = 0.0;
				return 0;
			}

			// calculate the FPS
			fps = (double)rtFrames * 10000000.0 / (double)rtDur;
			hr = NOERROR;

		} while (0);

		if (FAILED(hr)) {
			fps = -1.0;
			return -1;
		}

		return 0;
	}

	int DisplayGraph::GetPositions(double &current_ms, double &duration_ms)
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

				// in seconds
				duration_ms = (double)rtDur / 10000.0;
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
				current_ms = (double)rtCur / 10000.0;
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

	void DisplayGraph::DeleteSelected()
	{
		// first delete connections and then filters
		for (int i=0; i<filters.GetCount(); i++) {
			filters[i]->DeleteSelectedConnections();
		}
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i]->selected) {
				if (callback) callback->OnFilterRemoved(this, filters[i]);
				filters[i]->DeleteFilter();
			}
		}
		RefreshFilters();
		RefreshFPS();
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
		RefreshFPS();
		return NOERROR;
	}

	CStringA UTF16toUTF8(const CStringW &utf16)
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

	int DisplayGraph::SaveXML_Filter(Filter *filter, XML::XMLWriter *writer)
	{
		CComPtr<IFileSourceFilter>		src;
		CComPtr<IFileSinkFilter>		sink;
		CComPtr<IPersistStream>			persist;
		HRESULT							hr;
		
		hr = filter->filter->QueryInterface(IID_IFileSourceFilter, (void**)&src);
		if (SUCCEEDED(hr)) {
			LPOLESTR		fn = NULL;
            CMediaType media_type;
			if (SUCCEEDED(src->GetCurFile(&fn, &media_type))) {
				//	<ifilesourcefilter source="d:\sga.avi"/>
				writer->BeginNode(_T("ifilesourcefilter"));
					writer->WriteValue(_T("source"), CString(fn));
				writer->EndNode();
				if (fn) CoTaskMemFree(fn);
			}
			src = NULL;
		}

		hr = filter->filter->QueryInterface(IID_IFileSinkFilter, (void**)&sink);
		if (SUCCEEDED(hr)) {
			LPOLESTR		fn = NULL;
            CMediaType media_type;
			if (SUCCEEDED(sink->GetCurFile(&fn, &media_type))) {
				//	<ifilesinkfilter dest="d:\sga.avi"/>
				writer->BeginNode(_T("ifilesinkfilter"));
					writer->WriteValue(_T("dest"), CString(fn));
				writer->EndNode();
				if (fn) CoTaskMemFree(fn);
			}
			sink = NULL;
		}



		return 0;
	}

	int DisplayGraph::SaveXML(CString fn)
	{
		XML::XMLWriter			xml;

		xml.BeginNode(_T("graph"));
			xml.WriteValue(_T("name"), _T("Unnamed Graph"));

			/*
				First we add all filters into the graph
			*/
			for (int i=0; i<filters.GetCount(); i++) {
				Filter	*filter = filters[i];

				xml.BeginNode(_T("filter"));
					// save the filter CLSID. If the filter is a wrapper it will initialize properly
					// after loading IPersistStream 
					xml.WriteValue(_T("name"), filter->name);
					xml.WriteValue(_T("index"), i);

					LPOLESTR	strclsid = NULL;
					StringFromCLSID(filter->clsid, &strclsid);
					xml.WriteValue(_T("clsid"), strclsid);
					CoTaskMemFree(strclsid);

					// now check for interfaces
					SaveXML_Filter(filter, &xml);

				xml.EndNode();
			}

			std::set<Pin*> saved_input_pins;	// The input pins whose connections have already been saved
			bool all_inputs_saved = false;		// true when all filters have had their connected inputs saved so we can stop iterating
			int iterations = 0;

			// Now let's add all the connections
			// Loop over filters only saving connections from filters whose inputs have been saved already
			while (!all_inputs_saved) {		
				if (iterations++ > 500)	{	// Sanity check to prevent pathological infinite looping
					ASSERT(false);
					break;
				}

				all_inputs_saved = true;	// test every filter in loop below

				for (int i=0; i<filters.GetCount(); i++) {
				
					Filter * const filter = filters[i];
					bool inputs_saved = true;

					// Look for any unsaved connections on input pins
					for (int j=0; j<filter->input_pins.GetCount(); j++) {
						Pin* const pin = filter->input_pins[j];
						if (pin->peer && saved_input_pins.find(pin) == saved_input_pins.end()) {		
							inputs_saved = false;
							break;
						}
					}

					all_inputs_saved = all_inputs_saved && inputs_saved;

					// Only save output connections for filters whose connected input pins are already saved
					if (inputs_saved) {
						for (int j=0; j<filter->output_pins.GetCount(); j++) {
							Pin * const pin = filter->output_pins[j];
							if (pin->peer && saved_input_pins.find(pin->peer) == saved_input_pins.end()) {
								saved_input_pins.insert(pin->peer);		// record that we've saved this input pin's connection

								int inFilterIndex = -1;
								for (int f=0; f<filters.GetCount(); f++) {
									if (pin->peer->filter == filters[f]) {
										inFilterIndex = f;
									}
								}
								ASSERT(inFilterIndex >= 0);

								xml.BeginNode(_T("connect"));
									xml.WriteValue(_T("out"), filter->name + _T("/") + pin->name);
									xml.WriteValue(_T("outFilterIndex"), i);
									xml.WriteValue(_T("outPinId"), pin->id);
									xml.WriteValue(_T("in"), pin->peer->filter->name + _T("/") + pin->peer->name);
									xml.WriteValue(_T("inFilterIndex"), inFilterIndex);
									xml.WriteValue(_T("inPinId"), pin->peer->id);
									xml.WriteValue(_T("direct"), _T("true"));
								xml.EndNode();
							}
						}
					}
				}
			}

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

		graph_name = fn;

		// load graph
		XML::XMLNode * const root = xml.root;
		XML::XMLIterator it;
		if (root->Find(_T("graph"), &it) < 0) 
			return VFW_E_NOT_FOUND;

		XML::XMLNode * const gn = *it;
		const CString gn_name = gn->GetValue(_T("name"));
		if (gn_name != _T("")) 
			graph_name = gn_name;

		for (it = gn->nodes.begin(); it != gn->nodes.end(); it++) {
			XML::XMLNode * const node = *it;

			if (node->name == _T("filter"))	
				hr = LoadXML_Filter(node);
			else if (node->name == _T("render")) 
				hr = LoadXML_Render(node); 
			else if (node->name == _T("connect")) 
				hr = LoadXML_Connect(node); 
			else if (node->name == _T("config")) 
				hr = LoadXML_Config(node); 
			else if (node->name == _T("iamgraphstreams")) 
				hr = LoadXML_IAMGraphStreams(node); 
			else if (node->name == _T("schedule")) 
				hr = LoadXML_Schedule(node); 
			else if (node->name == _T("command")) 
				hr = LoadXML_Command(node); 

			if (FAILED(hr)) {		// TODO continue trying to load after node fails to load
				return hr;
			}
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
			mt.formattype == FORMAT_WaveFormatEx
			) {
			int		latency_ms = conf->GetValue(_T("latency"), -1);
			if (latency_ms > 0) {
			
				WAVEFORMATEX * const wfx = (WAVEFORMATEX*)mt.pbFormat;

				// just like MSDN said: -1 = we don't care
				ALLOCATOR_PROPERTIES		alloc;
				alloc.cbAlign	= -1;
				alloc.cbBuffer	= (wfx->nAvgBytesPerSec * latency_ms) / 1000;
				alloc.cbPrefix	= -1;
				alloc.cBuffers	= 20;

				hr = buf_neg->SuggestAllocatorProperties(&alloc);
				if (FAILED(hr)) {
                    DSUtil::ShowError(_T("IAMBufferNegotiation::SuggestAllocatorProperties failed"));
				}
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
				if (FAILED(hr)) {
					DSUtil::ShowError(_T("Failed to set IAMGraphStreams::SyncUsingStreamOffset"));
				}
			}

			// max latency
			if (sync == 1 && latency >= 0) {
				const REFERENCE_TIME rtMaxLatency = latency;
				hr = gs->SetMaxGraphLatency(rtMaxLatency);
				if (FAILED(hr)) {
					DSUtil::ShowError(_T("Failed to set IAMGraphStreams::SetMaxGraphLatency"));
				}
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

	HRESULT DisplayGraph::LoadXML_Connect(XML::XMLNode *node)
	{
		Pin * opin = NULL;
		Pin * ipin = NULL;

		const int ofilter_index = _ttoi(node->GetValue(_T("outFilterIndex")));
		const int ifilter_index = _ttoi(node->GetValue(_T("inFilterIndex")));

		if (ofilter_index >= 0 || ofilter_index < filters.GetCount()
				|| ifilter_index >= 0 || ifilter_index < filters.GetCount()) {

			const CString opin_id = node->GetValue(_T("outPinId"));
			const CString ipin_id = node->GetValue(_T("inPinId"));

			// Filters get reversed in order during loading
			Filter * const ofilter = filters[filters.GetCount() - 1 - ofilter_index];
			Filter * const ifilter = filters[filters.GetCount() - 1 - ifilter_index];

			if (!ofilter || !ifilter)
				return VFW_E_NOT_FOUND;

			opin = ofilter->FindPinByID(opin_id);
			ipin = ifilter->FindPinByID(ipin_id);
		}

		if (!opin || !ipin) {
			ASSERT(false);

			const CString ofilter_path = node->GetValue(_T("out"));
			const CString ifilter_path = node->GetValue(_T("in"));

			opin = FindPinByPath(ofilter_path);
			ipin = FindPinByPath(ifilter_path);
		}

		if (!opin || !ipin) 
			return VFW_E_NOT_FOUND;

		const CString	direct    = node->GetValue(_T("direct"));

		HRESULT hr;
		if (direct.IsEmpty() || direct == _T("false"))
			hr = gb->Connect(opin->pin, ipin->pin);
		else
			hr = gb->ConnectDirect(opin->pin, ipin->pin, NULL);

		if (FAILED(hr)) 
			return hr;

		// reload newly added filters
		RefreshFilters();
		return S_OK;
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

	HRESULT DisplayGraph::LoadXML_Filter(XML::XMLNode *node)
	{
		const CString			name		= node->GetValue(_T("name"));
		CString					clsid_str	= node->GetValue(_T("clsid"));
		const CString			dn			= node->GetValue(_T("displayname"));
		GUID					clsid;
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr = NOERROR;
		int						filter_id_type = -1;
		bool					is_configured = false;

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
			} else {
				SmartPlacement();
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

		// we're done
		instance = NULL;
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

	int DisplayGraph::LoadGRF(CString fn)
	{
		IStorage *pStorage = 0;
		if (StgIsStorageFile(fn) != S_OK) return -1;

		graph_name = fn;

		HRESULT hr = StgOpenStorage(fn, 0, STGM_TRANSACTED | STGM_READ | STGM_SHARE_DENY_WRITE, 0, 0, &pStorage);
		if (FAILED(hr)) return hr;

		IPersistStream *pPersistStream = 0;
		hr = gb->QueryInterface(IID_IPersistStream, (void**)&pPersistStream);
		if (SUCCEEDED(hr)) {
			IStream *pStream = 0;
			hr = pStorage->OpenStream(L"ActiveMovieGraph", 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream);
			if (SUCCEEDED(hr)) {
				hr = pPersistStream->Load(pStream);
				pStream->Release();
			}
			pPersistStream->Release();
		}
		pStorage->Release();
		RefreshFPS();
		return hr;
	}

	int DisplayGraph::SaveGRF(CString fn)
	{
		const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
		HRESULT hr;
		    
		graph_name = fn;

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
	int DisplayGraph::RenderFile(CString fn)
	{
		HRESULT	hr = E_FAIL;

		do {

			// mark 
			params->MarkRender(true);
			if (!gb) {
				hr = E_POINTER;	
				break;
			}
			hr = gb->RenderFile(fn, NULL);
			params->MarkRender(false);
			if (callback) callback->OnRenderFinished();

			if (FAILED(hr)) break;

			graph_name = fn;

		} while (0);

		if (FAILED(hr)) {
			return -1;
		}

		RefreshFPS();
		return 0;
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

	void DisplayGraph::ZeroTags()
	{
		for (int i=0; i<filters.GetCount(); i++) {
			Filter *filt = filters[i];
			filt->tag = 0;
		}
	}

	void DisplayGraph::RemoveUnusedFilters()
	{
		// remove all those, whose TAG is zero
		for (int i=filters.GetCount()-1; i>=0; i--) {
			Filter *filt = filters[i];
			if (filt->tag == 0) {				
				if (callback) callback->OnFilterRemoved(this, filt);
				delete filt;
				filters.RemoveAt(i);
			}
		}
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

		if (FAILED(hr)) 
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
		ZeroTags();

		IEnumFilters	*efilt;
		IBaseFilter		*filt;
		ULONG			ff;

		if (!gb) {
			RemoveUnusedFilters();
			return ;
		}

		HRESULT			hr = gb->EnumFilters(&efilt);
		if (SUCCEEDED(hr) && efilt) {
			efilt->Reset();
			while (efilt->Next(1, &filt, &ff) == NOERROR) {
				Filter	*filter = FindFilter(filt);
				if (!filter) {
					filter = new Filter(this);
					filter->LoadFromFilter(filt);
					filters.InsertAt(0, filter);
				} else {
					filter->Refresh();
				}

				// mark this one as active...
				if (filter) {
					filter->tag = 1;
				}
				filt->Release();
			}
			efilt->Release();
		}
	
		// kill those inactive
		RemoveUnusedFilters();
		LoadPeers();
		RefreshClock();
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

	void DisplayGraph::SmartPlacement()
	{
		bins.RemoveAll();

		// do some nice automatic filter placement
		int i;
		for (i=0; i<filters.GetCount(); i++) {
			Filter	* const filter = filters[i];
			filter->Refresh();

			// reset placement helpers
			filter->depth = -1;		// flag not placed in bin
			filter->posy = 0;
			filter->posx = 0;
		}

		// First position the sources that have connected outputs
		for (i=0; i<filters.GetCount(); i++) {
			Filter	* const filter = filters[i];
			filter->LoadPeers();
			if (filter->NumOfConnectedPins(PINDIR_INPUT) == 0
					&& filter->NumOfConnectedPins(PINDIR_OUTPUT) > 0) {
				// For filters with no connected inputs, and some connected outputs
				filter->CalculatePlacementChain(0, 8);
			}
		}

		// then align the not connected filters
		for (i=0; i<filters.GetCount(); i++) {
			Filter	* const filter = filters[i];
			if (filter->depth < 0) {		// if not already placed
				filter->CalculatePlacementChain(0, 8);
			}
		}

		// now set proper posX
		for (i=0; i<filters.GetCount(); i++) {
			Filter	* const filter = filters[i];
			ASSERT(filter->depth >= 0);
			filter->depth = max(0, filter->depth);		// sanity check on depth
			CPoint	&pt     = bins[filter->depth];
			filter->posx = pt.x;
		}
	}

	//-------------------------------------------------------------------------
	//
	//	Filter class
	//
	//-------------------------------------------------------------------------

	Filter::Filter(DisplayGraph *parent)
	{
		graph = parent;
		params = (graph != NULL ? graph->params : NULL);
		name = _T("");
		clsid = CLSID_VideoMixingRenderer9;
		clsid_str = _T("");
		filter = NULL;
		posx = 0;
		posy = 0;
		selected = false;
		basic_audio = NULL;
		clock = NULL;
		videowindow = NULL;
		overlay_icon_active = -1;
		overlay_icons.RemoveAll();
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
		name = _T("");

		basic_audio = NULL;
		clock = NULL;
		if (filter != NULL) {
			filter = NULL;
		}

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

	void Filter::DeleteSelectedConnections()
	{
		int i;
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			if (pin->selected) {
				pin->Disconnect();
			}
		}
	}

	void Filter::DeleteFilter()
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
			f->QueryInterface(IID_IBaseFilter, (void**)&filter);

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

		// todo: check for IFileSourceFilter
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
		CPen	pen(nPenStyle, 1, color);
        CPen    penArrow(PS_SOLID, 1, color);
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
			DWORD	color = RGB(0,0,0);

			if (pin && peer) {
				CPoint	pt1, pt2;
				pin->GetCenterPoint(&pt1);
				peer->GetCenterPoint(&pt2);
				if (pin->selected) color = params->select_color;
			
				DoDrawArrow(dc, pt1, pt2, color);
			}
		}
	}

	void Filter::Draw(CDC *dc)
	{
		DWORD	back_color = params->filter_type_colors[0];
		if (connected) {
			back_color = params->filter_type_colors[filter_type];
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
			CPen	sel_pen(PS_SOLID, 1, params->select_color);
			CBrush	sel_brush(params->select_color);
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

	void Filter::CalculatePlacementChain(int new_depth, int x)
	{
		if (new_depth > graph->bins.GetCount()) {
			// this is an error case !!
			return ;
		} else
		if (new_depth == graph->bins.GetCount()) {
			// add one more
			CPoint	pt;
			pt.x = x;
			pt.y = 8;
			graph->bins.Add(pt);
			pt.x = (x+width+40+7) &~ 0x07;
			graph->bins.Add(pt);
		} else
		if (new_depth == graph->bins.GetCount()-1) {
			// check the next bin X position
			CPoint	pt;
			int		newx = (x+width+40+7)&~ 0x07;
			pt.x = newx;
			pt.y = 8;
			graph->bins.Add(pt);
		}

		CPoint		&pt = graph->bins[new_depth];

		// we distribute new values, if they are larger
		if (new_depth == 0 || new_depth > depth || x > pt.x) {
			depth = new_depth;

			if (x > pt.x) {
				pt.x = x;

				if (x > pt.x) {
					int dif = x - pt.x;
					// move all following bins
					for (int i=new_depth; i<graph->bins.GetCount(); i++) {
						CPoint	&p = graph->bins[i];
						p.x += dif;
					}
				}
			}
			int	newx = ((pt.x + width + 40) + 7) &~ 0x07;

			if (posy == 0)	{
				// next row
				posy = pt.y;
				posy = (posy + 7) &~ 0x07;
				pt.y += height + 30;
				pt.y = (pt.y + 7) &~ 0x07;
			}

			// find downstream filters
			for (int i=0; i<output_pins.GetCount(); i++) {
				Pin *pin = output_pins[i];
				IPin *peer_pin = NULL;
				if (pin && pin->pin) pin->pin->ConnectedTo(&peer_pin);

				// find parent of the downstream filter
				if (peer_pin) {
					Filter	*down_filter = graph->FindParentFilter(peer_pin);
					if (down_filter) {
						down_filter->CalculatePlacementChain(new_depth + 1, newx);
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
		// verify so we don't go outside the area
		if (start_drag_pos.x + (*deltax) < 8) {
			*deltax = 8 - start_drag_pos.x;
		}
		if (start_drag_pos.y + (*deltay) < 8) {
			*deltay = 8 - start_drag_pos.y;
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

	Pin *Filter::FindPinByID(CString name)
	{
		CArray<Pin*> *lists[] = {&output_pins, &input_pins};
		const size_t num_lists = sizeof(lists)/sizeof(lists[0]);

		CComPtr<IPin> ipin;
		filter->FindPin(name, &ipin);

		for (CArray<Pin*> ** pins = lists; pins<lists+num_lists; pins++) {
			for (int p=0; p<(**pins).GetCount(); p++) {
				Pin * const pin = (**pins)[p];
				if (pin->pin == ipin) 
					return pin;
			}
		}
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
		if (!connected || !peer) return 0;

		// we need to disconnect both pins
		HRESULT	hr;
		hr = filter->graph->gb->Disconnect(peer->pin);
		if (FAILED(hr)) return -1;
		hr = filter->graph->gb->Disconnect(pin);
		if (FAILED(hr)) return -1;

		// clear variables
		peer->peer = NULL;
		peer->selected = false;
		peer->connected = false;
		peer = NULL;
		connected = false;
		selected = false;

		// we're okay
		return 0;
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
		CPen	pen_back(PS_SOLID, 1, params->filter_type_colors[0]);
		CBrush	brush_back(params->filter_type_colors[0]);

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
	//	RenderParameters class
	//
	//-------------------------------------------------------------------------

	RenderParameters::RenderParameters()
	{
		color_back = RGB(192, 192, 192);		// default background color
        color_back_remote = RGB(255, 192, 192);		// default background color
		select_color = RGB(0,0,255);

		// filter colors
		color_filter_border_light = RGB(255, 255, 255);
		color_filter_border_dark = RGB(128, 128, 128);

		filter_type_colors[Filter::FILTER_UNKNOWN] = RGB(192,192,192);
		filter_type_colors[Filter::FILTER_STANDARD] = RGB(192,192,255);
		filter_type_colors[Filter::FILTER_WDM] = RGB(255,128,0);
		filter_type_colors[Filter::FILTER_DMO] = RGB(0,192,64);

		// default size at 100%
		def_min_width = 92;
		def_min_height = 86;
		def_pin_spacing = 27;
		def_filter_text_size = 10;
		def_pin_text_size = 7;

		display_file_name = true;
		connect_mode = 0;
		exact_match_mode = false;
		abort_timeout = true;

		render_start_time = 0;
		in_render = false;
		render_actions.clear();

		Zoom(1.0);

		// no preferred renderer
		preferred_video_renderer = _T("");
		video_renderers = NULL;

        use_media_info = false;
        is_remote = false;

		// load bitmaps
		BOOL ok;	
		ok = bmp_volume_hi.LoadBitmap(IDB_BITMAP_VOLUME_HI);					if (!ok) return ;
		ok = bmp_volume_lo.LoadBitmap(IDB_BITMAP_VOLUME_LO);					if (!ok) return ;
		ok = bmp_clock_inactive_hi.LoadBitmap(IDB_BITMAP_CLOCK_INACTIVE_HI);	if (!ok) return ;
		ok = bmp_clock_inactive_lo.LoadBitmap(IDB_BITMAP_CLOCK_INACTIVE_LO);	if (!ok) return ;
		ok = bmp_clock_active_hi.LoadBitmap(IDB_BITMAP_CLOCK_ACTIVE_HI);		if (!ok) return ;
		ok = bmp_clock_active_lo.LoadBitmap(IDB_BITMAP_CLOCK_ACTIVE_LO);		if (!ok) return ;
	}

	RenderParameters::~RenderParameters()
	{
	}

	void RenderParameters::Zoom(int z)
	{
		zoom = z;

		min_filter_width = (((int)(z * def_min_width / 100.0))); // &~ 0x08;
		min_filter_height = (((int)(z * def_min_height / 100.0))); // &~ 0x08;
		pin_spacing = (int)(z * def_pin_spacing / 100.0);

		if (font_filter.m_hObject != 0) { font_filter.DeleteObject(); }
		if (font_pin.m_hObject != 0) { font_pin.DeleteObject(); }

		int size = 5 + (5.0*z / 100.0);
		MakeFont(font_filter, _T("Arial"), size, false, false); 
		size = 5 + (2.0*z / 100.0);
		MakeFont(font_pin, _T("Arial"), size, false, false);
	}

	void RenderParameters::MarkRender(bool start)
	{
		if (in_render == start) return ;

		// remember the time
		in_render = start;
		if (start) {
			render_start_time = GetTickCount();
			render_can_proceed = true;
			render_actions.clear();
		}
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
		if (graph->params->abort_timeout && graph->params->in_render) {

			DWORD		timenow = GetTickCount();

			if (graph->params->render_can_proceed &&
				(timenow > (graph->params->render_start_time + 10*1000))
				) {
	
				// TODO: perhaps display some error message
				graph->params->render_can_proceed = false;
			}

			// we stop accepting filters - graph builder will run out of options
			// and the operation will stop
			if (!graph->params->render_can_proceed) {
				return E_FAIL;
			}
		}

		/**********************************************************************
			List of filters used in a render operation
		***********************************************************************/
		RenderAction	ra;

		// moniker name
		LPOLESTR	moniker_name;
		hr = pMon->GetDisplayName(NULL, NULL, &moniker_name);
		if (SUCCEEDED(hr)) {
			ra.type	= RenderAction::ACTION_SELECT;
			ra.displ_name = CString(moniker_name);

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

		return NOERROR;
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
	


};


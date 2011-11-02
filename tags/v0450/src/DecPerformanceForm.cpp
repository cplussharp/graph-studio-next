//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "DecPerformanceForm.h"


//-----------------------------------------------------------------------------
//
//	CDecPerformanceForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CDecPerformanceForm, CDialog)

BEGIN_MESSAGE_MAP(CDecPerformanceForm, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CDecPerformanceForm::OnBrowseClick)
	ON_BN_CLICKED(IDC_BUTTON_START, &CDecPerformanceForm::OnStartClick)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CDecPerformanceForm::OnStopClick)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CDecPerformanceForm class
//
//-----------------------------------------------------------------------------

CDecPerformanceForm::CDecPerformanceForm(CWnd* pParent) : 
	CDialog(CDecPerformanceForm::IDD, pParent)
{
	running = false;
	time_filter = NULL;
	perf_operation = false;
}

CDecPerformanceForm::~CDecPerformanceForm()
{
	time_filter = NULL;
}

void CDecPerformanceForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_COMBO_PRESETS, cb_presets);
	DDX_Control(pDX, IDC_COMBO_FILE, cb_files);
	DDX_Control(pDX, IDC_COMBO_DECODER, cb_decoders);
	DDX_Control(pDX, IDC_COMBO_RENDERER, cb_renderers);
	DDX_Control(pDX, IDC_BUTTON_SAVE, btn_save);
	DDX_Control(pDX, IDC_BUTTON_CLEAR, btn_clear);
	DDX_Control(pDX, IDC_BUTTON_START, btn_start);
	DDX_Control(pDX, IDC_BUTTON_STOP, btn_stop);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, btn_browse);
	DDX_Control(pDX, IDC_EDIT_PASSES, edit_passes);
	DDX_Control(pDX, IDC_SPIN_PASSES, btn_spin);
	DDX_Control(pDX, IDC_LIST_RESULTS, list_results);
}

BOOL CDecPerformanceForm::DoCreateDialog()
{
	BOOL ret = Create(IDD);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	OnInitialize();
	return TRUE;
};

void CDecPerformanceForm::OnInitialize()
{
	// Fill the filters & decoders & stuff ...
	int i;

	// load the file list
	file_list.LoadList(_T("DecPerfFileList"));
	cb_files.ResetContent();
	for (i=0; i<file_list.GetCount(); i++) {
		cb_files.AddString(file_list[i]);
	}

	// create the columns
	DWORD ex_style = list_results.GetExtendedStyle();
	ex_style = ex_style | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	list_results.SetExtendedStyle(ex_style);

	list_results.InsertColumn(0, _T("#"), LVCFMT_LEFT, 40);
	list_results.InsertColumn(1, _T("Work Time"), LVCFMT_LEFT, 120);
	list_results.InsertColumn(2, _T("FPS"), LVCFMT_LEFT, 100);


	// now create the list of renderers
	cb_renderers.AddString(_T("Null Renderer"));
	DSUtil::FilterTemplates	*renderers = &view->video_renderers;
	for (i=0; i<renderers->filters.GetCount(); i++) {
		cb_renderers.AddString(renderers->filters[i].name);
	}
	cb_renderers.SetCurSel(0);

	// create the list of video decoders
	ScanVideoDecoders();
	for (i=0; i<decoders.GetCount(); i++) {
		int index = cb_decoders.AddString(decoders[i].name);
		cb_decoders.SetItemDataPtr(index, (void*)&decoders[i]);
	}
	if (decoders.GetCount() > 0) {
		cb_decoders.SetCurSel(0);
	}

	phase_count = 3;

	CString		str;
	str.Format(_T("%d"), phase_count);
	edit_passes.SetWindowText(str);
	btn_spin.SetRange(1, 10);
	btn_spin.SetPos(phase_count);
}

void CDecPerformanceForm::ScanVideoDecoders()
{
	// now find all filters that have both input and output pins
	// and support video media type.
	decoders.RemoveAll();

	DSUtil::FilterTemplates			all_filters;
	int								i, j, k;

	all_filters.Enumerate(CLSID_LegacyAmFilterCategory);
	for (i=0; i<all_filters.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = all_filters.filters[i];

		// now check the pins
		bool	video_in = false;
		bool	video_out = false;

		for (j=0; j<filter.input_pins.GetCount(); j++) {
			DSUtil::PinTemplate	&pin = filter.input_pins[j];

			// check media types
			for (k=0; k<pin.types; k++) {
				if (pin.major[k] == MEDIATYPE_Video &&
					!DSUtil::IsVideoUncompressed(pin.minor[k])
					) {
					// at least one compressed input
					video_in = true;
				}
				if (video_in) break;
			}	
			if (video_in) break;
		}

		for (j=0; j<filter.output_pins.GetCount(); j++) {
			DSUtil::PinTemplate	&pin = filter.output_pins[j];

			// check media types
			for (k=0; k<pin.types; k++) {
				if (pin.major[k] == MEDIATYPE_Video &&
					DSUtil::IsVideoUncompressed(pin.minor[k])
					) {
					// at least one uncompressed output
					video_out = true;
				}
				if (video_out) break;
			}	
			if (video_out) break;
		}

		// if it has video in and out, we can take it for a video decoder
		if (video_in && video_out) {
			decoders.Add(filter);
		}
	}

	// we also display DMOs
	all_filters.filters.RemoveAll();
	all_filters.EnumerateDMO(DMOCATEGORY_VIDEO_DECODER);
	for (i=0; i<all_filters.filters.GetCount(); i++) {
		decoders.Add(all_filters.filters[i]);
	}
}

void CDecPerformanceForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	if (::IsWindow(title)) {
		title.GetClientRect(&rc2);
		title.SetWindowPos(NULL, 0, 0, cx, rc2.Height(), SWP_SHOWWINDOW);
		
		// invalidate all controls
		title.Invalidate();
	}
}

void CDecPerformanceForm::OnBrowseClick()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter =  _T("");
	filter += _T("Video Files (avi,mp4,mpg,mpeg,ts,mkv,ogg,ogm,pva,evo,flv,mov,hdmov,ifo,vob,rm,rmvb,wmv,asf)|*.avi;*.mp4;*.mpg;*.mpeg;*.ts;*.mkv;*.ogg;*.ogm;*.pva;*.evo;*.flv;*.mov;*.hdmov;*.ifo;*.vob;*.rm;*.rmvb;*.wmv;*.asf|");
	filter += _T("Audio Files (aac,ac3,mp3,wma,mka,ogg,mpc,flac,ape,wav,ra,wv,m4a,tta,dts,spx,mp2,ofr,ofs,mpa)|*.aac;*.ac3;*.mp3;*.wma;*.mka;*.ogg;*.mpc;*.flac;*.ape;*.wav;*.ra;*.wv;*.m4a;*.tta;*.dts;*.spx;*.mp2;*.ofr;*.ofs;*.mpa|");
	filter += _T("All Files|*.*|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		cb_files.SetWindowText(filename);
	}	
}

void CDecPerformanceForm::Start()
{
	if (perf_operation) return ;

	// just to be sure
	Stop();

	// try to build the graph.
	int i;
	int	ret = BuildPerformanceGraph(view->graph.gb);
	if (ret < 0) {
		// some message
		MessageBox(_T("Cannot build performance graph"), _T("Error"), MB_ICONERROR);
		return ;
	}

	// update the file list
	CString			source_file;
	cb_files.GetWindowText(source_file);
	file_list.UpdateList(source_file);
	file_list.SaveList(_T("DecPerfFileList"));
	cb_files.ResetContent();
	for (i=0; i<file_list.GetCount(); i++) { cb_files.AddString(file_list[i]); }
	cb_files.SetWindowText(source_file);

	list_results.DeleteAllItems();

	// update view
	view->UpdateGraphState();
	view->graph.RefreshFilters();
	view->graph.SmartPlacement();
	view->Invalidate();

	// reset the clock 
	// if this was done before the filters were refreshed, the graph was not reset for some reason :-/
	view->graph.SetClock(false, NULL);

	// update the display again
	view->graph.RefreshFilters();
	view->graph.SmartPlacement();
	view->Invalidate();

	// run the graph
	phase_cur			= 0;
	runtime_total_ns	= 0;
	frames_total		= 0;
	running				= true;

	view->OnPlayClick();
}

void CDecPerformanceForm::Stop()
{
	if (perf_operation) return ;

	// remember it was us who made the call...
	perf_operation = true;
	view->OnNewClick();
	time_filter = NULL;
	perf_operation = false;

	running = false;

	// enable controls again
}

void MakeNiceTime(__int64 timens, CString &ret)
{
	// convert to milliseconds
	timens /= 1000000;

	__int64			h = timens / (3600 * 1000);		timens -= h*(3600*1000);
	__int64			m = timens / (  60 * 1000);		timens -= m*(  60*1000);
	__int64			s = timens / (   1 * 1000);		timens -= s*(   1*1000);
	__int64			ms= timens;

	ret.Format(_T("%.02d:%.02d:%.02d.%.03d"), h, m, s, ms);
}

void CDecPerformanceForm::OnPhaseComplete()
{
	if (!running) return ;
	
	// if there's no time filter, it must have been a mistake... abort everything
	if (!time_filter) {
		Stop();
		return ;
	}

	phase_cur ++;

	// retrieve the stats
	__int64		runtime_ns;
	__int64		frames;

	time_filter->GetStats(&runtime_ns, &frames);
	if (runtime_ns <= 0) runtime_ns = 1;				// avoid division by zero.

	runtime_total_ns	+= runtime_ns;
	frames_total		+= frames;

	// save the results
	double		fps = (frames * 1000000000.0) / (double)runtime_ns;
	CString		phase_idx_str;
	CString		runtime_str;
	CString		fps_str;

	phase_idx_str.Format(_T("%d"), phase_cur);
	MakeNiceTime(runtime_ns, runtime_str);
	fps_str.Format(_T("%7.4f FPS"), (float)(fps));

	list_results.InsertItem(phase_cur-1, phase_idx_str);
	list_results.SetItemText(phase_cur-1, 1, runtime_str);
	list_results.SetItemText(phase_cur-1, 2, fps_str);

	if (phase_cur >= phase_count) {
	
		// we're done
		Stop();

		// write average stats
		phase_idx_str	= _T("Avg.");
		fps				= (frames_total * 1000000000.0) / (double)runtime_total_ns;
		runtime_ns		= runtime_total_ns / phase_count;

		MakeNiceTime(runtime_ns, runtime_str);
		fps_str.Format(_T("%7.4f FPS"), (float)(fps));

		int cnt = list_results.GetItemCount();
		list_results.InsertItem(cnt, phase_idx_str);
		list_results.SetItemText(cnt, 1, runtime_str);
		list_results.SetItemText(cnt, 2, fps_str);

	} else {

		// start the next run
		view->OnStopClick();
		view->OnPlayClick();
	}
}

int CDecPerformanceForm::BuildPerformanceGraph(IGraphBuilder *gb)
{
	HRESULT					hr;
	int						ret = 0;
	CComPtr<IBaseFilter>	src;
	CComPtr<IBaseFilter>	decoder;
	CComPtr<IBaseFilter>	time;
	CComPtr<IBaseFilter>	renderer;

	time_filter = NULL;

	do {
		// 1. Try to create a source filter for the file
		CString			source_file;
		CString			short_name;

		cb_files.GetWindowText(source_file);
		CPath			path(source_file);
		path.StripPath();
		short_name = CString(path);

		hr = gb->AddSourceFilter(source_file.GetBuffer(), short_name.GetBuffer(), &src);
		if (FAILED(hr)) break;

		// 2. Create the decoder instance
		int		idx = cb_decoders.GetCurSel();
		if (idx < 0) { hr = E_FAIL; break; }

		DSUtil::FilterTemplate		*filter_template = (DSUtil::FilterTemplate*)cb_decoders.GetItemDataPtr(idx);
		if (!filter_template) { hr = E_FAIL; break; }

		// and insert it into the graph
		ret = filter_template->CreateInstance(&decoder);
		if (ret < 0) { hr = E_FAIL; break; }
		hr = gb->AddFilter(decoder, filter_template->name.GetBuffer());
		if (FAILED(hr)) break;

		// 3. Now connect the source with the decoder
		hr = DSUtil::ConnectFilters(gb, src, decoder);
		if (FAILED(hr)) break;

		// 4. Insert the time measure filter
		hr = NOERROR;
		CMonoTimeMeasure	*timeflt = new CMonoTimeMeasure(NULL, &hr);
		if (!timeflt || FAILED(hr)) { hr = E_FAIL; break; }
		timeflt->NonDelegatingQueryInterface(IID_IBaseFilter, (void**)&time);
		timeflt->NonDelegatingQueryInterface(IID_IMonoTimeMeasure, (void**)&time_filter);
		hr = gb->AddFilter(time, L"Time Measure Filter");
		if (FAILED(hr)) break;
		hr = DSUtil::ConnectFilters(gb, decoder, time);
		if (FAILED(hr)) break;

		// 5. Insert the renderer filter
		idx = cb_renderers.GetCurSel();
		if (idx == 0) {
			// create the NULL Renderer
			hr = CoCreateInstance(DSUtil::CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&renderer);
			if (FAILED(hr)) break;
			hr = gb->AddFilter(renderer, L"Null Renderer");
			if (FAILED(hr)) break;
		} else {
			// instantiate the video renderer
			idx -= 1;
			if (idx >= 0 && idx < view->video_renderers.filters.GetCount()) {
				DSUtil::FilterTemplate	&ren_template = view->video_renderers.filters[idx];
				hr = ren_template.CreateInstance(&renderer);
				if (FAILED(hr)) break;
				hr = gb->AddFilter(renderer, ren_template.name.GetBuffer());
				if (FAILED(hr)) break;
			} else {
				hr = E_FAIL;
				break;
			}
		}

		// connect the time filter
		hr = DSUtil::ConnectFilters(gb, time, renderer);
		if (FAILED(hr)) break;

		hr = NOERROR;
	} while (0);

	// done with the filters
	renderer = NULL;
	src = NULL;
	decoder = NULL;
	time = NULL;

	return 0;
}



void CDecPerformanceForm::OnStartClick()
{
	Start();
}

void CDecPerformanceForm::OnStopClick()
{
	Stop();
}

BOOL CDecPerformanceForm::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int		id = (int)wParam;
	switch (id) {
	case IDC_SPIN_PASSES:
		{
			NMUPDOWN	*ud = (NMUPDOWN*)lParam;
			int			p   = ud->iPos + ud->iDelta;

			// clip <1; 10>
			p = (p < 1 ? 1 : p > 10 ? 10 : p);
			if (phase_count != p) {
				phase_count = p;
				CString		str;
				str.Format(_T("%d"), p);
				edit_passes.SetWindowText(str);
			}		
		}
		break;
	}

	return __super::OnNotify(wParam, lParam, pResult);
}









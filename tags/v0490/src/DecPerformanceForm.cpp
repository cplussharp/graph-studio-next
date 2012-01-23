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
    ON_CBN_SELCHANGE(IDC_COMBO_TYPE, &CDecPerformanceForm::OnCbnSelChange)
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
    null_renderer.clsid = DSUtil::CLSID_NullRenderer;
    null_renderer.name = _T("Null Renderer");
    time_filter_template.clsid = CLSID_MonoTimeMeasure;
    time_filter_template.name = _T("Time Measure Filter");
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
	DDX_Control(pDX, IDC_COMBO_TYPE, cb_type);
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

	list_results.InsertColumn(0, _T("#"), LVCFMT_RIGHT, 36);
	list_results.InsertColumn(1, _T("Work Time"), LVCFMT_LEFT, 80);
	list_results.InsertColumn(2, _T("FPS"), LVCFMT_RIGHT, 100);
    list_results.InsertColumn(3, _T("Rate"), LVCFMT_RIGHT, 100);
    list_results.InsertColumn(4, _T("Real Time"), LVCFMT_LEFT, 80);
    list_results.InsertColumn(5, _T("Frames"), LVCFMT_RIGHT, 80);

	// now create the list of renderers
	cb_type.AddString(_T("Audio"));
	cb_type.AddString(_T("Video"));
	cb_type.SetCurSel(1);
	
    OnCbnSelChange();

	phase_count = 3;

	CString		str;
	str.Format(_T("%d"), phase_count);
	edit_passes.SetWindowText(str);
	btn_spin.SetRange(1, 10);
	btn_spin.SetPos(phase_count);
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
    filter += _T("Audio Files |*.aac;*.ac3;*.mp3;*.wma;*.mka;*.ogg;*.mpc;*.flac;*.ape;*.wav;*.ra;*.wv;*.m4a;*.tta;*.dts;*.spx;*.mp2;*.ofr;*.ofs;*.mpa|");
	filter += _T("Video Files |*.avi;*.mp4;*.mpg;*.mpeg;*.m2ts;*.mts;*.ts;*.mkv;*.ogg;*.ogm;*.pva;*.evo;*.flv;*.mov;*.hdmov;*.ifo;*.vob;*.rm;*.rmvb;*.wmv;*.asf|");
	filter += _T("All Files|*.*|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    dlg.m_ofn.nFilterIndex = cb_type.GetCurSel() + 1;
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		cb_files.SetWindowText(filename);
	}	
}

void CDecPerformanceForm::OnCbnSelChange()
{
    cb_renderers.ResetContent();
    cb_decoders.ResetContent();

    DSUtil::FilterTemplates	*renderers;

    if(!cb_type.GetCurSel())
    {
        // Audio Renderer
	    renderers = &view->audio_renderers;

	    // create the list of Audio decoders
        decoders.EnumerateAudioDecoder();
    }
    else
    {
        // Video Renderer
	    renderers = &view->video_renderers;

        // create the list of Video decoders
        decoders.EnumerateVideoDecoder();
    }

    int index = cb_renderers.AddString(_T("Null Renderer"));
    cb_renderers.SetItemDataPtr(index, (void*)&null_renderer);
    for (int i=0; i<renderers->filters.GetCount(); i++) {
		index = cb_renderers.AddString(renderers->filters[i].name);
        cb_renderers.SetItemDataPtr(index, (void*)&renderers->filters[i]);
	}
	cb_renderers.SetCurSel(0);

    for (int i=0; i<decoders.filters.GetCount(); i++) {
		index = cb_decoders.AddString(decoders.filters[i].name);
		cb_decoders.SetItemDataPtr(index, (void*)&decoders.filters[i]);
	}
	if (decoders.filters.GetCount() > 0) {
		cb_decoders.SetCurSel(0);
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
    realtime_total_ns   = 0;
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

	int			h = timens / (3600 * 1000);		timens -= h*(3600*1000);
	int			m = timens / (  60 * 1000);		timens -= m*(  60*1000);
	int			s = timens / (   1 * 1000);		timens -= s*(   1*1000);
	int			ms= timens;

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
    __int64     realtime_ns;

	time_filter->GetStats(&runtime_ns, &frames, &realtime_ns);
	if (runtime_ns <= 0) runtime_ns = 1;				// avoid division by zero.

	runtime_total_ns	+= runtime_ns;
	frames_total		+= frames;
    realtime_total_ns   += realtime_ns;

	// save the results
	double		fps = (frames * 1000000000.0) / (double)runtime_ns;
	CString		phase_idx_str;
	CString		runtime_str;
	CString		fps_str;
    CString     realtime_str;
    CString     rate_str;
    CString     frames_str;

	phase_idx_str.Format(_T("%d"), phase_cur);
	MakeNiceTime(runtime_ns, runtime_str);
	fps_str.Format(_T("%7.4f FPS"), (float)(fps));
    rate_str.Format(_T("x%7.4f"), (double(realtime_ns) / double(runtime_ns)));
    MakeNiceTime(realtime_ns, realtime_str);
    frames_str.Format(_T("%d"), frames);

	list_results.InsertItem(phase_cur-1, phase_idx_str);
	list_results.SetItemText(phase_cur-1, 1, runtime_str);
	list_results.SetItemText(phase_cur-1, 2, fps_str);
    list_results.SetItemText(phase_cur-1, 3, rate_str);
    list_results.SetItemText(phase_cur-1, 4, realtime_str);
    list_results.SetItemText(phase_cur-1, 5, frames_str);

	if (phase_cur >= phase_count) {
	
		// we're done
		Stop();

		// write average stats
		phase_idx_str	= _T("Avg.");
		fps				= (frames_total * 1000000000.0) / (double)runtime_total_ns;
		runtime_ns		= runtime_total_ns / phase_count;
        realtime_ns     = realtime_total_ns / phase_count;
        frames          = frames_total / phase_count;

		MakeNiceTime(runtime_ns, runtime_str);
		fps_str.Format(_T("%7.4f FPS"), (float)(fps));
        rate_str.Format(_T("x%7.4f"), (double(realtime_ns) / double(runtime_ns)));
        MakeNiceTime(realtime_ns, realtime_str);
        frames_str.Format(_T("%d"), frames);

		int cnt = list_results.GetItemCount();
		list_results.InsertItem(cnt, phase_idx_str);
		list_results.SetItemText(cnt, 1, runtime_str);
		list_results.SetItemText(cnt, 2, fps_str);
        list_results.SetItemText(cnt, 3, rate_str);
        list_results.SetItemText(cnt, 4, realtime_str);
        list_results.SetItemText(cnt, 5, frames_str);

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

		DSUtil::FilterTemplate		*filter_template_decoder = (DSUtil::FilterTemplate*)cb_decoders.GetItemDataPtr(idx);
		if (!filter_template_decoder) { hr = E_FAIL; break; }

		// and insert it into the graph
		ret = filter_template_decoder->CreateInstance(&decoder);
		if (ret < 0) { hr = E_FAIL; break; }
		hr = gb->AddFilter(decoder, filter_template_decoder->name.GetBuffer());
		if (FAILED(hr)) break;

		// 3. Now connect the source with the decoder
		hr = DSUtil::ConnectFilters(gb, src, decoder);
		if (FAILED(hr)) break;

		// 4. Insert the time measure filter
		hr = NOERROR;
        hr = time_filter_template.CreateInstance(&time);
		if (FAILED(hr)) { hr = E_FAIL; break; }
        time->QueryInterface(IID_IMonoTimeMeasure, (void**)&time_filter);
        hr = gb->AddFilter(time, time_filter_template.name);
		if (FAILED(hr)) break;
		hr = DSUtil::ConnectFilters(gb, decoder, time);
		if (FAILED(hr)) break;

		// 5. Insert the renderer filter
		idx = cb_renderers.GetCurSel();
		if (idx < 0) { hr = E_FAIL; break; }

		DSUtil::FilterTemplate		*filter_template_renderer = (DSUtil::FilterTemplate*)cb_renderers.GetItemDataPtr(idx);
		if (!filter_template_renderer) { hr = E_FAIL; break; }

		// and insert it into the graph
		ret = filter_template_renderer->CreateInstance(&renderer);
		if (ret < 0) { hr = E_FAIL; break; }
		hr = gb->AddFilter(renderer, filter_template_renderer->name.GetBuffer());
		if (FAILED(hr)) break;

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









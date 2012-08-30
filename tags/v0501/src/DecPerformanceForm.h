//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


//-----------------------------------------------------------------------------
//
//	CDecPerformanceForm class
//
//-----------------------------------------------------------------------------
class CDecPerformanceForm : 
	public CDialog
{
protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CDecPerformanceForm)

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	GraphStudio::TitleBar		title;
	CComboBox					cb_decoders;
	CComboBox					cb_renderers;
	CComboBox					cb_presets;
	CComboBox					cb_files;
	CComboBox                   cb_type;
	CButton						btn_start;
	CButton						btn_stop;
	CButton						btn_save;
	CButton						btn_clear;
	CButton						btn_browse;
	CSpinButtonCtrl				btn_spin;
	CEdit						edit_passes;
	CListCtrl					list_results;

	// view to work with
	CGraphView						*view;
	DSUtil::FilterTemplates        	decoders;
    DSUtil::FilterTemplate          null_renderer;
    DSUtil::FilterTemplate          time_filter_template;
	GraphStudio::FilenameList		file_list;


	// is the test currently running ?
	bool							running;
	bool							perf_operation;			// internal flag
	CComPtr<IMonoTimeMeasure>       time_filter;
    CComPtr<IBaseFilter>	        cur_decoder;

	int								phase_count;			// total number of runs
	int								phase_cur;				// the current run

	struct Timings
	{
		Timings() : runtime_ns(0LL), frames(0LL), realtime_ns(0LL) {}

		__int64		runtime_ns;
		__int64		frames;
		__int64		realtime_ns;
	};

	// average stats
	Timings							timings_min;
	Timings							timings_max;
	Timings							timings_avg;

public:
	CDecPerformanceForm(CGraphView* parent_view, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDecPerformanceForm();

	BOOL DoCreateDialog();

	// start / stop test
	void StartTiming();
	void StopTiming();
	void OnPhaseComplete();

private:
	// Dialog Data
	enum { IDD = IDD_DIALOG_DEC_PERFORMANCE };

	void OnInitialize();
	void OnSize(UINT nType, int cx, int cy);
	void OnBrowseClick();
    void OnCbnSelChange();
    void OnComboDecoderSelChange();
    void OnBnClickedButtonPropertypage();

	int BuildPerformanceGraph(IGraphBuilder *gb);
	void OnStartClick();
	void OnStopClick();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg void OnBuildGraphClick();

	void InsertListItem(const Timings& timings, int index, const CString& label);
};



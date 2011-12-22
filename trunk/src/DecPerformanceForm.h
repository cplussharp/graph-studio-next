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

public:
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

	int								phase_count;			// total number of runs
	int								phase_cur;				// the current run

	// average stats
	__int64							runtime_total_ns;
	__int64							frames_total;
    __int64                         realtime_total_ns;
	
public:
	CDecPerformanceForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDecPerformanceForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_DEC_PERFORMANCE };

	BOOL DoCreateDialog();
	void OnInitialize();
	void OnSize(UINT nType, int cx, int cy);
	void OnBrowseClick();
    void OnCbnSelChange();

	// start / stop test
	void Start();
	void Stop();
	void OnPhaseComplete();

	int BuildPerformanceGraph(IGraphBuilder *gb);
	void OnStartClick();
	void OnStopClick();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
};



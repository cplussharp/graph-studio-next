//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "AnalyzerPage.h"
#include "time_utils.h"

//-----------------------------------------------------------------------------
//
//	CAnalyzerPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAnalyzerPage, CDSPropertyPage)
	ON_WM_SIZE()
    ON_COMMAND(IDC_CHECK_ENABLED, &CAnalyzerPage::OnCheckClick)
    ON_COMMAND(IDC_CHECK_CRC, &CAnalyzerPage::OnCheckClick)
    ON_COMMAND(IDC_CHECK_ONLYSAMPLES, &CAnalyzerPage::OnCheckClick)
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CAnalyzerPage::OnBnClickedButtonReset)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_DATA, &CAnalyzerPage::OnLvnGetdispinfoListData)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_DATA, &CAnalyzerPage::OnCustomDrawListData)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PREVIEWBYTECOUNT, OnSpinDeltaByteCount)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CAnalyzerPage::OnBnClickedButtonRefresh)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CAnalyzerPage::OnBnClickedButtonSave)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CAnalyzerPage class
//
//-----------------------------------------------------------------------------
CAnalyzerPage::CAnalyzerPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("AnalyzerPage"), pUnk, IDD, strTitle), isActiv(false), m_firstTimeStamp(0), m_nPreviewByteCount(0)
{
	// retval
	if (phr) *phr = NOERROR;
	filter = NULL;

    if(GraphStudio::HasFont(_T("Consolas")))
        GraphStudio::MakeFont(font_entries, _T("Consolas"), 8, false, false);
    else
        GraphStudio::MakeFont(font_entries, _T("Courier New"), 8, false, false);
}

CAnalyzerPage::~CAnalyzerPage()
{
	filter = NULL;
}


BOOL CAnalyzerPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

    // prepare listView
	m_listCtrl.SetFont(&font_entries);

    m_listCtrl.InsertColumn(Number,				_T("Nr"),					LVCFMT_RIGHT,	30);
    m_listCtrl.InsertColumn(TimeStamp,			_T("TimeStamp"),		    LVCFMT_RIGHT,	110);
    m_listCtrl.InsertColumn(TimeStampDif,		_T("TS dif (µs)"),  		LVCFMT_RIGHT,	70);
    m_listCtrl.InsertColumn(Kind,				_T("Kind"),					LVCFMT_LEFT,	180);
    m_listCtrl.InsertColumn(Discontinuity,		_T("Disc."),				LVCFMT_CENTER,	40);
    m_listCtrl.InsertColumn(Sync,				_T("Sync"),					LVCFMT_CENTER,	40);
    m_listCtrl.InsertColumn(Preroll,			_T("Prer."),				LVCFMT_CENTER,	40);
    m_listCtrl.InsertColumn(Start,				_T("Start"),				LVCFMT_RIGHT,	80);
    m_listCtrl.InsertColumn(Stop,				_T("Stop"),					LVCFMT_RIGHT,	80);
    m_listCtrl.InsertColumn(MediaStart,			_T("MediaStart"),			LVCFMT_RIGHT,	80);
    m_listCtrl.InsertColumn(MediaStop,			_T("MediaStop"),			LVCFMT_RIGHT,	80);
    m_listCtrl.InsertColumn(TypeSpecificFlags,	_T("TypeSpecificFlags"),	LVCFMT_LEFT,	150);
    m_listCtrl.InsertColumn(SampleFlags,		_T("SampleFlags"),			LVCFMT_LEFT,	150);
    m_listCtrl.InsertColumn(StreamID,			_T("HRESULT / StreamID"),	LVCFMT_LEFT,	150);
    m_listCtrl.InsertColumn(DataLength,			_T("DataLength"),			LVCFMT_RIGHT,	75);
    m_listCtrl.InsertColumn(DataCrc,			_T("DataCRC"),			    LVCFMT_RIGHT,	75);
    m_listCtrl.InsertColumn(Data,				_T("Data"),					LVCFMT_LEFT,	350);

    m_listCtrl.SetExtendedStyle( m_listCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP );

	m_listCtrl.GetWindowRect(&m_listCtrlBorder);
	ScreenToClient(&m_listCtrlBorder);
	CRect client;
	GetClientRect(&client);
	m_listCtrlBorder -= client;

    m_spinPreviewByteCount.SetRange32(0,64);

	return TRUE;
}

void CAnalyzerPage::OnSize(UINT nType, int cx, int cy)
{
	if (m_hWnd && m_listCtrlBorder.top > 0) {			// don't resize list control until we've worked out the list control border
		CRect client;
		GetClientRect(&client);
		if (m_listCtrl.m_hWnd)
			m_listCtrl.MoveWindow(m_listCtrlBorder + client, TRUE);

		if (title.m_hWnd) {
			title.GetClientRect(&client);
			title.SetWindowPos(NULL, 0, 0, cx, client.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
		}
	}
}

void CAnalyzerPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLEBAR, title);
    DDX_Control(pDX, IDC_LIST_DATA, m_listCtrl);
    DDX_Text(pDX, IDC_EDIT_PREVIEWBYTECOUNT, m_nPreviewByteCount);
    DDX_Control(pDX, IDC_SPIN_PREVIEWBYTECOUNT, m_spinPreviewByteCount);
}

HRESULT CAnalyzerPage::OnConnect(IUnknown *pUnknown)
{
    HRESULT hr = pUnknown->QueryInterface(__uuidof(IAnalyzerCommon), (void**)&filter);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CAnalyzerPage::OnActivate()
{
	// Read values
    VARIANT_BOOL isEnabled;
    filter->get_Enabled(&isEnabled);
    CheckDlgButton(IDC_CHECK_ENABLED, isEnabled ? BST_CHECKED : BST_UNCHECKED);

    int captureConfig;
    filter->get_CaptureConfiguration(&captureConfig);
    CheckDlgButton(IDC_CHECK_CRC, captureConfig&SCF_DataCrc ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_CHECK_ONLYSAMPLES, (captureConfig&~SCF_DataCrc) == SCF_MediaSample ? BST_CHECKED : BST_UNCHECKED);
    

    WORD previewByteCount;
    filter->get_PreviewSampleByteCount(&previewByteCount);
    m_nPreviewByteCount = previewByteCount;
    UpdateData(FALSE);

    __int64 entryCount;
    filter->get_EntryCount(&entryCount);
    m_listCtrl.SetItemCount((int) entryCount);

    isActiv = true;
    //set->SetCallback(this);

	return NOERROR;
}

HRESULT CAnalyzerPage::OnDisconnect()
{
    isActiv = false;
    //filter->SetCallback(NULL);
	filter = NULL;
	return NOERROR;
}

HRESULT CAnalyzerPage::OnApplyChanges()
{
    UpdateData();
    filter->put_Enabled(IsDlgButtonChecked(IDC_CHECK_ENABLED) ? VARIANT_TRUE : VARIANT_FALSE);
	ASSERT(m_nPreviewByteCount <= 0xFFFF);
    filter->put_PreviewSampleByteCount((unsigned short) m_nPreviewByteCount);

    int captureConfig = SCF_ALL;
    if (IsDlgButtonChecked(IDC_CHECK_ONLYSAMPLES))
        captureConfig = SCF_MediaSample;

    if(IsDlgButtonChecked(IDC_CHECK_CRC))
        captureConfig |= SCF_DataCrc;
    else
        captureConfig &= ~SCF_DataCrc;

    filter->put_CaptureConfiguration(captureConfig);

	return NOERROR;
}

void CAnalyzerPage::OnCheckClick()
{
    if(isActiv)
	    SetDirty();
}

void CAnalyzerPage::OnSpinDeltaByteCount(NMHDR* pNMHDR, LRESULT* pResult)
{
    if(isActiv)
	    SetDirty();
}

void CAnalyzerPage::OnBnClickedButtonReset()
{
    if(filter)
        filter->ResetStatistic();

    m_listCtrl.SetItemCount(0);
    m_firstTimeStamp = 0;
}

void CAnalyzerPage::FreeEntryData(StatisticRecordEntry &entry) const
{
    if(entry.aData != NULL && entry.nDataCount > 0) 
    {
        CoTaskMemFree(entry.aData);
        entry.aData = NULL;
        entry.nDataCount = 0;
    }
}

const CString CAnalyzerPage::GetEntryString(__int64 entryNr, int field, bool commaFormattedTimestamps) const
{
    CString val;
    StatisticRecordEntry entry;
    if (SUCCEEDED(filter->GetEntry(entryNr, &entry)))
    {
        //Which column?
        switch(field)
        {
            case Number:
                val.Format(_T("%d"), entry.EntryNr);
                break;

            case TimeStamp:
                {
                    __int64 timeStamp = entry.EntryTimeStamp;
					__int64 tHour = timeStamp / 60 / 60 / 1000 / 1000 / 10;
                    timeStamp -= tHour * 60 * 60 * 1000 * 1000 * 10;
                    __int64 tMin = timeStamp / 60 / 1000 / 1000 / 10;
                    timeStamp -= tMin * 60 * 1000 * 1000 * 10;
                    __int64 tSec = timeStamp / 1000 / 1000 / 10;
                    timeStamp -= tSec * 1000 * 1000 * 10;
                    __int64 tMs = timeStamp / 1000 / 10;
                    timeStamp -= tMs * 1000 * 10;
                    __int64 tMys = timeStamp / 10;

                    val.Format(_T("%d:%02d:%02d.%03d,%03d"), (int)tHour, (int)tMin, (int)tSec, (int)tMs, (int)tMys);
                }
                break;

            case TimeStampDif:
                {
                    __int64 timeStamp = entry.EntryTimeStamp;
                    StatisticRecordEntry prevEntry;
                    if (SUCCEEDED(filter->GetEntry(entry.EntryNr - 1, &prevEntry)))
                    {
                        timeStamp -= prevEntry.EntryTimeStamp;
                        FreeEntryData(prevEntry);
                    }
                    else
                        timeStamp = 0;

                    timeStamp /= 10;
					if (commaFormattedTimestamps)
						val = CommaFormat(timeStamp);
					else
						val.Format(_T("%I64d"),timeStamp);
                }
                break;

            case Kind:
                switch(entry.EntryKind)
                {
                    case SRK_Empty:						val = _T("Empty");									break;
                    case SRK_MediaSample:				val = _T("Sample");									break;
                    case SRK_StartStreaming:			val = _T("StartStr");								break;
                    case SRK_StopStreaming:				val = _T("StopStr");								break;

                    case SRK_IS_Write:					val = _T("IStream::Write");							break;
                    case SRK_IS_Read:					val = _T("IStream::Read");							break;
                    case SRK_IS_Seek:					val = _T("IStream::Seek");							break;
                    case SRK_IS_Commit:					val = _T("IStream::Commit");						break;

					case SRK_MS_SetPositions:			val = _T("IMediaSeeking::SetPositions");			break;
					case SRK_MS_SetRate:				val = _T("IMediaSeeking::SetRate");					break;
					case SRK_MS_SetTimeFormat:			val = _T("IMediaSeeking::SetTimeFormat");			break;

					case SRK_MP_SetCurrentPosition:		val = _T("IMediaPosition::put_CurrentPosition");	break;
					case SRK_MP_SetPrerollTime:			val = _T("IMediaPosition::put_PrerollTime");		break;
					case SRK_MP_SetRate:				val = _T("IMediaPosition::put_Rate");				break;
					case SRK_MP_SetStopTime:			val = _T("IMediaPosition::put_StopTime");			break;

					case SRK_BF_JoinFilterGraph:		val = _T("IBaseFilter::JoinFilterGraph");			break;
					case SRK_BF_Pause:					val = _T("IBaseFilter::Pause");						break;
					case SRK_BF_Run:					val = _T("IBaseFilter:Run:");						break;
					case SRK_BF_SetSyncSource:			val = _T("IBaseFilter::SetSyncSource");				break;
					case SRK_BF_Stop:					val = _T("IBaseFilter::Stop");						break;

					case SRK_IP_BeginFlush:				val = _T("IPin::BeginFlush");						break;
					case SRK_IP_Connect:				val = _T("IPin::Connect");							break;
					case SRK_IP_Disconnect:				val = _T("IPin::Disconnect");						break;
					case SRK_IP_EndFlush:				val = _T("IPin::EndFlush");							break;
					case SRK_IP_EndOfStream:			val = _T("IPin::EndOfStream");						break;
					case SRK_IP_NewSegment:				val = _T("IPin::NewSegment");						break;
					case SRK_IP_ReceiveConnection:		val = _T("IPin::ReceiveConnection");				break;

					case SRK_MIP_NotifyAllocator:		val = _T("IMemInputPin::NotifyAllocator");			break;

					case SRK_QC_Notify:					val = _T("IQualityControl::Notify");				break;
					case SRK_QC_SetSink:				val = _T("IQualityControl::SetSink");				break;

					default:							val = _T("** UNKNOWN **");							break;
				}
                break;

            case Discontinuity:
                val = entry.IsDiscontinuity ? _T("x") : _T("");
                break;

            case Sync:
                val = entry.IsSyncPoint ? _T("x") : _T("");
                break;

            case Preroll:
                val = entry.IsPreroll ? _T("x") : _T("");
                break;

            case Start:
                if (entry.StreamTimeStart >= 0) {
					if (commaFormattedTimestamps)
						val = CommaFormat(entry.StreamTimeStart);
					else
						val.Format(_T("%I64d"),entry.StreamTimeStart);
				}
				break;

            case Stop:
                if (entry.StreamTimeStop >= 0) {
					if (commaFormattedTimestamps)
						val = CommaFormat(entry.StreamTimeStop);
					else
						val.Format(_T("%I64d"),entry.StreamTimeStop);
				}
                break;

            case MediaStart:
				// For rate we store double as LONGLONG multiplied up by UNITS
				if  (entry.EntryKind == SRK_MS_SetRate || entry.EntryKind == SRK_MP_SetRate)
                    val.Format(_T("%f"), ((double)entry.MediaTimeStart)/UNITS);
                else if (entry.MediaTimeStart >= 0) {
					if (commaFormattedTimestamps)
						val = CommaFormat(entry.MediaTimeStart);
					else
						val.Format(_T("%I64d"), entry.MediaTimeStart);
				}
                break;

            case MediaStop:
                if (entry.MediaTimeStop >= 0) {
					if (commaFormattedTimestamps)
						val = CommaFormat(entry.MediaTimeStop);
					else
						val.Format(_T("%I64d"),entry.MediaTimeStop);
				}
                break;

            case DataLength:
				if (entry.ActualDataLength != 0) {
					if (entry.EntryKind != SRK_IS_Seek && entry.EntryKind != SRK_MS_SetPositions 
								&& entry.EntryKind != SRK_MS_SetRate && entry.EntryKind != SRK_MS_SetTimeFormat)
						val.Format(_T("%d"),entry.ActualDataLength);
				}
                break;

            case DataCrc:
				if (entry.crcData != 0)
					val.Format(_T("0x%08lX"), entry.crcData);
                break;

            case Data:
                if (entry.nDataCount > 0 && entry.aData != NULL)
                {
                    for(int i=0; i<entry.nDataCount; i++)
                    {
                        CString b;
                        b.Format(_T("%02X"), entry.aData[i]);

                        if (i>0)
                        {
                            if (i%8 != 0)
                                val.Append(_T(" "));
                            else
                                val.Append(_T("   "));
                        }

                        val.Append(b);
                    }
                }
                break;

            case TypeSpecificFlags:
				if (entry.EntryKind == SRK_QC_Notify) {
					switch (entry.TypeSpecificFlags) {
						case Famine:	val = _T("Famine");				break;
						case Flood:		val = _T("Flood");				break;
						default:		val = _T("** UNKNOWN **");		break;
					}

                } else if (entry.HadIMediaSample2) {
                    CStringArray values;
					long fieldMask = entry.TypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK;
                    if (fieldMask == AM_VIDEO_FLAG_INTERLEAVED_FRAME)
                        values.Add(_T("INTERLEAVED_FRAME"));
                    else if (fieldMask == AM_VIDEO_FLAG_FIELD1)
                        values.Add(_T("FIELD1"));
                    else if (fieldMask == AM_VIDEO_FLAG_FIELD2)
                        values.Add(_T("FIELD2"));

                    if (entry.TypeSpecificFlags & AM_VIDEO_FLAG_FIELD1FIRST)
                        values.Add(_T("FIELD1FIRST"));

                    if (entry.TypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
                        values.Add(_T("WEAVE"));

                    if (entry.TypeSpecificFlags & AM_VIDEO_FLAG_REPEAT_FIELD)
                        values.Add(_T("REPEAT_FIELD"));

                    if (entry.TypeSpecificFlags & AM_ReverseBlockStart)
                        values.Add(_T("AM_ReverseBlockStart"));

                    if (entry.TypeSpecificFlags & AM_ReverseBlockEnd)
                        values.Add(_T("AM_ReverseBlockEnd"));

                    if (entry.TypeSpecificFlags & AM_UseNewCSSKey)
                        values.Add(_T("AM_UseNewCSSKey"));

                    for(int i=0;i<values.GetCount(); i++)
                    {
                        if(i > 0) val += _T(", ");
                        val += values[i];
                    }
				}
                break;

            case SampleFlags:
				if (entry.EntryKind == SRK_MS_SetPositions) {
					val = FormatSetPositionsFlags(entry.SampleFlags);
                } else if (entry.EntryKind == SRK_IS_Seek) {
                    if (entry.SampleFlags == STREAM_SEEK_SET)
                        val = _T("STREAM_SEEK_SET");
                    else if (entry.SampleFlags == STREAM_SEEK_CUR)
                        val = _T("STREAM_SEEK_CUR");
                    else if (entry.SampleFlags == STREAM_SEEK_END)
                        val = _T("STREAM_SEEK_END");
                } else if (entry.EntryKind == SRK_IS_Commit) {
                    if (entry.SampleFlags == STGC_DEFAULT)
                        val = _T("STGC_DEFAULT");
                    else
                    {
                        CStringArray values;
                        if (entry.SampleFlags & STGC_OVERWRITE)
                            values.Add(_T("STGC_OVERWRITE"));
                        if (entry.SampleFlags & STGC_ONLYIFCURRENT)
                            values.Add(_T("STGC_ONLYIFCURRENT"));
                        if (entry.SampleFlags & STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE)
                            values.Add(_T("STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE"));
                        if (entry.SampleFlags & STGC_CONSOLIDATE)
                            values.Add(_T("STGC_CONSOLIDATE"));

                        for (int i=0;i<values.GetCount(); i++)
                        {
                            if(i > 0) val += _T(", ");
                            val += values[i];
                        }
                    }
                } else if (entry.HadIMediaSample2) {
                    CStringArray values;

                    if (entry.SampleFlags & AM_SAMPLE_SPLICEPOINT)
                        values.Add(_T("SPLICEPOINT"));
                    if (entry.SampleFlags & AM_SAMPLE_PREROLL)
                        values.Add(_T("PREROLL"));
                    if (entry.SampleFlags & AM_SAMPLE_DATADISCONTINUITY)
                        values.Add(_T("DATADISCONTINUITY"));
                    if (entry.SampleFlags & AM_SAMPLE_TYPECHANGED)
                        values.Add(_T("TYPECHANGED"));
                    if (entry.SampleFlags & AM_SAMPLE_TIMEVALID)
                        values.Add(_T("TIMEVALID"));
                    if (entry.SampleFlags & AM_SAMPLE_TIMEDISCONTINUITY)
                        values.Add(_T("TIMEDISCONTINUITY"));
                    if (entry.SampleFlags & AM_SAMPLE_FLUSH_ON_PAUSE)
                        values.Add(_T("FLUSH_ON_PAUSE"));
                    if (entry.SampleFlags & AM_SAMPLE_STOPVALID)
                        values.Add(_T("STOPVALID"));
                    if (entry.SampleFlags & AM_SAMPLE_ENDOFSTREAM)
                        values.Add(_T("ENDOFSTREAM"));

                    for(int i=0;i<values.GetCount(); i++)
                    {
                        if(i > 0) val += _T(", ");
                        val += values[i];
                    }
                }
                break;

            case StreamID:
                if (entry.HadIMediaSample2)
                {
                    if(entry.StreamId == 0)
                        val = _T("AM_STREAM_MEDIA");
                    else if(entry.StreamId == 1)
                        val = _T("AM_STREAM_CONTROL");
                    else
                        val.Format(_T("0x%08lX"),entry.StreamId);
                } else {
					if (entry.StreamId != S_OK)				// zero default same as S_OK
						GraphStudio::NameHResult(entry.StreamId, val);
				}
                break;
        }

        // Free the additional Data of the entry
        FreeEntryData(entry);
    }

    return val;
}

CString CAnalyzerPage::FormatSetPositionsFlags(DWORD flags)
{
	CStringArray values;
	CString val;

	const DWORD pos = flags & AM_SEEKING_PositioningBitsMask;

	switch (pos) {
		case AM_SEEKING_AbsolutePositioning:		values.Add(_T("Absolute"));		break;
		case AM_SEEKING_RelativePositioning:		values.Add(_T("Relative"));		break;
		case AM_SEEKING_IncrementalPositioning:		values.Add(_T("Incremental"));		break;
		default:									return val;
	}

	if (flags & AM_SEEKING_SeekToKeyFrame)
		values.Add(_T("SeekToKeyFrame"));
	if (flags & AM_SEEKING_ReturnTime)
		values.Add(_T("ReturnTime"));
	if (flags & AM_SEEKING_Segment)
		values.Add(_T("Segment"));
	if (flags & AM_SEEKING_NoFlush)
		values.Add(_T("NoFlush"));

	for(int i=0;i<values.GetCount(); i++)
	{
		if(i > 0) val += _T(", ");
		val += values[i];
	}

	return val;
}

// from http://www.codeproject.com/Articles/7891/Using-virtual-lists
void CAnalyzerPage::OnLvnGetdispinfoListData(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
    
    //Create a pointer to the item
    LV_ITEM* pItem= &(pDispInfo)->item;

    //Do the list need text information?
    if (pItem->mask & LVIF_TEXT)
    {
        CString val = GetEntryString(pItem->iItem, pItem->iSubItem, true);		// format timestamps with commas

        //Copy the text to the LV_ITEM structure
        //Maximum number of characters is in pItem->cchTextMax
        lstrcpyn(pItem->pszText, val, pItem->cchTextMax);
    }

    *pResult = 0;
}

void CAnalyzerPage::OnCustomDrawListData(NMHDR *pNMHDR, LRESULT *pResult)
{
    COLORREF clrRow1 = RGB(255,255,255);
    COLORREF clrRow2 = RGB(224,224,224);

    *pResult = 0;

    LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
    DWORD_PTR iRow = lplvcd->nmcd.dwItemSpec;

    switch(lplvcd->nmcd.dwDrawStage)
    {
        case CDDS_PREPAINT :
            *pResult = CDRF_NOTIFYITEMDRAW;
            break;

        // Modify item text and or background
        case CDDS_ITEMPREPAINT:
            lplvcd->clrText = RGB(0,0,0);
            if(iRow % 2)
                lplvcd->clrTextBk = clrRow2;
            else
                lplvcd->clrTextBk = clrRow1;

            // If you want the sub items other then the item,
            // set *pResult to CDRF_NOTIFYSUBITEMDRAW
            *pResult = CDRF_NEWFONT;
            break;

        // Modify sub item text and/or background
        case CDDS_SUBITEM | CDDS_PREPAINT | CDDS_ITEM:
            if(iRow % 2)
                lplvcd->clrTextBk = clrRow2;
            else
                lplvcd->clrTextBk = clrRow1;
            *pResult = CDRF_DODEFAULT;
            break;
    }
}

void CAnalyzerPage::OnBnClickedButtonRefresh()
{
     __int64 entryCount;
    filter->get_EntryCount(&entryCount);
    m_listCtrl.SetItemCount((int) entryCount);
}


void CAnalyzerPage::OnBnClickedButtonSave()
{
    CString	fileFilter;
	CString	filename;

	fileFilter = _T("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*|");

	CFileDialog dlg(FALSE,_T("analyzer"),NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,fileFilter);
    INT_PTR ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK)
    {
		CPath path(filename);
		if (path.GetExtension() == _T(""))
        {
			path.AddExtension(_T(".csv"));
			filename = CString(path);
		}

        CFile file(filename, CFile::modeCreate|CFile::modeWrite);
        CString csvSep = CStatisticForm::GetCsvSeparator();

        // Output header
        CString row = _T("");
        for(int field=0; field < NumColumns; field++)
        {
            LVCOLUMN column = { 0 };
            column.mask = LVCF_TEXT;
            column.cchTextMax = 50;
            column.pszText = new TCHAR[column.cchTextMax];
            m_listCtrl.GetColumn(field, &column);

            if (field > 0)
                row.Append(csvSep);
            row.Append(column.pszText);

            delete [] column.pszText;
        }
        row.Append(_T("\n"));
        CT2CA outputHeaderText(row, CP_UTF8);
        file.Write(outputHeaderText, (DWORD) ::strlen(outputHeaderText));

        // Output data
        __int64 entryCount;
        filter->get_EntryCount(&entryCount);
        for(__int64 i = 0; i<entryCount; i++)
        {
            row = _T("");
            for(int field=0; field<NumColumns; field++)
            {
                if (field > 0)
                    row.Append(csvSep);
                row.Append(GetEntryString(i,field, false));		// don't format CSV with commas in timestamps
            }
            row.Append(_T("\n"));

            CT2CA outputText(row, CP_UTF8);
            file.Write(outputText, (DWORD) ::strlen(outputText));
        }
    }
}

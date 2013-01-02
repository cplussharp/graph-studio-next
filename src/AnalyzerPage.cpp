//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "AnalyzerPage.h"


//-----------------------------------------------------------------------------
//
//	CAnalyzerPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAnalyzerPage, CDSPropertyPage)
	ON_WM_SIZE()
    ON_COMMAND(IDC_CHECK_ENABLED, &CAnalyzerPage::OnCheckClick)
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CAnalyzerPage::OnBnClickedButtonReset)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_DATA, &CAnalyzerPage::OnLvnGetdispinfoListData)
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
    if(GraphStudio::HasFont(_T("Consolas")))
        GraphStudio::MakeFont(font_entries, _T("Consolas"), 8, false, false);
    else
        GraphStudio::MakeFont(font_entries, _T("Courier New"), 8, false, false);
	m_listCtrl.SetFont(&font_entries);

    m_listCtrl.InsertColumn(0, _T("Nr"), LVCFMT_RIGHT, 60);
    m_listCtrl.InsertColumn(1, _T("TimeStamp (dif)"), LVCFMT_RIGHT, 80);
    m_listCtrl.InsertColumn(2, _T("Kind"), LVCFMT_LEFT, 50);
    m_listCtrl.InsertColumn(3, _T("Disc."), LVCFMT_CENTER, 40);
    m_listCtrl.InsertColumn(4, _T("Sync"), LVCFMT_CENTER, 40);
    m_listCtrl.InsertColumn(5, _T("Prer."), LVCFMT_CENTER, 40);
    m_listCtrl.InsertColumn(6, _T("Start"), LVCFMT_RIGHT, 70);
    m_listCtrl.InsertColumn(7, _T("Stop"), LVCFMT_RIGHT, 70);
    m_listCtrl.InsertColumn(8, _T("MediaStart"), LVCFMT_RIGHT, 70);
    m_listCtrl.InsertColumn(9, _T("MediaStop"), LVCFMT_RIGHT, 70);
    m_listCtrl.InsertColumn(10, _T("DataLength"), LVCFMT_RIGHT, 70);
    m_listCtrl.InsertColumn(11, _T("Data"), LVCFMT_LEFT, 150);
    m_listCtrl.InsertColumn(12, _T("TypeSpecificFlags"), LVCFMT_LEFT, 150);
    m_listCtrl.InsertColumn(13, _T("SampleFlags"), LVCFMT_LEFT, 150);
    m_listCtrl.InsertColumn(14, _T("StreamID"), LVCFMT_LEFT, 70);

    m_listCtrl.SetExtendedStyle( m_listCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );

    m_spinPreviewByteCount.SetRange32(0,64);

	return TRUE;
}

void CAnalyzerPage::OnSize(UINT nType, int cx, int cy)
{
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
    HRESULT hr = pUnknown->QueryInterface(__uuidof(IAnalyzerFilter), (void**)&filter);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CAnalyzerPage::OnActivate()
{
	// Read values
    VARIANT_BOOL isEnabled;
    filter->get_Enabled(&isEnabled);

    CheckDlgButton(IDC_CHECK_ENABLED, isEnabled ? BST_CHECKED : BST_UNCHECKED);

    WORD previewByteCount;
    filter->get_PreviewSampleByteCount(&previewByteCount);
    m_nPreviewByteCount = previewByteCount;
    UpdateData(FALSE);

    __int64 entryCount;
    filter->get_EntryCount(&entryCount);
    m_listCtrl.SetItemCount(entryCount);

    isActiv = true;
    //set->SetCallback(this);

	return NOERROR;
}

HRESULT CAnalyzerPage::OnDisconnect()
{
    isActiv = false;
    filter->SetCallback(NULL);
	filter = NULL;
	return NOERROR;
}

HRESULT CAnalyzerPage::OnApplyChanges()
{
    UpdateData();
    filter->put_Enabled(IsDlgButtonChecked(IDC_CHECK_ENABLED) ? VARIANT_TRUE : VARIANT_FALSE);
    filter->put_PreviewSampleByteCount(m_nPreviewByteCount);
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

const CString CAnalyzerPage::GetEntryString(__int64 entryNr, int field) const
{
    CString val;
    StatisticRecordEntry entry;
    if (SUCCEEDED(filter->GetEntry(entryNr, &entry)))
    {
        //Which column?
        switch(field)
        {
            case 0:
                val.Format(_T("%d"), entry.EntryNr);
                break;

            case 1:
                {
                    __int64 timeStamp = entry.EntryTimeStamp;
                    StatisticRecordEntry prevEntry;
                    if (SUCCEEDED(filter->GetEntry(entry.EntryNr - 1, &prevEntry)))
                    {
                        timeStamp -= prevEntry.EntryTimeStamp;
                        FreeEntryData(prevEntry);
                    }
                    val.Format(_T("%I64d"),timeStamp);
                }
                break;

            case 2:
                switch(entry.EntryKind)
                {
                    case SRK_Empty:
                        val = _T("Empty");
                        break;
                    case SRK_MediaSample:
                        val = _T("Sample");
                        break;
                    case SRK_StartStreaming:
                        val = _T("StartStr");
                        break;
                    case SRK_StopStreaming:
                        val = _T("StopStr");
                        break;
                    case SRK_IS_Write:
                        val = _T("IStream::Write");
                        break;
                    case SRK_IS_Read:
                        val = _T("IStream::Read");
                        break;
                    case SRK_IS_Seek:
                        val = _T("IStream::Seek");
                        break;
                }
                break;
        }

        if (entry.EntryKind == SRK_MediaSample || entry.EntryKind == SRK_IS_Write || entry.EntryKind == SRK_IS_Read || entry.EntryKind == SRK_IS_Seek)
        {
            switch(field)
            {
                case 3:
                    val = entry.IsDiscontinuity ? _T("x") : _T("");
                    break;

                case 4:
                    val = entry.IsSyncPoint ? _T("x") : _T("");
                    break;

                case 5:
                    val = entry.IsPreroll ? _T("x") : _T("");
                    break;

                case 6:
                    if (entry.StreamTimeStart >= 0 && entry.EntryKind != SRK_IS_Write)
                        val.Format(_T("%I64d"),entry.StreamTimeStart);
                    break;

                case 7:
                    if (entry.StreamTimeStop >= 0 && entry.EntryKind == SRK_MediaSample)
                        val.Format(_T("%I64d"),entry.StreamTimeStop);
                    break;

                case 8:
                    if (entry.MediaTimeStart >= 0 && entry.EntryKind == SRK_MediaSample)
                        val.Format(_T("%I64d"),entry.MediaTimeStart);
                    break;

                case 9:
                    if (entry.MediaTimeStop >= 0 && entry.EntryKind == SRK_MediaSample)
                        val.Format(_T("%I64d"),entry.MediaTimeStop);
                    break;

                case 10:
                    if (entry.EntryKind != SRK_IS_Seek)
                        val.Format(_T("%d"),entry.ActualDataLength);
                    break;

                case 11:
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

                case 12:
                    if (entry.HadIMediaSample2)
                    {
                        CStringArray values;
                        long fieldMask = entry.SampleFlags & AM_VIDEO_FLAG_FIELD_MASK;
                        if (fieldMask == AM_VIDEO_FLAG_INTERLEAVED_FRAME)
                            values.Add(_T("INTERLEAVED_FRAME"));
                        else if (fieldMask == AM_VIDEO_FLAG_FIELD1)
                            values.Add(_T("FIELD1"));
                        else if (fieldMask == AM_VIDEO_FLAG_FIELD2)
                            values.Add(_T("FIELD2"));

                        if (entry.SampleFlags & AM_VIDEO_FLAG_FIELD1FIRST)
                            values.Add(_T("FIELD1FIRST"));

                        if (entry.SampleFlags & AM_VIDEO_FLAG_WEAVE)
                            values.Add(_T("WEAVE"));

                        if (entry.SampleFlags & AM_VIDEO_FLAG_REPEAT_FIELD)
                            values.Add(_T("REPEAT_FIELD"));

                        if (entry.SampleFlags & AM_ReverseBlockStart)
                            values.Add(_T("AM_ReverseBlockStart"));

                        if (entry.SampleFlags & AM_ReverseBlockEnd)
                            values.Add(_T("AM_ReverseBlockEnd"));

                        if (entry.SampleFlags & AM_UseNewCSSKey)
                            values.Add(_T("AM_UseNewCSSKey"));

                        for(int i=0;i<values.GetCount(); i++)
                        {
                            if(i > 0) val += _T(", ");
                            val += values[i];
                        }
                    }
                    break;

                case 13:
                    if (entry.HadIMediaSample2)
                    {
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

                case 14:
                    if (entry.HadIMediaSample2)
                    {
                        if(entry.StreamId == 0)
                            val = _T("AM_STREAM_MEDIA");
                        else if(entry.StreamId == 1)
                            val = _T("AM_STREAM_CONTROL");
                        else
                            val.Format(_T("0x%08lX"),entry.StreamId);
                    }
                    break;
            }
        }

        // Free the additional Data of the entry
        FreeEntryData(entry);
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
        CString val = GetEntryString(pItem->iItem, pItem->iSubItem);

        //Copy the text to the LV_ITEM structure
        //Maximum number of characters is in pItem->cchTextMax
        lstrcpyn(pItem->pszText, val, pItem->cchTextMax);
    }

    *pResult = 0;
}

void CAnalyzerPage::OnBnClickedButtonRefresh()
{
     __int64 entryCount;
    filter->get_EntryCount(&entryCount);
    m_listCtrl.SetItemCount(entryCount);
}


void CAnalyzerPage::OnBnClickedButtonSave()
{
    CString	fileFilter;
	CString	filename;

	fileFilter = _T("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*|");

	CFileDialog dlg(FALSE,_T("analyzer"),NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,fileFilter);
    int ret = dlg.DoModal();

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

        // Output header
        CString row = _T("");
        for(int field=0; field < 15; field++)
        {
            LVCOLUMN column = { 0 };
            column.mask = LVCF_TEXT;
            column.cchTextMax = 50;
            column.pszText = new TCHAR[column.cchTextMax];
            m_listCtrl.GetColumn(field, &column);

            if (field > 0)
                row.Append(_T(";"));
            row.Append(column.pszText);

            delete [] column.pszText;
        }
        row.Append(_T("\n"));
        CT2CA outputHeaderText(row, CP_UTF8);
        file.Write(outputHeaderText, ::strlen(outputHeaderText));

        // Output data
        __int64 entryCount;
        filter->get_EntryCount(&entryCount);
        for(__int64 i = 0; i<entryCount; i++)
        {
            row = _T("");
            for(int field=0; field<15; field++)
            {
                if (field > 0)
                    row.Append(_T(";"));
                row.Append(GetEntryString(i,field));
            }
            row.Append(_T("\n"));

            CT2CA outputText(row, CP_UTF8);
            file.Write(outputText, ::strlen(outputText));
        }
    }
}

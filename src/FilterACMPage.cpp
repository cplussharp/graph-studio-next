//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CAudioCompressionPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAudioCompressionPage, CDSPropertyPage)
	ON_CBN_SELCHANGE(IDC_COMBO_FORMAT, &CAudioCompressionPage::OnFormatChange)
	ON_CBN_SELCHANGE(IDC_COMBO_SAMPLERATE, &CAudioCompressionPage::OnSampleRateChange)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELS, &CAudioCompressionPage::OnChannelChange)
	ON_CBN_SELCHANGE(IDC_COMBO_BITRATE, &CAudioCompressionPage::OnBitrateChange)
	ON_COMMAND(IDC_BUTTON_APPLY, &CAudioCompressionPage::OnBtnApplyClick)
END_MESSAGE_MAP()

int ValueIndex(CArray<int> &items, int val)
{
	for (int i=0; i<items.GetCount(); i++) { if (items[i] == val) return i; }
	return -1;
}

bool IsValueInside(CArray<int> &items, int val) { int idx = ValueIndex(items, val); return (idx >= 0); }
int ArrayCompare(int &v1, int &v2) { return (v1 > v2 ? 1 : -1); }
void ArraySwap(CArray<int> &items, SSIZE_T i, SSIZE_T j)
{
	if (i == j) return ;	int temp = items[i];	
	items[i] = items[j];	items[j] = temp;
}
void _ArraySort_(CArray<int> &items, SSIZE_T lo, SSIZE_T hi)
{
	SSIZE_T i = lo, j = hi;
	int m;
	m = items[ (lo+hi)>>1 ];
	do {
		while (ArrayCompare(m, items[i])>0) i++;
		while (ArrayCompare(items[j], m)>0) j--;
		if (i <= j) { ArraySwap(items, i, j); i++; j--;	}
	} while (i <= j);
	if (j > lo) _ArraySort_(items, lo, j);
	if (i < hi) _ArraySort_(items, i, hi);
}
void ArraySort(CArray<int> &items)
{
	if (items.GetCount() == 0) return ;
	_ArraySort_(items, 0, items.GetCount()-1);
}	



//-----------------------------------------------------------------------------
//
//	CAudioCompressionPage class
//
//-----------------------------------------------------------------------------

CAudioCompressionPage::CAudioCompressionPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("ACMPage"), pUnk, IDD, strTitle)
{
	// retval
	if (phr) *phr = NOERROR;
	streamconfig = NULL;
}

CAudioCompressionPage::~CAudioCompressionPage()
{
	streamconfig = NULL;
}

void CAudioCompressionPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_STATIC_FORMAT, label_current_format);
	DDX_Control(pDX, IDC_STATIC_CHANNELS, label_current_channels);
	DDX_Control(pDX, IDC_STATIC_SAMPLERATE, label_current_samplerate);
	DDX_Control(pDX, IDC_STATIC_BITRATE, label_current_bitrate);
	DDX_Control(pDX, IDC_COMBO_FORMAT, cb_format);
	DDX_Control(pDX, IDC_COMBO_SAMPLERATE, cb_samplerate);
	DDX_Control(pDX, IDC_COMBO_CHANNELS, cb_channels);
	DDX_Control(pDX, IDC_COMBO_BITRATE, cb_bitrate);
}

// overriden
BOOL CAudioCompressionPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	// load the caps list
	caps_list.Refresh(streamconfig);
	ReloadFormats();
	ReloadCurrent();

	return TRUE;
}

void CAudioCompressionPage::OnBtnApplyClick()
{
	// must have valid values
	if (sel_format <= 0 ||
		sel_samplerate <= 0 ||
		sel_channels <= 0 ||
		sel_bitrate <= 0
		) return ;
	if (!streamconfig) return ;

	// get the format we want
	AudioFormatCaps	*cap = caps_list.Find(sel_format, sel_samplerate, sel_channels, sel_bitrate);
	if (!cap) return ;

	// okay. now create the media type
	CMediaType		mt;
	mt.InitMediaType();

	// copy all relevant media type values
	mt.bFixedSizeSamples	= cap->mt.bFixedSizeSamples;
	mt.bTemporalCompression	= cap->mt.bTemporalCompression;
	mt.formattype			= cap->mt.formattype;
	mt.lSampleSize			= cap->mt.lSampleSize;
	mt.majortype			= cap->mt.majortype;
	mt.subtype				= cap->mt.subtype;

	BYTE			*extrabuf = (BYTE*)mt.AllocFormatBuffer(cap->mt.cbFormat);
	BYTE			*extradata = extrabuf + sizeof(WAVEFORMATEX);
	WAVEFORMATEX	*wfx = (WAVEFORMATEX*)extrabuf;
	int				wfxsize = sizeof(WAVEFORMATEX);

	memcpy(wfx, &cap->wfx, wfxsize);

	// extradata
	if (cap->extrasize > 0) {
		memcpy(extradata, cap->extradata, cap->extrasize);
	}

	// now try to set the format
	HRESULT	hr = streamconfig->SetFormat(&mt);
	
	// and refresh the changes
	ReloadCurrent();
}

void CAudioCompressionPage::OnFormatChange()
{
	sel_format = formats[cb_format.GetCurSel()];
	ReloadSampleRates();
}

void CAudioCompressionPage::OnSampleRateChange()
{
	sel_samplerate = samplerates[cb_samplerate.GetCurSel()];
	ReloadChannels();
}

void CAudioCompressionPage::OnChannelChange()
{
	sel_channels = channels[cb_channels.GetCurSel()];
	ReloadBitrates();
}

void CAudioCompressionPage::OnBitrateChange()
{
	sel_bitrate = bitrates[cb_bitrate.GetCurSel()];
}

void CAudioCompressionPage::ReloadCurrent()
{
	if (!streamconfig) return ;

	AM_MEDIA_TYPE	*mt;
	HRESULT			hr;

	hr = streamconfig->GetFormat(&mt);
	if (FAILED(hr)) {

		label_current_format.SetWindowText(_T("Filter must be connected first"));
		label_current_channels.SetWindowText(_T(""));
		label_current_samplerate.SetWindowText(_T(""));
		label_current_bitrate.SetWindowText(_T(""));

		return ;
	}

	if (mt->formattype == FORMAT_WaveFormatEx && mt->cbFormat >= sizeof(WAVEFORMATEX)) {
		const WAVEFORMATEX	* const wfx	= (WAVEFORMATEX*)mt->pbFormat;

		// get parameters
		int		format = wfx->wFormatTag;
		int		bitrate = wfx->nAvgBytesPerSec * 8;
		int		channs = wfx->nChannels;
		int		samplerate = wfx->nSamplesPerSec;

		// fill in the UI values
		CString	str;

		GraphStudio::GetFormatName(format, str);		label_current_format.SetWindowText(str);
		str.Format(_T("%d"), channs);					label_current_channels.SetWindowText(str);
		str.Format(_T("%d Hz"), samplerate);			label_current_samplerate.SetWindowText(str);
		str.Format(_T("%5.3f kbps"), (float)(bitrate / 1000.0));	label_current_bitrate.SetWindowText(str);

		sel_format = format;
		sel_samplerate = samplerate;
		sel_channels = channs;
		sel_bitrate = bitrate;

		// update the combo boxes
		ReloadFormats();
	}

	DeleteMediaType(mt);
}

void CAudioCompressionPage::ReloadFormats()
{
	int i;

	cb_format.ResetContent();

	// load the formats
	caps_list.EnumFormats(formats);
	if (formats.GetCount() <= 0) {
		sel_format = -1;
	} else {
		sel_format = formats[0];
		for (i=0; i<formats.GetCount(); i++) {
			CString		fmt_name;
			GraphStudio::GetFormatName(formats[i], fmt_name);
			cb_format.AddString(fmt_name);
		}
		cb_format.SetCurSel(0);
	}

	// load the samplerates
	ReloadSampleRates();
}

void CAudioCompressionPage::ReloadSampleRates()
{
	cb_samplerate.ResetContent();

	if (sel_format >= 0) {

		int i;

		// load the formats
		caps_list.EnumSampleRates(samplerates, sel_format);
		if (samplerates.GetCount() <= 0) {
			sel_samplerate = 0;
		} else {

			int		idx = ValueIndex(samplerates, sel_samplerate);
			if (idx < 0) {
				sel_samplerate = samplerates[0];
				idx = 0;
			}

			for (i=0; i<samplerates.GetCount(); i++) {
				CString		str;
				str.Format(_T("%d Hz"), samplerates[i]);
				cb_samplerate.AddString(str);
			}

			cb_samplerate.SetCurSel(idx);
		}
	}

	ReloadChannels();
}

void CAudioCompressionPage::ReloadChannels()
{
	cb_channels.ResetContent();

	if (sel_format > 0 && sel_samplerate > 0) {

		int i;

		// load the formats
		caps_list.EnumChannels(channels, sel_format, sel_samplerate);
		if (channels.GetCount() <= 0) {
			sel_channels = 0;
		} else {

			int		idx = ValueIndex(channels, sel_channels);
			if (idx < 0) {
				sel_channels = channels[0];
				idx = 0;
			}

			for (i=0; i<channels.GetCount(); i++) {
				CString		str;
				str.Format(_T("%d"), channels[i]);
				cb_channels.AddString(str);
			}

			cb_channels.SetCurSel(idx);
		}
	}

	ReloadBitrates();
}

void CAudioCompressionPage::ReloadBitrates()
{
	cb_bitrate.ResetContent();

	if (sel_format > 0 && sel_samplerate > 0 && sel_channels > 0) {

		int i;

		// load the formats
		caps_list.EnumBitrates(bitrates, sel_format, sel_samplerate, sel_channels);
		if (bitrates.GetCount() <= 0) {
			sel_bitrate = 0;
		} else {

			int		idx = ValueIndex(bitrates, sel_bitrate);
			if (idx < 0) {
				sel_bitrate = bitrates[0];
				idx = 0;
			}

			for (i=0; i<bitrates.GetCount(); i++) {
				CString		str;

				str.Format(_T("%5.3f kbps"), (float)(bitrates[i]/1000.0));
				cb_bitrate.AddString(str);
			}
			cb_bitrate.SetCurSel(idx);
		}
	}
}


HRESULT CAudioCompressionPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IAMStreamConfig, (void**)&streamconfig);
	if (FAILED(hr)) return E_FAIL;
	
	return NOERROR;
}

HRESULT CAudioCompressionPage::OnDisconnect()
{
	caps_list.Clear();
	streamconfig = NULL;
	return NOERROR;
}



//-----------------------------------------------------------------------------
//
//	AudioFormatCaps class
//
//-----------------------------------------------------------------------------

AudioFormatCaps::AudioFormatCaps() :
	cap_index(0),
	extrasize(0),
	extradata(NULL)
{

}

AudioFormatCaps::~AudioFormatCaps()
{
	if (extradata) {
		free(extradata);
		extradata = NULL;
	}
}

int AudioFormatCaps::Load(int capindex, AM_MEDIA_TYPE *mtSrc, WAVEFORMATEX *wfxSrc)
{
	// store the capability index
	cap_index				= capindex;

	// copy all relevant media type values
	mt.bFixedSizeSamples	= mtSrc->bFixedSizeSamples;
	mt.bTemporalCompression	= mtSrc->bTemporalCompression;
	mt.cbFormat				= mtSrc->cbFormat;
	mt.formattype			= mtSrc->formattype;
	mt.lSampleSize			= mtSrc->lSampleSize;
	mt.majortype			= mtSrc->majortype;
	mt.subtype				= mtSrc->subtype;
	mt.pUnk					= NULL;
	mt.pbFormat				= NULL;
	
	// now load the waveformatex (Todo: Support for WAVEFORMATEXTENSIBLE)
	wfx.cbSize				= wfxSrc->cbSize;
	wfx.nAvgBytesPerSec		= wfxSrc->nAvgBytesPerSec;
	wfx.nBlockAlign			= wfxSrc->nBlockAlign;
	wfx.nChannels			= wfxSrc->nChannels;
	wfx.nSamplesPerSec		= wfxSrc->nSamplesPerSec;
	wfx.wBitsPerSample		= wfxSrc->wBitsPerSample;
	wfx.wFormatTag			= wfxSrc->wFormatTag;

	// and store the extradata
	if (extradata) { free(extradata); extradata = NULL; }
	extrasize = wfx.cbSize;
	if (extrasize > 0) {
		extradata = (uint8*)malloc(extrasize);
		if (extradata)
		{
			uint8	*src = (uint8*)wfxSrc + sizeof(WAVEFORMATEX);
			memcpy(extradata, src, extrasize);
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
//
//	AudioCapsList class
//
//-----------------------------------------------------------------------------

AudioCapsList::AudioCapsList()
{
}

AudioCapsList::~AudioCapsList()
{
	Clear();
}

void AudioCapsList::Clear()
{
	for (int i=0; i<caps.GetCount(); i++) {
		AudioFormatCaps *cap = caps[i];
		if (cap) delete cap;
	}
	caps.RemoveAll();
}

int	AudioCapsList::EnumFormats(CArray<int> &formats)
{
	formats.RemoveAll();
	for (int i=0; i<caps.GetCount(); i++) {
		AudioFormatCaps	*cap = caps[i];

		// all unique formats
		if (IsValueInside(formats, cap->wfx.wFormatTag) == false) {
			formats.Add(cap->wfx.wFormatTag);
		}
	}
	return 0;
}

int AudioCapsList::EnumSampleRates(CArray<int> &samplerates, int format)
{
	samplerates.RemoveAll();
	for (int i=0; i<caps.GetCount(); i++) {
		AudioFormatCaps	*cap = caps[i];

		// all unique samplerates for this format
		if (cap->wfx.wFormatTag == format) {
			if (IsValueInside(samplerates, cap->wfx.nSamplesPerSec) == false) {
				samplerates.Add(cap->wfx.nSamplesPerSec);
			}
		}
	}
	ArraySort(samplerates);
	return 0;
}

int AudioCapsList::EnumChannels(CArray<int> &channels, int format, int samplerate)
{
	channels.RemoveAll();
	for (int i=0; i<caps.GetCount(); i++) {
		AudioFormatCaps	*cap = caps[i];

		// all unique samplerates for this format
		if (cap->wfx.wFormatTag == format &&
			cap->wfx.nSamplesPerSec == samplerate
			) {
			if (IsValueInside(channels, cap->wfx.nChannels) == false) {
				channels.Add(cap->wfx.nChannels);
			}
		}
	}
	ArraySort(channels);
	return 0;
}

int AudioCapsList::EnumBitrates(CArray<int> &bitrates, int format, int samplerate, int channels)
{
	bitrates.RemoveAll();
	for (int i=0; i<caps.GetCount(); i++) {
		AudioFormatCaps	*cap = caps[i];

		// all unique samplerates for this format
		if (cap->wfx.wFormatTag == format &&
			cap->wfx.nSamplesPerSec == samplerate &&
			cap->wfx.nChannels == channels
			) {

			int		val = cap->wfx.nAvgBytesPerSec * 8;

			if (IsValueInside(bitrates, val) == false) {
				bitrates.Add(val);
			}
		}
	}
	ArraySort(bitrates);
	return 0;
}

AudioFormatCaps *AudioCapsList::Find(int format, int samplerate, int channels, int bitrate)
{
	AudioFormatCaps		*ret = NULL;
	AudioFormatCaps		*temp = NULL;
	int					idx = 0;

	while (true) {
		temp = FindFromIndex(idx, format, samplerate, channels, bitrate);
		if (!temp) break;

		if (ret) {
			// if multiple matches were found we prefer those with extradata
			if (temp->extrasize > 0) {
				ret = temp;
			}
		} else {
			ret = temp;
		}
		idx++;
	}

	return ret;
}

AudioFormatCaps *AudioCapsList::FindFromIndex(int &start_index, int format, int samplerate, int channels, int bitrate)
{
	for (int i=start_index; i<caps.GetCount(); i++) {
		AudioFormatCaps	*cap = caps[i];

		if (cap->wfx.wFormatTag == format &&
			cap->wfx.nSamplesPerSec == samplerate &&
			cap->wfx.nChannels == channels &&
			(cap->wfx.nAvgBytesPerSec*8) == bitrate
			) {
			start_index = i;
			return cap;
		}
	}
	start_index = (int) caps.GetCount();
	return NULL;
}

int AudioCapsList::Refresh(IAMStreamConfig *config)
{
	/*
		Refresh the complete list of capabilities
	*/

	Clear();
	if (!config) return -1;

	int		count, size;
	HRESULT	hr;

	// get the numbers...
	hr = config->GetNumberOfCapabilities(&count, &size);
	if (FAILED(hr)) return -1;

	// sanity checks...
	if (count <= 0) return -1;
	if (size != sizeof(AUDIO_STREAM_CONFIG_CAPS)) return -1;
	
	AUDIO_STREAM_CONFIG_CAPS		audiocaps;
	AM_MEDIA_TYPE					*mt;

	// loop through all the capabilities
	for (int i=0; i<count; i++) {
		hr = config->GetStreamCaps(i, &mt, (BYTE*)&audiocaps);
		if (SUCCEEDED(hr)) {

			// for now we only accept WAVEFORMATEX
			if (mt->formattype == FORMAT_WaveFormatEx && mt->cbFormat >= sizeof(WAVEFORMATEX)) {

				WAVEFORMATEX * const wfx = (WAVEFORMATEX*)mt->pbFormat;
				int				added_count = 0;

				// check the audiocaps
				if (audiocaps.MinimumBitsPerSample == 0 && audiocaps. MaximumBitsPerSample == 0) {
					audiocaps.MinimumBitsPerSample = wfx->wBitsPerSample;
					audiocaps.MaximumBitsPerSample = wfx->wBitsPerSample;
					audiocaps.BitsPerSampleGranularity = 1;
				}
				if (audiocaps.MinimumChannels == 0 && audiocaps.MaximumChannels == 0) {
					audiocaps.MinimumChannels = wfx->nChannels;
					audiocaps.MaximumChannels = wfx->nChannels;
					audiocaps.ChannelsGranularity = 1;
				}
				if (audiocaps.MinimumSampleFrequency == 0 && audiocaps.MaximumSampleFrequency == 0) {
					audiocaps.MinimumSampleFrequency = wfx->nSamplesPerSec;
					audiocaps.MaximumSampleFrequency = wfx->nSamplesPerSec;
					audiocaps.SampleFrequencyGranularity = 1;
				}

				// loop through all the possibilities
				for (ULONG bits = audiocaps.MinimumBitsPerSample; 
					 bits <= audiocaps.MaximumBitsPerSample; 
					 bits += audiocaps.BitsPerSampleGranularity) {

					for (ULONG chans = audiocaps.MinimumChannels;
						 chans <= audiocaps.MaximumChannels;
						 chans += audiocaps.ChannelsGranularity) {

						for (ULONG samplerate = audiocaps.MinimumSampleFrequency;
							 samplerate <= audiocaps.MaximumSampleFrequency;
							 samplerate += audiocaps.SampleFrequencyGranularity) {

							// set the parameters
							wfx->wBitsPerSample = (WORD) bits;
							wfx->nChannels = (WORD) chans;
							wfx->nSamplesPerSec = samplerate;

							// create the capability
							AudioFormatCaps	*cap = new AudioFormatCaps();
							cap->Load(i, mt, wfx);
							caps.Add(cap);
						}
					}
				}
			}

			DeleteMediaType(mt);
		}
	}

	return 0;
}

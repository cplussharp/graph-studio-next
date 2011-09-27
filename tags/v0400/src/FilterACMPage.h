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
//	AudioFormatCaps class
//
//-----------------------------------------------------------------------------
class AudioFormatCaps
{
public:

	AM_MEDIA_TYPE	mt;					// does not hold any associated buffers
	int				cap_index;			// index of the capability this format was taken from
	WAVEFORMATEX	wfx;
	int				extrasize;
	uint8			*extradata;

public:
	AudioFormatCaps();
	virtual ~AudioFormatCaps();

	int Load(int capindex, AM_MEDIA_TYPE *mtSrc, WAVEFORMATEX *wfxSrc);
};

//-----------------------------------------------------------------------------
//
//	AudioCapsList class
//
//-----------------------------------------------------------------------------
class AudioCapsList
{
public:

	CArray<AudioFormatCaps *>		caps;

public:
	AudioCapsList();
	virtual ~AudioCapsList();

	void Clear();
	int Refresh(IAMStreamConfig *config);

	// enumerating allowed values
	int	EnumFormats(CArray<int> &formats);
	int EnumSampleRates(CArray<int> &samplerates, int format);
	int EnumChannels(CArray<int> &channels, int format, int samplerate);
	int EnumBitrates(CArray<int> &bitrates, int format, int samplerate, int channels);

	AudioFormatCaps *FindFromIndex(int &start_index, int format, int samplerate, int channels, int bitrate);
	AudioFormatCaps *Find(int format, int samplerate, int channels, int bitrate);
};


//-----------------------------------------------------------------------------
//
//	CAudioCompressionPage class
//
//-----------------------------------------------------------------------------
class CAudioCompressionPage : public CDSPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

public:

	GraphStudio::TitleBar			title;
	CComPtr<IAMStreamConfig>		streamconfig;
	AudioCapsList					caps_list;

	// current configuration
	CStatic							label_current_format;
	CStatic							label_current_channels;
	CStatic							label_current_samplerate;
	CStatic							label_current_bitrate;

	// customizable fields
	CComboBox						cb_format;
	CComboBox						cb_samplerate;
	CComboBox						cb_channels;
	CComboBox						cb_bitrate;

	// allowed values
	CArray<int>						formats;
	CArray<int>						samplerates;
	CArray<int>						channels;
	CArray<int>						bitrates;

	// selected values
	int								sel_format;
	int								sel_samplerate;
	int								sel_channels;
	int								sel_bitrate;

	enum { IDD = IDD_DIALOG_AUDIO_COMPRESS };
public:
	CAudioCompressionPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CAudioCompressionPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();

	void ReloadFormats();
	void ReloadSampleRates();
	void ReloadChannels();
	void ReloadBitrates();
	void ReloadCurrent();

	void OnFormatChange();
	void OnSampleRateChange();
	void OnChannelChange();
	void OnBitrateChange();

	void OnBtnApplyClick();
};

//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


namespace DSUtil
{

	// {17CCA71B-ECD7-11D0-B908-00A0C9223196}
	static const GUID CLSID_KSProxy = 
	{ 0x17CCA71B, 0xECD7, 0x11D0, { 0xB9, 0x08, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96 } };

	// {C1F400A4-3F08-11D3-9F0B-006008039E37}
	static const GUID CLSID_NullRenderer =
	{ 0xC1F400A4, 0x3F08, 0x11D3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

	// { 36a5f770-fe4c-11ce-a8ed-00aa002feab5 }
	static const GUID CLSID_Dump =
	{ 0x36a5f770, 0xfe4c, 0x11ce, { 0xa8, 0xed, 0x00, 0xaa, 0x00, 0x2f, 0xea, 0xb5 } };

	typedef CArray<CMediaType>	MediaTypes;

    //-------------------------------------------------------------------------
	//
	//	ClassFactory class
	//
	//-------------------------------------------------------------------------

    class CClassFactory : public IClassFactory, public CBaseObject
    {
    private:
        const CFactoryTemplate *const m_pTemplate;
        ULONG m_cRef;
        static int m_cLocked;

    public:
        CClassFactory(const CFactoryTemplate *);
        
        STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
        STDMETHODIMP_(ULONG)AddRef();
        STDMETHODIMP_(ULONG)Release();
        
        STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **pv);
        STDMETHODIMP LockServer(BOOL fLock);

        void Register();
    
        static BOOL IsLocked() {
            return (m_cLocked > 0);
        };
    };  

	//-------------------------------------------------------------------------
	//
	//	URI class
	//
	//-------------------------------------------------------------------------

	class URI
	{
	public:
		CString		protocol;
		CString		host;
		CString		request_url;
		CString		complete_request;
		int			port;

	public:
		URI();
		URI(const URI &u);
		URI(CString url);
		virtual ~URI();

		URI &operator =(const URI &u);
		URI &operator =(CString url);

		int Parse(CString url);
	public:
		// standardne operatory
		operator CString() { return complete_request; }
	};

	//-------------------------------------------------------------------------
	//
	//	Helper Classes
	//
	//-------------------------------------------------------------------------

	class PinTemplate
	{
	public:
		PIN_DIRECTION	dir;
		BOOL			rendered;
		BOOL			many;
		int				types;
		CArray<GUID>	major;
		CArray<GUID>	minor;	

	public:
		PinTemplate();
		PinTemplate(const PinTemplate &pt);
		virtual ~PinTemplate();
		PinTemplate &operator =(const PinTemplate &pt);
	};

	class FilterTemplate
	{
	public:
		CString		name;
		CString		moniker_name;
		GUID		clsid;
		GUID		category;
		DWORD		version;
		DWORD		merit;
		IMoniker	*moniker;
		CString		file;
		bool		file_exists;

		CArray<PinTemplate>		input_pins;
		CArray<PinTemplate>		output_pins;

		enum {
			FT_FILTER	= 0,
			FT_DMO		= 1,
			FT_KSPROXY	= 2,
			FT_ACM_ICM	= 3,
			FT_PNP		= 4
		};
		int			type;

	public:
		FilterTemplate();
		FilterTemplate(const FilterTemplate &ft);
		virtual ~FilterTemplate();
		FilterTemplate &operator =(const FilterTemplate &ft);

		// vytvorenie instancie
		HRESULT CreateInstance(IBaseFilter **filter);
		HRESULT FindFilename();

		int LoadFromMoniker(CString displayname);
		int Load(char *buf, int size);
		int WriteMerit();
		int ParseMonikerName();
	};

	class FilterCategory
	{
	public:
		CString		name;
		GUID		clsid;
		bool		is_dmo;				// is this category DMO ?
	public:
		FilterCategory();
		FilterCategory(CString nm, GUID cat_clsid, bool dmo=false);
		FilterCategory(const FilterCategory &fc);
		virtual ~FilterCategory();
		FilterCategory &operator =(const FilterCategory &fc);
	};

	class FilterCategories
	{
	public:
		CArray<FilterCategory>	categories;
	public:
		FilterCategories();
		virtual ~FilterCategories();

		// enumeracia kategorii
		int Enumerate();
	};


	class FilterTemplates
	{
	public:
		CArray<FilterTemplate>	filters;
	public:
		FilterTemplates();
		virtual ~FilterTemplates();

		// enumeracia kategorii
		int Enumerate(FilterCategory &cat);
		int Enumerate(GUID clsid);
		int EnumerateDMO(GUID clsid);

		// enumerating compatible filters
		int EnumerateCompatible(MediaTypes &mtypes, DWORD min_merit, bool need_output, bool exact);

		// enumerating filters
        int EnumerateAudioSources();
		int EnumerateVideoSources();
		int EnumerateAudioRenderers();
		int EnumerateVideoRenderers();
        int EnumerateInternalFilters();
        int EnumerateAudioDecoder();
        int EnumerateVideoDecoder();
        int EnumerateAudioEncoder();
        int EnumerateVideoEncoder();
        int EnumerateMuxer();
        int EnumerateDemuxer();

		// vyhladavanie
		int Find(CString name, FilterTemplate *filter);
		int Find(GUID clsid, FilterTemplate *filter); 
		int AddFilters(IEnumMoniker *emoniker, int enumtype=0, GUID category=GUID_NULL);

		// testing
		int IsVideoRenderer(FilterTemplate &filter);

		void SortByName();
		void SwapItems(int i, int j);
		void _Sort_(int lo, int hi);

		// vytvaranie
		HRESULT CreateInstance(CString name, IBaseFilter **filter);
		HRESULT CreateInstance(GUID clsid, IBaseFilter **filter);
	};

	class Pin
	{
	public:
		IBaseFilter		*filter;
		IPin			*pin;
		CString			name;
		PIN_DIRECTION	dir;

		enum {
			PIN_FLAG_INPUT = 1,
			PIN_FLAG_OUTPUT = 2,
			PIN_FLAG_CONNECTED = 4,
			PIN_FLAG_NOT_CONNECTED = 8,
			PIN_FLAG_ALL = 0xffff
		};
	public:
		Pin();
		Pin(const Pin &p);
		virtual ~Pin();
		Pin &operator =(const Pin &p);
	};

	typedef CArray<Pin>			PinArray;

	// zobrazenie property pagesy
	HRESULT DisplayPropertyPage(IBaseFilter *filter, HWND parent = NULL);
	
	// enumeracie pinov
	HRESULT EnumPins(IBaseFilter *filter, PinArray &pins, int flags);
	HRESULT EnumMediaTypes(IPin *pin, MediaTypes &types);
	HRESULT ConnectFilters(IGraphBuilder *gb, IBaseFilter *output, IBaseFilter *input, bool direct=false);
	HRESULT ConnectPin(IGraphBuilder *gb, IPin *output, IBaseFilter *input, bool direct=false);

	bool IsVideoUncompressed(GUID subtype);
    bool IsAudioUncompressed(GUID subtype);

	CString get_next_token(CString &str, CString separator);

	// remove registry information
	HRESULT UnregisterFilter(GUID clsid, GUID category);
	HRESULT UnregisterCOM(GUID clsid);

    void ShowError(HRESULT hr, LPCTSTR title=NULL);
    void ShowError(LPCTSTR text, LPCTSTR title=NULL);
    void ShowInfo(LPCTSTR text, LPCTSTR title=NULL);
    void ShowWarning(LPCTSTR text, LPCTSTR title=NULL);

    bool InitSbeObject(CComQIPtr<IStreamBufferInitialize> pInit);

    bool IsOsWin7OrLater();
    bool IsOsWinVistaOrLater();
};







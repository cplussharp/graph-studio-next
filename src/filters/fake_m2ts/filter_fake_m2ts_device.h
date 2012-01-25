//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

// {609D207F-E89B-4839-9E17-1CBE285D1013}
static const GUID CLSID_FakeM2tsDevice = 
{ 0x609d207f, 0xe89b, 0x4839, { 0x9e, 0x17, 0x1c, 0xbe, 0x28, 0x5d, 0x10, 0x13 } };


//-----------------------------------------------------------------------------
//
//	CFakeM2tsDevice class
//
//-----------------------------------------------------------------------------

class CFakeM2tsDevice : public CSource, public IFileSourceFilter, public IAMFilterMiscFlags
{
public:
    CFakeM2tsDevice(LPUNKNOWN pUnk, HRESULT *phr);
    ~CFakeM2tsDevice(void) {}

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
    static const CFactoryTemplate g_Template;

    // IFileSourceFilter
    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetCurFile( LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt);

    // IAMFilterMiscFlags
    STDMETHODIMP_(ULONG) GetMiscFlags(void) { return AM_FILTER_MISC_FLAGS_IS_SOURCE; }

private:
    CComBSTR m_filename;
};



//-----------------------------------------------------------------------------
//
//	CFakeM2tsDeviceStream class
//
//-----------------------------------------------------------------------------

class CFakeM2tsDeviceStream: public CSourceStream
{
public:
    CFakeM2tsDeviceStream(HRESULT *phr, CSource *pParent, LPCWSTR pPinName, CString filename);
    ~CFakeM2tsDeviceStream(void);

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    // plots into the supplied video frame
    HRESULT FillBuffer(IMediaSample *pms);

    // Ask for buffers of the size appropriate to the agreed media type
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProperties);

    // Because we calculate there is no reason why we
    // can't calculate it in any one of a set of formats...
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);

    virtual HRESULT OnThreadCreate();

private:
    CFile m_file;
    BOOL m_hasStride;
};

//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


// {C6E6AFDF-338D-4bb6-8A16-ACA266701E76}
static const GUID CLSID_MonoDump = 
{ 0xc6e6afdf, 0x338d, 0x4bb6, { 0x8a, 0x16, 0xac, 0xa2, 0x66, 0x70, 0x1e, 0x76 } };


//-----------------------------------------------------------------------------
//
//	CMonoDump class
//
//-----------------------------------------------------------------------------

class CMonoDump : public CBaseRenderer, public IFileSinkFilter
{
public:

	// write the file here
	CFile       file;
	CString		filename;

	HRESULT DoOpenFile();
	HRESULT DoCloseFile();

public:
	CMonoDump(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CMonoDump();

	// expose IFileSinkFilter
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // Implements the IFileSinkFilter interface
    STDMETHODIMP SetFileName(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName,AM_MEDIA_TYPE *pmt);

	// overriden
	virtual HRESULT CheckMediaType(const CMediaType *pmt);
	virtual HRESULT DoRenderSample(IMediaSample *pMediaSample);

	// ignore time stamps...
	virtual HRESULT ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime);

	// open and close the dump file
	virtual HRESULT OnStartStreaming();
	virtual HRESULT OnStopStreaming();

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
};


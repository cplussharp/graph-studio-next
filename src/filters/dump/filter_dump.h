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
//	CMonoDump class
//
//-----------------------------------------------------------------------------

class CMonoDump : public CBaseRenderer, public IFileSinkFilter
{
public:

	// write the file here
#ifndef __AFX_H__
	CAtlFile       file;
#else
    CFile       file;
#endif
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


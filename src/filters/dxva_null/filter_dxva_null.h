//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
//
//	CDxvaNullRenderer class
//
//-----------------------------------------------------------------------------
class CDxvaNullRenderer : public CBaseRenderer
{
public:
	CDxvaNullRenderer(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CDxvaNullRenderer();

	// expose IFileSinkFilter
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// overriden
	virtual HRESULT CheckMediaType(const CMediaType *pmt);
	virtual HRESULT DoRenderSample(IMediaSample *pMediaSample);

	// ignore time stamps...
	virtual HRESULT ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime);

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
};


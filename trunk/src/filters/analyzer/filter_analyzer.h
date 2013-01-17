//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once


//-----------------------------------------------------------------------------
//
//	CAnalyzerFilter class
//
//-----------------------------------------------------------------------------

class CAnalyzerFilter : public CTransInPlaceFilter
{
private:
    CAnalyzer*  m_analyzer;

public:
	CAnalyzerFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CAnalyzerFilter();

    virtual CBasePin *GetPin(int n);

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// keep track of samples
	virtual HRESULT Transform(IMediaSample *pSample);
	virtual HRESULT StartStreaming();
	virtual HRESULT StopStreaming();
    virtual HRESULT CheckInputType(const CMediaType* mtIn);

    static const CFactoryTemplate g_Template;
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

	CAnalyzer* Analyzer() { return m_analyzer; }
};


//-----------------------------------------------------------------------------
//
//	CAnalyzerInputPin class
//
//-----------------------------------------------------------------------------

class CAnalyzerInputPin : public CTransInPlaceInputPin
{
public:
	CAnalyzerInputPin( __in_opt LPCTSTR     pObjectName
			, __inout CTransInPlaceFilter	*pFilter
			, __inout HRESULT				*phr
			, __in_opt LPCWSTR				 pName);

protected:
	CAnalyzer* Analyzer()
	{
		CAnalyzerFilter * const filter = dynamic_cast<CAnalyzerFilter*>(m_pFilter);
		return filter ? filter->Analyzer() : NULL;
	}
};

//-----------------------------------------------------------------------------
//
//	CAnalyzerOutputPin class
//
//-----------------------------------------------------------------------------

class CAnalyzerOutputPin : public CTransInPlaceOutputPin, protected IMediaSeeking
{
public:
    CAnalyzerOutputPin(
        __in_opt LPCTSTR				pObjectName,
        __inout CTransInPlaceFilter	*	pFilter,
        __inout HRESULT	*				phr,
        __in_opt LPCWSTR				pName);
    
	~CAnalyzerOutputPin();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv);

protected:
	CAnalyzer*		m_Analyzer;

	// Seeking helpers (based on CPosPassThru)
    HRESULT GetPeerSeeking(__deref_out IMediaSeeking **ppMS);
    HRESULT GetSeekingLongLong( HRESULT (__stdcall IMediaSeeking::*pMethod)( LONGLONG * ),
                                __out LONGLONG * pll );

    // IMediaSeeking methods (based on CPosPassThru)
    STDMETHODIMP GetCapabilities( __out DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( __inout DWORD * pCapabilities );
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetTimeFormat(__out GUID *pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsFormatSupported( const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat( __out GUID *pFormat);
    STDMETHODIMP ConvertTimeFormat(__out LONGLONG * pTarget, 
                                   __in_opt const GUID * pTargetFormat,
                                   LONGLONG Source, 
                                   __in_opt const GUID * pSourceFormat );
    STDMETHODIMP SetPositions( __inout_opt LONGLONG * pCurrent, DWORD CurrentFlags
                             , __inout_opt LONGLONG * pStop, DWORD StopFlags );

    STDMETHODIMP GetPositions( __out_opt LONGLONG * pCurrent, __out_opt LONGLONG * pStop );
    STDMETHODIMP GetCurrentPosition( __out LONGLONG * pCurrent );
    STDMETHODIMP GetStopPosition( __out LONGLONG * pStop );
    STDMETHODIMP SetRate( double dRate);
    STDMETHODIMP GetRate( __out double * pdRate);
    STDMETHODIMP GetDuration( __out LONGLONG *pDuration);
    STDMETHODIMP GetAvailable( __out_opt LONGLONG *pEarliest, __out_opt LONGLONG *pLatest );
    STDMETHODIMP GetPreroll( __out LONGLONG *pllPreroll );


};


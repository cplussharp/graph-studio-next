//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : Mike Copperwhite
//
//-----------------------------------------------------------------------------
#pragma once


// Class to delegate IMediaSeeking upstream and log to CAnalyzer

class CAnalyzerPosPassThru : public CPosPassThru
{
public:
	CAnalyzerPosPassThru(const TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr, IPin *pPin, CAnalyzer* analyzer)
		: CPosPassThru(pName, pUnk, phr, pPin)
		, m_Analyzer(analyzer)
	{
	}

    STDMETHODIMP SetPositions( __inout_opt LONGLONG * pCurrent, DWORD CurrentFlags
                             , __inout_opt LONGLONG * pStop, DWORD StopFlags )
	{
		const HRESULT hr = __super::SetPositions(pCurrent, CurrentFlags, pStop, StopFlags);
		if (m_Analyzer)
			m_Analyzer->AddMSSetPositions(hr, pCurrent, CurrentFlags, pStop, StopFlags);
		return hr;
	}

	STDMETHODIMP SetRate(double dRate)
	{
		const HRESULT hr = __super::SetRate(dRate);
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MS_SetRate, hr, dRate);
		return hr;
	}

	STDMETHODIMP SetTimeFormat(const GUID * pFormat)
	{
		const HRESULT hr = __super::SetTimeFormat(pFormat);
		if (m_Analyzer)
			m_Analyzer->AddMSSetTimeFormat(hr, pFormat);
		return hr;
	}

    STDMETHODIMP put_CurrentPosition(REFTIME llTime)
	{
		const HRESULT hr = __super::put_CurrentPosition(llTime);
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MP_SetCurrentPosition, hr, llTime);
		return hr;
	}

    STDMETHODIMP put_StopTime(REFTIME llTime)
	{
		const HRESULT hr = __super::put_StopTime(llTime);
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MP_SetStopTime, hr, llTime);
		return hr;
	}

    STDMETHODIMP put_PrerollTime(REFTIME llTime)
	{
		const HRESULT hr = __super::put_PrerollTime(llTime);
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MP_SetPrerollTime, hr, llTime);
		return hr;
	}

    STDMETHODIMP put_Rate(double dRate)
	{
		const HRESULT hr = __super::put_Rate(dRate);
		if (m_Analyzer)
			m_Analyzer->AddDouble(SRK_MP_SetRate, hr, dRate);
		return hr;
	}

private:
	CAnalyzer*		m_Analyzer;
};




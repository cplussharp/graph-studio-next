//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CVideoAnalyzer class
//
//-----------------------------------------------------------------------------

CVideoAnalyzer::CVideoAnalyzer(LPUNKNOWN pUnk) :
	CUnknown(NAME("Video Analyzer"), pUnk),
    m_enabled(VARIANT_TRUE), m_callback(NULL),
    m_db(NULL),
    m_sqlHistogram(NULL),
    m_sqlEntropy(NULL)
{
    sqlite3_open(CT2A(_T(":memory:"), CP_UTF8), &m_db);

    sqlite3_exec(m_db,"CREATE TABLE histogram (time INTEGER NOT NULL, value INTEGER NOT NULL, red INTEGER NOT NULL, green INTEGER NOT NULL, blue INTEGER NOT NULL, grey INTEGER NOT NULL);",NULL,NULL,NULL);
    sqlite3_exec(m_db,"CREATE TABLE entropy (time INTEGER NOT NULL, value INTEGER NOT NULL);",NULL,NULL,NULL);


    sqlite3_prepare(m_db, CT2A(_T("INSERT OR IGNORE INTO histogram(time,value,red,green,blue,grey) VALUES (?,?,?,?,?,?);"), CP_UTF8), -1, &m_sqlHistogram, NULL);
    sqlite3_prepare(m_db, CT2A(_T("INSERT OR IGNORE INTO entropy(time,value,red,green,blue,grey) VALUES (?,?);"), CP_UTF8), -1, &m_sqlEntropy, NULL);
}

CVideoAnalyzer::~CVideoAnalyzer()
{
    if (m_callback != NULL)
    {
        m_callback->Release();
        m_callback = NULL;
    }

    if (m_sqlHistogram)   sqlite3_finalize(m_sqlHistogram);
    if (m_sqlEntropy) sqlite3_finalize(m_sqlEntropy);
    if (m_db) sqlite3_close(m_db);
}

STDMETHODIMP CVideoAnalyzer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IVideoAnalyzerFilter)) {
		return GetInterface((IVideoAnalyzerFilter*)this, ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CVideoAnalyzer::CheckInputType(const CMediaType* mtIn)
{
    CheckPointer(mtIn, E_POINTER);

    if (mtIn->majortype != MEDIATYPE_Video)
        return VFW_E_INVALIDMEDIATYPE;

    if (mtIn->subtype != MEDIASUBTYPE_RGB24 && mtIn->subtype != MEDIASUBTYPE_RGB32 && 
        mtIn->subtype != MEDIASUBTYPE_ARGB32)
            return VFW_E_INVALIDMEDIATYPE;

	return S_OK;
}

HRESULT CVideoAnalyzer::AnalyzeSample(IMediaSample *pSample)
{
    if (!m_enabled) return S_OK;

	return S_OK;
}


#pragma region IVideoAnalyzerFilter Members

STDMETHODIMP CVideoAnalyzer::get_Enabled(VARIANT_BOOL *pVal)
{
    CheckPointer(pVal, E_POINTER);
    *pVal = m_enabled;
    return S_OK;
}

STDMETHODIMP CVideoAnalyzer::put_Enabled(VARIANT_BOOL val)
{
    m_enabled = val;
    return S_OK;
}

STDMETHODIMP CVideoAnalyzer::SetCallback(IVideoAnalyzerFilterCallback* pCallback)
{
    if (pCallback != NULL)
        pCallback->AddRef();

    if (m_callback != NULL)
        m_callback->Release();

    m_callback = pCallback;

    return S_OK;
}

#pragma endregion
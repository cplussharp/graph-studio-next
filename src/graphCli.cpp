//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

CGraphStudioCommandLineInfo::CGraphStudioCommandLineInfo()
    :m_bRunGraph(false),
    m_bExitAfterRun(false),
    m_bNoClock(false),
    m_bShowFilters(false),
    m_bProgressView(false),
    m_bExitOnError(false)
{
}

CGraphStudioCommandLineInfo::~CGraphStudioCommandLineInfo()
{
}

void CGraphStudioCommandLineInfo::ParseParam(const TCHAR *pszParam, BOOL bFlag, BOOL bLast)
{
    if (bFlag)
    {
        if (_tcsicmp(pszParam, TEXT("run")) == 0)
            m_bRunGraph = true;
        else if (_tcsicmp(pszParam, TEXT("noclock")) == 0)
            m_bNoClock = true;
        else if (_tcsicmp(pszParam, TEXT("exitafterrun")) == 0)
            m_bExitAfterRun = true;
        else if (_tcsicmp(pszParam, TEXT("exitonerror")) == 0)
            m_bExitOnError = true;
        else if (_tcsicmp(pszParam, TEXT("filters")) == 0)
            m_bShowFilters = true;
        else if (_tcsicmp(pszParam, TEXT("progressview")) == 0)
            m_bProgressView = true;
        else if (_tcsicmp(pszParam, TEXT("?")) == 0)
            m_bShowCliHelp = true;
       
        CCommandLineInfo::ParseParam(pszParam, bFlag, bLast );
    }
    else
    {
        CCommandLineInfo::ParseParam(pszParam, bFlag, bLast );
    }
}

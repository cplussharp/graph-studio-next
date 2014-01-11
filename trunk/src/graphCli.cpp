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
        else if (_tcsicmp(pszParam, TEXT("a")) == 0)
        {
            m_bRemoteGraph = true;

            // if the flag is set after the moniker, we need to reset the openFlile command
            if (!m_strFileName.IsEmpty())
            {
                m_strRemoteGraph = m_strFileName;
                m_strFileName.Empty();
                m_nShellCommand = FileNew;
            }
        }
        
        CCommandLineInfo::ParseParam(pszParam, bFlag, bLast );
    }
    else
    {
        if (m_bRemoteGraph && m_strRemoteGraph.IsEmpty())
        {
            m_strRemoteGraph = pszParam;
            ParseLast(bLast);
        }
        else
            CCommandLineInfo::ParseParam(pszParam, bFlag, bLast );
    }
}

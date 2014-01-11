//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

class CGraphStudioCommandLineInfo: public CCommandLineInfo
{
public:
    CGraphStudioCommandLineInfo();
    virtual ~CGraphStudioCommandLineInfo();

    // aditional flags
    bool m_bRunGraph;
    bool m_bExitAfterRun;
    bool m_bExitOnError;
    bool m_bNoClock;
    bool m_bShowFilters;
    bool m_bProgressView;
    
    bool m_bShowCliHelp;

    bool m_bRemoteGraph;
    CString m_strRemoteGraph;

protected:
    //Override for Base class virtual
    void ParseParam(const TCHAR *pszParam, BOOL bFlag, BOOL bLast);
};
//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif



//-----------------------------------------------------------------------------
//
//	CgraphstudioApp class
//
//-----------------------------------------------------------------------------
class CgraphstudioApp : public CWinApp
{
protected:
	DECLARE_MESSAGE_MAP()

public:
	CgraphstudioApp();

	virtual BOOL InitInstance();
	afx_msg void OnAppAbout();
    virtual int ExitInstance();

    static bool g_showGuidsOfKnownTypes;
};

extern CgraphstudioApp theApp;
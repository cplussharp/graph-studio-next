#include "stdafx.h"

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance
CFactoryTemplate g_Templates[] =
{
    CAnalyzerFilter::g_Template,
    CAnalyzerWriterFilter::g_Template,
    CAnalyzerPage::g_Template
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

// If we declare the correct C runtime entrypoint and then forward it to the DShow base
// classes we will be sure that both the C/C++ runtimes and the base classes are initialized
// correctly
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

// Because we're using MFC for the property page, initialise baseclasses from MFC app entry points and leave MFC to handle DllMain
// baseclass entry point does not process the thread attach/detach entry points
class AnalyzerFilterApp : public CWinApp
{
	virtual BOOL InitInstance()
	{
		return DllEntryPoint(m_hInstance, DLL_PROCESS_ATTACH, NULL);	// reserved param not used
	}

    virtual int ExitInstance()
	{
		return DllEntryPoint(m_hInstance, DLL_PROCESS_DETACH, NULL);	// reserved param not used
	}
};
AnalyzerFilterApp theApp;		// the singleton application object


STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );
}
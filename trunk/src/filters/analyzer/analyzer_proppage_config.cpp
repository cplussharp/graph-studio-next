//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "analyzer_proppage_resource.h"

const CFactoryTemplate CAnalyzerPropPageConfig::g_Template = {
		L"PropPage Config",
        &__uuidof(AnalyzerPropPageConfig),
		CAnalyzerPropPageConfig::CreateInstance,
		NULL,
		NULL
	};

CUnknown* CAnalyzerPropPageConfig::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);
    CUnknown *punk = new CAnalyzerPropPageConfig(lpunk, phr);
    if (punk == NULL) {
        if (phr)
        	*phr = E_OUTOFMEMORY;
    }

    return punk;
}

// Constructor
CAnalyzerPropPageConfig::CAnalyzerPropPageConfig(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage(NAME("PropPage Config"), pUnk, IDD_PROPPAGE_ANALYZER_CONFIG, IDS_PROPPAGE_ANALYZER_CONFIG_TITLE), m_pAnalyzer(NULL)
{
    ASSERT(phr);
} 

// OnReceiveMessage
// Handles the messages for our property window
INT_PTR CAnalyzerPropPageConfig::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
            if (m_bIsInitialized)
            {
                m_bDirty = TRUE;
                if (m_pPageSite) m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
            }
            return (LRESULT) 1;
    }

    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

// OnConnect
// Called when we connect to a transform filter
HRESULT CAnalyzerPropPageConfig::OnConnect(IUnknown *pUnknown)
{
    CheckPointer(pUnknown,E_POINTER);
    ASSERT(m_pAnalyzer == NULL);

    HRESULT hr = pUnknown->QueryInterface(__uuidof(IAnalyzerCommon), (void **) &m_pAnalyzer);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    // Get the initial properties
    CheckPointer(m_pAnalyzer,E_FAIL);

	// load config values

    m_bIsInitialized = FALSE;
    return NOERROR;
}


// OnDisconnect
// Likewise called when we disconnect from a filter
HRESULT CAnalyzerPropPageConfig::OnDisconnect()
{
    // Release of Interface after setting the appropriate old effect value
    if(m_pAnalyzer)
    {
        m_pAnalyzer->Release();
        m_pAnalyzer = NULL;
    }

    return NOERROR;
}

// OnActivate
// We are being activated
HRESULT CAnalyzerPropPageConfig::OnActivate()
{
    // Set Values in view and init controls

    m_bIsInitialized = TRUE;

    return NOERROR;
}

// OnDeactivate
// We are being deactivated
HRESULT CAnalyzerPropPageConfig::OnDeactivate(void)
{
    m_bIsInitialized = FALSE;

    return NOERROR;
} 

// OnApplyChanges
// Apply any changes so far made
HRESULT CAnalyzerPropPageConfig::OnApplyChanges()
{
    // werte am Filter setzen
    CheckPointer(m_pAnalyzer,E_POINTER);

    // get current values from controls

    // set values on filter

    return NOERROR;
}


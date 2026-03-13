//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "PropertyBagListWriter.h"
#include "VariantTypeHelper.h"

CPropertyBagListWriter::CPropertyBagListWriter(LPUNKNOWN pUnk) :
    CUnknown(NAME("PropertyBagListWriter"), pUnk),
    m_entries()
{
    DbgLog((LOG_MEMORY,1,TEXT("PropertyBagListWriter created")));
}

CPropertyBagListWriter::~CPropertyBagListWriter()
{
    DbgLog((LOG_MEMORY,1,TEXT("PropertyBagListWriter destroyed")));
}

STDMETHODIMP CPropertyBagListWriter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IPropertyBag)) {
		return GetInterface(static_cast<IPropertyBag*>(this), ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CPropertyBagListWriter::Read(LPCOLESTR pszPropertyName, VARIANT* pvar, IErrorLog* pErrorLog)
{
    // Not used for saving
    return E_NOTIMPL;
}

STDMETHODIMP CPropertyBagListWriter::Write(LPCOLESTR pszPropertyName, VARIANT* pvar)
{
    CheckPointer(pszPropertyName, E_POINTER);
    CheckPointer(pvar, E_POINTER);

    CPropertyBagListWriterEntry entry;
    entry.name = CString(pszPropertyName);
    entry.type = GraphStudio::VarTypeToString(pvar->vt);

    // Convert variant to string
    CString value;
    CComVariant vt = *pvar;
    HRESULT hr = vt.ChangeType(VT_BSTR);
    if (SUCCEEDED(hr) && vt.bstrVal) {
        entry.value = CString(vt.bstrVal);
    } else {
        entry.value = _T("<unrepresentable value>");
    }

    // Store the property
    m_entries.push_back(entry);

    return S_OK;
}

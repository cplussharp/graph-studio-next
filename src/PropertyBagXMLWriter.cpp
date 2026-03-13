//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "PropertyBagXMLWriter.h"
#include "VariantTypeHelper.h"

CPropertyBagXMLWriter::CPropertyBagXMLWriter(LPUNKNOWN pUnk, XML::XMLWriter& xml) :
    CUnknown(NAME("PropertyBagXMLWriter"), pUnk),
    m_xml(xml)
{
    DbgLog((LOG_MEMORY,1,TEXT("PropertyBagXMLWriter created")));
}

CPropertyBagXMLWriter::~CPropertyBagXMLWriter()
{
    DbgLog((LOG_MEMORY,1,TEXT("PropertyBagXMLWriter destroyed")));
}

STDMETHODIMP CPropertyBagXMLWriter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IPropertyBag)) {
		return GetInterface(static_cast<IPropertyBag*>(this), ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CPropertyBagXMLWriter::Read(LPCOLESTR pszPropertyName, VARIANT* pvar, IErrorLog* pErrorLog)
{
    // Not used for saving
    return E_NOTIMPL;
}

STDMETHODIMP CPropertyBagXMLWriter::Write(LPCOLESTR pszPropertyName, VARIANT* pvar)
{
    CheckPointer(pszPropertyName, E_POINTER);
    CheckPointer(pvar, E_POINTER);

    const CString name(pszPropertyName);
    const VARTYPE type = pvar->vt;
    const CString typeName = GraphStudio::VarTypeToString(type);

    // Convert variant to string
    CString value;
    CComVariant vt = *pvar;
    HRESULT hr = vt.ChangeType(VT_BSTR);
    if (SUCCEEDED(hr) && vt.bstrVal) {
        value = CString(vt.bstrVal);
    } else {
        DbgLog((LOG_ERROR, 0, TEXT("Failed to convert %s [%d] to string for property \"%s\""), typeName, type, name));
        return E_FAIL;
    }

    // Store the property
    m_xml.BeginNode(_T("property"));
    m_xml.WriteValue(_T("name"), name);
    m_xml.WriteValue(_T("type"), typeName);
    m_xml.WriteValue(_T("value"), value);
    m_xml.EndNode();

    return S_OK;
}

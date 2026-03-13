//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#pragma once

// Helper class to write property bags to XML
class CPropertyBagXMLWriter : public CUnknown, public IPropertyBag
{
private:
    XML::XMLWriter& m_xml;

public:
	CPropertyBagXMLWriter(LPUNKNOWN pUnk, XML::XMLWriter& xml);
	virtual ~CPropertyBagXMLWriter();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// IPropertyBag
    STDMETHODIMP Read(LPCOLESTR pszPropertyName, VARIANT* pvar, IErrorLog* pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropertyName, VARIANT* pvar);
};

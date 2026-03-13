//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#pragma once

// Helper class to read property bag values from XML
class CPropertyBagXMLReader : public CUnknown, public IPropertyBag
{
private:
    XML::XMLList& m_xml;

public:
	CPropertyBagXMLReader(LPUNKNOWN pUnk, XML::XMLList& xml);
	virtual ~CPropertyBagXMLReader();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// IPropertyBag
    STDMETHODIMP Read(LPCOLESTR pszPropertyName, VARIANT* pvar, IErrorLog* pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropertyName, VARIANT* pvar);
};

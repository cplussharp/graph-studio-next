//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#pragma once

class CPropertyBagListWriterEntry
{
public:
	CString name;
	CString type;
	CString value;
};

// Helper class to write property bags entries to a list
class CPropertyBagListWriter : public CUnknown, public IPropertyBag
{
private:
	std::vector<CPropertyBagListWriterEntry> m_entries;

public:
	CPropertyBagListWriter(LPUNKNOWN pUnk);
	virtual ~CPropertyBagListWriter();

	// expose some interfaces
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	// IPropertyBag
    STDMETHODIMP Read(LPCOLESTR pszPropertyName, VARIANT* pvar, IErrorLog* pErrorLog);
    STDMETHODIMP Write(LPCOLESTR pszPropertyName, VARIANT* pvar);

	// Get the list of entries
	const std::vector<CPropertyBagListWriterEntry>& GetEntries() const { return m_entries; }
};

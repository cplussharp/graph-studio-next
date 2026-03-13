//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "PropertyBagXMLReader.h"
#include "VariantTypeHelper.h"

CPropertyBagXMLReader::CPropertyBagXMLReader(LPUNKNOWN pUnk, XML::XMLList& xml) :
    CUnknown(NAME("PropertyBagXMLReader"), pUnk),
    m_xml(xml)
{
    DbgLog((LOG_MEMORY,1,TEXT("PropertyBagXMLReader created")));
}

CPropertyBagXMLReader::~CPropertyBagXMLReader()
{
    DbgLog((LOG_MEMORY,1,TEXT("PropertyBagXMLReader destroyed")));
}

STDMETHODIMP CPropertyBagXMLReader::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	if (riid == __uuidof(IPropertyBag)) {
		return GetInterface(static_cast<IPropertyBag*>(this), ppv);
	}
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CPropertyBagXMLReader::Read(LPCOLESTR pszPropertyName, VARIANT* pvar, IErrorLog* pErrorLog)
{
    CheckPointer(pszPropertyName, E_POINTER);
    CheckPointer(pvar, E_POINTER);

    const CString name(pszPropertyName);

    // search the node
    XML::XMLNode* pNode = nullptr;
    for (auto it = m_xml.begin(); it != m_xml.end(); ++it) {
        XML::XMLNode* pCurNode = *it;
        if (pCurNode && pCurNode->name == _T("property")) {
            const CString curNodeName = pCurNode->GetValue(_T("name"));
            if (curNodeName.CompareNoCase(name) == 0) {
                pNode = pCurNode;
                break;
            }
        }
    }

    if (!pNode) {
        DbgLog((LOG_ERROR, 0, TEXT("Failed to find property \"%s\""), name));
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    // target type is determined by the caller's requested type or the stored type if caller does not specify a type
    VARTYPE vtType = pvar->vt;
    if (vtType == VT_EMPTY) {
        // use stored type if target does not request a specific type
        CString sType = pNode->GetValue(_T("type"));
        vtType = GraphStudio::StringToVarType(sType);

        // if the stored type is empty, default to VT_BSTR
        if (vtType == VT_EMPTY) {
            vtType = VT_BSTR;
        }
    }

    // load the value as string
    CString sValue = pNode->GetValue(_T("value"));
    CComVariant vValue(sValue);

    // convert to target type
    HRESULT hr = vValue.ChangeType(vtType);
    if (FAILED(hr)) {
        const CString targetTypeName = GraphStudio::VarTypeToString(vtType);
        DbgLog((LOG_ERROR, 0, TEXT("Failed to convert string \"%s\" to %s for property \"%s\""), sValue, targetTypeName, name));

        // 4. LOG THE ERROR: The bag is failing to satisfy the caller's type request
        if (pErrorLog) {
            EXCEPINFO ei = {0};
            ei.scode = hr; // DISP_E_TYPEMISMATCH
            ei.bstrSource = SysAllocString(L"MyPropertyBag");
            ei.bstrDescription = SysAllocString(L"The property value could not be converted to the requested type.");

            pErrorLog->AddError(pszPropertyName, &ei);

            SysFreeString(ei.bstrSource);
            SysFreeString(ei.bstrDescription);
        }
        return hr;
    }

    // detach the value to output
    vValue.Detach(pvar);

    return S_OK;
}

STDMETHODIMP CPropertyBagXMLReader::Write(LPCOLESTR pszPropertyName, VARIANT* pvar)
{
    // Not used for loading
    return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

const CFactoryTemplate CFakeM2tsDevice::g_Template = {
		L"Fake M2TS Device",
        &CLSID_FakeM2tsDevice,
		CFakeM2tsDevice::CreateInstance,
		NULL,
		NULL
	};

CUnknown* CFakeM2tsDevice::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CFakeM2tsDevice* pNewObject = new CFakeM2tsDevice(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//-----------------------------------------------------------------------------
//
//	CFakeM2tsDevice class
//
//-----------------------------------------------------------------------------

CFakeM2tsDevice::CFakeM2tsDevice(LPUNKNOWN pUnk, HRESULT *phr)
: CSource(NAME("FakeM2tsDevice"), pUnk, CLSID_FakeM2tsDevice, phr)
{	
    DbgLog((LOG_MEMORY,1,TEXT("FakeM2tsDevice created")));
}

CFakeM2tsDevice::~CFakeM2tsDevice()
{
    DbgLog((LOG_MEMORY,1,TEXT("FakeM2tsDevice destroyed")));
}

STDMETHODIMP CFakeM2tsDevice::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

    // Do we have this interface
    if (IID_IFileSourceFilter == riid)
        return GetInterface(static_cast<IFileSourceFilter*>(this), ppv);
    else if (IID_IAMFilterMiscFlags == riid)
        return GetInterface(static_cast<IAMFilterMiscFlags*>(this), ppv);
    
    return CSource::NonDelegatingQueryInterface(riid, ppv);
}

/*********************************************************************************************
* ITSSourceFilter implementierung
*********************************************************************************************/
STDMETHODIMP CFakeM2tsDevice::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt)
{
    // kann nur einmal geladen werden
    if(GetPinCount() != 0) return E_FAIL;

    // Dateinamen setzen
    m_filename = pszFileName;

	// Pin erzeugen
    HRESULT hr = S_OK;
    CFakeM2tsDeviceStream* pPin = new CFakeM2tsDeviceStream(&hr,this, _T("MPEG2TS"), (LPCTSTR)m_filename);

	if(NULL == pPin)
    {
        m_filename = _T("");
        return E_OUTOFMEMORY;
    }
	else if(FAILED(hr))
	{
		delete pPin;
		pPin = NULL;
        m_filename = _T("");
        return hr;
	}
	
	return hr;
}

STDMETHODIMP CFakeM2tsDevice::GetCurFile( LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt)
{
    if(!m_filename) return E_UNEXPECTED;
	
    *ppszFileName = (BSTR)CoTaskMemAlloc(sizeof(WCHAR) * (1+lstrlenW(m_filename)));
    if (*ppszFileName != NULL) {
        lstrcpyW(*ppszFileName, m_filename);
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
//
//	CFakeM2tsDeviceStream class
//
//-----------------------------------------------------------------------------

CFakeM2tsDeviceStream::CFakeM2tsDeviceStream(HRESULT *phr, CSource *pParent, LPCWSTR pPinName, CString filename) :
    CSourceStream(NAME("MPEG2TS"),phr, pParent, pPinName), m_hasStride(FALSE)
{
    BOOL ok = m_file.Open(filename, CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone);

    if(!m_file)
    {
        if(phr) *phr = E_FAIL;
        return;
    }

    BYTE buf[192 + 5];
    if((192 + 5) != m_file.Read(&buf, 192 + 5))
    {
        m_file.Close();
        if(phr) *phr = E_FAIL;
        return;
    }

    if(buf[0] != 0x47)
    {
        if(buf[4] != 0x47)
        {
            m_file.Close();
            if(phr) *phr = E_FAIL;
            return;
        }

        // TS with Stride
        m_hasStride = TRUE;

        if(buf[192+4] != 0x47)
        {
            // strange the first byte was a Sync byte, but the second not?
            m_file.Close();
            if(phr) *phr = E_FAIL;
            return;
        }
    }
    else if(buf[188] != 0x47)
    {
        // strange the first byte was a Sync byte, but the second not?
        m_file.Close();
        if(phr) *phr = E_FAIL;
        return;
    }

    if(phr) *phr = S_OK;
    m_file.SeekToBegin();
}

CFakeM2tsDeviceStream::~CFakeM2tsDeviceStream(void)
{
    m_file.Close();
}

STDMETHODIMP CFakeM2tsDeviceStream::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}


/*********************************************************************************************
* Implementation of the Pin functions
*********************************************************************************************/
HRESULT CFakeM2tsDeviceStream::FillBuffer(IMediaSample *pms)
{
    CheckPointer(pms,E_POINTER);

    long packetsize = m_hasStride ? 192 : 188;
    long buffersize = pms->GetSize() / packetsize;

    BYTE* pBuf;
    HRESULT hr = pms->GetPointer(&pBuf);
    if(FAILED(hr)) return hr;

    UINT cbread = m_file.Read(pBuf, buffersize);
    if(!cbread) return S_FALSE;

    pms->SetActualDataLength(cbread);
    return S_OK;
}


HRESULT CFakeM2tsDeviceStream::GetMediaType(int iPosition, CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    if(iPosition < 0) return E_INVALIDARG;
    if(iPosition > 0) return VFW_S_NO_MORE_ITEMS;

    pmt->majortype = MEDIATYPE_Stream;
    pmt->subtype = MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE;
    pmt->formattype = FORMAT_None;
    pmt->bFixedSizeSamples = TRUE;
    pmt->bTemporalCompression = FALSE;
    pmt->cbFormat = 0;
    pmt->pbFormat = NULL;
    if(m_hasStride)
    {
        pmt->lSampleSize = 192*256;
    }
    else
    {
        pmt->lSampleSize = 188*256;
    }

    return NOERROR;
}

HRESULT CFakeM2tsDeviceStream::CheckMediaType(const CMediaType *pMediaType)
{
    CheckPointer(pMediaType,E_POINTER);

    if(pMediaType->majortype != MEDIATYPE_Stream)
        return VFW_E_INVALIDMEDIATYPE;

    if(pMediaType->subtype != MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE)
        return VFW_E_INVALIDMEDIATYPE;

    if((pMediaType->lSampleSize < 192 && m_hasStride) || (pMediaType->lSampleSize < 188 && !m_hasStride))
        return VFW_E_INVALIDMEDIATYPE;

    return S_OK;
}


//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
HRESULT CFakeM2tsDeviceStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = m_mt.lSampleSize;

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted
    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if(FAILED(hr)) return hr;

    // Is this allocator unsuitable
    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;

    // Make sure that we have only 1 buffer (we erase the ball in the
    // old buffer to save having to zero a 200k+ buffer every time
    // we draw a frame)
    ASSERT(Actual.cBuffers == 1);

    return NOERROR;
}

HRESULT CFakeM2tsDeviceStream::OnThreadCreate()
{
    m_file.SeekToBegin();
    return S_OK;
}


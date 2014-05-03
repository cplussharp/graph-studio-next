//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : CPlusSharp
//
//  some parts are from https://github.com/mpc-hc/mpc-hc/blob/master/src/DSUtil/NullRenderers.cpp
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

// dxva.dll
typedef HRESULT(__stdcall* PTR_DXVA2CreateDirect3DDeviceManager9)(UINT* pResetToken, IDirect3DDeviceManager9** ppDeviceManager);
typedef HRESULT(__stdcall* PTR_DXVA2CreateVideoService)(IDirect3DDevice9* pDD, REFIID riid, void** ppService);

static const AMOVIESETUP_FILTER s_FilterInfo = 
{
	&__uuidof(DxvaNullRenderer),		// Filter CLSID
	L"GraphStudioNext DXVA NullRenderer",// String name
	MERIT_DO_NOT_USE,					// Filter merit
	0,									// Number pins
	NULL								// Pin details
};

const CFactoryTemplate CDxvaNullRenderer::g_Template = {
		L"GraphStudioNext DXVA NullRenderer",
        &__uuidof(DxvaNullRenderer),
		CDxvaNullRenderer::CreateInstance,
		NULL,
		&s_FilterInfo
	};

CUnknown* CDxvaNullRenderer::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CDxvaNullRenderer* pNewObject = new CDxvaNullRenderer(punk, phr);
    if (NULL == pNewObject) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//-----------------------------------------------------------------------------
//
//	CDxvaNullRendererInputPin class
//
//-----------------------------------------------------------------------------
class CDxvaNullRendererInputPin : public CRendererInputPin,
    public IMFGetService,
    public IDirectXVideoMemoryConfiguration,
    public IMFVideoDisplayControl
{
public:
    CDxvaNullRendererInputPin(CBaseRenderer* pRenderer, HRESULT* phr, LPCWSTR Name);
    ~CDxvaNullRendererInputPin()
    {
        if (m_pD3DDeviceManager)
        {
            if (m_hDevice != INVALID_HANDLE_VALUE)
            {
                m_pD3DDeviceManager->CloseDeviceHandle(m_hDevice);
                m_hDevice = INVALID_HANDLE_VALUE;
            }
            m_pD3DDeviceManager = NULL;
        }
        if (m_pD3DDev)
            m_pD3DDev = NULL;
        if (m_hDXVA2Lib)
            FreeLibrary(m_hDXVA2Lib);
    }

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    STDMETHODIMP GetAllocator(IMemAllocator** ppAllocator)
    {
        // Renderer shouldn't manage allocator for DXVA
        return E_NOTIMPL;
    }

    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES* pProps)
    {
        // 1 buffer required
        ZeroMemory(pProps, sizeof(ALLOCATOR_PROPERTIES));
        pProps->cbBuffer = 1;
        return S_OK;
    }

    // IMFGetService
    STDMETHODIMP GetService(REFGUID guidService, REFIID riid, LPVOID* ppvObject);

    // IDirectXVideoMemoryConfiguration
    STDMETHODIMP GetAvailableSurfaceTypeByIndex(DWORD dwTypeIndex, DXVA2_SurfaceType* pdwType);
    STDMETHODIMP SetSurfaceType(DXVA2_SurfaceType dwType) { return S_OK; }


    // IMFVideoDisplayControl
    STDMETHODIMP GetNativeVideoSize(SIZE* pszVideo, SIZE* pszARVideo) { return E_NOTIMPL; };
    STDMETHODIMP GetIdealVideoSize(SIZE* pszMin, SIZE* pszMax) { return E_NOTIMPL; };
    STDMETHODIMP SetVideoPosition(const MFVideoNormalizedRect* pnrcSource,
                                  const LPRECT prcDest) { return E_NOTIMPL; };
    STDMETHODIMP GetVideoPosition(MFVideoNormalizedRect* pnrcSource,
                                  LPRECT prcDest) { return E_NOTIMPL; };
    STDMETHODIMP SetAspectRatioMode(DWORD dwAspectRatioMode) { return E_NOTIMPL; };
    STDMETHODIMP GetAspectRatioMode(DWORD* pdwAspectRatioMode) { return E_NOTIMPL; };
    STDMETHODIMP SetVideoWindow(HWND hwndVideo) { return E_NOTIMPL; };
    STDMETHODIMP GetVideoWindow(HWND* phwndVideo);
    STDMETHODIMP RepaintVideo() { return E_NOTIMPL; };
    STDMETHODIMP GetCurrentImage(BITMAPINFOHEADER* pBih, BYTE** pDib,
                                 DWORD* pcbDib, LONGLONG* pTimeStamp) { return E_NOTIMPL; };
    STDMETHODIMP SetBorderColor(COLORREF Clr) { return E_NOTIMPL; };
    STDMETHODIMP GetBorderColor(COLORREF* pClr) { return E_NOTIMPL; };
    STDMETHODIMP SetRenderingPrefs(DWORD dwRenderFlags) { return E_NOTIMPL; };
    STDMETHODIMP GetRenderingPrefs(DWORD* pdwRenderFlags) { return E_NOTIMPL; };
    STDMETHODIMP SetFullscreen(BOOL fFullscreen) { return E_NOTIMPL; };
    STDMETHODIMP GetFullscreen(BOOL* pfFullscreen) { return E_NOTIMPL; };

private:
    HMODULE m_hDXVA2Lib;
    PTR_DXVA2CreateDirect3DDeviceManager9 pfDXVA2CreateDirect3DDeviceManager9;
    PTR_DXVA2CreateVideoService pfDXVA2CreateVideoService;

    CComPtr<IDirect3D9> m_pD3D;
    CComPtr<IDirect3DDevice9> m_pD3DDev;
    CComPtr<IDirect3DDeviceManager9> m_pD3DDeviceManager;
    UINT m_nResetTocken;
    HANDLE m_hDevice;
    HWND m_hWnd;

    void CreateSurface();
};

//-----------------------------------------------------------------------------
//
//	CDxvaNullRenderer class
//
//-----------------------------------------------------------------------------

CDxvaNullRenderer::CDxvaNullRenderer(LPUNKNOWN pUnk, HRESULT *phr) :
	CBaseRenderer(__uuidof(DxvaNullRenderer), TEXT("Dump"), pUnk, phr)
{
    m_pInputPin = new CDxvaNullRendererInputPin(this, phr, L"In");
    DbgLog((LOG_MEMORY,1,TEXT("DxvaNullRenderer created")));
}

CDxvaNullRenderer::~CDxvaNullRenderer()
{
    DbgLog((LOG_MEMORY,1,TEXT("DxvaNullRenderer destroyed")));
}

STDMETHODIMP CDxvaNullRenderer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CDxvaNullRenderer::CheckMediaType(const CMediaType *pmt)
{
    CheckPointer(pmt, E_POINTER);

    if (pmt->majortype != MEDIATYPE_Video)
        return E_FAIL;

    if (!(pmt->subtype == MEDIASUBTYPE_YV12
        || pmt->subtype == MEDIASUBTYPE_NV12
        || pmt->subtype == MEDIASUBTYPE_I420
        || pmt->subtype == MEDIASUBTYPE_YUYV
        || pmt->subtype == MEDIASUBTYPE_IYUV
        || pmt->subtype == MEDIASUBTYPE_YVU9
        || pmt->subtype == MEDIASUBTYPE_Y411
        || pmt->subtype == MEDIASUBTYPE_Y41P
        || pmt->subtype == MEDIASUBTYPE_YUY2
        || pmt->subtype == MEDIASUBTYPE_YVYU
        || pmt->subtype == MEDIASUBTYPE_UYVY
        || pmt->subtype == MEDIASUBTYPE_Y211
        || pmt->subtype == MEDIASUBTYPE_RGB1
        || pmt->subtype == MEDIASUBTYPE_RGB4
        || pmt->subtype == MEDIASUBTYPE_RGB8
        || pmt->subtype == MEDIASUBTYPE_RGB565
        || pmt->subtype == MEDIASUBTYPE_RGB555
        || pmt->subtype == MEDIASUBTYPE_RGB24
        || pmt->subtype == MEDIASUBTYPE_RGB32
        || pmt->subtype == MEDIASUBTYPE_ARGB1555
        || pmt->subtype == MEDIASUBTYPE_ARGB4444
        || pmt->subtype == MEDIASUBTYPE_ARGB32
        || pmt->subtype == MEDIASUBTYPE_A2R10G10B10
        || pmt->subtype == MEDIASUBTYPE_A2B10G10R10))
            return E_FAIL;

	return NOERROR;
}

HRESULT CDxvaNullRenderer::ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime)
{
	// ignore timestamps
	return NOERROR;
}

HRESULT CDxvaNullRenderer::DoRenderSample(IMediaSample *pMediaSample)
{
    CComQIPtr<IMFGetService> pService = pMediaSample;
    if (pService)
    {
        CComPtr<IDirect3DSurface9>  pSurface;
        if (SUCCEEDED(pService->GetService(MR_BUFFER_SERVICE, IID_PPV_ARGS(&pSurface))))
        {
            // TODO : render surface...
        }
    }

	return NOERROR;
}





CDxvaNullRendererInputPin::CDxvaNullRendererInputPin(CBaseRenderer* pRenderer, HRESULT* phr, LPCWSTR Name)
    : CRendererInputPin(pRenderer, phr, Name)
    , m_hDXVA2Lib(NULL)
    , pfDXVA2CreateDirect3DDeviceManager9(NULL)
    , pfDXVA2CreateVideoService(NULL)
    , m_nResetTocken(0)
    , m_hDevice(INVALID_HANDLE_VALUE)
{
    CreateSurface();

    m_hDXVA2Lib = LoadLibrary(L"dxva2.dll");
    if (m_hDXVA2Lib)
    {
        pfDXVA2CreateDirect3DDeviceManager9 = reinterpret_cast<PTR_DXVA2CreateDirect3DDeviceManager9>(GetProcAddress(m_hDXVA2Lib, "DXVA2CreateDirect3DDeviceManager9"));
        pfDXVA2CreateVideoService = reinterpret_cast<PTR_DXVA2CreateVideoService>(GetProcAddress(m_hDXVA2Lib, "DXVA2CreateVideoService"));
        pfDXVA2CreateDirect3DDeviceManager9(&m_nResetTocken, &m_pD3DDeviceManager);
    }

    // Initialize Device Manager with DX surface
    if (m_pD3DDev)
    {
        m_pD3DDeviceManager->ResetDevice(m_pD3DDev, m_nResetTocken);
        m_pD3DDeviceManager->OpenDeviceHandle(&m_hDevice);
    }
}

void CDxvaNullRendererInputPin::CreateSurface()
{
    m_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
    if (!m_pD3D)
        m_pD3D.Attach(Direct3DCreate9(D3D9b_SDK_VERSION));

    m_hWnd = NULL;

    D3DDISPLAYMODE d3ddm;
    ZeroMemory(&d3ddm, sizeof(d3ddm));
    m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

    D3DPRESENT_PARAMETERS pp;
    ZeroMemory(&pp, sizeof(pp));

    pp.Windowed = TRUE;
    pp.hDeviceWindow = m_hWnd;
    pp.SwapEffect = D3DSWAPEFFECT_COPY;
    pp.Flags = D3DPRESENTFLAG_VIDEO;
    pp.BackBufferCount = 1;
    pp.BackBufferWidth = d3ddm.Width;
    pp.BackBufferHeight = d3ddm.Height;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    m_pD3D->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, //| D3DCREATE_MANAGED,
        &pp, &m_pD3DDev);
}

STDMETHODIMP CDxvaNullRendererInputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    if (riid == __uuidof(IMFGetService))
        return GetInterface(static_cast<IMFGetService*>(this), ppv);

    return __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CDxvaNullRendererInputPin::GetService(REFGUID guidService, REFIID riid, LPVOID* ppvObject)
{
    if (m_pD3DDeviceManager && guidService == MR_VIDEO_ACCELERATION_SERVICE)
    {
        if (riid == __uuidof(IDirect3DDeviceManager9))
            return m_pD3DDeviceManager->QueryInterface(riid, ppvObject);
        else if (riid == __uuidof(IDirectXVideoDecoderService) || riid == __uuidof(IDirectXVideoProcessorService))
            return m_pD3DDeviceManager->GetVideoService(m_hDevice, riid, ppvObject);
        else if (riid == __uuidof(IDirectXVideoAccelerationService))
            return pfDXVA2CreateVideoService(m_pD3DDev, riid, ppvObject);
        else if (riid == __uuidof(IDirectXVideoMemoryConfiguration))
            return GetInterface(static_cast<IDirectXVideoMemoryConfiguration*>(this), ppvObject);
    }
    else if (guidService == MR_VIDEO_RENDER_SERVICE)
    {
        if (riid == __uuidof(IMFVideoDisplayControl))
            return GetInterface(static_cast<IMFVideoDisplayControl*>(this), ppvObject);
    }

    return E_NOINTERFACE;
}

STDMETHODIMP CDxvaNullRendererInputPin::GetAvailableSurfaceTypeByIndex(DWORD dwTypeIndex, DXVA2_SurfaceType* pdwType)
{
    if (dwTypeIndex == 0)
    {
        *pdwType = DXVA2_SurfaceType_DecoderRenderTarget;
        return S_OK;
    }

    return MF_E_NO_MORE_TYPES;
}

STDMETHODIMP CDxvaNullRendererInputPin::GetVideoWindow(HWND* phwndVideo)
{
    CheckPointer(phwndVideo, E_POINTER);
    *phwndVideo = m_hWnd;
    return S_OK;
}

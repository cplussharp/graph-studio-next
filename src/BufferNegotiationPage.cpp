//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CBufferNegotiationPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CBufferNegotiationPage, CDSPropertyPage)
	ON_WM_SIZE()
	ON_COMMAND(IDC_BUTTON_SET, &CBufferNegotiationPage::OnSetClick)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CBufferNegotiationPage class
//
//-----------------------------------------------------------------------------
CBufferNegotiationPage::CBufferNegotiationPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle) :
	CDSPropertyPage(_T("BufferNegotiationPage"), pUnk, IDD, strTitle)
{
	// retval
	if (phr) *phr = NOERROR;
	bufneg = NULL;
}

CBufferNegotiationPage::~CBufferNegotiationPage()
{
	bufneg = NULL;
}


BOOL CBufferNegotiationPage::OnInitDialog()
{
	BOOL ok = CDSPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	RefreshInfo();
	return TRUE;
}

void CBufferNegotiationPage::OnSetClick()
{
	CString			t;
	edit_latency.GetWindowText(t);

	int				latency_ms;
	if (_stscanf(t.GetBuffer(), _T("%d"), &latency_ms) != 1) {
		MessageBox(_T("Value must be an integer"));
		return ;
	}
	if (latency_ms < 0) latency_ms = 0;


	CComPtr<IPin>		pin;
	HRESULT				hr;

	do {
		if (!bufneg) break;;
		
		hr = bufneg->QueryInterface(IID_IPin, (void**)&pin);
		if (FAILED(hr)) break;;

		// media types
		DSUtil::MediaTypes				mtlist;

		hr = DSUtil::EnumMediaTypes(pin, mtlist);
		if (FAILED(hr) || mtlist.GetCount() <= 0) break;

		// must be an audio media type
		CMediaType		mt = mtlist[0];
		if (mt.majortype == MEDIATYPE_Audio && 
			mt.subtype == MEDIASUBTYPE_PCM &&
			mt.formattype == FORMAT_WaveFormatEx
			) {

			WAVEFORMATEX	*wfx = (WAVEFORMATEX*)mt.pbFormat;

			// just like MSDN said: -1 = we don't care
			ALLOCATOR_PROPERTIES		alloc;
			alloc.cbAlign	= -1;
			alloc.cbBuffer	= (wfx->nAvgBytesPerSec * latency_ms) / 1000;
			alloc.cbPrefix	= -1;
			alloc.cBuffers	= -1;

			hr = bufneg->SuggestAllocatorProperties(&alloc);
			if (FAILED(hr)) {
				::MessageBox(0, _T("IAMBufferNegotiation::SuggestAllocatorProperties failed"), _T(""), 0);
			}
		}
	} while (0);

	pin = NULL;
}

void CBufferNegotiationPage::OnSize(UINT nType, int cx, int cy)
{
}


void CBufferNegotiationPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_STATIC_RATE, label_samplerate);
	DDX_Control(pDX, IDC_STATIC_CHANNELS, label_channels);
	DDX_Control(pDX, IDC_STATIC_BYTES, label_avgbytes);
	DDX_Control(pDX, IDC_STATIC_ALIGN, label_align);
	DDX_Control(pDX, IDC_STATIC_BUFFER, label_buffer);
	DDX_Control(pDX, IDC_STATIC_PREFIX, label_prefix);
	DDX_Control(pDX, IDC_STATIC_BUFFERS, label_buffers);
	DDX_Control(pDX, IDC_EDIT_LATENCY, edit_latency);
}

int CBufferNegotiationPage::RefreshInfo()
{
	int					ret = -1;
	CComPtr<IPin>		pin;
	HRESULT				hr;

	do {
		if (!bufneg) break;;
		
		hr = bufneg->QueryInterface(IID_IPin, (void**)&pin);
		if (FAILED(hr)) break;;

		// media types
		DSUtil::MediaTypes				mtlist;

		hr = DSUtil::EnumMediaTypes(pin, mtlist);
		if (FAILED(hr) || mtlist.GetCount() <= 0) break;

		// must be an audio media type
		CMediaType		mt = mtlist[0];
		if (mt.majortype == MEDIATYPE_Audio && 
			mt.subtype == MEDIASUBTYPE_PCM &&
			mt.formattype == FORMAT_WaveFormatEx
			) {

			WAVEFORMATEX	*wfx = (WAVEFORMATEX*)mt.pbFormat;
			CString			t;

			t.Format(_T("%d"), wfx->nSamplesPerSec);	label_samplerate.SetWindowText(t);
			t.Format(_T("%d"), wfx->nChannels);			label_channels.SetWindowText(t);
			t.Format(_T("%d"), wfx->nAvgBytesPerSec);	label_avgbytes.SetWindowText(t);


			// and now display the allocator properties stuff
			ALLOCATOR_PROPERTIES	props;
			hr = bufneg->GetAllocatorProperties(&props);
			if (SUCCEEDED(hr)) {
				t.Format(_T("%d"), props.cbAlign);		label_align.SetWindowText(t);

				int	ms = -1;
				
				t.Format(_T("%d"), props.cbBuffer);		
				if (wfx->nAvgBytesPerSec > 0) {
					ms = props.cbBuffer * 1000 / wfx->nAvgBytesPerSec;					
					CString		m;
					m.Format(_T(" (%d ms)"), ms);
					t += m;
				}
				label_buffer.SetWindowText(t);

				t.Format(_T("%d"), props.cbPrefix);		label_prefix.SetWindowText(t);
				t.Format(_T("%d"), props.cBuffers);		label_buffers.SetWindowText(t);

			} else {
				// also the allocator stuff
				label_align.SetWindowText(_T("Unknown"));
				label_buffer.SetWindowText(_T("Unknown"));
				label_prefix.SetWindowText(_T("Unknown"));
				label_buffers.SetWindowText(_T("Unknown"));
			}

			ret = 0;
		}

	} while (0);

	if (ret < 0) {
		// reset the values
		label_samplerate.SetWindowText(_T("Unknown"));
		label_channels.SetWindowText(_T("Unknown"));
		label_avgbytes.SetWindowText(_T("Unknown"));

		// also the allocator stuff
		label_align.SetWindowText(_T("Unknown"));
		label_buffer.SetWindowText(_T("Unknown"));
		label_prefix.SetWindowText(_T("Unknown"));
		label_buffers.SetWindowText(_T("Unknown"));
	}

	pin = NULL;
	return 0;
}

HRESULT CBufferNegotiationPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IAMBufferNegotiation, (void**)&bufneg);
	if (FAILED(hr)) return E_FAIL;

	return NOERROR;
}

HRESULT CBufferNegotiationPage::OnDisconnect()
{
	bufneg = NULL;
	return NOERROR;
}


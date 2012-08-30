//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


//-----------------------------------------------------------------------------
//
//	CBufferNegotiationPage class
//
//-----------------------------------------------------------------------------
class CBufferNegotiationPage : public CDSPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

public:

	GraphStudio::TitleBar				title;
	CStatic								label_samplerate;
	CStatic								label_channels;
	CStatic								label_avgbytes;
	CStatic								label_align;
	CStatic								label_buffer;
	CStatic								label_prefix;
	CStatic								label_buffers;
	CEdit								edit_latency;

	CComPtr<IAMBufferNegotiation>		bufneg;

	enum { IDD = IDD_DIALOG_BUFFERNEG };
public:
	CBufferNegotiationPage(LPUNKNOWN pUnk, HRESULT *phr, LPCTSTR strTitle);
	virtual ~CBufferNegotiationPage();
	
	// overriden
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnSize(UINT nType, int cx, int cy);

    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();

	int RefreshInfo();
	void OnSetClick();
};


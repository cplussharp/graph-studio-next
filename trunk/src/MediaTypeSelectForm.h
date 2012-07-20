#pragma once

#include "dsutil.h"
#include "afxcmn.h"
#include "afxwin.h"


class CMediaTypeSelectForm : public CDialog
{
private:
	DECLARE_DYNAMIC(CMediaTypeSelectForm)

	DSUtil::MediaTypes	media_types;
	int					selected_media_type_index;		// selected index within in media_types or out of range if none selected

	void InitializeList();
	int GetSelectedMediaType();

public:
	CMediaTypeSelectForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMediaTypeSelectForm();

	void SetMediaTypes(const DSUtil::MediaTypes& types) { media_types.Copy(types); }
	int SelectedMediaTypeIndex() const { return selected_media_type_index; }	// if >=0, the index of the selected media type in media_types

// Dialog Data
	enum { IDD = IDD_DIALOG_MEDIA_TYPE_SELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CListCtrl media_types_list;
	afx_msg void OnDblclkListMediatypes(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CButton ok_button;
	CButton cancel_button;
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
};

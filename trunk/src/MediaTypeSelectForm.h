#pragma once

#include "dsutil.h"
#include "afxcmn.h"
#include "afxwin.h"


class CMediaTypeSelectForm : public CDialog
{
public:
	CMediaTypeSelectForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMediaTypeSelectForm();

	void SetMediaTypes(const DSUtil::MediaTypes& types) { media_types.Copy(types); }
	int SelectedMediaTypeIndex() const { return selected_media_type_index; }	// if >=0, the index of the selected media type in media_types

	// static vars for check box state persisted for the application lifetime
	static BOOL s_use_major_type;
	static BOOL s_use_sub_type;
	static BOOL s_use_sample_size;
	static BOOL s_use_format_block;

	DECLARE_DYNAMIC(CMediaTypeSelectForm)
	DECLARE_MESSAGE_MAP()

private:
    GraphStudio::TitleBar	m_title;
	DSUtil::MediaTypes		media_types;
	int						selected_media_type_index;		// selected index within in media_types or out of range if none selected
    CCrc32					m_crc32Calculator;
	CListCtrl				media_types_list;
	CButton					ok_button;
	CButton					cancel_button;
    CButton					checkUseMajorType;
    CButton					checkUseSubType;
    CButton					checkUseSampleSize;
    CButton					checkUseFormatBlock;

	// static vars for sorting which are persisted for the application lifetime
	static int				s_sorted_column;				// currently sorted column
	static int				s_sort_reverse;					// 1 if sorting up, -1 if reverse sorting

	virtual  BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	int GetSelectedMediaType();

// Dialog Data
	enum { IDD = IDD_DIALOG_MEDIA_TYPE_SELECT };

	afx_msg void OnDblclkListMediatypes(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnColumnclickListMediatypes(NMHDR *pNMHDR, LRESULT *pResult);

	static int CALLBACK CompareListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};

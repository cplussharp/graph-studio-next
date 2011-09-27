//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


namespace GraphStudio
{

	//-------------------------------------------------------------------------
	//
	//	FilenameList class
	//
	//-------------------------------------------------------------------------

	FilenameList::FilenameList(int maxsize) :
		max_size(maxsize)
	{
	}

	FilenameList::~FilenameList()
	{
		// nothing yet
	}


	void FilenameList::UpdateList(CString item)
	{
		for (int i=0; i<GetCount(); i++) {
			CString	&str = GetAt(i);
			if (str == item) {
				RemoveAt(i);
				break;
			}
		}

		// make sure we don't have more than our max size
		if (GetCount() >= max_size) {
			RemoveAt(GetCount()-1);
		}

		InsertAt(0, item);
	}

	void FilenameList::SaveList(CString name)
	{
		CString		count_name;
		count_name = name + _T("_count");
		int count = GetCount();

		if (count > max_size) count = max_size;

		AfxGetApp()->WriteProfileInt(_T("Settings"), count_name, count);

		for (int i=0; i<count; i++) {
			CString		key;
			key.Format(_T("%s_%d"), name, i);
			AfxGetApp()->WriteProfileString(_T("Settings"), key, GetAt(i));
		}
	}

	void FilenameList::LoadList(CString name)
	{
		CString		count_name;
		count_name = name + _T("_count");
		int count = AfxGetApp()->GetProfileInt(_T("Settings"), count_name, 0);

		// limit to max_size
		if (count > max_size) count = max_size;
		RemoveAll();

		for (int i=0; i<count; i++) {
			CString		item;
			CString		key;
			key.Format(_T("%s_%d"), name, i);
			item = AfxGetApp()->GetProfileString(_T("Settings"), key, _T(""));
			if (item != _T("")) {
				Add(item);
			}
		}
	}

};


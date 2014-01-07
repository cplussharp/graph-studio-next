//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	//-------------------------------------------------------------------------
	//
	//	MRUList class
	//
	//-------------------------------------------------------------------------

	MRUList::MRUList()
		: initial_menu_length(-1)
		, max_count(8)
	{
	}

	MRUList::~MRUList()
	{
		list.RemoveAll();
	}

	void MRUList::Save()
	{
		CString		count_name = _T("MRU_count");
		int count = (int) list.GetCount();
		if (count > max_count) count = max_count;

		AfxGetApp()->WriteProfileInt(_T("MRU"), count_name, count);

		for (int i=0; i<count; i++) {
			CString		key;
			key.Format(_T("MRU_%d"), i);
			AfxGetApp()->WriteProfileString(_T("MRU"), key, list[i]);
		}
	}

	void MRUList::Load()
	{
		CString		count_name = _T("MRU_count");
		int count = AfxGetApp()->GetProfileInt(_T("MRU"), count_name, 0);
		if (count > max_count) count = max_count;
		list.RemoveAll();

		for (int i=0; i<count; i++) {
			CString		item;
			CString		key;
			key.Format(_T("MRU_%d"), i);
			item = AfxGetApp()->GetProfileString(_T("MRU"), key, _T(""));
			if (item != _T("")) {
				list.Add(item);
			}
		}
	}

	void MRUList::NotifyEntry(CString filename)
	{
		for (int i=0; i<list.GetCount(); i++) {
			if (list[i] == filename) {
				list.RemoveAt(i);
				break;
			}
		}
		list.InsertAt(0, filename);
		Save();

#if defined(UNICODE)
        SHAddToRecentDocs(SHARD_PATHW, (LPCTSTR)filename);
#else
        SHAddToRecentDocs(SHARD_PATHA, (LPCTSTR)filename);
#endif
	}

	void MRUList::Clear()
	{
		list.RemoveAll();
		Save();
	}

	void MRUList::GenerateMenu(CMenu *menu)
	{
		const int menu_length = menu->GetMenuItemCount();

		if (initial_menu_length < 0)
			initial_menu_length = menu_length;		// store the menu length the first time we're called before we modify it

		// we insert our items just after print setup item (Exit)

		// find print setup menu item
		int insert_pos = 0;
		for (; insert_pos < menu_length; insert_pos++) {
			if (menu->GetMenuItemID(insert_pos) == ID_FILE_PRINT_SETUP)
				break;
		}
		insert_pos += 2;	// position after print setup and separator

		// delete menu items added in previous calls
		for (int i=0; i < menu_length-initial_menu_length; i++) {
			menu->DeleteMenu(insert_pos, MF_BYPOSITION);
		}

		// add recent files
		const int count = min(max_count, (int) list.GetCount());
		for (int i=0; i<count; i++) {
			CString	t;
			t.Format(_T("&%i %s"), (i+1), list[i]);
			menu->InsertMenu(insert_pos + i, MF_BYPOSITION | MF_STRING, ID_LIST_MRU_FILE0 + i, t);
		}
		
		// add clear list menu item and separator after file list
		menu->InsertMenu(insert_pos + count, MF_BYPOSITION | MF_STRING, ID_LIST_MRU_CLEAR, _T("Clear Recent &List"));
		menu->InsertMenu(insert_pos + count + 1, MF_BYPOSITION | MF_SEPARATOR);
	
	}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation


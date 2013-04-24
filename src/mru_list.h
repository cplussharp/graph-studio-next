//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	//-------------------------------------------------------------------------
	//
	//	MRUList class
	//
	//-------------------------------------------------------------------------

	class MRUList
	{
	public:
		CArray<CString>		list;
		int					max_count;
	public:
		MRUList();
		virtual ~MRUList();

		// I/O
		void NotifyEntry(CString filename);
		void Clear();
		void Save();
		void Load();

		// generate menu
		void GenerateMenu(CMenu *menu);
	};

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

namespace GraphStudio
{

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


};


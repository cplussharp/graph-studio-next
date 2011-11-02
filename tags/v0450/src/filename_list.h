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
	//	FilenameList class
	//
	//-------------------------------------------------------------------------

	class FilenameList : public CArray<CString>
	{
	public:

		int			max_size;						// maximum number of items

	public:
		FilenameList(int maxsize=30);
		virtual ~FilenameList();

		// methods
		void LoadList(CString name);
		void SaveList(CString name);
		void UpdateList(CString item);

	};

};

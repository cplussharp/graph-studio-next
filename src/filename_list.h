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

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

namespace XML
{
	/*
		I've been using these in some linux projects with expat. I've
		decided to port them to windows/MFC because the microsoft
		API sucks really hard.
	*/

	class XMLNode;

	//-------------------------------------------------------------------------
	//
	//	XMLField
	//
	//-------------------------------------------------------------------------
	class XMLField
	{
	public:
		CString		key;
		CString		value;
	public:
		XMLField();
		XMLField(CString k, CString v);
		XMLField(const XMLField &f);
		virtual ~XMLField();
		XMLField &operator =(const XMLField &f);
	};

	typedef list<XMLNode*>				XMLList;
	typedef XMLList::iterator			XMLIterator;
	typedef list<XMLField*>				FieldList;
	typedef FieldList::iterator			FieldIterator;

	//-------------------------------------------------------------------------
	//
	//	ValueList
	//
	//-------------------------------------------------------------------------
	class ValueList
	{
	public:
		FieldList		fields;
	public:
		ValueList();
		virtual ~ValueList();

		// I/O
		void RemoveFields();
		void AddField(CString k, CString v);
		void AddFieldForce(CString k, CString v);
		CString GetValue(CString key);
		int GetValue(CString key, int def);

		// operators
		CString operator [](const CString &x) { return GetValue(x); }
		CString operator [](LPCTSTR x) { return GetValue(CString(x)); }

		// helpers
		FieldIterator fieldsBegin() { return fields.begin(); }
		FieldIterator fieldsEnd() { return fields.end(); }
	};

	//-------------------------------------------------------------------------
	//
	//	XMLNode
	//
	//-------------------------------------------------------------------------
	class XMLNode : public ValueList
	{
	public:
		CString			name;
		int				depth;
		XMLList			nodes;
	public:
		XMLNode();
		virtual ~XMLNode();

		void AddNode(XMLNode *node);
		void Copy(XMLNode **new_node);
		void RemoveNodes();
		void Clear();

		int Find(CString name, XMLIterator *iter);
	};

	//-------------------------------------------------------------------------
	//
	//	XMLWriter
	//
	//-------------------------------------------------------------------------
	class XMLWriter
	{
	public:
		list<CString>		stack;
		list<bool>			has_children;
		list<bool>			has_params;
		list<bool>			in_params;
		CString				ret;
		int					depth;
		bool				use_depth;
		CString				offset_string;

		void PrepareOffset();
	public:
		XMLWriter();
		virtual ~XMLWriter();

		// writing
		void Clear();
		int BeginNode(CString name);
		int EndNode();
		int WriteValue(CString name, CString value);
		int WriteValue(CString name, int value);

		CString XML() { return ret; }
	};

	//-------------------------------------------------------------------------
	//
	//	XMLFile
	//
	//-------------------------------------------------------------------------
	class XMLFile
	{
	public:
		XMLNode				*root;
		CString				version;
	public:
		XMLFile();
		virtual ~XMLFile();

		// loading
		int LoadFromFile(CString fn);
		int LoadFromXmlReader(IXmlReader *reader);
	};



};









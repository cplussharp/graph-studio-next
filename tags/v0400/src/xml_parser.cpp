//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

namespace XML
{

	//-------------------------------------------------------------------------
	//
	//	XMLField
	//
	//-------------------------------------------------------------------------

	XMLField::XMLField() :
		key(_T("")),
		value(_T(""))
	{
	}
	
	XMLField::XMLField(CString k, CString v) :
		key(k),
		value(v)
	{
	}
	
	XMLField::XMLField(const XMLField &f) :
		key(f.key),
		value(f.value)
	{
	}
	
	XMLField::~XMLField()
	{
	}
	
	XMLField &XMLField::operator =(const XMLField &f)
	{
		key = f.key;
		value = f.value;
		return *this;
	}

	//-------------------------------------------------------------------------
	//
	//	ValueList
	//
	//-------------------------------------------------------------------------

	ValueList::ValueList()
	{
	}
	
	ValueList::~ValueList()
	{
		RemoveFields();
	}

	void ValueList::RemoveFields()
	{
		FieldIterator	it;
		for (it = fieldsBegin(); it != fieldsEnd(); it++) {
			XMLField *f = *it;
			delete f;
		}
		fields.clear();
	}
	
	void ValueList::AddField(CString k, CString v)
	{
		// pozrieme, ci uz taky nemame
		FieldIterator	it;
		for (it = fieldsBegin(); it != fieldsEnd(); it++) {
			XMLField *f = *it;
			if (f->key == k) {
				f->value = v;
				return ;
			}
		}
		
		XMLField *f = new XMLField(k, v);
		fields.push_back(f);
	}	
	
	void ValueList::AddFieldForce(CString k, CString v)
	{
		XMLField *f = new XMLField(k, v);
		fields.push_back(f);
	}
	
	CString ValueList::GetValue(CString key)
	{
		CString ret = _T("");
		FieldIterator it;
		for (it = fieldsBegin(); it != fieldsEnd(); it++) {
			XMLField *f = *it;
			if (f->key == key) {
				ret = f->value;
				break;
			}
		}
		return ret;
	}
	
	int ValueList::GetValue(CString key, int def)
	{
		FieldIterator it;
		for (it = fieldsBegin(); it != fieldsEnd(); it++) {
			XMLField *f = *it;
			if (f->key == key) {			
				int val;
				if (_stscanf_s(f->value, _T("%d"), &val) == 1) {
					return val;
				} else {
					return def;
				}
			}
		}
		return def;
	}
	
	
	XMLNode::XMLNode() :
		ValueList(),
		name(""),
		depth(0)
	{
	}

	XMLNode::~XMLNode()
	{
		Clear();
	}
	
	void XMLNode::RemoveNodes()
	{
		list<XMLNode*>::iterator	it;
		for (it = nodes.begin(); it != nodes.end(); it++) {
			XMLNode *n = *it;
			delete n;
		}
		nodes.clear();
	}
	
	void XMLNode::Clear()
	{
		RemoveFields();
		RemoveNodes();
	}

	void XMLNode::AddNode(XMLNode *node)
	{
		node->depth = depth + 1;
		nodes.push_back(node);
	}	
	
	// hladanie
	int XMLNode::Find(CString name, XMLIterator *iter)
	{
		XMLIterator it;
		for (it = nodes.begin(); it != nodes.end(); it++) {
			XMLNode *n = *it;
			if (n->name == name) {
				*iter = it;
				return 0;
			}
		}
		
		return -1;
	}
	
	void XMLNode::Copy(XMLNode **new_node)
	{
		XMLNode *newroot = new XMLNode();
		newroot->name = name;
		newroot->depth = depth;
	
		// pridame rovnake polia
		list<XMLField*>::iterator	itf;
		for (itf=fields.begin(); itf!=fields.end(); itf++) {
			XMLField *f = *itf;
			newroot->AddField(f->key, f->value);
		}
		
		// a este aj kopie nodov
		list<XMLNode*>::iterator	itn;
		for (itn=nodes.begin(); itn!=nodes.end(); itn++) {
			XMLNode *n = *itn;
			XMLNode *newnode = NULL;
			
			// novu kopiu
			n->Copy(&newnode);
			newroot->AddNode(newnode);						
		}
	}

	XMLFile::XMLFile()
	{
		root = new XMLNode();
	}
	
	XMLFile::~XMLFile()
	{
		delete root;
	}

	int XMLFile::LoadFromFile(CString fn)
	{
		root->Clear();

		CComPtr<IXmlReader>		reader;
		CComPtr<IStream>		stream;
		HRESULT					hr = NOERROR;

		do {
			// create file stream
			hr = SHCreateStreamOnFile(fn, STGM_READ | STGM_SHARE_DENY_WRITE, &stream);
			if (FAILED(hr)) break;

			// reader
			hr = CreateXmlReader(IID_IXmlReader, (void**)&reader, NULL);
			if (FAILED(hr)) break;

			// select input
			hr = reader->SetInput(stream);
			if (FAILED(hr)) break;

			int ret = LoadFromXmlReader(reader);
			if (ret < 0) {
				hr = E_FAIL;
			} else {
				hr = NOERROR;
			}
		} while (0);

		// release objects
		stream = NULL;
		reader = NULL;

		if (FAILED(hr)) return -1;
		return 0;
	}	

	int XMLFile::LoadFromXmlReader(IXmlReader *reader)
	{
		XmlNodeType		type;
		int				depth = 0;
		LPCWSTR			text, value;
		HRESULT			hr;

		list<XMLNode*>	stack;	
		stack.push_back(root);

		while (reader->Read(&type) == NOERROR) {

			switch (type) {
			case XmlNodeType_Element:
				{
					BOOL	is_empty = reader->IsEmptyElement();

					XMLNode	*parent = stack.back();
					XMLNode	*current = new XMLNode();
					stack.push_back(current);

					// add a new node
					parent->AddNode(current);

					hr = reader->GetLocalName(&text, NULL);
					if (FAILED(hr)) return -1;
					current->name = CString(text);

					// load attributes
					while (true) {
						hr = reader->MoveToNextAttribute();
						if (hr != NOERROR) break;

						// read attribute name and value name
						hr = reader->GetLocalName(&text, NULL);
						if (FAILED(hr)) return -1;
						hr = reader->GetValue(&value, NULL);
						if (FAILED(hr)) return -1;

						CString	attr_name(text);
						CString	attr_value(value);

						current->AddField(attr_name, attr_value);
					}

					if (is_empty) {
						stack.pop_back();
					}
				}
				break;
			case XmlNodeType_EndElement:
				{
					stack.pop_back();
				}
				break;

			}

		}
		return 0;
	}

	XMLWriter::XMLWriter()
	{
		depth = -4;
		use_depth = true;
		offset_string = _T("");
		Clear();
	}

	XMLWriter::~XMLWriter()
	{
		Clear();
	}
	
	void XMLWriter::Clear()
	{
		ret = _T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
		stack.clear();
		has_children.clear();
		has_params.clear();
		in_params.clear();
		depth = -4;		
	}

	void XMLWriter::PrepareOffset()
	{
		offset_string = _T("");
		for (int i=0; i<depth; i++) offset_string+= _T(" ");
	}

	int XMLWriter::BeginNode(CString name)
	{
		depth += 4;
		PrepareOffset();

		// update children list of the parent
		if (!has_children.empty()) {
			bool child = has_children.back();
			if (!child) {
				has_children.pop_back();
				has_children.push_back(true);
			}

			bool p = in_params.back();
			if (p) {
				ret += _T(">\n");
				in_params.pop_back();
				in_params.push_back(false);
			}
		}

		stack.push_back(name);
		has_children.push_back(false);
		has_params.push_back(false);
		in_params.push_back(true);
		
		if (use_depth) ret += offset_string;
		ret += _T("<") + name;		

		// hlbka v urovniach...
		return ((depth+4)>>2);
	}

	int XMLWriter::EndNode()
	{	

		bool child = has_children.back();
		bool param = has_params.back();		
		CString name = stack.back();

		if (child) {
			if (use_depth) ret += offset_string;
			ret += _T("</") + name + _T(">\n");
		} else {
			if (!param) {
				ret += _T(" />\n");
			} else {
				ret += _T("/>\n");
			}
		}		

		depth -= 4;
		if (use_depth) PrepareOffset();

		has_children.pop_back();
		has_params.pop_back();
		in_params.pop_back();
		stack.pop_back();
		return ((depth+4)>>2);
	}

	int XMLWriter::WriteValue(CString name, int value)
	{
		CString	temp;
		temp.Format(_T("%d"), value);
		return WriteValue(name, temp);
	}

	int XMLWriter::WriteValue(CString name, CString value)
	{
		if (!has_params.empty()) {
			bool param = has_params.back();
			if (!param) {
				has_params.pop_back();
				has_params.push_back(true);
			}
		}

		// escape quotes
		for (int i=value.GetLength()-1; i>=0; i--) {
			if (value[i] == L'\"') {
				value.Insert(i, _T("\\"));
			}
		}

		ret += _T(" ")+name+_T("=\"") + value + _T("\"");
		return 0;
	}

};

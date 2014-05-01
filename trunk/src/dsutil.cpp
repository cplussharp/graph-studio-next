//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <atlbase.h>
#include <atlpath.h>
#include <afxtaskdialog.h>

#include "MediaTypeSelectForm.h"

namespace DSUtil
{

	CString get_next_token(CString &str, CString separator)
	{
		CString ret;
		int pos = str.Find(separator);
		if (pos < 0) {
			ret = str;
			ret = ret.Trim();
			str = _T("");
			return ret;
		}

		// mame ho
		ret = str.Left(pos);
		ret = ret.Trim();
		str.Delete(0, pos + separator.GetLength());
		str = str.Trim();
		return ret;
	}

    //-------------------------------------------------------------------------
	//
	//	ClassFactory class
	//
	//-------------------------------------------------------------------------
    int CClassFactory::m_cLocked = 0;
    
    CClassFactory::CClassFactory(const CFactoryTemplate *pTemplate)
    : CBaseObject(NAME("Class Factory")), m_cRef(0), m_pTemplate(pTemplate)
    {
    }  
    
    STDMETHODIMP CClassFactory::QueryInterface(REFIID riid,void **ppv)
    {
        CheckPointer(ppv,E_POINTER)
        ValidateReadWritePtr(ppv,sizeof(PVOID));
        *ppv = NULL;
        
        if ((riid == IID_IUnknown) || (riid == IID_IClassFactory)) {
            *ppv = (LPVOID) this;
            
            ((LPUNKNOWN) *ppv)->AddRef();
            return NOERROR;
        }
    
        return ResultFromScode(E_NOINTERFACE);
    }  
    
    STDMETHODIMP_(ULONG) CClassFactory::AddRef()
    {
        return ++m_cRef;
    }
    
    STDMETHODIMP_(ULONG) CClassFactory::Release()
    {
        if (--m_cRef == 0) {
            delete this;
            return 0;
        } else {
            return m_cRef;
        }
    }
    
    STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **pv)
    {
        CheckPointer(pv,E_POINTER)
        ValidateReadWritePtr(pv,sizeof(void *));
    
        if (pUnkOuter != NULL) {
            if (IsEqualIID(riid,IID_IUnknown) == FALSE) {
                return ResultFromScode(E_NOINTERFACE);
            }
        }
    
        HRESULT hr = NOERROR;
        CUnknown *pObj = m_pTemplate->CreateInstance(pUnkOuter, &hr);
    
        if (pObj == NULL) {
            if (SUCCEEDED(hr)) {
                hr = E_OUTOFMEMORY;
            }
            return hr;
        }
    
        if (FAILED(hr)) {
            delete pObj;
            return hr;
        }
        
        pObj->NonDelegatingAddRef();
        hr = pObj->NonDelegatingQueryInterface(riid, pv);
        pObj->NonDelegatingRelease();
     
        if (SUCCEEDED(hr)) {
            ASSERT(*pv);
        }
    
        return hr;
    }
    
    STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
    {
        if (fLock) {
            m_cLocked++;
        } else {
            m_cLocked--;
        }
        return NOERROR;
    }

    void CClassFactory::Register()
    {
        DWORD dwRegister;
        HRESULT hr = CoRegisterClassObject(*m_pTemplate->m_ClsID, this, CLSCTX_INPROC_SERVER, REGCLS_MULTI_SEPARATE, &dwRegister);
    }


	//-------------------------------------------------------------------------
	//
	//	URI class
	//
	//-------------------------------------------------------------------------

	URI::URI() :
		protocol(_T("")),
		host(_T("")),
		request_url(_T("")),
		complete_request(_T("")),
		port(80)
	{
	}

	URI::URI(const URI &u) :
		protocol(u.protocol),
		host(u.host),
		request_url(u.request_url),
		complete_request(u.complete_request),
		port(u.port)
	{
	}

	URI::URI(CString url)
	{
		Parse(url);
	}

	URI::~URI()
	{
		// zatial nic
	}

	URI &URI::operator =(const URI &u)
	{
		protocol = u.protocol;
		host = u.host;
		request_url = u.request_url;
		complete_request = u.complete_request;
		port = u.port;
		return *this;
	}

	URI &URI::operator =(CString url)
	{
		Parse(url);
		return *this;
	}

	int URI::Parse(CString url)
	{
		// protocol://host:port/request_url
		protocol = _T("http");
		host = _T("");
		request_url = _T("");

		int pos;
		pos = url.Find(_T("://"));
		if (pos > 0) {
			protocol = url.Left(pos);
			url.Delete(0, pos+3);
		}
		port = 80;		// map protocol->port

		// najdeme lomitko
		pos = url.Find(_T("/"));
		if (pos < 0) {
			request_url = _T("/");
			host = url;
		} else {
			host = url.Left(pos);
			url.Delete(0, pos);
			request_url = url;
		}

		// parsneme host a port
		pos = host.Find(_T(":"));
		if (pos > 0) {
			CString temp_host = host;
			host = temp_host.Left(pos);
			temp_host.Delete(0, pos+1);

			temp_host.Trim();
			_stscanf_s(temp_host, _T("%d"), &port);
		}

		// testy spravnosti
		if (host == _T("") || request_url == _T("")) return -1;

		// spravime kompletny request
		complete_request.Format(_T("%s://%s:%d%s"), protocol, host, port, request_url);
		return 0;
	}

	Pin::Pin() :
		filter(NULL),
		pin(NULL),
		name(_T("")),
		dir(PINDIR_INPUT)
	{
	}

	Pin::Pin(const Pin &p) :
		dir(p.dir),
		name(p.name)
	{
		filter = p.filter;	if (filter) filter->AddRef();
		pin = p.pin;		if (pin) pin->AddRef();
	}

	Pin::~Pin()
	{
		if (filter) filter->Release();
		if (pin) pin->Release();
	}

	Pin &Pin::operator =(const Pin &p)
	{
		dir = p.dir;
		name = p.name;
		if (filter) filter->Release();
		filter = p.filter;
		if (filter) filter->AddRef();

		if (pin) pin->Release();
		pin = p.pin;
		if (pin) pin->AddRef();

		return *this;
	}

	PinTemplate::PinTemplate() :
		dir(PINDIR_INPUT),
		rendered(FALSE),
		many(FALSE),
		types(0)
	{
	}

	PinTemplate::PinTemplate(const PinTemplate &pt) :
		dir(pt.dir),
		rendered(pt.rendered),
		many(pt.many),
		types(pt.types)
	{
		major.Append(pt.major);
		minor.Append(pt.minor);
	}

	PinTemplate::~PinTemplate()
	{
		major.RemoveAll();
		minor.RemoveAll();
	}

	PinTemplate &PinTemplate::operator =(const PinTemplate &pt)
	{
		dir = pt.dir;
		rendered = pt.rendered;
		many = pt.many;
		major.RemoveAll();	major.Append(pt.major);
		minor.RemoveAll();	minor.Append(pt.minor);
		types = pt.types;
		return *this;
	}

	FilterTemplate::FilterTemplate() :
		type(FilterTemplate::FT_FILTER),
		wave_in_id(-1),
		file_exists(false),
		clsid(GUID_NULL),
		category(GUID_NULL),
		moniker(NULL),
		version(0),
		merit(0)
	{
	}

	FilterTemplate::FilterTemplate(const FilterTemplate &ft) :
		name(ft.name),
		moniker_name(ft.moniker_name),
		description(ft.description),
		device_path(ft.device_path),
		type(ft.type),
		file(ft.file),
		wave_in_id(ft.wave_in_id),
		file_exists(ft.file_exists),
		clsid(ft.clsid),
		category(ft.category),
		version(ft.version),
		merit(ft.merit),
		moniker(NULL)
	{
		if (ft.moniker) {
			moniker = ft.moniker;
			moniker->AddRef();
		}

		input_pins.Append(ft.input_pins);
		output_pins.Append(ft.output_pins);
	}

	FilterTemplate::~FilterTemplate()
	{
		input_pins.RemoveAll();
		output_pins.RemoveAll();
		if (moniker) {
			moniker->Release();
			moniker = NULL;
		}
	}

	FilterTemplate &FilterTemplate::operator =(const FilterTemplate &ft)
	{
		if (moniker) { moniker->Release(); moniker = NULL; }
		moniker = ft.moniker;
		if (moniker) moniker->AddRef();

		input_pins.RemoveAll();
		output_pins.RemoveAll();
		input_pins.Append(ft.input_pins);
		output_pins.Append(ft.output_pins);

		name = ft.name;
		moniker_name = ft.moniker_name;
		description = ft.description;
		device_path = ft.device_path;
		file = ft.file;
		wave_in_id = ft.wave_in_id;
		file_exists = ft.file_exists;
		clsid = ft.clsid;
		category = ft.category;
		version = ft.version;
		merit = ft.merit;
		type = ft.type;
		return *this;
	}

	void DoReplace(CString &str, CString old_str, CString new_str)
	{
		CString	temp = str;
		temp.MakeUpper();
		int p = temp.Find(old_str);
		if (p >= 0) {
			str.Delete(p, old_str.GetLength());
			str.Insert(p, new_str);
		}
	}

	HRESULT FilterTemplate::FindFilename()
	{
		// HKEY_CLASSES_ROOT\CLSID\{07C9CB2C-F51C-47EA-B551-7DA02541D586}
		LPOLESTR	str;
		StringFromCLSID(clsid, &str);
		CString		str_clsid(str);
		CString		key_name;
		if (str) CoTaskMemFree(str);

		key_name.Format(_T("CLSID\\%s\\InprocServer32"), str_clsid);
		CRegKey		key;
		if (key.Open(HKEY_CLASSES_ROOT, key_name, KEY_READ) != ERROR_SUCCESS) { 
			file_exists = false;
			return -1;
		}

		TCHAR		temp[4*1024];
		TCHAR		fullpath[4*1024];
		LPWSTR		fn;
		ULONG		chars=4*1024;

		key.QueryStringValue(_T(""), temp, &chars);
		temp[chars]=0;
		file = temp;
		key.Close();

		// replace characters
		CString		progfiles, sysdir, windir;

		SHGetSpecialFolderPath(NULL, temp, CSIDL_PROGRAM_FILES, FALSE);	progfiles = temp;
		SHGetSpecialFolderPath(NULL, temp, CSIDL_SYSTEM, FALSE);		sysdir = temp;
		SHGetSpecialFolderPath(NULL, temp, CSIDL_WINDOWS, FALSE);		windir = temp;
		
		DoReplace(file, _T("%PROGRAMFILES%"), progfiles);
		DoReplace(file, _T("%SYSTEMROOT%"), windir);
		DoReplace(file, _T("%WINDIR%"), windir);

		DWORD ret = SearchPath(NULL, file.GetBuffer(), NULL, 4*1024, fullpath, &fn);
		if (ret > 0) {
			file_exists = true;
			file = fullpath;
		} else {
			CPath	path(file);
			if (path.FileExists()) {
				file_exists = true;
			} else {
				file_exists = false;
			}
		}
		return NOERROR;
	}

	int FilterTemplate::WriteMerit()
	{
		/*
			Currently works only for DMO and normal filters
		*/

		LPOLESTR	str;
		StringFromCLSID(clsid, &str);
		CString		str_clsid(str);
		CString		key_name;
		if (str) CoTaskMemFree(str);

		if (type == FilterTemplate::FT_DMO) {

			key_name.Format(_T("CLSID\\%s"), str_clsid);
			CRegKey		key;
			if (key.Open(HKEY_CLASSES_ROOT, key_name, KEY_READ | KEY_WRITE) == ERROR_SUCCESS) { 
				// simply write the new merit.
				key.SetDWORDValue(_T("Merit"), merit);
			} else {
				// cannot update merit
				return -1;
			}
			key.Close();
			return 0;

		} else
		if (type == FilterTemplate::FT_FILTER) {

			/*
				Load the FilterData buffer, then change the merit value and
				write it back.
			*/

			key_name.Format(_T("CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\%s"), str_clsid);
			CRegKey		key;
			if (key.Open(HKEY_CLASSES_ROOT, key_name, KEY_READ | KEY_WRITE) == ERROR_SUCCESS) { 

				ULONG		size = 256*1024;
				BYTE		*largebuf = (BYTE*)malloc(size);
				LONG		lret;

				if (!largebuf) { key.Close(); return -1; }

				lret = key.QueryBinaryValue(_T("FilterData"), largebuf, &size);
				if (lret != ERROR_SUCCESS) { free(largebuf); key.Close(); return -1; }

				// change the merit
				DWORD		*dwbuf = (DWORD*)largebuf;
				dwbuf[1] = merit;

				// and write the buffer back
				lret = key.SetBinaryValue(_T("FilterData"), largebuf, size);
				if (lret != ERROR_SUCCESS) { free(largebuf); key.Close(); return -1; }

				free(largebuf);
				
			} else {
				// cannot update merit
				return -1;
			}
			key.Close();
			return 0;

		}

		return -1;
	}

	int FilterTemplate::LoadFilterData(char *buf, int size)
	{
		DWORD	*b = (DWORD*)buf;

		version = b[0];
		merit   = b[1];

		int cpins1, cpins2;
		cpins1  = b[2];
		cpins2  = b[3];

		CArray<PinTemplate>		temp_pins;

		DWORD	*ps = b+4;
		for (int i=0; i<cpins1; i++) {
			if ((char*)ps > (buf + size - 6*4)) break;

			PinTemplate	pin;

			DWORD	flags = ps[1];
			pin.rendered  = (flags & 0x02 ? TRUE : FALSE);
			pin.many      = (flags & 0x04 ? TRUE : FALSE);
			pin.dir       = (flags & 0x08 ? PINDIR_OUTPUT : PINDIR_INPUT);
			pin.types     = ps[3];
			
			// skip dummy data
			ps += 6;
			for (int j=0; j<pin.types; j++) {

				// make sure we have at least 16 bytes available
				if ((char*)ps > (buf + size - 16)) break;
				
				DWORD maj_offset = ps[2];
				DWORD min_offset = ps[3];

				if ((maj_offset + 16 <= (DWORD) size) && (min_offset + 16 <= (DWORD) size)) {
					GUID	g;
					BYTE	*m = (BYTE*)(&buf[maj_offset]);
					if ((char*)m > (buf+size - 16)) break;
					g.Data1 = m[0] | (m[1] << 8) | (m[2] << 16) | (m[3] << 24);
					g.Data2 = m[4] | (m[5] << 8);
					g.Data3 = m[6] | (m[7] << 8);
					memcpy(g.Data4, m+8, 8);
					pin.major.Add(g);

					m = (BYTE*)(&buf[min_offset]);
					if ((char*)m > (buf+size - 16)) break;
					g.Data1 = m[0] | (m[1] << 8) | (m[2] << 16) | (m[3] << 24);
					g.Data2 = m[4] | (m[5] << 8);
					g.Data3 = m[6] | (m[7] << 8);
					memcpy(g.Data4, m+8, 8);
					pin.minor.Add(g);
				}
			
				ps += 4;
			}
			pin.types = (int) pin.major.GetCount();

			if (pin.dir == PINDIR_OUTPUT) {
				output_pins.Add(pin);
			} else {
				input_pins.Add(pin);
			}
		}

		return 0;
	}

	// vytvorenie instancie
	HRESULT FilterTemplate::CreateInstance(IBaseFilter **filter)
	{
		HRESULT hr;
		if (!filter) return E_POINTER;

		// do we have a moniker ?
		if (moniker) {
			return moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)filter);
		}
	
		// let's try it with a moniker display name
		do {
			CComPtr<IMoniker>		loc_moniker;
			CComPtr<IBindCtx>		bind;
			ULONG					eaten = 0;

			if (FAILED(CreateBindCtx(0, &bind))) break;
			hr = MkParseDisplayName(bind, moniker_name, &eaten, &loc_moniker);
			if (hr != NOERROR) { bind = NULL; break; }

			hr = loc_moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)filter);
			if (SUCCEEDED(hr)) return NOERROR;

			loc_moniker = NULL;
			bind = NULL;
		} while (0);

		// last resort
		return CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)filter);
	}

	HRESULT FilterTemplate::ReadFromMoniker(IMoniker* moniker)
	{
		CheckPointer(moniker, E_POINTER);

		CComPtr<IPropertyBag> propbag;

		HRESULT hr = moniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&propbag);
		if (SUCCEEDED(hr)) {
			VARIANT				var;

			VariantInit(&var);
			HRESULT hr_prop = propbag->Read(L"FriendlyName", &var, 0);
			if (SUCCEEDED(hr_prop) && var.vt == VT_BSTR) {
				name = CString(var.bstrVal);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr_prop = propbag->Read(L"DevicePath", &var, 0);
			if (SUCCEEDED(hr_prop) && var.vt == VT_BSTR) {
				device_path = CString(var.bstrVal);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr_prop = propbag->Read(L"Description", &var, 0);
			if (SUCCEEDED(hr_prop) && var.vt == VT_BSTR) {
				description = CString(var.bstrVal);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr_prop = propbag->Read(L"WaveInID", &var, 0);
			if (SUCCEEDED(hr_prop) && var.vt == VT_I4) {
				wave_in_id = var.intVal;
			}
			VariantClear(&var);

			VariantInit(&var);
			hr_prop = propbag->Read(L"FilterData", &var, 0);
			if (SUCCEEDED(hr_prop)) {
				SAFEARRAY	*ar = var.parray;
				int	size = ar->rgsabound[0].cElements;

				// load merit and version
				LoadFilterData((char*)ar->pvData, size);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr_prop = propbag->Read(L"CLSID", &var, 0);
			if (SUCCEEDED(hr_prop) && var.vt == VT_BSTR) {			// allow failure to read CLSID as this happens with DMOs
				hr_prop = CLSIDFromString(var.bstrVal, &clsid);
				ASSERT(SUCCEEDED(hr_prop));
				ASSERT(GUID_NULL != clsid);
			}

			VariantClear(&var);
		}
		return hr;
	}

	bool FilterTemplate::ParseCategoryFromMonikerName(const CString& display_name, GUID& category)
	{
		bool success = false;

		const int guid_start_pos = display_name.Find(_T("{"));	// search for first opening and closing brace
		const int guid_end_pos = display_name.Find(_T("}"), guid_start_pos);
		if (guid_start_pos > 0 && guid_end_pos > guid_start_pos) {
			const CString guid_str = display_name.Mid(guid_start_pos, guid_end_pos - guid_start_pos + 1);	// include closing brace
			success = SUCCEEDED(CLSIDFromString(guid_str, &category));
		}
		return success;
	}

	int FilterTemplate::ParseMonikerName()
	{
		if (moniker_name == _T("")) {
			type = FilterTemplate::FT_FILTER;
			return 0;
		}

		// we parse out the filter type
		if (moniker_name.Find(_T(":sw:")) >= 0)		type = FilterTemplate::FT_FILTER; else
		if (moniker_name.Find(_T(":dmo:")) >= 0)	type = FilterTemplate::FT_DMO; else
		if (moniker_name.Find(_T(":cm:")) >= 0)		type = FilterTemplate::FT_ACM_ICM; else
		if (moniker_name.Find(_T(":pnp:")) >= 0)		type = FilterTemplate::FT_PNP; else
			type = FilterTemplate::FT_KSPROXY;

		if (category == GUID_NULL) {		// as a backup get the category from the display name
			ParseCategoryFromMonikerName(moniker_name, category);
		}
		return 0;
	}

	int FilterTemplate::LoadFromMonikerName(CString displayname)
	{
		/*
			First create the moniker and then extract all the information just like when
			enumerating a category.
		*/

		HRESULT					hr;
		CComPtr<IMoniker>		loc_moniker;
		CComPtr<IBindCtx>		bind;
		CComPtr<IPropertyBag>	propbag;
		ULONG					eaten = 0;

		if (FAILED(CreateBindCtx(0, &bind))) 
			return -1;
		hr = MkParseDisplayName(bind, displayname, &eaten, &loc_moniker);
		bind = NULL;
		if (hr != NOERROR) { 
			return -1; 
		}

		hr = ReadFromMoniker(loc_moniker);
		if (SUCCEEDED(hr)) {
			FindFilename();
			moniker_name = displayname;
			ParseMonikerName();
		}
		return 0;
	}

	FilterCategory::FilterCategory() :
		name(_T("")),
		clsid(GUID_NULL),
		is_dmo(false)
	{
	}

	FilterCategory::FilterCategory(CString nm, GUID cat_clsid, bool dmo) :
		name(nm),
		clsid(cat_clsid),
		is_dmo(dmo)
	{
	}

	FilterCategory::FilterCategory(const FilterCategory &fc) :
		name(fc.name),
		clsid(fc.clsid),
		is_dmo(fc.is_dmo)
	{
	}

	FilterCategory::~FilterCategory()
	{
	}

	FilterCategory &FilterCategory::operator =(const FilterCategory &fc)
	{
		name = fc.name;
		clsid = fc.clsid;
		is_dmo = fc.is_dmo;
		return *this;
	}


	FilterCategories::FilterCategories()
	{
		Enumerate();
	}

	FilterCategories::~FilterCategories()
	{
	}

	int FilterCategories::Enumerate()
	{
		// ideme nato
		ICreateDevEnum		*sys_dev_enum = NULL;
		IEnumMoniker		*enum_moniker = NULL;
		IMoniker			*moniker = NULL;
		IPropertyBag		*propbag = NULL;
		ULONG				f;
		HRESULT				hr;
		int					ret = -1;

		// Add the special wildcard categories
		categories.Add(FilterCategory(_T(" (ALL) DirectShow Filters"), GUID_NULL, false));
		categories.Add(FilterCategory(_T(" (ALL) DMO"), GUID_NULL, true));

		do {
			hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&sys_dev_enum);
			if (FAILED(hr)) break;

			// ideme enumerovat clasy
			hr = sys_dev_enum->CreateClassEnumerator(CLSID_ActiveMovieCategories, &enum_moniker, 0);
			if ((hr != NOERROR) || !enum_moniker) break;

			enum_moniker->Reset();
			while (enum_moniker->Next(1, &moniker, &f) == NOERROR) {
				hr = moniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&propbag);
				if (SUCCEEDED(hr)) {
					VARIANT				var;
					FilterCategory		category;

					VariantInit(&var);
					hr = propbag->Read(L"FriendlyName", &var, 0);
					if (SUCCEEDED(hr)) {
						category.name = CString(var.bstrVal);
					}
					VariantClear(&var);

					VariantInit(&var);
					hr = propbag->Read(L"CLSID", &var, 0);
					if (SUCCEEDED(hr)) {
						if (SUCCEEDED(CLSIDFromString(var.bstrVal, &category.clsid))) {
							// mame novu kategoriu
							categories.Add(category);
						}
					}
					VariantClear(&var);


					propbag->Release();
					propbag = NULL;
				}
				moniker->Release();
				moniker = NULL;
			}

			// sme okej
			ret = 0;
		} while (0);

		// now add the DMO categories
		categories.Add(FilterCategory(_T("DMO Video Decoder"),			DMOCATEGORY_VIDEO_DECODER, true));
		categories.Add(FilterCategory(_T("DMO Video Effect"),			DMOCATEGORY_VIDEO_EFFECT, true));
		categories.Add(FilterCategory(_T("DMO Video Encoder"),			DMOCATEGORY_VIDEO_ENCODER, true));
		categories.Add(FilterCategory(_T("DMO Audio Decoder"),			DMOCATEGORY_AUDIO_DECODER, true));
		categories.Add(FilterCategory(_T("DMO Audio Effect"),			DMOCATEGORY_AUDIO_EFFECT, true));
		categories.Add(FilterCategory(_T("DMO Audio Encoder"),			DMOCATEGORY_AUDIO_ENCODER, true));
		categories.Add(FilterCategory(_T("DMO Audio Capture Effect"),	DMOCATEGORY_AUDIO_CAPTURE_EFFECT, true));
		
		if (propbag) propbag->Release();
		if (moniker) moniker->Release();
		if (enum_moniker) enum_moniker->Release();
		if (sys_dev_enum) sys_dev_enum->Release();

		return ret;
	}


	FilterTemplates::FilterTemplates()
	{
		filters.RemoveAll();
	}

	FilterTemplates::~FilterTemplates()
	{
		filters.RemoveAll();
	}


	int _FilterCompare(FilterTemplate &f1, FilterTemplate &f2)
	{
		CString s1 = f1.name; s1.MakeUpper();
		CString s2 = f2.name; s2.MakeUpper();

		return s1.Compare(s2);
	}

	void FilterTemplates::SwapItems(SSIZE_T i, SSIZE_T j)
	{	
		if (i == j) return ;
		FilterTemplate	temp = filters[i];
		filters[i] = filters[j];
		filters[j] = temp;
	}

	void FilterTemplates::_Sort_(SSIZE_T lo, SSIZE_T hi)
	{
		// TODO: This should be really converted to template or otherwise reworked: the pattern is copy/pasted a few times throughout the project
		SSIZE_T i = lo, j = hi;
		FilterTemplate m;

		// pivot
		m = filters[ (lo+hi)>>1 ];

		do {
			while (_FilterCompare(m, filters[i])>0) i++;
			while (_FilterCompare(filters[j], m)>0) j--;

			if (i <= j) {
				SwapItems(i, j);
				i++;
				j--;
			}
		} while (i <= j);

		if (j > lo) _Sort_(lo, j);
		if (i < hi) _Sort_(i, hi);
	}

	void FilterTemplates::SortByName()
	{
		if (filters.GetCount() == 0) return ;
		_Sort_(0, filters.GetCount()-1);
	}

	int FilterTemplates::Enumerate(const FilterCategory &cat)
	{
		filters.RemoveAll();

		if (cat.is_dmo) {
			return EnumerateDMO(cat.clsid);
		} else if (GUID_NULL == cat.clsid) {
			return EnumerateAllRegisteredFilters();
		} else {
			return Enumerate(cat.clsid);
		}
	}

	bool FilterTemplates::FindTemplateByName(CString name, FilterTemplate *filter)
	{
		if (!filter) return false;
		for (int i=0; i<filters.GetCount(); i++) {
			if (name == filters[i].name) {
				*filter = filters[i];
				return true;
			}
		}

		// nemame nic
		return false;
	}

	bool FilterTemplates::FindTemplateByCLSID(GUID clsid, FilterTemplate *filter)
	{
		if (!filter) return false;
		for (int i=0; i<filters.GetCount(); i++) {
			if (clsid == filters[i].clsid) {
				*filter = filters[i];
				return true;
			}
		}

		// nemame nic
		return false;
	}

	// vytvaranie
	HRESULT FilterTemplates::CreateInstance(CString name, IBaseFilter **filter)
	{
		FilterTemplate	ft;
		if (FindTemplateByName(name, &ft)) {
			return ft.CreateInstance(filter);
		}
		return E_FAIL;
	}

	HRESULT FilterTemplates::CreateInstance(GUID clsid, IBaseFilter **filter)
	{
		FilterTemplate	ft;
		if (FindTemplateByCLSID(clsid, &ft)) {
			return ft.CreateInstance(filter);
		}
		return E_FAIL;
	}

    int FilterTemplates::EnumerateAudioSources()
	{
		filters.RemoveAll();
		int ret = Enumerate(CLSID_AudioInputDeviceCategory);
		if (ret < 0) return ret;
		SortByName();
		return ret;
	}

    int FilterTemplates::EnumerateVideoSources()
	{
		filters.RemoveAll();
        int ret = Enumerate(CLSID_VideoInputDeviceCategory);
		if (ret < 0) return ret;
		SortByName();
		return ret;
	}

	int FilterTemplates::EnumerateAudioRenderers()
	{
		filters.RemoveAll();
		int ret = Enumerate(CLSID_AudioRendererCategory);
		if (ret < 0) return ret;
		SortByName();
		return ret;
	}

	int FilterTemplates::EnumerateVideoRenderers()
	{
		CComPtr<IFilterMapper2>		mapper;
		CComPtr<IEnumMoniker>		emoniker;
		HRESULT						hr;
		int							ret = 0;

		// we're only interested in video types
		GUID						types[] = {
			MEDIATYPE_Video, MEDIASUBTYPE_None,
			MEDIATYPE_Video, MEDIASUBTYPE_RGB32,
			MEDIATYPE_Video, MEDIASUBTYPE_YUY2
		};

		filters.RemoveAll();

		hr = mapper.CoCreateInstance(CLSID_FilterMapper2);
		if (FAILED(hr)) return -1;

		// find all matching filters
		hr = mapper->EnumMatchingFilters(&emoniker, 0, FALSE,
					MERIT_DO_NOT_USE,
					TRUE, 3, types,	NULL, NULL,
					FALSE,
					FALSE, 0, NULL, NULL, NULL);
		if (SUCCEEDED(hr)) {
			ret = AddFilters(emoniker, 1, CLSID_LegacyAmFilterCategory);
		}

		emoniker = NULL;
		mapper = NULL;

		SortByName();

		return ret;
	}

	HRESULT FilterTemplates::EnumerateAllRegisteredFilters()
	{
		CComPtr<IFilterMapper2>		mapper;
		CComPtr<IEnumMoniker>		emoniker;
		HRESULT						hr;

		filters.RemoveAll();

		hr = mapper.CoCreateInstance(CLSID_FilterMapper2);
		if (FAILED(hr)) return hr;

		// find all matching filters
		hr = mapper->EnumMatchingFilters(&emoniker, 0, FALSE,
					0,								// Passing zero lists more filters than MERIT_DO_NOT_USE
					FALSE, 0, NULL,	NULL, NULL,
					FALSE,
					FALSE, 0, NULL, NULL, NULL);
		if (SUCCEEDED(hr)) {
			hr = AddFilters(emoniker) < 0 ? E_FAIL : S_OK;
		}

		SortByName();
		return hr;
	}

    int FilterTemplates::EnumerateInternalFilters()
    {
        filters.RemoveAll();

        FilterTemplate filter;
        filter.name = _T("Dump Filter");
        filter.clsid = __uuidof(DumpFilter);
        filters.Add(filter);

        filter.name = _T("Time Measure Filter");
        filter.clsid = __uuidof(TimeMeasureFilter);
        filters.Add(filter);

        filter.name = _T("DXVA Null Renderer");
        filter.clsid = __uuidof(DxvaNullRenderer);
        filters.Add(filter);

        filter.name = _T("Fake M2TS Device Filter");
        filter.clsid = CLSID_FakeM2tsDevice;
        filters.Add(filter);

        filter.name = _T("Psi Config Filter");
        filter.clsid = __uuidof(PsiConfigFilter);
        filters.Add(filter);

        filter.name = _T("Analyzer Filter");
        filter.clsid = __uuidof(AnalyzerFilter);
        filters.Add(filter);

        filter.name = _T("Analyzer Writer Filter");
        filter.clsid = __uuidof(AnalyzerWriterFilter);
        filters.Add(filter);

#ifdef DEBUG
        filter.name = _T("H264 Analyzer");
        filter.clsid = __uuidof(H264AnalyzerFilter);
        filters.Add(filter);

        filter.name = _T("Video Analyzer");
        filter.clsid = __uuidof(VideoAnalyzerFilter);
        filters.Add(filter);

        filter.name = _T("Audio Analyzer");
        filter.clsid = __uuidof(AudioAnalyzerFilter);
        filters.Add(filter);
#endif

        return (int) filters.GetCount();
    }

    int FilterTemplates::EnumerateAudioDecoder()
    {
        filters.RemoveAll();

        DSUtil::FilterTemplates			all_filters;
	    int								i, j, k;

	    all_filters.Enumerate(CLSID_LegacyAmFilterCategory);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    DSUtil::FilterTemplate	&filter = all_filters.filters[i];

		    // now check the pins
		    bool	audio_in = false;
		    bool	audio_out = false;

		    for (j=0; j<filter.input_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.input_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Audio &&
					    !DSUtil::IsAudioUncompressed(pin.minor[k])
					    ) {
					    // at least one compressed input
					    audio_in = true;
				    }
				    if (audio_in) break;
			    }	
			    if (audio_in) break;
		    }

		    for (j=0; j<filter.output_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.output_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Audio &&
					    DSUtil::IsAudioUncompressed(pin.minor[k])
					    ) {
					    // at least one uncompressed output
					    audio_out = true;
				    }
				    if (audio_out) break;
			    }	
			    if (audio_out) break;
		    }

		    // if it has audio in and out, we can take it for a audio decoder
		    if (audio_in && audio_out) {
			    filters.Add(filter);
		    }
	    }

	    // we also display DMOs
	    all_filters.filters.RemoveAll();
        all_filters.EnumerateDMO(DMOCATEGORY_AUDIO_DECODER);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    filters.Add(all_filters.filters[i]);
	    }

        return (int) filters.GetCount();
    }

    int FilterTemplates::EnumerateVideoDecoder()
    {
        filters.RemoveAll();

        DSUtil::FilterTemplates			all_filters;
	    int								i, j, k;

	    all_filters.Enumerate(CLSID_LegacyAmFilterCategory);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    DSUtil::FilterTemplate	&filter = all_filters.filters[i];

		    // now check the pins
		    bool	video_in = false;
		    bool	video_out = false;

		    for (j=0; j<filter.input_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.input_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Video &&
					    !DSUtil::IsVideoUncompressed(pin.minor[k])
					    ) {
					    // at least one compressed input
					    video_in = true;
				    }
				    if (video_in) break;
			    }	
			    if (video_in) break;
		    }

		    for (j=0; j<filter.output_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.output_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Video &&
					    DSUtil::IsVideoUncompressed(pin.minor[k])
					    ) {
					    // at least one uncompressed output
					    video_out = true;
				    }
				    if (video_out) break;
			    }	
			    if (video_out) break;
		    }

		    // if it has video in and out, we can take it for a video encoder
		    if (video_in && video_out) {
			    filters.Add(filter);
		    }
	    }

	    // we also display DMOs
	    all_filters.filters.RemoveAll();
        all_filters.EnumerateDMO(DMOCATEGORY_VIDEO_DECODER);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    filters.Add(all_filters.filters[i]);
	    }

        return (int) filters.GetCount();
    }

    int FilterTemplates::EnumerateAudioEncoder()
    {
        filters.RemoveAll();

        DSUtil::FilterTemplates			all_filters;
	    int								i, j, k;

	    all_filters.Enumerate(CLSID_LegacyAmFilterCategory);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    DSUtil::FilterTemplate	&filter = all_filters.filters[i];

		    // now check the pins
		    bool	audio_in = false;
		    bool	audio_out = false;

		    for (j=0; j<filter.input_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.input_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Audio &&
					    DSUtil::IsAudioUncompressed(pin.minor[k])
					    ) {
					    // at least one compressed input
					    audio_in = true;
				    }
				    if (audio_in) break;
			    }	
			    if (audio_in) break;
		    }

		    for (j=0; j<filter.output_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.output_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Audio &&
					    !DSUtil::IsAudioUncompressed(pin.minor[k])
					    ) {
					    // at least one uncompressed output
					    audio_out = true;
				    }
				    if (audio_out) break;
			    }	
			    if (audio_out) break;
		    }

		    // if it has audio in and out, we can take it for a audio encoder
		    if (audio_in && audio_out) {
			    filters.Add(filter);
		    }
	    }

        // search in the right categorie
        all_filters.filters.RemoveAll();
        all_filters.Enumerate(CLSID_AudioCompressorCategory);
        for (i=0; i<all_filters.filters.GetCount(); i++) {
		    filters.Add(all_filters.filters[i]);
	    }

	    // we also display DMOs
	    all_filters.filters.RemoveAll();
        all_filters.EnumerateDMO(DMOCATEGORY_AUDIO_ENCODER);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    filters.Add(all_filters.filters[i]);
	    }

        return (int) filters.GetCount();
    }

    int FilterTemplates::EnumerateVideoEncoder()
    {
        filters.RemoveAll();

        DSUtil::FilterTemplates			all_filters;
	    int								i, j, k;

	    all_filters.Enumerate(CLSID_LegacyAmFilterCategory);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    DSUtil::FilterTemplate	&filter = all_filters.filters[i];

		    // now check the pins
		    bool	video_in = false;
		    bool	video_out = false;

		    for (j=0; j<filter.input_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.input_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Video &&
					    !DSUtil::IsVideoUncompressed(pin.minor[k])
					    ) {
					    // at least one compressed input
					    video_in = true;
				    }
				    if (video_in) break;
			    }	
			    if (video_in) break;
		    }

		    for (j=0; j<filter.output_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.output_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Video &&
					    !DSUtil::IsVideoUncompressed(pin.minor[k])
					    ) {
					    // at least one uncompressed output
					    video_out = true;
				    }
				    if (video_out) break;
			    }	
			    if (video_out) break;
		    }

		    // if it has video in and out, we can take it for a video decoder
		    if (video_in && video_out) {
			    filters.Add(filter);
		    }
	    }

        // search in the right categorie
        all_filters.filters.RemoveAll();
        all_filters.Enumerate(CLSID_VideoCompressorCategory);
        for (i=0; i<all_filters.filters.GetCount(); i++) {
		    filters.Add(all_filters.filters[i]);
	    }

	    // we also display DMOs
	    all_filters.filters.RemoveAll();
        all_filters.EnumerateDMO(DMOCATEGORY_VIDEO_ENCODER);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    filters.Add(all_filters.filters[i]);
	    }

        return (int) filters.GetCount();
    }

    int FilterTemplates::EnumerateDemuxer()
    {
        filters.RemoveAll();

        DSUtil::FilterTemplates			all_filters;
	    int								i, j, k;

	    all_filters.Enumerate(CLSID_LegacyAmFilterCategory);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    DSUtil::FilterTemplate	&filter = all_filters.filters[i];

		    // now check the pins
		    bool	stream_in = false;

		    for (j=0; j<filter.input_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.input_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Stream) {
					    stream_in = true;
				    }
				    if (stream_in) break;
			    }	
			    if (stream_in) break;
		    }

		    // if it has stream out, we can take it for a muxer
		    if (stream_in) {
			    filters.Add(filter);
		    }
	    }

        return (int) filters.GetCount();
    }

    int FilterTemplates::EnumerateMuxer()
    {
        filters.RemoveAll();

        DSUtil::FilterTemplates			all_filters;
	    int								i, j, k;

	    all_filters.Enumerate(CLSID_LegacyAmFilterCategory);
	    for (i=0; i<all_filters.filters.GetCount(); i++) {
		    DSUtil::FilterTemplate	&filter = all_filters.filters[i];

		    // now check the pins
		    bool	stream_out = false;

		    for (j=0; j<filter.output_pins.GetCount(); j++) {
			    DSUtil::PinTemplate	&pin = filter.output_pins[j];

			    // check media types
			    for (k=0; k<pin.types; k++) {
				    if (pin.major[k] == MEDIATYPE_Stream) {
					    stream_out = true;
				    }
				    if (stream_out) break;
			    }	
			    if (stream_out) break;
		    }

		    // if it has stream out, we can take it for a muxer
		    if (stream_out) {
			    filters.Add(filter);
		    }
	    }

        return (int) filters.GetCount();
    }

	int FilterTemplates::EnumerateDMO(GUID clsid)
	{

		IEnumDMO		*enum_dmo = NULL;
		ULONG			f;
		HRESULT			hr;

		// create the enum object. Need to use DMO_ENUMF_INCLUDE_KEYED to include Audio Resampler DMO
		hr = DMOEnum(clsid, DMO_ENUMF_INCLUDE_KEYED, 0, NULL, 0, NULL, &enum_dmo);
		if (FAILED(hr) || !enum_dmo) return -1;

		CLSID			dmo_clsid;
		WCHAR			*name = NULL;

		enum_dmo->Reset();
		while (enum_dmo->Next(1, &dmo_clsid, &name, &f) == NOERROR) {

			if (dmo_clsid != GUID_NULL) {
				FilterTemplate		filter;

				// let's fill any information
				filter.name = CString(name);
				filter.clsid = dmo_clsid;
				filter.category = clsid;
				filter.type = FilterTemplate::FT_DMO;
				filter.moniker = NULL;

				LPOLESTR		str = NULL;
				CString			display_name;

				display_name = _T("@device:dmo:");
				StringFromCLSID(dmo_clsid, &str);	
				if (str) {	display_name += CString(str);	CoTaskMemFree(str);	str = NULL;	}
				StringFromCLSID(clsid, &str);	
				if (str) {	display_name += CString(str);	CoTaskMemFree(str);	str = NULL;	}
				filter.moniker_name = display_name;
				filter.version = 2;
				filter.FindFilename();

				// find out merit

				// HKEY_CLASSES_ROOT\CLSID\{07C9CB2C-F51C-47EA-B551-7DA02541D586}
				StringFromCLSID(dmo_clsid, &str);
				CString		str_clsid(str);
				CString		key_name;
				if (str) CoTaskMemFree(str);

				key_name.Format(_T("CLSID\\%s"), str_clsid);
				CRegKey		key;
				if (key.Open(HKEY_CLASSES_ROOT, key_name, KEY_READ) != ERROR_SUCCESS) { 
					filter.merit = 0x00600000 + 0x800;
				} else {

					DWORD	dwVal;
					if (key.QueryDWORDValue(_T("Merit"), dwVal) != ERROR_SUCCESS) {
						filter.merit = 0x00600000 + 0x800;
					} else {
						filter.merit = dwVal;
					}
					key.Close();
				}

				filters.Add(filter);
			}

			// release any memory held for the name
			if (name) {
				CoTaskMemFree(name);
				name = NULL;
			}
		}

		enum_dmo->Release();
		return 0;
	}

	int FilterTemplates::EnumerateCompatible(MediaTypes &mtypes, DWORD min_merit, bool need_output, bool exact)
	{
		if (mtypes.GetCount() <= 0) return -1;

		// our objects
		IFilterMapper2		*mapper       = NULL;
		IEnumMoniker		*enum_moniker = NULL;
		GUID				*inlist       = NULL;
		HRESULT				hr;
		int					ret = -1;

		do {
			// create filter mapper object
			hr = CoCreateInstance(CLSID_FilterMapper, NULL, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void**)&mapper);
			if (FAILED(hr)) break;

			// prepare the media type list
			SSIZE_T cnt = mtypes.GetCount();
			inlist = (GUID*)malloc(cnt * 2 * sizeof(GUID));
			if (!inlist) break;

			for (int i=0; i<cnt; i++) {
				inlist[2*i + 0] = mtypes[i].majortype;
				inlist[2*i + 1] = mtypes[i].subtype;
			}

			// search for the matching filters
			hr = mapper->EnumMatchingFilters(&enum_moniker, 0, exact, min_merit,
											 TRUE, (DWORD) cnt, inlist, NULL, NULL,
											 FALSE,
											 need_output, 0, NULL, NULL, NULL
											 );
			if (FAILED(hr)) break;

			// add them to the list
			ret = AddFilters(enum_moniker);

			// finally we kick "ACM Wrapper" and "AVI Decompressor"
			for (SSIZE_T j=filters.GetCount()-1; j >= 0; j--) {
				if (filters[j].name == _T("ACM Wrapper") ||
					filters[j].name == _T("AVI Decompressor")
					) {
					filters.RemoveAt(j);
				}
			}

			// and make sure "Video Renderer", "VMR-7" and "VMR-9" are
			// named properly
			for (int k=0; k<filters.GetCount(); k++) {
				FilterTemplate	&filt = filters[k];

				if (filt.clsid == CLSID_VideoRendererDefault) {
					filt.name = _T("Default Video Renderer");
				} else
				if (filt.clsid == CLSID_VideoMixingRenderer) {
					filt.name = _T("Video Mixing Renderer 7");
				} else
				if (filt.clsid == CLSID_VideoMixingRenderer9) {
					filt.name = _T("Video Mixing Renderer 9");
				}
			}

		} while (0);

		// get rid of the objects
		if (mapper) mapper->Release();
		if (enum_moniker) enum_moniker->Release();

		if (inlist) {
			free(inlist);
		}

		return ret;
	}

	int FilterTemplates::Enumerate(GUID clsid)
	{

		// ideme nato
		ICreateDevEnum		*sys_dev_enum = NULL;
		IEnumMoniker		*enum_moniker = NULL;
		HRESULT				hr;
		int					ret = -1;

		do {
			hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&sys_dev_enum);
			if (FAILED(hr)) break;

			// ideme enumerovat filtre
			hr = sys_dev_enum->CreateClassEnumerator(clsid, &enum_moniker, 0);
			if (hr != NOERROR) break;
		
			ret = AddFilters(enum_moniker, 0, clsid);
		} while (0);

		if (enum_moniker) enum_moniker->Release();
		if (sys_dev_enum) sys_dev_enum->Release();

		// let's append DMO filters for this category
		EnumerateDMO(clsid);

		return ret;
	}

	int FilterTemplates::AddFilters(IEnumMoniker *emoniker, int enumtype, GUID category)
	{
		IMoniker			*moniker = NULL;
		ULONG						f;
		HRESULT						hr;

		emoniker->Reset();
		while (emoniker->Next(1, &moniker, &f) == NOERROR) {

			FilterTemplate		filter;
			hr = filter.ReadFromMoniker(moniker);
			if (SUCCEEDED(hr)) {
				filter.FindFilename();

				// mame novy filter
				filter.moniker = moniker;
				filter.moniker->AddRef();

				int	can_go = 0;
				switch (enumtype) {
				case 0:		can_go = 0; break;
				case 1:		can_go = IsVideoRenderer(filter); break;
				}

				if (can_go == 0) {
					LPOLESTR	moniker_name;
					hr = moniker->GetDisplayName(NULL, NULL, &moniker_name);
					if (SUCCEEDED(hr)) {
						filter.moniker_name = CString(moniker_name);

						IMalloc *alloc = NULL;
						hr = CoGetMalloc(1, &alloc);
						if (SUCCEEDED(hr)) {
							alloc->Free(moniker_name);
							alloc->Release();
						}
					} else {
						filter.moniker_name = _T("");
					}
					filter.category = category;
					filter.ParseMonikerName();
					filters.Add(filter);
				}
			}
			moniker->Release();
			moniker = NULL;
		}

		if (moniker) 
			moniker->Release();
		return 0;
	}

	int FilterTemplates::IsVideoRenderer(FilterTemplate &filter)
	{
		// manually accept these
		if (filter.clsid == CLSID_OverlayMixer) return 0;
		
		// manually reject these
		if (filter.name == _T("Windows Media Update Filter")) return -1;

		// video renderer must have no output pins
		if (filter.output_pins.GetCount() > 0) return -1;

		// video renderer must have MEDIATYPE_Video registered for input pin
		bool	okay = false;
		for (int i=0; i<filter.input_pins.GetCount(); i++) {
			PinTemplate &pin = filter.input_pins[i];

			for (int j=0; j<pin.types; j++) {
				if (pin.major[j] == MEDIATYPE_Video) {
					okay = true;
					break;
				}
			}

			if (okay) break;
		}
		if (!okay) return -1;

		// VMR-7 and old VR have the same name
		if (filter.clsid == CLSID_VideoRendererDefault) {
			filter.name = _T("Video Mixing Renderer 7");
		}

		return 0;
	}


	HRESULT DisplayPropertyPage(IBaseFilter *filter, HWND parent)
	{
		if (!filter) return E_POINTER;

		ISpecifyPropertyPages *pProp = NULL;
		HRESULT hr = filter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
		if (SUCCEEDED(hr)) {
			// Get the filter's name and IUnknown pointer.
			FILTER_INFO FilterInfo;
			hr = filter->QueryFilterInfo(&FilterInfo); 
			IUnknown *pFilterUnk;
			filter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

			// Show the page. 
			CAUUID caGUID;
			pProp->GetPages(&caGUID);
			pProp->Release();

			OleCreatePropertyFrame(
				parent,                 // Parent window
				0, 0,                   // Reserved
				FilterInfo.achName,     // Caption for the dialog box
				1,                      // Number of objects (just the filter)
				&pFilterUnk,            // Array of object pointers. 
				caGUID.cElems,          // Number of property pages
				caGUID.pElems,          // Array of property page CLSIDs
				0,                      // Locale identifier
				0, NULL                 // Reserved
			);

			// Clean up.
			pFilterUnk->Release();
			if (FilterInfo.pGraph) FilterInfo.pGraph->Release(); 
			CoTaskMemFree(caGUID.pElems);

			return NOERROR;
		}
		return E_FAIL;
	}

	HRESULT EnumMediaTypes(IPin *pin, MediaTypes &types)
	{
		types.RemoveAll();
		if (!pin) return E_POINTER;

		IEnumMediaTypes	*emt;
		HRESULT			hr;
		AM_MEDIA_TYPE	*pmt;
		ULONG			f;

		hr = pin->EnumMediaTypes(&emt);
		if (FAILED(hr)) return hr;

		emt->Reset();
		while (emt->Next(1, &pmt, &f) == NOERROR) {
			CMediaType		mt(*pmt);
			types.Add(mt);
			DeleteMediaType(pmt);
		}

		emt->Release();
		return NOERROR;
	}


	// enumeracie pinov
	HRESULT EnumPins(IBaseFilter *filter, PinArray &pins, int flags)
	{
		pins.RemoveAll();
		if (!filter) return NOERROR;

		IEnumPins	*epins;
		IPin		*pin;
		ULONG		f;
		HRESULT		hr;

		hr = filter->EnumPins(&epins);
		if (FAILED(hr)) return hr;

		epins->Reset();
		while (epins->Next(1, &pin, &f) == NOERROR) {
			PIN_DIRECTION	dir;
			PIN_INFO		info;
			Pin				npin;

			pin->QueryDirection(&dir);

			// pozreme, ci zodpoveda flagom
			if (dir == PINDIR_INPUT && (!(flags&Pin::PIN_FLAG_INPUT))) goto label_next;
			if (dir == PINDIR_OUTPUT && (!(flags&Pin::PIN_FLAG_OUTPUT))) goto label_next;

			IPin	*other_pin = NULL;
			bool	is_connected;
			pin->ConnectedTo(&other_pin);
			is_connected = (other_pin == NULL ? false : true);
			if (other_pin) other_pin->Release();

			// zodpoveda ?
			if (is_connected && (!(flags&Pin::PIN_FLAG_CONNECTED))) goto label_next;
			if (!is_connected && (!(flags&Pin::PIN_FLAG_NOT_CONNECTED))) goto label_next;

			// pin info
			pin->QueryPinInfo(&info);

			// naplnime info
			npin.name = CString(info.achName);
			npin.filter = info.pFilter; info.pFilter->AddRef();
			npin.pin = pin;	pin->AddRef();
			npin.dir = dir;
			pins.Add(npin);

			if (info.pFilter)
				info.pFilter->Release();

		label_next:
			pin->Release();
		}
		epins->Release();		
		return NOERROR;
	}

	// Currently only used for building decoder performance test graphs
	HRESULT ConnectFilters(IGraphBuilder *gb, IBaseFilter *output, IBaseFilter *input, bool direct)
	{
		PinArray		opins;
		PinArray		ipins;
		HRESULT			hr;

		EnumPins(output, opins, Pin::PIN_FLAG_OUTPUT | Pin::PIN_FLAG_NOT_CONNECTED);
		EnumPins(input, ipins, Pin::PIN_FLAG_INPUT | Pin::PIN_FLAG_NOT_CONNECTED);

		// a teraz skusame
		for (int i=0; i<opins.GetCount(); i++) {
			for (int j=0; j<ipins.GetCount(); j++) {

				if (direct) {
					hr = gb->ConnectDirect(opins[i].pin, ipins[j].pin, NULL);
				} else {
					hr = gb->Connect(opins[i].pin, ipins[j].pin);
				}
				if (SUCCEEDED(hr)) {
					opins.RemoveAll();
					ipins.RemoveAll();
					return NOERROR;
				}

				gb->Disconnect(opins[i].pin);
				gb->Disconnect(ipins[j].pin);
			}
		}

		opins.RemoveAll();
		ipins.RemoveAll();
		return hr;
	}

	// pin1 and pin2 can be in either order.
	// If direct is false, intelligent connect is used
	// If direct is true, direct connection is used and:
	//		if chooseMediaType is true, media types are enumerated from pin1 and offered to the user for selection. If the user cancels S_FALSE is returned and no connection is made
	//		if chooseMediaType is false, mediaType parameter is used for connection
	HRESULT ConnectPin(IGraphBuilder *gb, IPin *pin1, IPin *pin2, bool direct, bool chooseMediaType /* = false */, AM_MEDIA_TYPE* mediaType /* = NULL */ )
	{
		if (!gb || !pin1 || !pin2)
			return E_POINTER;

		HRESULT hr = S_FALSE;			// S_FALSE if user cancels media type selection and connection

		PIN_DIRECTION direction = PINDIR_OUTPUT;
		hr = pin1->QueryDirection(&direction);
		if (FAILED(hr))
			return hr;

		IPin *out_pin = pin1, *in_pin = pin2;
		if (PINDIR_INPUT == direction) {
			out_pin = pin2;
			in_pin = pin1;
		}

		if (!direct) {
			hr = gb->Connect(out_pin, in_pin);
		} else {

            if (!chooseMediaType) {
				hr = gb->ConnectDirect(out_pin, in_pin, NULL);
			} else {

				do {														// attempt connection until we succeed or user cancels dialog
					DSUtil::MediaTypes outputMediaTypes;					// enumerate media types every time in case modified below or by filter

					hr = DSUtil::EnumMediaTypes(pin1, outputMediaTypes);	// enum media types from first pin whether output or input
					if (FAILED(hr)) {
						DSUtil::ShowError(hr, _T("Error enumerating media types"));
						hr = S_OK;
					}

					CMediaTypeSelectForm dlg;
					dlg.SetMediaTypes(outputMediaTypes);

					if (IDOK != dlg.DoModal()) {		
						hr = S_FALSE;
						break;								// break out of loop if user cancels dialog
					}
					const int selectedIndex = dlg.SelectedMediaTypeIndex();
					if (selectedIndex >= 0 && selectedIndex < outputMediaTypes.GetSize()) {

						CMediaType &mt = outputMediaTypes[selectedIndex];
						if (!dlg.s_use_major_type)
							mt.majortype = GUID_NULL;
						if (!dlg.s_use_sub_type)
							mt.subtype = GUID_NULL;
						if (!dlg.s_use_sample_size)
							mt.lSampleSize = 0;
						if (!dlg.s_use_format_block) {
							mt.formattype = GUID_NULL;
							mt.ResetFormatBuffer();
						}

						mediaType = &mt;
					} else {
						mediaType = NULL;
					}
					hr = gb->ConnectDirect(out_pin, in_pin, mediaType);
					DSUtil::ShowError(hr, _T("Connecting Pins"));

				} while (FAILED(hr));
			}
		}
		return hr;
	}

	// pin is input/output pin and will only be connected to unconnected output/input pins
	// cf ConnectPin comments
	HRESULT ConnectPinToFilter(IGraphBuilder *gb, IPin *pin, IBaseFilter *filter, bool direct, bool chooseMediaType /* = false */, AM_MEDIA_TYPE* mediaType /* = NULL */ )
	{
		if (!gb) 
			return E_FAIL;

		PinArray		pins;
		HRESULT			hr = S_OK;

		PIN_DIRECTION pinDirection = PINDIR_INPUT;
		hr = pin->QueryDirection(&pinDirection);
		if (FAILED(hr))
			return hr;

		// enumerate pins of opposite direction on filter
		const int directionFlag = PINDIR_INPUT==pinDirection ? Pin::PIN_FLAG_OUTPUT : Pin::PIN_FLAG_INPUT;
		EnumPins(filter, pins, directionFlag | Pin::PIN_FLAG_NOT_CONNECTED);

		for (int j=0; j<pins.GetCount(); j++) {

			hr = ConnectPin(gb, pin, pins[j].pin, direct, chooseMediaType, mediaType);
			if (SUCCEEDED(hr)) {
				pins.RemoveAll();
				return NOERROR;
			}
			gb->Disconnect(pin);
			gb->Disconnect(pins[j].pin);
		}
		pins.RemoveAll();
		return FAILED(hr) ? hr : E_FAIL;
	}

	bool IsVideoUncompressed(GUID subtype)
	{
		if (subtype == MEDIASUBTYPE_RGB1 ||
			subtype == MEDIASUBTYPE_RGB16_D3D_DX7_RT ||
			subtype == MEDIASUBTYPE_RGB16_D3D_DX9_RT ||
			subtype == MEDIASUBTYPE_RGB24 ||
			subtype == MEDIASUBTYPE_RGB32 ||
			subtype == MEDIASUBTYPE_RGB32_D3D_DX7_RT ||
			subtype == MEDIASUBTYPE_RGB32_D3D_DX9_RT ||
			subtype == MEDIASUBTYPE_RGB4 ||
			subtype == MEDIASUBTYPE_RGB555 ||
			subtype == MEDIASUBTYPE_RGB565 ||
			subtype == MEDIASUBTYPE_RGB8 ||
			subtype == MEDIASUBTYPE_ARGB1555 ||
			subtype == MEDIASUBTYPE_ARGB1555_D3D_DX7_RT ||
			subtype == MEDIASUBTYPE_ARGB1555_D3D_DX9_RT ||
			subtype == MEDIASUBTYPE_ARGB32 ||
			subtype == MEDIASUBTYPE_ARGB32_D3D_DX7_RT ||
			subtype == MEDIASUBTYPE_ARGB32_D3D_DX9_RT ||
			subtype == MEDIASUBTYPE_ARGB4444 ||
			subtype == MEDIASUBTYPE_ARGB4444_D3D_DX7_RT ||
			subtype == MEDIASUBTYPE_ARGB4444_D3D_DX9_RT ||
			subtype == MEDIASUBTYPE_UYVY ||
			subtype == MEDIASUBTYPE_YUY2 ||
			subtype == MEDIASUBTYPE_YUYV ||
			subtype == MEDIASUBTYPE_NV12 ||
			subtype == MEDIASUBTYPE_YV12 ||
			subtype == MEDIASUBTYPE_Y211 ||
			subtype == MEDIASUBTYPE_Y411 ||
			subtype == MEDIASUBTYPE_Y41P ||
			subtype == MEDIASUBTYPE_YVU9 ||
			subtype == MEDIASUBTYPE_YVYU ||
			subtype == MEDIASUBTYPE_IYUV ||
			subtype == Monogram::MEDIASUBTYPE_I420 ||
			subtype == MEDIASUBTYPE_CLJR ||
			subtype == MEDIASUBTYPE_UYVY ||
			subtype == MEDIASUBTYPE_P010 ||
			subtype == MEDIASUBTYPE_P016 ||
			subtype == MEDIASUBTYPE_P208 ||
			subtype == MEDIASUBTYPE_P210 ||
			subtype == MEDIASUBTYPE_P216 ||
			subtype == MEDIASUBTYPE_Y210 ||
			subtype == MEDIASUBTYPE_Y216 ||
			subtype == MEDIASUBTYPE_P408 ||
			subtype == MEDIASUBTYPE_NV11 ||
			subtype == MEDIASUBTYPE_AI44 ||
			subtype == MEDIASUBTYPE_IA44 ||
			subtype == MEDIASUBTYPE_AYUV ||
			subtype == GUID_NULL
			) {
			return true;
		}

		return false;
	}

    bool IsAudioUncompressed(GUID subtype)
	{
		if (subtype == MEDIASUBTYPE_PCM ||
			subtype == MEDIASUBTYPE_IEEE_FLOAT ||
			subtype == GUID_NULL
			) {
			return true;
		}

		return false;
	}

	// get formatDetails (like '640x480' or '2 channels 44khz')
	CString FormatBlockSummary(const AM_MEDIA_TYPE& mediaType)
	{
		CString formatDetails;
        if (mediaType.pbFormat)
        {
            const BITMAPINFOHEADER* bmi = NULL;
			if(mediaType.formattype == FORMAT_VideoInfo && mediaType.cbFormat >= sizeof(VIDEOINFOHEADER))
                bmi = &((const VIDEOINFOHEADER*)mediaType.pbFormat)->bmiHeader;
            else if( (mediaType.formattype == FORMAT_VideoInfo2 || mediaType.formattype == FORMAT_MPEG2_VIDEO)
					&& mediaType.cbFormat >= sizeof(VIDEOINFOHEADER2) )
                bmi = &((const VIDEOINFOHEADER2*)mediaType.pbFormat)->bmiHeader;

            if(bmi != NULL) 
			{
				const int pixels = bmi->biWidth * bmi->biHeight;
				const float averageBPP = pixels ? (8.0f * bmi->biSizeImage) / pixels : 0;
				formatDetails.Format(_T("%4d x %4d, %3d bpp, %6.3f av"), 
							bmi->biWidth, bmi->biHeight, bmi->biBitCount, averageBPP);
			} 
			else if(mediaType.formattype == FORMAT_WaveFormatEx && mediaType.cbFormat >= sizeof(WAVEFORMATEX))
            {
                const WAVEFORMATEX* const wfx = (WAVEFORMATEX*)mediaType.pbFormat;
                formatDetails.Format(_T("%dx %dHz with %dBits"), wfx->nChannels, wfx->nSamplesPerSec, wfx->wBitsPerSample);
            }
        }
		return formatDetails;
	}

#define MAX_KEY_LEN  260

	int EliminateSubKey(HKEY hkey, LPCTSTR strSubKey)
	{
		HKEY hk;
		if (lstrlen(strSubKey) == 0) return -1;
  
		LONG lreturn = RegOpenKeyEx(hkey, strSubKey, 0, MAXIMUM_ALLOWED, &hk);
		if (lreturn == ERROR_SUCCESS) {
			while (true) {
				TCHAR Buffer[MAX_KEY_LEN];
				DWORD dw = MAX_KEY_LEN;
				FILETIME ft;

				lreturn = RegEnumKeyEx(hk, 0, Buffer, &dw, NULL, NULL, NULL, &ft);
				if (lreturn == ERROR_SUCCESS) {
					DSUtil::EliminateSubKey(hk, Buffer);
				} else {
					break;
				}
			}
			RegCloseKey(hk);
			RegDeleteKey(hkey, strSubKey);
		}
		return 0;
	}



	HRESULT UnregisterCOM(GUID clsid)
	{
		/*
			We remove the registry key
				HKEY_CLASSES_ROOT\CLSID\<clsid>
		*/
		
		OLECHAR szCLSID[CHARS_IN_GUID];
		StringFromGUID2(clsid, szCLSID, CHARS_IN_GUID);

		CString	keyname;
		keyname.Format(_T("CLSID\\%s"), szCLSID);

		// delete subkey
		int ret = DSUtil::EliminateSubKey(HKEY_CLASSES_ROOT, keyname.GetBuffer());
		if (ret < 0) return -1;

		return 0;
	}

	HRESULT UnregisterFilter(GUID clsid, GUID category)
	{
		/*
			Remove using the filter mapper object.
		*/

		CComPtr<IFilterMapper2>		mapper;
		HRESULT						hr;

		hr = mapper.CoCreateInstance(CLSID_FilterMapper2);
		if (FAILED(hr)) return E_FAIL;

		hr = mapper->UnregisterFilter(&category, NULL, clsid);

		// done with the mapper
		mapper = NULL;
		if (FAILED(hr)) return E_FAIL;

		return NOERROR;
	}

    bool ShowError(HRESULT hr, LPCTSTR title)
    {
		bool ok_pressed = true;

        if (FAILED(hr))
        {
            if (m_bExitOnError)
            {
                ASSERT(AfxGetMainWnd() != NULL);
                ((CgraphstudioApp*)AfxGetApp())->m_nExitCode = hr;
                AfxGetMainWnd()->SendMessage(WM_CLOSE);
            }

            TCHAR szErr[MAX_ERROR_TEXT_LEN];
            DWORD res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
            CString strHR;
            CString strError;
            GraphStudio::NameHResult(hr, strHR);
            if (res == 0)
            {
                _com_error error(hr);
                strError = error.ErrorMessage();
            }
            else
            {
                strError = szErr;
				strError += "\r\n";
				strError += title;
            }
                
            if (CTaskDialog::IsSupported())
            {
                CTaskDialog taskDialog(strError, strHR, title != NULL ? title : _T("Error"), TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON);
                taskDialog.SetMainIcon(TD_ERROR_ICON);
                taskDialog.LoadCommandControls(IDS_SEARCH_FOR_ERROR, IDS_SHOW_GRAPH_CONSTRUCTION_REPORT);
				taskDialog.SetDefaultCommandControl(TDCBF_OK_BUTTON);
                INT_PTR result = taskDialog.DoModal();

                if(result == IDS_SEARCH_FOR_ERROR)
                {
                    CString url;
                    url.Format(TEXT("http://www.google.com/search?q=%s"), strHR);
                    ShellExecute(NULL, _T("open"),url, NULL, NULL, SW_SHOWNORMAL);
                }
                else if(result == IDS_SHOW_GRAPH_CONSTRUCTION_REPORT)
                {
                    AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_VIEW_GRAPHCONSTRUCTIONREPORT);
				} else if (result == IDCANCEL) {
					ok_pressed = false;
				}
            }
            else
            {
                CString strMsg;
                strMsg.Format(_T("%s\n%s"), strHR, szErr);
                ok_pressed = ShowError(strMsg,title);
            }
        }
		return ok_pressed;
    }

    bool ShowError(LPCTSTR text, LPCTSTR title)
    {
		bool ok_pressed = true;

        if (m_bExitOnError)
        {
            ASSERT(AfxGetMainWnd() != NULL);
            ((CgraphstudioApp*)AfxGetApp())->m_nExitCode = -1;
            AfxGetMainWnd()->SendMessage(WM_CLOSE);
        }

        if(CTaskDialog::IsSupported())
        {
            CTaskDialog taskDialog(text, NULL, title != NULL ? title : _T("Error"), TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON);
            taskDialog.SetMainIcon(TD_ERROR_ICON);
            ok_pressed = IDCANCEL != taskDialog.DoModal();
        }
        else
			ok_pressed = IDCANCEL != MessageBox(0, text, title != NULL ? title : _T("Error"), MB_OK | MB_ICONERROR);

		return ok_pressed;
    }

    bool ShowInfo(LPCTSTR text, LPCTSTR title)
    {
        if(CTaskDialog::IsSupported())
        {
            CTaskDialog taskDialog(text, NULL, title != NULL ? title : _T("Info"), TDCBF_OK_BUTTON);
            taskDialog.SetMainIcon(TD_INFORMATION_ICON);
            return IDCANCEL != taskDialog.DoModal();
        }
        else
            return IDCANCEL != MessageBox(0, text, title != NULL ? title : _T("Info"), MB_OK | MB_ICONERROR);
    }

    bool ShowWarning(LPCTSTR text, LPCTSTR title)
    {
        if(CTaskDialog::IsSupported())
        {
            CTaskDialog taskDialog(text, NULL, title != NULL ? title : _T("Warning"), TDCBF_OK_BUTTON);
            taskDialog.SetMainIcon(TD_WARNING_ICON);
            return IDCANCEL != taskDialog.DoModal();
        }
        else
            return IDCANCEL != MessageBox(0, text, title != NULL ? title : _T("Warning"), MB_OK | MB_ICONWARNING);
    }

    bool InitSbeObject(CComQIPtr<IStreamBufferInitialize> pInit)
    {
        if(!pInit) return false;

        HKEY hkey = 0;
        long lRes = RegCreateKey(HKEY_CURRENT_USER, TEXT("Software\\MONOGRAM\\GraphStudioNext\\SbeSettings"), &hkey);

        HRESULT hr = pInit->SetHKEY(hkey);
        if(FAILED(hr))
        {
            ShowError(hr, _T("Can't init sbe object."));
            return false;
        }

        return true;
    }

    bool IsOsWin7OrLater()
    {
        DWORD dwMajor = LOBYTE(LOWORD(GetVersion()));
        DWORD dwMinor = HIBYTE(LOWORD(GetVersion()));

        return dwMajor >= 6 && dwMinor >= 1;
    }

    bool IsOsWinVistaOrLater()
    {
        DWORD dwMajor = LOBYTE(LOWORD(GetVersion()));
        DWORD dwMinor = HIBYTE(LOWORD(GetVersion()));

        return dwMajor >= 6;
    }

    // Thanks to http://www.codeproject.com/Tips/676464/How-to-Parse-Empty-Tokens-using-CString-Tokenize
    void Tokenizer(const CString& strFields, const CString& strDelimiters, CStringArray& arFields)
    {
        arFields.RemoveAll();
  
        // Do not process empty strings.
        if (!strFields.IsEmpty() && !strDelimiters.IsEmpty())
        {
            int nPosition = 0, nTotalFields = 0;
  
            do
            {
                int nOldPosition = nPosition;   // Store the previous position value.
  
                CString strField = strFields.Tokenize(strDelimiters, nPosition);
                if (nPosition != -1)
                {
                    nTotalFields += (nPosition - nOldPosition - strField.GetLength());
                }
                else
                {
                    nTotalFields += (strFields.GetLength() + 1 - nOldPosition);
                }
  
                // By using SetAtGrow(), empty strings are automatically added to the array.
                arFields.SetAtGrow(nTotalFields - 1, strField);
             } while (nPosition != -1 && nPosition <= strFields.GetLength());
        }
    }

    bool SetClipboardText(HWND hwnd, CString& text)
    {
        if (!IsWindow(hwnd)) return false;
        if (!OpenClipboard(hwnd)) return false;

	    EmptyClipboard();
	
	    HGLOBAL		hClipboardData  = GlobalAlloc(GMEM_DDESHARE, sizeof(TCHAR) * (text.GetLength() + 1));
	    TCHAR		*buf			= (TCHAR*)GlobalLock(hClipboardData);

	    memset(buf, 0, sizeof(TCHAR)*(text.GetLength() + 1));
	    memcpy(buf, text.GetBuffer(), sizeof(TCHAR)*(text.GetLength()));
	
	    GlobalUnlock(hClipboardData);
	    SetClipboardData(CF_UNICODETEXT, hClipboardData);
	    CloseClipboard();

        return true;
    }
};



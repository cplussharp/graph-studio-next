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
		name(_T("")),
		moniker_name(_T("")),
		type(FilterTemplate::FT_FILTER),
		file(_T("")),
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
		type(ft.type),
		file(ft.file),
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
		file = ft.file;
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

	int FilterTemplate::Load(char *buf, int size)
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

				if ((maj_offset + 16 <= size) && (min_offset + 16 <= size)) {
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
			pin.types = pin.major.GetCount();

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

		return 0;
	}

	int FilterTemplate::LoadFromMoniker(CString displayname)
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

		if (FAILED(CreateBindCtx(0, &bind))) return -1;
		hr = MkParseDisplayName(bind, displayname, &eaten, &loc_moniker);
		bind = NULL;
		if (hr != NOERROR) { return -1; }

		// get a property bagy object
		hr = loc_moniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&propbag);
		if (SUCCEEDED(hr)) {
			VARIANT				var;

			VariantInit(&var);
			hr = propbag->Read(L"FriendlyName", &var, 0);
			if (SUCCEEDED(hr)) {
				name = CString(var.bstrVal);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr = propbag->Read(L"FilterData", &var, 0);
			if (SUCCEEDED(hr)) {
				SAFEARRAY	*ar = var.parray;
				int	size = ar->rgsabound[0].cElements;

				// load merit and version
				Load((char*)ar->pvData, size);
			}
			VariantClear(&var);

			VariantInit(&var);
			hr = propbag->Read(L"CLSID", &var, 0);
			if (SUCCEEDED(hr)) {
				if (SUCCEEDED(CLSIDFromString(var.bstrVal, &clsid))) {
					FindFilename();
					moniker_name = displayname;
					ParseMonikerName();
				}
			}
			VariantClear(&var);
		}

		propbag = NULL;
		loc_moniker = NULL;		
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
		categories.RemoveAll();
		Enumerate();
	}

	FilterCategories::~FilterCategories()
	{
		categories.RemoveAll();
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
		categories.Add(FilterCategory(_T("DMO Audio Decoder"), DMOCATEGORY_AUDIO_DECODER, true));
		categories.Add(FilterCategory(_T("DMO Audio Effect"), DMOCATEGORY_AUDIO_EFFECT, true));
		categories.Add(FilterCategory(_T("DMO Video Decoder"), DMOCATEGORY_VIDEO_DECODER, true));
		categories.Add(FilterCategory(_T("DMO Video Effect"), DMOCATEGORY_VIDEO_EFFECT, true));
		categories.Add(FilterCategory(_T("DMO Audio Capture Effect"), DMOCATEGORY_AUDIO_CAPTURE_EFFECT, true));
		
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

	void FilterTemplates::SwapItems(int i, int j)
	{	
		if (i == j) return ;
		FilterTemplate	temp = filters[i];
		filters[i] = filters[j];
		filters[j] = temp;
	}

	void FilterTemplates::_Sort_(int lo, int hi)
	{
		int i = lo, j = hi;
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

	int FilterTemplates::Enumerate(FilterCategory &cat)
	{
		filters.RemoveAll();

		if (cat.is_dmo) {
			return EnumerateDMO(cat.clsid);
		} else {
			return Enumerate(cat.clsid);
		}
	}

	int FilterTemplates::Find(CString name, FilterTemplate *filter)
	{
		if (!filter) return -1;
		for (int i=0; i<filters.GetCount(); i++) {
			if (name == filters[i].name) {
				*filter = filters[i];
				return 0;
			}
		}

		// nemame nic
		return -1;
	}

	int FilterTemplates::Find(GUID clsid, FilterTemplate *filter)
	{
		if (!filter) return -1;
		for (int i=0; i<filters.GetCount(); i++) {
			if (clsid == filters[i].clsid) {
				*filter = filters[i];
				return 0;
			}
		}

		// nemame nic
		return -1;
	}

	// vytvaranie
	HRESULT FilterTemplates::CreateInstance(CString name, IBaseFilter **filter)
	{
		FilterTemplate	ft;
		if (Find(name, &ft) >= 0) {
			return ft.CreateInstance(filter);
		}
		return E_FAIL;
	}

	HRESULT FilterTemplates::CreateInstance(GUID clsid, IBaseFilter **filter)
	{
		FilterTemplate	ft;
		if (Find(clsid, &ft) >= 0) {
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

	int FilterTemplates::EnumerateDMO(GUID clsid)
	{

		IEnumDMO		*enum_dmo = NULL;
		ULONG			f;
		HRESULT			hr;

		// create the enum object
		hr = DMOEnum(clsid, 0, 0, NULL, 0, NULL, &enum_dmo);
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

				LPOLESTR		str;
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
			int	cnt = mtypes.GetCount();
			inlist = (GUID*)malloc(cnt * 2 * sizeof(GUID));
			if (!inlist) break;

			for (int i=0; i<cnt; i++) {
				inlist[2*i + 0] = mtypes[i].majortype;
				inlist[2*i + 1] = mtypes[i].subtype;
			}

			// search for the matching filters
			hr = mapper->EnumMatchingFilters(&enum_moniker, 0, exact, min_merit,
											 TRUE, cnt, inlist, NULL, NULL,
											 FALSE,
											 need_output, 0, NULL, NULL, NULL
											 );
			if (FAILED(hr)) break;

			// add them to the list
			ret = AddFilters(enum_moniker);

			// finally we kick "ACM Wrapper" and "AVI Decompressor"
			for (int j=filters.GetCount()-1; j >= 0; j--) {
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
		IPropertyBag		*propbag = NULL;
		ULONG				f;
		HRESULT				hr;

		emoniker->Reset();
		while (emoniker->Next(1, &moniker, &f) == NOERROR) {

			hr = moniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&propbag);
			if (SUCCEEDED(hr)) {
				VARIANT				var;
				FilterTemplate		filter;

				VariantInit(&var);
				hr = propbag->Read(L"FriendlyName", &var, 0);
				if (SUCCEEDED(hr)) {
					filter.name = CString(var.bstrVal);
				}
				VariantClear(&var);

				VariantInit(&var);
				hr = propbag->Read(L"FilterData", &var, 0);
				if (SUCCEEDED(hr)) {
					SAFEARRAY	*ar = var.parray;
					int	size = ar->rgsabound[0].cElements;

					// load merit and version
					filter.Load((char*)ar->pvData, size);
				}
				VariantClear(&var);

				VariantInit(&var);
				hr = propbag->Read(L"CLSID", &var, 0);
				if (SUCCEEDED(hr)) {
					if (SUCCEEDED(CLSIDFromString(var.bstrVal, &filter.clsid))) {
						// mame novy filter
						filter.moniker = moniker;
						filter.moniker->AddRef();
						filter.FindFilename();

						int	can_go = 0;
						switch (enumtype) {
						case 0:		can_go = 0; break;
						case 1:		can_go = IsVideoRenderer(filter); break;
						}

						// moniker name
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
						filter.ParseMonikerName();

						filter.category = category;

						if (can_go == 0) filters.Add(filter);
					}
				}
				VariantClear(&var);

				propbag->Release();
				propbag = NULL;
			}
			moniker->Release();
			moniker = NULL;
		}

		if (propbag) propbag->Release();
		if (moniker) moniker->Release();
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

			info.pFilter->Release();

		label_next:
			pin->Release();
		}
		epins->Release();		
		return NOERROR;
	}

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
		return E_FAIL;
	}

	HRESULT ConnectPin(IGraphBuilder *gb, IPin *output, IBaseFilter *input, bool direct)
	{
		if (!gb) return E_FAIL;

		PinArray		ipins;
		HRESULT			hr;

		EnumPins(input, ipins, Pin::PIN_FLAG_INPUT | Pin::PIN_FLAG_NOT_CONNECTED);
		for (int j=0; j<ipins.GetCount(); j++) {

			if (direct) {
				hr = gb->ConnectDirect(output, ipins[j].pin, NULL);
			} else {
				hr = gb->Connect(output, ipins[j].pin);
			}
			if (SUCCEEDED(hr)) {
				ipins.RemoveAll();
				return NOERROR;
			}

			gb->Disconnect(output);
			gb->Disconnect(ipins[j].pin);
		}
		ipins.RemoveAll();
		return E_FAIL;
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
			subtype == GUID_NULL
			) {
			return true;
		}

		return false;
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


};



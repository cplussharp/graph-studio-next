//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "time_utils.h"

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	//-------------------------------------------------------------------------
	//
	//	Details
	//
	//-------------------------------------------------------------------------

	int GetFileDetails(CString filename, PropItem *filedetails)
	{
		// we load file details
		CFileFind	find;
		BOOL		found;

		found = find.FindFile(filename);
		if (!found) return -1;
		find.FindNextFile();
		
		CString		file_path;
		CString		file_name;

		file_name = find.GetFileName();
		file_path = find.GetFilePath();
		
		CPath path(file_path);
		int p = path.FindFileName();
		if (p >= 0) file_path = file_path.Left(p);

		filedetails->AddItem(new PropItem(_T("File Name"), file_name));
		filedetails->AddItem(new PropItem(_T("File Path"), file_path));

		// creation and modification date
		CTime	time_cr, time_mod;
		find.GetCreationTime(time_cr);
		find.GetLastWriteTime(time_mod);

		filedetails->AddItem(new PropItem(_T("Created"), time_cr));
		filedetails->AddItem(new PropItem(_T("Modified"), time_mod));

		__int64 size = find.GetLength();
		CString	fsize;
		fsize.Format(_T("%I64d"), size);
		filedetails->AddItem(new PropItem(_T("File Size"), fsize));

		PropItem	*vi = new PropItem(_T("Version Info"));
		if (GetVersionInfo(filename, vi) < 0) {
			delete vi;
		} else {
			filedetails->AddItem(vi);
		}

		return 0;
	}

	int GetVersionInfo(CString filename, PropItem *info)
	{
		DWORD		infosize = 0;
		DWORD		handle;

		infosize = GetFileVersionInfoSize(filename, &handle);
		if (infosize == 0) return -1;
	
		LPVOID		vi;
		vi = malloc(infosize);
		GetFileVersionInfo(filename, 0, infosize, vi);

		LPVOID		val;
		UINT		vallen;

		if (VerQueryValue((LPCVOID)vi, L"\\", &val, &vallen)) {

			CString				v;
			VS_FIXEDFILEINFO	*fi  = (VS_FIXEDFILEINFO*)val;

			int		version[4];
			version[0] = (fi->dwFileVersionMS >> 16) & 0xffff;
			version[1] = (fi->dwFileVersionMS >>  0) & 0xffff;
			version[2] = (fi->dwFileVersionLS >> 16) & 0xffff;
			version[3] = (fi->dwFileVersionLS >>  0) & 0xffff;

			v.Format(_T("%d.%d.%d.%d"), version[0], version[1], version[2], version[3]);
			info->AddItem(new PropItem(_T("File Version"), v));

			version[0] = (fi->dwProductVersionMS >> 16) & 0xffff;
			version[1] = (fi->dwProductVersionMS >>  0) & 0xffff;
			version[2] = (fi->dwProductVersionLS >> 16) & 0xffff;
			version[3] = (fi->dwProductVersionLS >>  0) & 0xffff;

			v.Format(_T("%d.%d.%d.%d"), version[0], version[1], version[2], version[3]);
			info->AddItem(new PropItem(_T("Product Version"), v));

			// todo:
			CString name;
			GetObjectName(CLSID_VideoRenderer, name);

		}
		
		free(vi);
		return 0;
	}

	int GetObjectFile(GUID clsid, CString &filename)
	{
		// we find this in the registry
		CRegKey		reg;
		int			ret = -1;

		CString		clsid_str;
		CLSIDToString(clsid, clsid_str);

		clsid_str = _T("\\CLSID\\") + clsid_str + _T("\\InprocServer32");
		if (reg.Open(HKEY_CLASSES_ROOT, clsid_str, KEY_READ) == ERROR_SUCCESS) {

			TCHAR		val[1024];
			ULONG		len = 1024;
			reg.QueryStringValue(_T(""), val, &len);
			filename = val;

			ret = 0;
			reg.Close();
		}

		return ret;
	}

	int GetObjectName(GUID clsid, CString &name)
	{
		// we find this in the registry
		CRegKey		reg;
		int			ret = -1;

		CString		clsid_str;
		CLSIDToString(clsid, clsid_str);

		clsid_str = _T("\\CLSID\\") + clsid_str;
		if (reg.Open(HKEY_CLASSES_ROOT, clsid_str, KEY_READ) == ERROR_SUCCESS) {

			TCHAR		val[1024];
			ULONG		len = 1024;
			reg.QueryStringValue(_T(""), val, &len);
			name = val;

			ret = 0;
			reg.Close();
		}

		return ret;
	}

	// Only usable for filters, not DMOs
	int GetFilterDetails(GraphStudio::Filter& filter, PropItem *info)
	{
		info->AddItem(new PropItem(_T("CLSID"), filter.clsid));

		CString name = _T("Unknown");
		GetObjectName(filter.clsid, name);
		info->AddItem(new PropItem(_T("Registry Name"), name));

		// get Dll file
		CString		filename = filter.GetDllFileName();
		if (filename.IsEmpty())
			GetObjectFile(filter.clsid, filename);

		// file details
		PropItem	* const fileinfo = new PropItem(_T("File"));
		if (GetFileDetails(filename, fileinfo) < 0) {
			delete fileinfo;
		} else {
			info->AddItem(fileinfo);
		}

		return 0;
	}

	int GetFilterDetails(const CLSID& clsid, PropItem *info)
	{
		info->AddItem(new PropItem(_T("CLSID"), clsid));

		CString name = _T("Unknown");
		GetObjectName(clsid, name);
		info->AddItem(new PropItem(_T("Object Name"), name));

		// file details
		CString filename;
		GetObjectFile(clsid, filename);
		PropItem	* const fileinfo = new PropItem(_T("File"));
		if (GetFileDetails(filename, fileinfo) < 0) {
			delete fileinfo;
		} else {
			info->AddItem(fileinfo);
		}

		return 0;
	}

	// returns number of categories of information (zero for none)
	int GetFilterInformationFromCLSID(const DSUtil::FilterCategories & categories, const GUID & clsid, PropItem * info)
	{
		int category_index = 0;

		// Check every category for information
		for (int c=0; c<categories.categories.GetCount(); c++) {
			const DSUtil::FilterCategory	&cat = categories.categories[c];

			// load binary data
			CString		filter_clsid_str;
			CLSIDToString(clsid, filter_clsid_str);
			CString cat_clsid_str;
			CLSIDToString(cat.clsid, cat_clsid_str);
			const CString reg_key_name = _T("\\CLSID\\") + cat_clsid_str + _T("\\Instance\\") + filter_clsid_str;

			CRegKey		reg;
			if (reg.Open(HKEY_CLASSES_ROOT, reg_key_name, KEY_READ) == ERROR_SUCCESS) {

				// get binary data
				ULONG		size = 0;
				if (reg.QueryBinaryValue(_T("FilterData"), NULL, &size) == ERROR_SUCCESS) {
					BYTE * const buf = new byte[size+1];
					reg.QueryBinaryValue(_T("FilterData"), buf, &size);

					// parse data
					DSUtil::FilterTemplate filter_template;
					const int ret = filter_template.LoadFilterData((char*)buf, size);
					delete[] buf;

					if (0 == ret) {
						CString		name;
						name.Format(_T("Category %d"), category_index++);
						PropItem * const category_info = info->AddItem(new PropItem(name));

						CString category_guid_str;
						GraphStudio::NameGuid(cat.clsid, category_guid_str, false);

						category_info->AddItem(new GraphStudio::PropItem(_T("Category Name"), cat.name));
						category_info->AddItem(new GraphStudio::PropItem(_T("Category GUID"), category_guid_str));

						GetFilterInformationFromTemplate(filter_template, category_info);
					}
				}
			}
		}
		return category_index;
	}

	int GetFilterInformationFromTemplate(const DSUtil::FilterTemplate & filter_template, PropItem *info)
	{
		CString		val;
		val.Format(_T("0x%08x"), filter_template.merit);
		info->AddItem(new PropItem(_T("Merit"), val));
		val.Format(_T("0x%08x"), filter_template.version);
		info->AddItem(new PropItem(_T("Version"), val));

		// registered pin details

		const SSIZE_T pin_count = filter_template.input_pins.GetCount() + filter_template.output_pins.GetCount();
		if (pin_count > 0) {
			PropItem * const pin_details = info->AddItem(new PropItem(_T("Registered Pins")));
			pin_details->AddItem(new PropItem(_T("Count"), pin_count));

			for (int i=0; i<filter_template.input_pins.GetCount(); i++) {
				const DSUtil::PinTemplate	&pin = filter_template.input_pins[i];
				CString		name;
				name.Format(_T("Input Pin %d"), i);
				PropItem * const pininfo = pin_details->AddItem(new PropItem(name));
				GetPinTemplateDetails(&pin, pininfo);
			}
			for (int i=0; i<filter_template.output_pins.GetCount(); i++) {
				const DSUtil::PinTemplate	&pin = filter_template.output_pins[i];
				CString		name;
				name.Format(_T("Output Pin %d"), i);
				PropItem	* const pininfo = pin_details->AddItem(new PropItem(name));
				GetPinTemplateDetails(&pin, pininfo);
			}
		}
		return 0;
	}

	int GetPinTemplateDetails(const DSUtil::PinTemplate *pin, PropItem *info)
	{
		if (pin->dir == PINDIR_INPUT) {
			info->AddItem(new PropItem(_T("Direction"), CString(_T("PINDIR_INPUT"))));
		} else {
			info->AddItem(new PropItem(_T("Direction"), CString(_T("PINDIR_OUTPUT"))));
		}		
		info->AddItem(new PropItem(_T("Rendered"), pin->rendered == TRUE));
		info->AddItem(new PropItem(_T("Many"), pin->many == TRUE));
		info->AddItem(new PropItem(_T("Media Types"), (int)pin->types));

		if (pin->types > 0) {
			PropItem	*types = info->AddItem(new PropItem(_T("Media Types")));
			for (int i=0; i<pin->types; i++) {
				CString		mn, mv, sv;
				mn.Format(_T("Media Type %d"), i);

				GraphStudio::NameGuid(pin->major[i],mv,CgraphstudioApp::g_showGuidsOfKnownTypes);
				GraphStudio::NameGuid(pin->minor[i],sv,CgraphstudioApp::g_showGuidsOfKnownTypes);
				types->AddItem(new PropItem(mn, mv + _T(", ") + sv));
			}
		}
		return 0;
	}

	int GetPinDetails(IPin *pin, PropItem *info)
	{
		PropItem	*group;

		group = info->AddItem(new PropItem(_T("Pin")));
		
		PIN_INFO	pi = {};
		HRESULT		hr;
		pin->QueryPinInfo(&pi);
		if (pi.pFilter) 
			pi.pFilter->Release();

		group->AddItem(new PropItem(_T("Name"), CString(pi.achName)));
		if (pi.dir == PINDIR_INPUT) {
			group->AddItem(new PropItem(_T("Direction"), CString(_T("PINDIR_INPUT"))));
		} else {
			group->AddItem(new PropItem(_T("Direction"), CString(_T("PINDIR_OUTPUT"))));
		}

        LPOLESTR strId = NULL;
        hr = pin->QueryId(&strId);
        if(hr == S_OK && strId != NULL)
            group->AddItem(new GraphStudio::PropItem(_T("Id"), CString(strId), false));
        if(strId) {
            CoTaskMemFree(strId);
			strId = NULL;
		}

		CComPtr<IMemInputPin> mem_input_pin;
		pin->QueryInterface(__uuidof(IMemInputPin), (void**)&mem_input_pin);
		if (mem_input_pin) {
			group->AddItem(new GraphStudio::PropItem(_T("ReceiveCanBlock"), S_OK == mem_input_pin->ReceiveCanBlock()));
		}

		CComPtr<IPin>	con_pin = NULL;
		pin->ConnectedTo(&con_pin);
		if (con_pin == NULL) {
			group->AddItem(new PropItem(_T("IsConnected"), (bool)FALSE));

			//-----------------------------------------------------------------
			// enumerate available media types
			//-----------------------------------------------------------------
			PropItem	*mtypes = group->AddItem(new PropItem(_T("Offered MediaTypes")));

			CComPtr<IEnumMediaTypes>	etypes;
			ULONG						f;

			hr = pin->EnumMediaTypes(&etypes);
			if (FAILED(hr)) {
				mtypes->AddItem(new PropItem(_T("Count"), (int)0));
			} else {

				int		count = 0;
				etypes->Reset();
				while (etypes->Skip(1) == NOERROR) count++;
				mtypes->AddItem(new PropItem(_T("Count"), count));

				etypes->Reset();
				int		cur = 1;
				AM_MEDIA_TYPE	*mt;
				while (etypes->Next(1, &mt, &f) == NOERROR) {
					CMediaType	mmt; mmt = *mt;
					CString		mtname;
					mtname.Format(_T("Type %d"), cur++);

					// append major/sub
					CString		maj_name, sub_name;
					GraphStudio::NameGuid(mt->majortype, maj_name, CgraphstudioApp::g_showGuidsOfKnownTypes);
					GraphStudio::NameGuid(mt->subtype,   sub_name, CgraphstudioApp::g_showGuidsOfKnownTypes);
					mtname += CString(_T(" [")) + maj_name + CString(_T(" / ")) + sub_name + CString(_T("]"));

					PropItem	*curmt = mtypes->AddItem(new PropItem(mtname));
					GetMediaTypeDetails(&mmt, curmt);

					DeleteMediaType(mt);
				}

			}

		} else {
			group->AddItem(new PropItem(_T("IsConnected"), (bool)TRUE));

			//-----------------------------------------------------------------
			// Allocator info
			//-----------------------------------------------------------------
			CComPtr<IMemInputPin>		input_pin;
			hr = pin->QueryInterface(IID_IMemInputPin, (void**)&input_pin);
			if (FAILED(hr)) {
				hr = con_pin->QueryInterface(IID_IMemInputPin, (void**)&input_pin);
			}

			if (SUCCEEDED(hr)) {

				CComPtr<IMemAllocator>	allocator;
				hr = input_pin->GetAllocator(&allocator);
				if (SUCCEEDED(hr)) {

					// retrieve current allocator properties
					ALLOCATOR_PROPERTIES	props;
					hr = allocator->GetProperties(&props);
					if (SUCCEEDED(hr)) {
						PropItem		*apinfo = group->AddItem(new PropItem(_T("ALLOCATOR_PROPERTIES")));
						GetAllocatorDetails(&props, apinfo);
					}
				}
			}

			//-----------------------------------------------------------------
			// current media type
			//-----------------------------------------------------------------
			AM_MEDIA_TYPE	mt;
			hr = pin->ConnectionMediaType(&mt);
			if (SUCCEEDED(hr)) {
				CMediaType	mmt; mmt = mt;

				PropItem		*curmt = group->AddItem(new PropItem(_T("Current MediaType")));
				GetMediaTypeDetails(&mmt, curmt);

				FreeMediaType(mt);
			}
		}
		con_pin = NULL;

        //-----------------------------------------------------------------
		// IKsPin
		//-----------------------------------------------------------------
        CComQIPtr<IKsPin> ksPin = pin;
        if(ksPin) {

			// Mediums
			KSMULTIPLE_ITEM * pmi = NULL;
			hr = ksPin->KsQueryMediums(&pmi);
			if (SUCCEEDED(hr) && pmi) {

				//-----------------------------------------------------------------
				// enumerate available mediums
				//-----------------------------------------------------------------
				PropItem * const mediums = group->AddItem(new PropItem(_T("Registered Pin Mediums (IKsPin)")));

				mediums->AddItem(new PropItem(_T("Count"), (unsigned int)pmi->Count));

				// Use pointer arithmetic to reference the first medium structure.
				REGPINMEDIUM *pmedium = (REGPINMEDIUM*)(pmi + 1);
				for (ULONG i = 0; i < pmi->Count; i++, pmedium++) {

					CString mediumName;
					mediumName.Format(_T("Medium %d"), i+1);		// use 1-based names

					PropItem * const mediumProp = mediums->AddItem(new PropItem(mediumName));

					//CString		strClsMedium;
					//GraphStudio::NameGuid(pmedium->clsMedium, strClsMedium, CgraphstudioApp::g_showGuidsOfKnownTypes);
					//mediumProp->AddItem(new PropItem(_T("clsMedium"),	strClsMedium));

					mediumProp->AddItem(new PropItem(_T("clsMedium"),	pmedium->clsMedium));
					mediumProp->AddItem(new PropItem(_T("dw1"),			(unsigned int)pmedium->dw1));
					mediumProp->AddItem(new PropItem(_T("dw2"),			(unsigned int)pmedium->dw2));
				}
			}
			CoTaskMemFree(pmi);
		}

        //-----------------------------------------------------------------
		// IMPEG2PIDMap
		//-----------------------------------------------------------------
        CComQIPtr<IMPEG2PIDMap> pidMap = pin;
        if(pidMap)
        {
            CComPtr<IEnumPIDMap> enumMap;
            if(SUCCEEDED(pidMap->EnumPIDMap(&enumMap)))
            {
                PropItem	*map = group->AddItem(new PropItem(_T("PIDMap")));

                PID_MAP pid;
                ULONG received = 0;
                while(enumMap->Next(1,&pid, &received) == S_OK)
                {
                    if(received)
                    {
                        CString pidVal;
                        CString contVal;
                        pidVal.Format(_T("PID %d"), pid.ulPID);
                        switch(pid.MediaSampleContent)
                        {
                        case MEDIA_TRANSPORT_PACKET:
                            contVal = _T("MEDIA_TRANSPORT_PACKET");
                            break;
                        case MEDIA_ELEMENTARY_STREAM:
                            contVal = _T("MEDIA_ELEMENTARY_STREAM");
                            break;
                        case MEDIA_MPEG2_PSI:
                            contVal = _T("MEDIA_MPEG2_PSI");
                            break;
                        case MEDIA_TRANSPORT_PAYLOAD:
                            contVal = _T("MEDIA_TRANSPORT_PAYLOAD");
                            break;
                        default:
                            contVal = _T("Unknown");
                        }
                        map->AddItem(new PropItem(pidVal, contVal));
                    }
                }
            }
        }

        //-----------------------------------------------------------------
		// IStreamBufferDataCounters
		//-----------------------------------------------------------------
        CComQIPtr<IStreamBufferDataCounters> sbDataCounters = pin;
        if(sbDataCounters)
        {
            SBE_PIN_DATA sbePinData;
            if(SUCCEEDED(sbDataCounters->GetData(&sbePinData)))
            {
                PropItem* sbdc = group->AddItem(new PropItem(_T("SBE_PIN_DATA")));
                sbdc->AddItem(new PropItem(_T("cDataBytes"),sbePinData.cDataBytes));
                sbdc->AddItem(new PropItem(_T("cSamplesProcessed"),sbePinData.cSamplesProcessed));
                sbdc->AddItem(new PropItem(_T("cDiscontinuities"),sbePinData.cDiscontinuities));
                sbdc->AddItem(new PropItem(_T("cSyncPoints"),sbePinData.cSyncPoints));
                sbdc->AddItem(new PropItem(_T("cTimestamps"),sbePinData.cTimestamps));
            }
        }

		return 0;
	}

	int GetAllocatorDetails(const ALLOCATOR_PROPERTIES *prop, PropItem *apinfo)
	{
		apinfo->AddItem(new PropItem(_T("cBuffers"), prop->cBuffers));
		apinfo->AddItem(new PropItem(_T("cbBuffer"), prop->cbBuffer));
		apinfo->AddItem(new PropItem(_T("cbAlign"), prop->cbAlign));
		apinfo->AddItem(new PropItem(_T("cbPrefix"), prop->cbPrefix));
		return 0;
	}

	int GetMediaTypeDetails(const CMediaType *pmt, PropItem *mtinfo)
	{
		CString		id_name;

		GraphStudio::NameGuid(pmt->majortype,	id_name,CgraphstudioApp::g_showGuidsOfKnownTypes);		mtinfo->AddItem(new PropItem(_T("majortype"), id_name));
		GraphStudio::NameGuid(pmt->subtype,		id_name,CgraphstudioApp::g_showGuidsOfKnownTypes);		mtinfo->AddItem(new PropItem(_T("subtype"), id_name));
		GraphStudio::NameGuid(pmt->formattype,	id_name,CgraphstudioApp::g_showGuidsOfKnownTypes);		mtinfo->AddItem(new PropItem(_T("formattype"), id_name));

		mtinfo->AddItem(new PropItem(_T("bFixedSizeSamples"), pmt->bFixedSizeSamples == TRUE));
		mtinfo->AddItem(new PropItem(_T("bTemporalCompression"), pmt->bTemporalCompression == TRUE));
		mtinfo->AddItem(new PropItem(_T("lSampleSize"), (int)pmt->lSampleSize));
		mtinfo->AddItem(new PropItem(_T("cbFormat"), (int)pmt->cbFormat));

		if (pmt->formattype == FORMAT_WaveFormatEx && pmt->cbFormat >= sizeof(WAVEFORMATEX)) {
			const WAVEFORMATEX	* const wfx = (WAVEFORMATEX*)pmt->pbFormat;
			uint8			*extra = NULL;
			int				len = 0;

			if (wfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
				PropItem	*wfxinfo = mtinfo->AddItem(new PropItem(_T("WAVEFORMATEXTENSIBLE")));
				GetWaveFormatExtensibleDetails((WAVEFORMATEXTENSIBLE*)wfx, wfxinfo);

				if (pmt->cbFormat > sizeof(WAVEFORMATEXTENSIBLE)) {
					extra = (uint8*)wfx + sizeof(WAVEFORMATEXTENSIBLE);
					len = pmt->cbFormat - sizeof(WAVEFORMATEXTENSIBLE);
				}

			} else 
			if (wfx->wFormatTag == 0x55 && wfx->cbSize == 12) {
				// WAVEFORMATEX
				PropItem	*wfxinfo = mtinfo->AddItem(new PropItem(_T("WAVEFORMATEX")));
				GetWaveFormatExDetails(wfx, wfxinfo);

				// MPEGLAYER3WAVEFORMAT
				PropItem	*mp3info = mtinfo->AddItem(new PropItem(_T("MPEGLAYER3WAVEFORMAT")));
				GetMpegLayer3InfoDetails((MPEGLAYER3WAVEFORMAT*)wfx, mp3info);
			} else
			if (wfx->wFormatTag == 0x50 && wfx->cbSize == 22) {
				// WAVEFORMATEX
				PropItem	*wfxinfo = mtinfo->AddItem(new PropItem(_T("WAVEFORMATEX")));
				GetWaveFormatExDetails(wfx, wfxinfo);

				// MPEG1WAVEFORMAT
				PropItem	*mpinfo = mtinfo->AddItem(new PropItem(_T("MPEG1WAVEFORMAT")));
				GetMpeg1WaveFormatDetails((MPEG1WAVEFORMAT*)wfx, mpinfo);				
			} else
			{
				PropItem	*wfxinfo = mtinfo->AddItem(new PropItem(_T("WAVEFORMATEX")));
				GetWaveFormatExDetails(wfx, wfxinfo);

				if (pmt->cbFormat > sizeof(WAVEFORMATEX)) {
					extra = (uint8*)wfx + sizeof(WAVEFORMATEX);
					len = pmt->cbFormat - sizeof(WAVEFORMATEX);
				}
			}

			// are there any extradata left ?
			if (len > 0) {
				PropItem	*info = mtinfo->AddItem(new PropItem(_T("Decoder Specific")));

				int				i;
				CString			extrabuf = _T("");
				for (i=0; i<len; i++) {
					CString	t;
					t.Format(_T("%02x "), extra[i]);
					extrabuf += t;
				}
				extrabuf = extrabuf.MakeUpper();
				info->AddItem(new PropItem(_T("Length"), len));
				info->AddItem(new PropItem(_T("Extradata"), extrabuf));
			}

		}
		else 
		if (pmt->formattype == FORMAT_WaveFormatExFFMPEG && pmt->cbFormat >= sizeof(WAVEFORMATEXFFMPEG)) {
			const WAVEFORMATEXFFMPEG * const wfxFfmpeg = (WAVEFORMATEXFFMPEG*)pmt->pbFormat;
			uint8			*extra = NULL;
			int				len = 0;
			
			if (wfxFfmpeg->wfex.wFormatTag == 21318) 
			{
				// WAVEFORMATEX
				PropItem	*wfxinfo = mtinfo->AddItem(new PropItem(_T("WAVEFORMATEX")));
				GetWaveFormatExDetails(&wfxFfmpeg->wfex, wfxinfo);
			}

			PropItem	*info = mtinfo->AddItem(new PropItem(_T("Decoder Specific")));
			if (wfxFfmpeg->nCodecId == 69645)
				info->AddItem(new PropItem(_T("CodecID"), CString(L"CODEC_ID_ADPCM_SWF (69645) ")));
			else
				info->AddItem(new PropItem(_T("CodecID"), wfxFfmpeg->nCodecId));

			if (pmt->cbFormat > sizeof(WAVEFORMATEXFFMPEG)) 
			{
				extra = (uint8*)&(wfxFfmpeg->wfex) + sizeof(WAVEFORMATEXFFMPEG);
				len = pmt->cbFormat - sizeof(WAVEFORMATEXFFMPEG);
			}


			// are there any extradata left ?
			if (len > 0) {

				int				i;
				CString			extrabuf = _T("");
				for (i=0; i<len; i++) {
					CString	t;
					t.Format(_T("%02x "), extra[i]);
					extrabuf += t;
				}
				extrabuf = extrabuf.MakeUpper();
				info->AddItem(new PropItem(_T("Length"), len));
				info->AddItem(new PropItem(_T("Extradata"), extrabuf));
			}
		}
		else 
		if (pmt->formattype == FORMAT_VideoInfo && pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) {
			const VIDEOINFOHEADER * const vih = (VIDEOINFOHEADER*)pmt->pbFormat;
			PropItem * const vihinfo = mtinfo->AddItem(new PropItem(_T("VIDEOINFOHEADER")));
			GetVideoInfoDetails(vih, vihinfo);

			uint8			*extra = NULL;
			int				len = 0;

			if (pmt->cbFormat > sizeof(VIDEOINFOHEADER)) {
				len = pmt->cbFormat - sizeof(VIDEOINFOHEADER);
				extra = (uint8*)vih + sizeof(VIDEOINFOHEADER);
			}

			// are there any extradata left ?
			if (len > 0) {
				PropItem	*info = mtinfo->AddItem(new PropItem(_T("Decoder Specific")));

				int				i;
				CString			extrabuf = _T("");
				for (i=0; i<len; i++) {
					CString	t;
					t.Format(_T("%02x "), extra[i]);
					extrabuf += t;
				}
				extrabuf = extrabuf.MakeUpper();
				info->AddItem(new PropItem(_T("Length"), len));
				info->AddItem(new PropItem(_T("Extradata"), extrabuf));

				GetExtradata_Video(pmt, mtinfo);
			}

		} else
		if (pmt->formattype == FORMAT_VideoInfo2 && pmt->cbFormat >= sizeof(VIDEOINFOHEADER2)) {
			const VIDEOINFOHEADER2 * const vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
			PropItem	* const vihinfo = mtinfo->AddItem(new PropItem(_T("VIDEOINFOHEADER2")));
			GetVideoInfo2Details(vih, vihinfo);

			uint8			*extra = NULL;
			int				len = 0;

			if (pmt->cbFormat > sizeof(VIDEOINFOHEADER2)) {
				len = pmt->cbFormat - sizeof(VIDEOINFOHEADER2);
				extra = (uint8*)vih + sizeof(VIDEOINFOHEADER2);
			}

			// are there any extradata left ?
			if (len > 0) {
				PropItem	*info = mtinfo->AddItem(new PropItem(_T("Decoder Specific")));

				int				i;
				CString			extrabuf = _T("");
				for (i=0; i<len; i++) {
					CString	t;
					t.Format(_T("%02x "), extra[i]);
					extrabuf += t;
				}
				extrabuf = extrabuf.MakeUpper();
				info->AddItem(new PropItem(_T("Length"), len));
				info->AddItem(new PropItem(_T("Extradata"), extrabuf));

				GetExtradata_Video(pmt, mtinfo);
			}
		} else
		if (pmt->formattype == FORMAT_MPEGVideo && pmt->cbFormat >= sizeof(MPEG1VIDEOINFO)) {
			const MPEG1VIDEOINFO	* const mvi = (MPEG1VIDEOINFO*)pmt->pbFormat;
			PropItem	* const mviinfo = mtinfo->AddItem(new PropItem(_T("MPEG1VIDEOINFO")));
			GetMpeg1VideoInfoDetails(mvi, mviinfo);
		} else
		if (pmt->formattype == FORMAT_MPEG2Video && pmt->cbFormat >= sizeof(MPEG2VIDEOINFO)) {
			const MPEG2VIDEOINFO * const mvi = (MPEG2VIDEOINFO*)pmt->pbFormat;
			PropItem * const mviinfo = mtinfo->AddItem(new PropItem(_T("MPEG2VIDEOINFO")));
			GetMpeg2VideoInfoDetails(mvi, mviinfo);

			GetExtradata_Video(pmt, mtinfo);
		}

		// we can also parse out decoder specific info in some cases
		if (pmt->majortype == MEDIATYPE_Audio) {
			if (pmt->subtype == MEDIASUBTYPE_AAC ||	pmt->subtype == MEDIASUBTYPE_LATM_AAC) {
				GetExtradata_AAC(pmt, mtinfo);
			}
		} else
		if (pmt->majortype == MEDIATYPE_MPEG2_PES) {
			if (pmt->subtype == MEDIASUBTYPE_AAC ||	pmt->subtype == MEDIASUBTYPE_LATM_AAC) {
				GetExtradata_AAC(pmt, mtinfo);
			}
		}

		return 0;
	}

	const TCHAR	*WfExNames[] = {
		_T("Front Left"),
		_T("Front Right"),
		_T("Front Center"),
		_T("Low Frequency"),
		_T("Back Left"),
		_T("Back Right"),
		_T("Front Left Of Center"),
		_T("Front Right Of Center"),
		_T("Back Center"),
		_T("Side Left"),
		_T("Side Right"),
		_T("Top Center"),
		_T("Top Front Left"),
		_T("Top Front Right"),
		_T("Top Back Left"),
		_T("Top Back Center"),
		_T("Top Back Right"),
	};
	const int WfxCount = sizeof(WfExNames)/sizeof(WfExNames[0]);

	int GetWaveFormatExtensibleDetails(const WAVEFORMATEXTENSIBLE *wfx, PropItem *wfxinfo)
	{
        if (wfx == NULL) return 0;
		// read waveformatex info
		PropItem	*wfxi = wfxinfo->AddItem(new PropItem(_T("WAVEFORMATEX")));
		GetWaveFormatExDetails(&wfx->Format, wfxi);

		// and add WFExtensible
		wfxinfo->AddItem(new PropItem(wfx->Format.wBitsPerSample==0 ? _T("wSamplesPerBlock") : _T("wValidBitsPerSample"), (int)wfx->Samples.wSamplesPerBlock));
		wfxinfo->AddItem(new PropItem(_T("dwChannelMask"), (int)wfx->dwChannelMask));

		CString		id_name;
		GraphStudio::NameGuid(wfx->SubFormat,	id_name, CgraphstudioApp::g_showGuidsOfKnownTypes);		
		wfxinfo->AddItem(new PropItem(_T("SubFormat"), id_name));

		if (wfx->dwChannelMask != 0) {

			PropItem	*chans = wfxinfo->AddItem(new PropItem(_T("Channels")));
			uint32		mask = 1;

			for (int i=0; i<WfxCount; i++) {
				CString	ok = (wfx->dwChannelMask & mask ? _T("TRUE") : _T("FALSE"));
				chans->AddItem(new PropItem(CString(WfExNames[i]), ok));
				mask <<= 1;
			}
		}
		return 0;
	}

	int GetWaveFormatExDetails(const WAVEFORMATEX *wfx, PropItem *wfxinfo)
	{
        if (wfx == NULL) return 0;
		CString		fmttag;
		fmttag.Format(_T("%d"), wfx->wFormatTag);

		switch (wfx->wFormatTag) {
		case WAVE_FORMAT_PCM:			fmttag += _T(" (WAVE_FORMAT_PCM)"); break;		
		case WAVE_FORMAT_IEEE_FLOAT:	fmttag += _T(" (WAVE_FORMAT_IEEE_FLOAT)"); break;		
		case WAVE_FORMAT_DRM:			fmttag += _T(" (WAVE_FORMAT_DRM)"); break;		
		case WAVE_FORMAT_ALAW:			fmttag += _T(" (WAVE_FORMAT_ALAW)"); break;		
		case WAVE_FORMAT_MULAW:			fmttag += _T(" (WAVE_FORMAT_MULAW)"); break;		
		case WAVE_FORMAT_ADPCM:			fmttag += _T(" (WAVE_FORMAT_ADPCM)"); break;		
		case WAVE_FORMAT_EXTENSIBLE:	fmttag += _T(" (WAVE_FORMAT_EXTENSIBLE)"); break;
		}
		wfxinfo->AddItem(new PropItem(_T("wFormatTag"), fmttag));
		wfxinfo->AddItem(new PropItem(_T("nChannels"), (int)wfx->nChannels));
		wfxinfo->AddItem(new PropItem(_T("nSamplesPerSec"), (int)wfx->nSamplesPerSec));
		wfxinfo->AddItem(new PropItem(_T("nAvgBytesPerSec"), (int)wfx->nAvgBytesPerSec));
		wfxinfo->AddItem(new PropItem(_T("nBlockAlign"), (int)wfx->nBlockAlign));
		wfxinfo->AddItem(new PropItem(_T("wBitsPerSample"), (int)wfx->wBitsPerSample));
		wfxinfo->AddItem(new PropItem(_T("cbSize"), (int)wfx->cbSize));
		return 0;
	}

	int GetVideoInfoDetails(const VIDEOINFOHEADER *vih, PropItem *vihinfo)
	{
        if(!vih) return 0;
		vihinfo->AddItem(new PropItem(_T("rcSource"), vih->rcSource));
		vihinfo->AddItem(new PropItem(_T("rcTarget"), vih->rcTarget));
		vihinfo->AddItem(new PropItem(_T("dwBitRate"), (int)vih->dwBitRate));
		vihinfo->AddItem(new PropItem(_T("dwBitErrorRate"), (int)vih->dwBitErrorRate));
		vihinfo->AddItem(new PropItem(_T("AvgTimePerFrame"), (__int64)vih->AvgTimePerFrame));

		PropItem	*bihinfo = vihinfo->AddItem(new PropItem(_T("BITMAPINFOHEADER")));
		GetBitmapInfoDetails(&vih->bmiHeader, bihinfo);
		return 0;
	}

	int GetVideoInfo2Details(const VIDEOINFOHEADER2 *vih, PropItem *vihinfo)
	{
        if(!vih) return 0;
		vihinfo->AddItem(new PropItem(_T("rcSource"), vih->rcSource));
		vihinfo->AddItem(new PropItem(_T("rcTarget"), vih->rcTarget));
		vihinfo->AddItem(new PropItem(_T("dwBitRate"), (int)vih->dwBitRate));
		vihinfo->AddItem(new PropItem(_T("dwBitErrorRate"), (int)vih->dwBitErrorRate));
		vihinfo->AddItem(new PropItem(_T("AvgTimePerFrame"), (__int64)vih->AvgTimePerFrame));

		CString		v;
		v.Format(_T("0x%08x"), vih->dwInterlaceFlags);		vihinfo->AddItem(new PropItem(_T("dwInterlaceFlags"), v));
		v.Format(_T("0x%08x"), vih->dwCopyProtectFlags);	vihinfo->AddItem(new PropItem(_T("dwCopyProtectFlags"), v));
		v.Format(_T("0x%08x (%d)"), vih->dwPictAspectRatioX, vih->dwPictAspectRatioX);	vihinfo->AddItem(new PropItem(_T("dwPictAspectRatioX"), v));
		v.Format(_T("0x%08x (%d)"), vih->dwPictAspectRatioY, vih->dwPictAspectRatioY);	vihinfo->AddItem(new PropItem(_T("dwPictAspectRatioY"), v));
		v.Format(_T("0x%08x"), vih->dwControlFlags);		vihinfo->AddItem(new PropItem(_T("dwControlFlags"), v));

		PropItem	*bihinfo = vihinfo->AddItem(new PropItem(_T("BITMAPINFOHEADER")));
		GetBitmapInfoDetails(&vih->bmiHeader, bihinfo);
		return 0;
	}

	int GetMpeg1VideoInfoDetails(const MPEG1VIDEOINFO *mvi, PropItem *mviinfo)
	{
        if(!mvi) return 0;
		mviinfo->AddItem(new PropItem(_T("dwStartTimeCode"), (int)mvi->dwStartTimeCode));
		mviinfo->AddItem(new PropItem(_T("cbSequenceHeader"), (int)mvi->cbSequenceHeader));

		// todo: sequence header
		int				i;
		CString			extrabuf = _T("");
		uint8			*extra = (uint8*)&mvi->bSequenceHeader[0];
		int				extralen = mvi->cbSequenceHeader;
		for (i=0; i<extralen; i++) {
			CString	t;
			t.Format(_T("%02x "), extra[i]);
			extrabuf += t;
		}
		extrabuf = extrabuf.MakeUpper();
		mviinfo->AddItem(new PropItem(_T("Sequence Header"), extrabuf));

		PropItem	*vihinfo = mviinfo->AddItem(new PropItem(_T("VIDEOINFOHEADER")));
		GetVideoInfoDetails(&mvi->hdr, vihinfo);

		return 0;
	}

	int GetMpeg2VideoInfoDetails(const MPEG2VIDEOINFO *mvi, PropItem *mviinfo)
	{
        if(!mvi) return 0;
		mviinfo->AddItem(new PropItem(_T("dwStartTimeCode"), (int)mvi->dwStartTimeCode));
		mviinfo->AddItem(new PropItem(_T("cbSequenceHeader"), (int)mvi->cbSequenceHeader));
		mviinfo->AddItem(new PropItem(_T("dwProfile"), (int)mvi->dwProfile));
		mviinfo->AddItem(new PropItem(_T("dwLevel"), (int)mvi->dwLevel));
		mviinfo->AddItem(new PropItem(_T("dwFlags"), (int)mvi->dwFlags));

		// todo: sequence header
		int				i;
		CString			extrabuf = _T("");
		uint8			*extra = (uint8*)&mvi->dwSequenceHeader[0];
		int				extralen = mvi->cbSequenceHeader;
		for (i=0; i<extralen; i++) {
			CString	t;
			t.Format(_T("%02x "), extra[i]);
			extrabuf += t;
		}
		extrabuf = extrabuf.MakeUpper();
		mviinfo->AddItem(new PropItem(_T("Sequence Header"), extrabuf));

		PropItem	*vihinfo = mviinfo->AddItem(new PropItem(_T("VIDEOINFOHEADER2")));
		GetVideoInfo2Details(&mvi->hdr, vihinfo);

		return 0;
	}

	int GetMpegLayer3InfoDetails(const MPEGLAYER3WAVEFORMAT *mp3, PropItem *mp3info)
	{
        if(!mp3) return 0;
		mp3info->AddItem(new PropItem(_T("wID"), (int)mp3->wID));

		CString		f;
		switch (mp3->fdwFlags) {
		case MPEGLAYER3_FLAG_PADDING_ISO:	f = _T("MPEGLAYER3_FLAG_PADDING_ISO"); break;
		case MPEGLAYER3_FLAG_PADDING_ON:	f = _T("MPEGLAYER3_FLAG_PADDING_ON"); break;
		case MPEGLAYER3_FLAG_PADDING_OFF:	f = _T("MPEGLAYER3_FLAG_PADDING_OFF"); break;
		default:							f = _T("0"); break;
		}
		mp3info->AddItem(new PropItem(_T("fdwFlags"), f));
		mp3info->AddItem(new PropItem(_T("nBlockSize"), (int)mp3->nBlockSize));
		mp3info->AddItem(new PropItem(_T("nFramesPerBlock"), (int)mp3->nFramesPerBlock));
		mp3info->AddItem(new PropItem(_T("nCodecDelay"), (int)mp3->nCodecDelay));

		return 0;
	}

	int GetMpeg1WaveFormatDetails(const MPEG1WAVEFORMAT *wfx, PropItem *mpinfo)
	{
        if(!wfx) return 0;
		CString		f;
		switch (wfx->fwHeadLayer) {
		case ACM_MPEG_LAYER1:		f = _T("ACM_MPEG_LAYER1"); break;
		case ACM_MPEG_LAYER2:		f = _T("ACM_MPEG_LAYER2"); break;
		case ACM_MPEG_LAYER3:		f = _T("ACM_MPEG_LAYER3"); break;
		default:					f.Format(_T("%d"), wfx->fwHeadLayer); break;
		}
		mpinfo->AddItem(new PropItem(_T("fwHeadLayer"), f));
		mpinfo->AddItem(new PropItem(_T("dwHeadBitrate"), (int)wfx->dwHeadBitrate));

		switch (wfx->fwHeadMode) {
		case ACM_MPEG_STEREO:		f = _T("ACM_MPEG_STEREO"); break;
		case ACM_MPEG_JOINTSTEREO:	f = _T("ACM_MPEG_JOINTSTEREO"); break;
		case ACM_MPEG_DUALCHANNEL:	f = _T("ACM_MPEG_DUALCHANNEL"); break;
		case ACM_MPEG_SINGLECHANNEL:f = _T("ACM_MPEG_SINGLECHANNEL"); break;
		default:					f.Format(_T("%d"), wfx->fwHeadMode);
		}
		mpinfo->AddItem(new PropItem(_T("fwHeadMode"), f));
		mpinfo->AddItem(new PropItem(_T("fwHeadModeExt"), (int)wfx->fwHeadModeExt));

		switch (wfx->wHeadEmphasis) {
		case 1:		f = _T("1 [00] (None)"); break;
		case 2:		f = _T("2 [01] (50/15 ms emphasis)"); break;
		case 3:		f = _T("3 [10] (Reserved)"); break;
		case 4:		f = _T("4 [11] (CCITT J.17)"); break;
		default:	f.Format(_T("%d"), wfx->wHeadEmphasis);
		}
		mpinfo->AddItem(new PropItem(_T("wHeadEmphasis"), f));
		mpinfo->AddItem(new PropItem(_T("fwHeadFlags"), (int)wfx->fwHeadFlags));
		mpinfo->AddItem(new PropItem(_T("dwPTSLow"), (int)wfx->dwPTSLow));
		mpinfo->AddItem(new PropItem(_T("dwPTSHigh"), (int)wfx->dwPTSHigh));

		PropItem	*flags = mpinfo->AddItem(new PropItem(_T("Flags")));
		bool	priv = (wfx->fwHeadFlags & ACM_MPEG_PRIVATEBIT) == ACM_MPEG_PRIVATEBIT;
		bool	copy = (wfx->fwHeadFlags & ACM_MPEG_COPYRIGHT) == ACM_MPEG_COPYRIGHT;
		bool	orig = (wfx->fwHeadFlags & ACM_MPEG_ORIGINALHOME) == ACM_MPEG_ORIGINALHOME;
		bool	prot = (wfx->fwHeadFlags & ACM_MPEG_PROTECTIONBIT) == ACM_MPEG_PROTECTIONBIT;
		bool	mpg1 = (wfx->fwHeadFlags & ACM_MPEG_ID_MPEG1) == ACM_MPEG_ID_MPEG1;

		flags->AddItem(new PropItem(_T("ACM_MPEG_PRIVATEBIT"), (priv ? CString(_T("True")) : CString(_T("False")))));
		flags->AddItem(new PropItem(_T("ACM_MPEG_COPYRIGHT"), (copy ? CString(_T("True")) : CString(_T("False")))));
		flags->AddItem(new PropItem(_T("ACM_MPEG_ORIGINALHOME"), (orig ? CString(_T("True")) : CString(_T("False")))));
		flags->AddItem(new PropItem(_T("ACM_MPEG_PROTECTIONBIT"), (prot ? CString(_T("True")) : CString(_T("False")))));
		flags->AddItem(new PropItem(_T("ACM_MPEG_ID_MPEG1"), (mpg1 ? CString(_T("True")) : CString(_T("False")))));

		return 0;
	}

	int GetBitmapInfoDetails(const BITMAPINFOHEADER *bih, PropItem *bihinfo)
	{
        if(!bih) return 0;
		CString		v, c;
		int			ret;

		bihinfo->AddItem(new PropItem(_T("biSize"), (int)bih->biSize));
		bihinfo->AddItem(new PropItem(_T("biWidth"), (int)bih->biWidth));
		bihinfo->AddItem(new PropItem(_T("biHeight"), (int)bih->biHeight));
		bihinfo->AddItem(new PropItem(_T("biPlanes"), (int)bih->biPlanes));
		bihinfo->AddItem(new PropItem(_T("biBitCount"), (int)bih->biBitCount));


		v.Format(_T("0x%08x"), bih->biCompression);		
		ret = GetFourCC(bih->biCompression, c);
		if (ret == 0) {
			v = v + _T(" [") + c + _T("]");
		}

		bihinfo->AddItem(new PropItem(_T("biCompression"), v));
		bihinfo->AddItem(new PropItem(_T("biSizeImage"), (int)bih->biSizeImage));

		bihinfo->AddItem(new PropItem(_T("biXPelsPerMeter"), (int)bih->biXPelsPerMeter));
		bihinfo->AddItem(new PropItem(_T("biYPelsPerMeter"), (int)bih->biYPelsPerMeter));
		bihinfo->AddItem(new PropItem(_T("biClrUsed"), (int)bih->biClrUsed));
		bihinfo->AddItem(new PropItem(_T("biClrImportant"), (int)bih->biClrImportant));

		return 0;
	}

	//-------------------------------------------------------------------------
	//
	//	AAC Format
	//
	//-------------------------------------------------------------------------

	const int AAC_Sample_Rates[] = {
			96000, 88200, 64000, 48000,
			44100, 32000, 24000, 22050, 
			16000, 12000, 11025, 8000,
			7350, 0, 0, 0
	};

	const LPCTSTR AAC_Object_Types[] = {
		_T("Unknown"),						// 0
		_T("Main"),							// 1
		_T("Low Complexity"),				// 2
		_T("Scalable Sampling Rate"),		// 3
		_T("Long Term Predictor"),			// 4
		_T("High Efficiency"),				// 5
		_T("Unknown"), _T("Unknown"), _T("Unknown"), _T("Unknown"),		// 6, 7, 8, 9
		_T("Unknown"), _T("Unknown"), _T("Unknown"), _T("Unknown"),		// 10, 11, 12, 13
		_T("Unknown"), _T("Unknown"), _T("Unknown"), 		// 14, 15, 16
		_T("Error Resilient LC"),			// 17
		_T("Unknown"),						// 18
		_T("Error Resilient LTP"),			// 19,
		_T("Unknown"), _T("Unknown"), _T("Unknown"), 		// 20, 21, 22
		_T("Low Delay"),					// 23
		_T("Unknown"), _T("Unknown"), _T("Unknown"),		// 24, 25, 26
		_T("DRM ER LC"),					// 27
		_T("Unknown"), _T("Unknown"), _T("Unknown"), _T("Unknown"), // 28, 29, 30, 31
	};

	int Parse_AAC_Raw(uint8 *buf, PropItem *aacinfo)
	{
		int sbr_present = -1;

		Bitstream		b(buf);
		b.NeedBits();

		// object
		int audioObjectType = b.UGetBits(5);
		if (audioObjectType == 31) {
			uint8 n = b.UGetBits(6);
			audioObjectType = 32 + n;
			b.NeedBits();
		}
		CString	ao;
		if (audioObjectType > 31) {
			ao.Format(_T("%d (Unknown)"), audioObjectType);
		} else {
			ao.Format(_T("%d (%s)"), audioObjectType, AAC_Object_Types[audioObjectType]);
		}
		aacinfo->AddItem(new PropItem(_T("Object Type"), ao));
		
		int samplingFrequencyIndex = b.UGetBits(4);	
		aacinfo->AddItem(new PropItem(_T("Freq. Index"), samplingFrequencyIndex));

		int samplingFrequency = AAC_Sample_Rates[samplingFrequencyIndex];
		if (samplingFrequencyIndex == 0x0f) {
			b.NeedBits24();
			uint32 f = b.UGetBits(24);
			samplingFrequency = f;	
			b.NeedBits();
		}
		int channelConfiguration = b.UGetBits(4);
		aacinfo->AddItem(new PropItem(_T("Channels"), channelConfiguration));

		if (audioObjectType == 5) {
			sbr_present = 1;

			// TODO: parsing !!!!!!!!!!!!!!!!
		}

		switch (audioObjectType) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 6:
		case 7:
		case 17:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			// GASpecificConfig(b);
			break;
		}

		if (sbr_present == -1) {
			if (samplingFrequency <= 24000) {
				samplingFrequency *= 2;
			}			
		}
		aacinfo->AddItem(new PropItem(_T("Frequency"), samplingFrequency));

		return 0;
	}

	int GetExtradata_AAC(const CMediaType *pmt, PropItem *mtinfo)
	{
        if (pmt->pbFormat == NULL
				|| pmt->formattype != FORMAT_WaveFormatEx
				|| pmt->cbFormat < sizeof(WAVEFORMATEX)) 
			return 0;

		const WAVEFORMATEX * const wfx = (WAVEFORMATEX*)pmt->pbFormat;
		int			extralen = wfx->cbSize;
		uint8		  *extra = (uint8*)(wfx) + sizeof(WAVEFORMATEX);

		// done with
		if (extralen <= 0) return 0;

		PropItem	*aacinfo = mtinfo->AddItem(new PropItem(_T("AAC Decoder Specific")));

		int				i;
		CString			extrabuf = _T("");
		for (i=0; i<extralen; i++) {
			CString	t;
			t.Format(_T("%02x "), extra[i]);
			extrabuf += t;
		}
		extrabuf = extrabuf.MakeUpper();
		aacinfo->AddItem(new PropItem(_T("Length"), extralen));
		aacinfo->AddItem(new PropItem(_T("Extradata"), extrabuf));

		// let's try to parse the AAC type
		Bitstream		b(extra);
		b.NeedBits();

		if (b.UBits(12) == 0xfff) {
			// ADTS

		} else 
		if (b.UBits(11) == 0x2b7) {
			// LATM

		} else {
			// RAW
			Parse_AAC_Raw(extra, aacinfo);
		}

		return 0;
	}

    static int H264NALParser(CBitStreamReader& br, PropItem *mtinfo)
    {
        UINT8 NALType = br.ReadU8() & 0x1f;

        if (NALType == 7)
        {
            PropItem *spsinfo = mtinfo->AddItem(new PropItem(_T("H264 Sequence Parameter Set")));

			sps_t sps = { 0 };
            CH264StructReader::ReadSPS(br, sps);

            spsinfo->AddItem(new PropItem(_T("Profile IDC"), sps.profile_idc));
            spsinfo->AddItem(new PropItem(_T("Constraint Set 0"), sps.constraint_set0_flag));
            spsinfo->AddItem(new PropItem(_T("Constraint Set 1"), sps.constraint_set1_flag));
            spsinfo->AddItem(new PropItem(_T("Constraint Set 2"), sps.constraint_set2_flag));
            spsinfo->AddItem(new PropItem(_T("Constraint Set 3"), sps.constraint_set3_flag));
            spsinfo->AddItem(new PropItem(_T("Constraint Set 4"), sps.constraint_set4_flag));
            spsinfo->AddItem(new PropItem(_T("Constraint Set 5"), sps.constraint_set5_flag));

            spsinfo->AddItem(new PropItem(_T("Level IDC"), sps.level_idc));

            spsinfo->AddItem(new PropItem(_T("Seq Parameter Set ID"), sps.seq_parameter_set_id));

            if (sps.profile_idc == 100 || sps.profile_idc == 110 || sps.profile_idc == 122 || sps.profile_idc == 144)
            {
                CString strChroma;
                strChroma.Format(_T("%d"), sps.chroma_format_idc);
                switch (sps.chroma_format_idc)
                {
                case 0: strChroma.Append(_T(" => Monochrome")); break;
                case 1: strChroma.Append(_T(" => 4:2:0")); break;
                case 2: strChroma.Append(_T(" => 4:2:2")); break;
                case 3: strChroma.Append(_T(" => 4:4:4")); break;
                }

                spsinfo->AddItem(new PropItem(_T("Chroma Format IDC"), strChroma));
                if (sps.chroma_format_idc == 3)
                    spsinfo->AddItem(new PropItem(_T("Residual Colour Transform"), sps.residual_colour_transform_flag));

                spsinfo->AddItem(new PropItem(_T("BitDepth Luma (-8)"), sps.bit_depth_luma_minus8));
                spsinfo->AddItem(new PropItem(_T("BitDepth Chroma (-8)"), sps.bit_depth_chroma_minus8));
                spsinfo->AddItem(new PropItem(_T("QPPrime Y Zero Transform Bypass"), sps.qpprime_y_zero_transform_bypass_flag));

                spsinfo->AddItem(new PropItem(_T("Seq Scaling Matrix Present"), sps.seq_scaling_matrix_present_flag));
                if (sps.seq_scaling_matrix_present_flag)
                {
                    // TODO maybe later
                }
            }

            spsinfo->AddItem(new PropItem(_T("Log2 Max Frame Num (-4)"), sps.log2_max_frame_num_minus4));
                
            spsinfo->AddItem(new PropItem(_T("Pic Order Cnt Type"), sps.pic_order_cnt_type));
            if (sps.pic_order_cnt_type == 0)
                spsinfo->AddItem(new PropItem(_T("Log2 Max Pic Order Cnt Lsb (-4)"), sps.log2_max_pic_order_cnt_lsb_minus4));
            else
            {
                spsinfo->AddItem(new PropItem(_T("Delta Pic Order Always Zero"),  sps.delta_pic_order_always_zero_flag));
                spsinfo->AddItem(new PropItem(_T("Offset For Non Ref Pic"), sps.offset_for_non_ref_pic));
                spsinfo->AddItem(new PropItem(_T("Offset For Top To Bottom Field"), sps.offset_for_top_to_bottom_field));

                spsinfo->AddItem(new PropItem(_T("Num Ref Frames In Pic Order Cnt Cycle"), sps.num_ref_frames_in_pic_order_cnt_cycle));
                if (sps.num_ref_frames_in_pic_order_cnt_cycle > 0)
                {
                    CString str = _T("");
                    for (int i=0; i<sps.num_ref_frames_in_pic_order_cnt_cycle; i++)
                    {
                        if(!str.IsEmpty())
                            str.Append(_T(", "));

                        str.AppendFormat(_T("%d"), br.ReadSE());
                    }

                    spsinfo->AddItem(new PropItem(_T("Offset For Ref Frames"), str));
                }
            }

            spsinfo->AddItem(new PropItem(_T("Num Ref Frames"), sps.num_ref_frames));
            spsinfo->AddItem(new PropItem(_T("Gaps In Frame Num Value Allowed"), sps.gaps_in_frame_num_value_allowed_flag));
            spsinfo->AddItem(new PropItem(_T("Pic Width In Mbs (-1)"), sps.pic_width_in_mbs_minus1));
            spsinfo->AddItem(new PropItem(_T("Pic Height In Map Units (-1)"), sps.pic_height_in_map_units_minus1));

            spsinfo->AddItem(new PropItem(_T("Frame Mbs Only"), sps.frame_mbs_only_flag));
            if (!sps.frame_mbs_only_flag)
                spsinfo->AddItem(new PropItem(_T("Mb Adaptive Frame Field"), sps.mb_adaptive_frame_field_flag));

            spsinfo->AddItem(new PropItem(_T("Direct 8x8 Inference"), sps.direct_8x8_inference_flag));

            spsinfo->AddItem(new PropItem(_T("Frame Cropping"), sps.frame_cropping_flag));
            if (sps.frame_cropping_flag)
            {
                RECT rect;
                rect.left = sps.frame_crop_left_offset;
                rect.right = sps.frame_crop_right_offset;
                rect.top = sps.frame_crop_top_offset;
                rect.bottom = sps.frame_crop_bottom_offset;

                spsinfo->AddItem(new PropItem(_T("Frame Crop (l,r,t,b)"), rect));
            }

            spsinfo->AddItem(new PropItem(_T("VUI Parameters present"), sps.vui_parameters_present_flag));
            if (sps.vui_parameters_present_flag)
            {
                PropItem* vui = new PropItem(_T("VUI Parameters"));
                vui->AddItem(new PropItem(_T("Aspect Ratio Info Present"), sps.vui.aspect_ratio_info_present_flag));
                if (sps.vui.aspect_ratio_info_present_flag)
                {
                    CString strAR;
                    strAR.Format(_T("%d"), sps.vui.aspect_ratio_idc);
                    switch (sps.vui.aspect_ratio_idc)
                    {
                    case 0: strAR.Append(_T(" => Unspecified")); break;
                    case 1: strAR.Append(_T(" => 1:1")); break;
                    case 2: strAR.Append(_T(" => 12:11")); break;
                    case 3: strAR.Append(_T(" => 10:11")); break;
                    case 4: strAR.Append(_T(" => 16:11")); break;
                    case 5: strAR.Append(_T(" => 40:30")); break;
                    case 6: strAR.Append(_T(" => 24:11")); break;
                    case 7: strAR.Append(_T(" => 20:11")); break;
                    case 8: strAR.Append(_T(" => 32:11")); break;
                    case 9: strAR.Append(_T(" => 80:33")); break;
                    case 10: strAR.Append(_T(" => 18:11")); break;
                    case 11: strAR.Append(_T(" => 15:11")); break;
                    case 12: strAR.Append(_T(" => 64:33")); break;
                    case 13: strAR.Append(_T(" => 160:99")); break;
                    case 14: strAR.Append(_T(" => 4:3")); break;
                    case 15: strAR.Append(_T(" => 3:2")); break;
                    case 16: strAR.Append(_T(" => 2:1")); break;
                    case 255: strAR.Append(_T(" => Extended_SAR")); break;
                    }

                    vui->AddItem(new PropItem(_T("Aspect Ratio IDC"), strAR));
                    if (sps.vui.aspect_ratio_idc == 255)
                    {
                        vui->AddItem(new PropItem(_T("SAR Width"), sps.vui.sar_width));
                        vui->AddItem(new PropItem(_T("SAR Height"), sps.vui.sar_height));
                    }
                }

                vui->AddItem(new PropItem(_T("Overscan Info Present"), sps.vui.overscan_info_present_flag));
                if (sps.vui.overscan_info_present_flag)
                    vui->AddItem(new PropItem(_T("Overscan Appropriate"), sps.vui.overscan_appropriate_flag));

                vui->AddItem(new PropItem(_T("Video Signal Type Present"), sps.vui.video_signal_type_present_flag));
                if (sps.vui.video_signal_type_present_flag)
                {
                    CString strVideoFormat;
                    strVideoFormat.Format(_T("%d"), sps.vui.video_format);
                    switch (sps.vui.video_format)
                    {
                    case 0: strVideoFormat.Append(_T(" => Component")); break;
                    case 1: strVideoFormat.Append(_T(" => PAL")); break;
                    case 2: strVideoFormat.Append(_T(" => NTSC")); break;
                    case 3: strVideoFormat.Append(_T(" => SECAM")); break;
                    case 4: strVideoFormat.Append(_T(" => MAC")); break;
                    }

                    vui->AddItem(new PropItem(_T("Video Format"), strVideoFormat));
                    vui->AddItem(new PropItem(_T("Video Full Range"), sps.vui.video_full_range_flag));
                    
                    vui->AddItem(new PropItem(_T("Colour Description Present"), sps.vui.colour_description_present_flag));
                    if (sps.vui.colour_description_present_flag)
                    {
                        vui->AddItem(new PropItem(_T("Colour Primaries"), sps.vui.colour_primaries));
                        vui->AddItem(new PropItem(_T("Transfer Characteristics"), sps.vui.transfer_characteristics));
                        vui->AddItem(new PropItem(_T("Matrix Coefficients"), sps.vui.matrix_coefficients));
                    }
                }
                    
                vui->AddItem(new PropItem(_T("Chroma Loc Info Present"), sps.vui.chroma_loc_info_present_flag));
                if (sps.vui.chroma_loc_info_present_flag)
                {
                    vui->AddItem(new PropItem(_T("Chroma Sample Loc Type Top Field"), sps.vui.chroma_sample_loc_type_top_field));
                    vui->AddItem(new PropItem(_T("Chroma Sample Loc Type Bottom Field"), sps.vui.chroma_sample_loc_type_bottom_field));
                }

                vui->AddItem(new PropItem(_T("Timing Info Present"), sps.vui.timing_info_present_flag));
                if (sps.vui.timing_info_present_flag)
                {
                    vui->AddItem(new PropItem(_T("Num Units In Tick"), sps.vui.num_units_in_tick));
                    vui->AddItem(new PropItem(_T("Time Scale (Hz)"), sps.vui.time_scale));
                    vui->AddItem(new PropItem(_T("=> AvgTimePerFrame"), CH264StructReader::GetAvgTimePerFrame(sps.vui.num_units_in_tick,sps.vui.time_scale)));
                    vui->AddItem(new PropItem(_T("Fixed Frame Rate"), sps.vui.fixed_frame_rate_flag));
                }

                vui->AddItem(new PropItem(_T("NAL HRD Parameters Present"), sps.vui.nal_hrd_parameters_present_flag));
                if (sps.vui.nal_hrd_parameters_present_flag)
                {
                    // TODO maybe later
                }

                vui->AddItem(new PropItem(_T("VCL HRD Parameters Present"), sps.vui.vcl_hrd_parameters_present_flag));
                if (sps.vui.vcl_hrd_parameters_present_flag)
                {
                    // TODO maybe later
                }

                if (sps.vui.nal_hrd_parameters_present_flag || sps.vui.vcl_hrd_parameters_present_flag)
                    vui->AddItem(new PropItem(_T("Low Delay HRD"), sps.vui.low_delay_hrd_flag));

                vui->AddItem(new PropItem(_T("Pic Struct Present"), sps.vui.pic_struct_present_flag));

                vui->AddItem(new PropItem(_T("Bitstream Restriction"), sps.vui.bitstream_restriction_flag));
                if (sps.vui.bitstream_restriction_flag)
                {
                    vui->AddItem(new PropItem(_T("Motion Vectors Over Pic Boundaries"), sps.vui.motion_vectors_over_pic_boundaries_flag));
                    vui->AddItem(new PropItem(_T("Max Bytes per Pic Denom"), sps.vui.max_bytes_per_pic_denom));
                    vui->AddItem(new PropItem(_T("Max Bits per MB Denom"), sps.vui.max_bits_per_mb_denom));
                    vui->AddItem(new PropItem(_T("log2 Max Mv Length Horizontal"), sps.vui.log2_max_mv_length_horizontal));
                    vui->AddItem(new PropItem(_T("log2 Max Mv Length Vertical"), sps.vui.log2_max_mv_length_vertical));
                    vui->AddItem(new PropItem(_T("Num Reorder Frames"), sps.vui.num_reorder_frames));
                    vui->AddItem(new PropItem(_T("Max Dec Frame Buffering"), sps.vui.max_dec_frame_buffering));
                }

                spsinfo->AddItem(vui);
                return 0;
            }
        }
        else if (NALType == 8)
        {
            PropItem *ppsinfo = mtinfo->AddItem(new PropItem(_T("H264 Picture Parameter Set")));
			pps_t pps = { 0 };
            CH264StructReader::ReadPPS(br, pps);

            ppsinfo->AddItem(new PropItem(_T("PPS ID"), pps.pic_parameter_set_id));
            ppsinfo->AddItem(new PropItem(_T("SPS ID"), pps.seq_parameter_set_id));
            ppsinfo->AddItem(new PropItem(_T("Entropy Coding Mode"), pps.entropy_coding_mode_flag));
            ppsinfo->AddItem(new PropItem(_T("Pic Order Present"), pps.pic_order_present_flag));

            ppsinfo->AddItem(new PropItem(_T("Num Slice Groups (-1)"), pps.num_slice_groups_minus1));
            if(pps.num_slice_groups_minus1 > 0)
            {
                // TODO maybe later
            }

            ppsinfo->AddItem(new PropItem(_T("Num Ref Idx 10 Activ (-1)"), pps.num_ref_idx_l0_active_minus1));
            ppsinfo->AddItem(new PropItem(_T("Num Ref Idx 11 Activ (-1)"), pps.num_ref_idx_l1_active_minus1));
            ppsinfo->AddItem(new PropItem(_T("Weighted Pred"), pps.weighted_pred_flag));
            ppsinfo->AddItem(new PropItem(_T("Weighted Bibred Idc"), pps.weighted_bipred_idc));
            ppsinfo->AddItem(new PropItem(_T("Pic Init QP (-26)"), pps.pic_init_qp_minus26));
            ppsinfo->AddItem(new PropItem(_T("Pic Init QS (-26)"), pps.pic_init_qs_minus26));
            ppsinfo->AddItem(new PropItem(_T("Chroma QP Idx Offset"), pps.chroma_qp_index_offset));
            ppsinfo->AddItem(new PropItem(_T("Deblocking Filter Ctrl Present"), pps.deblocking_filter_control_present_flag));
            ppsinfo->AddItem(new PropItem(_T("Constrained Intra Pred"), pps.constrained_intra_pred_flag));
            ppsinfo->AddItem(new PropItem(_T("Redundant Pic Count Present"), pps.redundant_pic_cnt_present_flag));

            if(pps.more_rbsp_data_present)
            {
                // TODO maybe later
            }

            return 0;
        }

        return 0;
    }

	static int GetExtradata_Video(const CMediaType *pmt, PropItem *mtinfo)
	{
		// we can also parse out decoder specific info in some cases
		if (pmt->subtype == MEDIASUBTYPE_H264 || pmt->subtype == MEDIASUBTYPE_AVC1 || pmt->subtype == MEDIASUBTYPE_MC_H264) {
			GetExtradata_H264(pmt, mtinfo);
		}
		else if (pmt->subtype == MEDIASUBTYPE_MPEG2_VIDEO || pmt->subtype == MEDIASUBTYPE_MPEG1Video) {
			GetExtradata_MPEGVideo(pmt, mtinfo);
		}
		else if (pmt->subtype == MEDIASUBTYPE_HVC1 || pmt->subtype == MEDIASUBTYPE_hvc1 || pmt->subtype == MEDIASUBTYPE_MC_H265 ||
			pmt->subtype == MEDIASUBTYPE_HEVC || pmt->subtype == MEDIASUBTYPE_hevc ||
			pmt->subtype == MEDIASUBTYPE_H265 || pmt->subtype == MEDIASUBTYPE_h265) {
			GetExtradata_H265(pmt, mtinfo);
		}
		return 0;
	}

	static uint8 * GetExtradataFromMediaType(const CMediaType * pmt, int & extralen)
	{
		uint8 * extra = NULL;
		if (pmt->formattype == FORMAT_MPEG2Video)
		{
			const MPEG2VIDEOINFO * const m2vi = (MPEG2VIDEOINFO*)pmt->pbFormat;
			extralen = m2vi->cbSequenceHeader;
			extra = (uint8*)m2vi->dwSequenceHeader;
		}
		else if (pmt->formattype == FORMAT_VIDEOINFO2)
		{
			extralen = pmt->cbFormat - sizeof(VIDEOINFOHEADER2);
			extra = pmt->pbFormat + sizeof(VIDEOINFOHEADER2);
		}
		else if (pmt->formattype == FORMAT_VideoInfo)
		{
			extralen = pmt->cbFormat - sizeof(VIDEOINFOHEADER);
			extra = pmt->pbFormat + sizeof(VIDEOINFOHEADER);
		}
		return extra;
	}

    int GetExtradata_H264(const CMediaType *pmt, PropItem *mtinfo)
	{
        if (pmt->pbFormat == NULL)
			return 0;

        int			extralen = 0;
		uint8*      extra = GetExtradataFromMediaType(pmt, extralen);

		// done with
		if (extralen <= 0) return 0;

        CBitStreamReader br(extra, extralen);
        int lastNullBytes = 0;

        if (pmt->subtype != MEDIASUBTYPE_AVC1)
        {
            // extradata uses NALU Startcode

            while (br.GetPos() < extralen-4)
            {
                BYTE val = br.ReadU8();

                if (val == 0) lastNullBytes++;
                else if (val == 1 && lastNullBytes >= 3)
                {
					H264NALParser(br, mtinfo);
                    // zum vollen byte springen
                    br.SetPos(br.GetPos());
                    lastNullBytes = 0;
                }
                else
                    lastNullBytes = 0;
            }
        }
        else
        {
            // extradata uses 2-Bytes for the length
            while (br.GetPos() < extralen-4)
            {
                WORD size = br.ReadU16();
                SSIZE_T pos = br.GetPos();
				H264NALParser(br, mtinfo);
                br.SetPos(pos + size);
            }
        }

		return 0;
	}

	static CString H265ProfileToString(uint8 general_profile_idc)
	{
		CString profile;
		profile.Format(_T("%d"), general_profile_idc);
		switch (general_profile_idc)
		{
		case 0: profile += " => Main"; break;
		case 1: profile += " => Main 10"; break;
		case 2: profile += " => Main Still Picture"; break;
		default: break;
		}
		return profile;
	}

	static CString H265LevelToString(uint8 general_level_idc)
	{
		CString level;
		if (general_level_idc % 30)
		{
			level.Format(_T("%d => %d.%d"), general_level_idc, general_level_idc / 30, (general_level_idc % 30) / 3);
		}
		else
		{
			level.Format(_T("%d => %d"), general_level_idc, general_level_idc / 30);
		}
		return level;
	}

	static CString H265ChromaFormatToString(uint8 chromaFormat)
	{
		CString chroma;
		chroma.Format(_T("%d"), chromaFormat);
		switch (chromaFormat)
		{
		case 0: chroma += " => monochrome"; break;
		case 1: chroma += " => 4:2:0"; break;
		case 2: chroma += " => 4:2:2"; break;
		case 3: chroma += " => 4:4:4"; break;
		default: break;
		}
		return chroma;
	}

	static void FillH265ProfileTierLevelInfo(PropItem * info, const h265ptl_t& ptl)
	{
		CString	 general_profile_compatibility_flag_hex;
		general_profile_compatibility_flag_hex.Format(_T("0x%08x"), ptl.general_profile_compatibility_flag);

		info->AddItem(new PropItem(_T("general_profile_space"), ptl.general_profile_space));
		info->AddItem(new PropItem(_T("general_tier_flag"), ptl.general_tier_flag));
		info->AddItem(new PropItem(_T("general_profile_idc"), H265ProfileToString(ptl.general_profile_idc)));
		info->AddItem(new PropItem(_T("general_profile_compatibility_flag"), general_profile_compatibility_flag_hex));
		info->AddItem(new PropItem(_T("general_progressive_source_flag"), ptl.general_progressive_source_flag));
		info->AddItem(new PropItem(_T("general_interlaced_source_flag"), ptl.general_interlaced_source_flag));
		info->AddItem(new PropItem(_T("general_non_packed_constraint_flag"), ptl.general_non_packed_constraint_flag));
		info->AddItem(new PropItem(_T("general_frame_only_constraint_flag"), ptl.general_frame_only_constraint_flag));
		info->AddItem(new PropItem(_T("general_level_idc"), H265LevelToString(ptl.general_level_idc)));
		for (uint8 i(0); i < ptl.maxNumSubLayersMinus1; ++i)
		{
			info->AddItem(new PropItem(_T("sub_layer_profile_present_flag"), ptl.sub_layer_profile_present_flag[i]));
			info->AddItem(new PropItem(_T("sub_layer_level_present_flag"), ptl.sub_layer_level_present_flag[i]));
			if (ptl.sub_layer_profile_present_flag[i])
			{
				info->AddItem(new PropItem(_T("sub_layer_profile_space"), ptl.sub_layer_profile_space[i]));
				info->AddItem(new PropItem(_T("sub_layer_tier_flag"), ptl.sub_layer_tier_flag[i]));
				info->AddItem(new PropItem(_T("sub_layer_profile_idc"), H265ProfileToString(ptl.sub_layer_profile_idc[i])));
				info->AddItem(new PropItem(_T("sub_layer_profile_compatibility_flag"), ptl.sub_layer_profile_compatibility_flag[i]));
				info->AddItem(new PropItem(_T("sub_layer_progressive_source_flag"), ptl.sub_layer_progressive_source_flag[i]));
				info->AddItem(new PropItem(_T("sub_layer_interlaced_source_flag"), ptl.sub_layer_interlaced_source_flag[i]));
				info->AddItem(new PropItem(_T("sub_layer_non_packed_constraint_flag"), ptl.sub_layer_non_packed_constraint_flag[i]));
				info->AddItem(new PropItem(_T("sub_layer_frame_only_constraint_flag"), ptl.sub_layer_frame_only_constraint_flag[i]));
			}
			if (ptl.sub_layer_level_present_flag[i])
			{
				info->AddItem(new PropItem(_T("sub_layer_level_idc"), H265LevelToString(ptl.sub_layer_level_idc[i])));
			}
		}
	}

	static void FillH265VPSInfo(PropItem * vpsinfo, const h265vps_t& vps)
	{
		FillH265ProfileTierLevelInfo(vpsinfo, vps.ptl);

		vpsinfo->AddItem(new PropItem(_T("video_parameter_set_id"), vps.vps_video_parameter_set_id));
		vpsinfo->AddItem(new PropItem(_T("max_layers_minus1"), vps.vps_max_layers_minus1));
		vpsinfo->AddItem(new PropItem(_T("max_sub_layers_minus1"), vps.vps_max_sub_layers_minus1));
		vpsinfo->AddItem(new PropItem(_T("temporal_id_nesting_flag"), vps.vps_temporal_id_nesting_flag));
		vpsinfo->AddItem(new PropItem(_T("sub_layer_ordering_info_present_flag"), vps.vps_sub_layer_ordering_info_present_flag));
		for (uint8 i = (vps.vps_sub_layer_ordering_info_present_flag ? 0 : vps.vps_max_sub_layers_minus1); i < vps.vps_max_layers_minus1; ++i)
		{
			vpsinfo->AddItem(new PropItem(_T("max_dec_pic_buffering_minus1"), vps.vps_max_dec_pic_buffering_minus1[i]));
			vpsinfo->AddItem(new PropItem(_T("max_num_reorder_pics"), vps.vps_max_num_reorder_pics[i]));
			vpsinfo->AddItem(new PropItem(_T("max_latency_increase_plus1"), vps.vps_max_latency_increase_plus1[i]));
		}
		vpsinfo->AddItem(new PropItem(_T("max_layer_id"), vps.vps_max_layer_id));
		vpsinfo->AddItem(new PropItem(_T("num_layer_sets_minus1"), vps.vps_num_layer_sets_minus1));
		vpsinfo->AddItem(new PropItem(_T("timing_info_present_flag"), vps.vps_timing_info_present_flag));
		if (vps.vps_timing_info_present_flag)
		{
			vpsinfo->AddItem(new PropItem(_T("num_units_in_tick"), vps.vps_num_units_in_tick));
			vpsinfo->AddItem(new PropItem(_T("time_scale"), vps.vps_time_scale));
			vpsinfo->AddItem(new PropItem(_T("poc_proportional_to_timing_flag"), vps.vps_poc_proportional_to_timing_flag));
			if (vps.vps_poc_proportional_to_timing_flag)
			{
				vpsinfo->AddItem(new PropItem(_T("num_ticks_poc_diff_one_minus1"), vps.vps_num_ticks_poc_diff_one_minus1));
			}
			vpsinfo->AddItem(new PropItem(_T("num_hrd_parameters"), vps.vps_num_hrd_parameters));
			for (uint32 i(0); i < vps.vps_num_hrd_parameters; ++i)
			{
				vpsinfo->AddItem(new PropItem(_T("hrd_layer_set_idx"), vps.hrd_layer_set_idx[i]));
				if (i > 0)
				{
					vpsinfo->AddItem(new PropItem(_T("cprms_present_flag"), vps.cprms_present_flag[i]));
				}
			}
		}
		vpsinfo->AddItem(new PropItem(_T("extension_flag"), vps.vps_extension_flag));
	}

	static void FillH265SPSInfo(PropItem * spsinfo, const h265sps_t& sps)
	{
		FillH265ProfileTierLevelInfo(spsinfo, sps.ptl);

		spsinfo->AddItem(new PropItem(_T("video_parameter_set_id"), sps.sps_video_parameter_set_id));
		spsinfo->AddItem(new PropItem(_T("max_sub_layers_minus1"), sps.sps_max_sub_layers_minus1));
		spsinfo->AddItem(new PropItem(_T("temporal_id_nesting_flag"), sps.sps_temporal_id_nesting_flag));
		spsinfo->AddItem(new PropItem(_T("seq_parameter_set_id"), sps.sps_seq_parameter_set_id));
		spsinfo->AddItem(new PropItem(_T("chroma_format_idc"), H265ChromaFormatToString(sps.chroma_format_idc)));
		if (sps.chroma_format_idc == 3) spsinfo->AddItem(new PropItem(_T("separate_colour_plane_flag"), sps.separate_colour_plane_flag));
		spsinfo->AddItem(new PropItem(_T("pic_width_in_luma_samples"), sps.pic_width_in_luma_samples));
		spsinfo->AddItem(new PropItem(_T("pic_height_in_luma_samples"), sps.pic_height_in_luma_samples));
		spsinfo->AddItem(new PropItem(_T("conformance_window_flag"), sps.conformance_window_flag));
		if (sps.conformance_window_flag)
		{
			RECT comformance_window = { sps.conf_win_left_offset, sps.conf_win_top_offset, sps.conf_win_right_offset, sps.conf_win_bottom_offset };
			spsinfo->AddItem(new PropItem(_T("conformance_window"), comformance_window));
		}
		spsinfo->AddItem(new PropItem(_T("bit_depth_luma_minus8"), sps.bit_depth_luma_minus8));
		spsinfo->AddItem(new PropItem(_T("bit_depth_chroma_minus8"), sps.bit_depth_chroma_minus8));
		spsinfo->AddItem(new PropItem(_T("log2_max_pic_order_cnt_lsb_minus4"), sps.log2_max_pic_order_cnt_lsb_minus4));
		spsinfo->AddItem(new PropItem(_T("sub_layer_ordering_info_present_flag"), sps.sps_sub_layer_ordering_info_present_flag));
		for (uint8 i = (sps.sps_sub_layer_ordering_info_present_flag ? 0 : sps.sps_max_sub_layers_minus1); i <= sps.sps_max_sub_layers_minus1; ++i)
		{
			spsinfo->AddItem(new PropItem(_T("max_dec_pic_buffering_minus1"), sps.sps_max_dec_pic_buffering_minus1[i]));
			spsinfo->AddItem(new PropItem(_T("max_num_reorder_pics"), sps.sps_max_num_reorder_pics[i]));
			spsinfo->AddItem(new PropItem(_T("max_latency_increase_plus1"), sps.sps_max_latency_increase_plus1[i]));
		}
		spsinfo->AddItem(new PropItem(_T("log2_min_luma_coding_block_size_minus3"), sps.log2_min_luma_coding_block_size_minus3));
		spsinfo->AddItem(new PropItem(_T("log2_diff_max_min_luma_coding_block_size"), sps.log2_diff_max_min_luma_coding_block_size));
		spsinfo->AddItem(new PropItem(_T("log2_min_transform_block_size_minus2"), sps.log2_min_transform_block_size_minus2));
		spsinfo->AddItem(new PropItem(_T("log2_diff_max_min_transform_block_size"), sps.log2_diff_max_min_transform_block_size));
		spsinfo->AddItem(new PropItem(_T("max_transform_hierarchy_depth_inter"), sps.max_transform_hierarchy_depth_inter));
		spsinfo->AddItem(new PropItem(_T("max_transform_hierarchy_depth_intra"), sps.max_transform_hierarchy_depth_intra));
		spsinfo->AddItem(new PropItem(_T("scaling_list_enabled_flag"), sps.scaling_list_enabled_flag));
		if (sps.scaling_list_enabled_flag)
		{
			spsinfo->AddItem(new PropItem(_T("scaling_list_data_present_flag"), sps.sps_scaling_list_data_present_flag));
		}
		spsinfo->AddItem(new PropItem(_T("amp_enabled_flag"), sps.amp_enabled_flag));
		spsinfo->AddItem(new PropItem(_T("sample_adaptive_offset_enabled_flag"), sps.sample_adaptive_offset_enabled_flag));
		spsinfo->AddItem(new PropItem(_T("pcm_enabled_flag"), sps.pcm_enabled_flag));
		if (sps.pcm_enabled_flag)
		{
			spsinfo->AddItem(new PropItem(_T("pcm_sample_bit_depth_luma_minus1"), sps.pcm_sample_bit_depth_luma_minus1));
			spsinfo->AddItem(new PropItem(_T("pcm_sample_bit_depth_chroma_minus1"), sps.pcm_sample_bit_depth_chroma_minus1));
			spsinfo->AddItem(new PropItem(_T("log2_min_pcm_luma_coding_block_size_minus3"), sps.log2_min_pcm_luma_coding_block_size_minus3));
			spsinfo->AddItem(new PropItem(_T("log2_diff_max_min_pcm_luma_coding_block_size"), sps.log2_diff_max_min_pcm_luma_coding_block_size));
			spsinfo->AddItem(new PropItem(_T("pcm_loop_filter_disabled_flag"), sps.pcm_loop_filter_disabled_flag));
		}
		spsinfo->AddItem(new PropItem(_T("num_short_term_ref_pic_sets"), sps.num_short_term_ref_pic_sets));
		spsinfo->AddItem(new PropItem(_T("long_term_ref_pics_present_flag"), sps.long_term_ref_pics_present_flag));
		if (sps.long_term_ref_pics_present_flag)
		{
			spsinfo->AddItem(new PropItem(_T("num_long_term_ref_pics_sps"), sps.num_long_term_ref_pics_sps));
			for (uint32 i(0); i < sps.num_long_term_ref_pics_sps; ++i)
			{
				spsinfo->AddItem(new PropItem(_T("lt_ref_pic_poc_lsb_sps"), sps.lt_ref_pic_poc_lsb_sps[i]));
				spsinfo->AddItem(new PropItem(_T("used_by_curr_pic_lt_sps_flag"), sps.used_by_curr_pic_lt_sps_flag[i]));
			}
		}
		spsinfo->AddItem(new PropItem(_T("temporal_mvp_enabled_flag"), sps.sps_temporal_mvp_enabled_flag));
		spsinfo->AddItem(new PropItem(_T("strong_intra_smoothing_enabled_flag"), sps.strong_intra_smoothing_enabled_flag));
		spsinfo->AddItem(new PropItem(_T("vui_parameters_present_flag"), sps.vui_parameters_present_flag));
		spsinfo->AddItem(new PropItem(_T("extension_flag"), sps.sps_extension_flag));
	}

	static void FillH265PPSInfo(PropItem * ppsinfo, const h265pps_t& pps)
	{
		ppsinfo->AddItem(new PropItem(_T("pic_parameter_set_id"), pps.pps_pic_parameter_set_id));
		ppsinfo->AddItem(new PropItem(_T("seq_parameter_set_id"), pps.pps_seq_parameter_set_id));
		ppsinfo->AddItem(new PropItem(_T("dependent_slice_segments_enabled_flag"), pps.dependent_slice_segments_enabled_flag));
		ppsinfo->AddItem(new PropItem(_T("output_flag_present_flag"), pps.output_flag_present_flag));
		ppsinfo->AddItem(new PropItem(_T("num_extra_slice_header_bits"), pps.num_extra_slice_header_bits));
		ppsinfo->AddItem(new PropItem(_T("sign_data_hiding_enabled_flag"), pps.sign_data_hiding_enabled_flag));
		ppsinfo->AddItem(new PropItem(_T("cabac_init_present_flag"), pps.cabac_init_present_flag));
		ppsinfo->AddItem(new PropItem(_T("num_ref_idx_l0_default_active_minus1"), pps.num_ref_idx_l0_default_active_minus1));
		ppsinfo->AddItem(new PropItem(_T("num_ref_idx_l1_default_active_minus1"), pps.num_ref_idx_l1_default_active_minus1));
		ppsinfo->AddItem(new PropItem(_T("init_qp_minus26"), pps.init_qp_minus26));
		ppsinfo->AddItem(new PropItem(_T("constrained_intra_pred_flag"), pps.constrained_intra_pred_flag));
		ppsinfo->AddItem(new PropItem(_T("transform_skip_enabled_flag"), pps.transform_skip_enabled_flag));
		ppsinfo->AddItem(new PropItem(_T("cu_qp_delta_enabled_flag"), pps.cu_qp_delta_enabled_flag));
		if (pps.cu_qp_delta_enabled_flag) ppsinfo->AddItem(new PropItem(_T("diff_cu_qp_delta_depth"), pps.diff_cu_qp_delta_depth));

		ppsinfo->AddItem(new PropItem(_T("cb_qp_offset"), pps.pps_cb_qp_offset));
		ppsinfo->AddItem(new PropItem(_T("cr_qp_offset"), pps.pps_cr_qp_offset));
		ppsinfo->AddItem(new PropItem(_T("slice_chroma_qp_offsets_present_flag"), pps.pps_slice_chroma_qp_offsets_present_flag));
		ppsinfo->AddItem(new PropItem(_T("weighted_pred_flag"), pps.weighted_pred_flag));
		ppsinfo->AddItem(new PropItem(_T("weighted_bipred_flag"), pps.weighted_bipred_flag));
		ppsinfo->AddItem(new PropItem(_T("transquant_bypass_enabled_flag"), pps.transquant_bypass_enabled_flag));
		ppsinfo->AddItem(new PropItem(_T("tiles_enabled_flag"), pps.tiles_enabled_flag));
		ppsinfo->AddItem(new PropItem(_T("entropy_coding_sync_enabled_flag"), pps.entropy_coding_sync_enabled_flag));
		if (pps.tiles_enabled_flag)
		{
			ppsinfo->AddItem(new PropItem(_T("num_tile_columns_minus1"), pps.num_tile_columns_minus1));
			ppsinfo->AddItem(new PropItem(_T("num_tile_rows_minus1"), pps.num_tile_rows_minus1));
			ppsinfo->AddItem(new PropItem(_T("uniform_spacing_flag"), pps.uniform_spacing_flag));
			if (!pps.uniform_spacing_flag)
			{
				for (uint32 i(0); i < pps.num_tile_columns_minus1; i++) ppsinfo->AddItem(new PropItem(_T("column_width_minus1"), pps.column_width_minus1[i]));
				for (uint32 i(0); i < pps.num_tile_rows_minus1; i++) ppsinfo->AddItem(new PropItem(_T("row_height_minus1"), pps.row_height_minus1[i]));
			}
			ppsinfo->AddItem(new PropItem(_T("loop_filter_across_tiles_enabled_flag"), pps.loop_filter_across_tiles_enabled_flag));
		}
		ppsinfo->AddItem(new PropItem(_T("loop_filter_across_slices_enabled_flag"), pps.pps_loop_filter_across_slices_enabled_flag));
		ppsinfo->AddItem(new PropItem(_T("deblocking_filter_control_present_flag"), pps.deblocking_filter_control_present_flag));
		if (pps.deblocking_filter_control_present_flag)
		{
			ppsinfo->AddItem(new PropItem(_T("deblocking_filter_override_enabled_flag"), pps.deblocking_filter_override_enabled_flag));
			ppsinfo->AddItem(new PropItem(_T("deblocking_filter_disabled_flag"), pps.pps_deblocking_filter_disabled_flag));
			if (!pps.pps_deblocking_filter_disabled_flag)
			{
				ppsinfo->AddItem(new PropItem(_T("beta_offset_div2"), pps.pps_beta_offset_div2));
				ppsinfo->AddItem(new PropItem(_T("tc_offset_div2"), pps.pps_tc_offset_div2));
			}
		}
		ppsinfo->AddItem(new PropItem(_T("scaling_list_data_present_flag"), pps.pps_scaling_list_data_present_flag));
		ppsinfo->AddItem(new PropItem(_T("lists_modification_present_flag"), pps.lists_modification_present_flag));
		ppsinfo->AddItem(new PropItem(_T("log2_parallel_merge_level_minus2"), pps.log2_parallel_merge_level_minus2));
		ppsinfo->AddItem(new PropItem(_T("slice_segment_header_extension_present_flag"), pps.slice_segment_header_extension_present_flag));
		ppsinfo->AddItem(new PropItem(_T("extension_flag"), pps.pps_extension_flag));
	}

	static void FillH265VUIInfo(PropItem * vuiinfo, const h265vui_t& vui)
	{
		vuiinfo->AddItem(new PropItem(_T("aspect_ratio_info_present_flag"), vui.aspect_ratio_info_present_flag));
		if (vui.aspect_ratio_info_present_flag)
		{
			vuiinfo->AddItem(new PropItem(_T("aspect_ratio_idc"), vui.aspect_ratio_idc));
		}
		vuiinfo->AddItem(new PropItem(_T("overscan_info_present_flag"), vui.overscan_info_present_flag));
		if (vui.overscan_info_present_flag) vuiinfo->AddItem(new PropItem(_T("overscan_appropriate_flag"), vui.overscan_appropriate_flag));
		vuiinfo->AddItem(new PropItem(_T("video_signal_type_present_flag"), vui.video_signal_type_present_flag));
		if (vui.video_signal_type_present_flag)
		{
			vuiinfo->AddItem(new PropItem(_T("video_format"), vui.video_format));
			vuiinfo->AddItem(new PropItem(_T("video_full_range_flag"), vui.video_full_range_flag));
			vuiinfo->AddItem(new PropItem(_T("colour_description_present_flag"), vui.colour_description_present_flag));
			if (vui.colour_description_present_flag)
			{
				vuiinfo->AddItem(new PropItem(_T("colour_primaries"), vui.colour_primaries));
				vuiinfo->AddItem(new PropItem(_T("transfer_characteristics"), vui.transfer_characteristics));
				vuiinfo->AddItem(new PropItem(_T("matrix_coeffs"), vui.matrix_coeffs));
			}
		}
		vuiinfo->AddItem(new PropItem(_T("chroma_loc_info_present_flag"), vui.chroma_loc_info_present_flag));
		if (vui.chroma_loc_info_present_flag)
		{
			vuiinfo->AddItem(new PropItem(_T("chroma_sample_loc_type_top_field"), vui.chroma_sample_loc_type_top_field));
			vuiinfo->AddItem(new PropItem(_T("chroma_sample_loc_type_bottom_field"), vui.chroma_sample_loc_type_bottom_field));
		}
		vuiinfo->AddItem(new PropItem(_T("neutral_chroma_indication_flag"), vui.neutral_chroma_indication_flag));
		vuiinfo->AddItem(new PropItem(_T("field_seq_flag"), vui.field_seq_flag));
		vuiinfo->AddItem(new PropItem(_T("frame_field_info_present_flag"), vui.frame_field_info_present_flag));
		vuiinfo->AddItem(new PropItem(_T("default_display_window_flag"), vui.default_display_window_flag));
		if (vui.default_display_window_flag)
		{
			RECT default_display_window = { vui.def_disp_win_left_offset, vui.def_disp_win_top_offset, vui.def_disp_win_right_offset, vui.def_disp_win_bottom_offset };
			vuiinfo->AddItem(new PropItem(_T("default_display_window"), default_display_window));
		}
		vuiinfo->AddItem(new PropItem(_T("timing_info_present_flag"), vui.vui_timing_info_present_flag));
		if (vui.vui_timing_info_present_flag)
		{
			vuiinfo->AddItem(new PropItem(_T("num_units_in_tick"), vui.vui_num_units_in_tick));
			vuiinfo->AddItem(new PropItem(_T("time_scale"), vui.vui_time_scale));
			vuiinfo->AddItem(new PropItem(_T("poc_proportional_to_timing_flag"), vui.vui_poc_proportional_to_timing_flag));
			if (vui.vui_poc_proportional_to_timing_flag)
			{
				vuiinfo->AddItem(new PropItem(_T("num_ticks_poc_diff_one_minus1"), vui.vui_num_ticks_poc_diff_one_minus1));
				vuiinfo->AddItem(new PropItem(_T("hrd_parameters_present_flag"), vui.vui_hrd_parameters_present_flag));
			}
			vuiinfo->AddItem(new PropItem(_T("bitstream_restriction_flag"), vui.bitstream_restriction_flag));
			if (vui.bitstream_restriction_flag)
			{
				vuiinfo->AddItem(new PropItem(_T("tiles_fixed_structure_flag"), vui.tiles_fixed_structure_flag));
				vuiinfo->AddItem(new PropItem(_T("motion_vectors_over_pic_boundaries_flag"), vui.motion_vectors_over_pic_boundaries_flag));
				vuiinfo->AddItem(new PropItem(_T("restricted_ref_pic_lists_flag"), vui.restricted_ref_pic_lists_flag));
				vuiinfo->AddItem(new PropItem(_T("min_spatial_segmentation_idc"), vui.min_spatial_segmentation_idc));
				vuiinfo->AddItem(new PropItem(_T("max_bytes_per_pic_denom"), vui.max_bytes_per_pic_denom));
				vuiinfo->AddItem(new PropItem(_T("max_bits_per_min_cu_denom"), vui.max_bits_per_min_cu_denom));
				vuiinfo->AddItem(new PropItem(_T("log2_max_mv_length_horizontal"), vui.log2_max_mv_length_horizontal));
				vuiinfo->AddItem(new PropItem(_T("log2_max_mv_length_vertical"), vui.log2_max_mv_length_vertical));
			}
		}
	}

	int GetExtradata_H265(const CMediaType *pmt, PropItem *mtinfo)
	{
		if (pmt->pbFormat == NULL)
			return 0;

		int			extralen = 0;
		uint8*      extra = GetExtradataFromMediaType(pmt, extralen);

		// done with
		if (extralen <= 0) return 0;

		std::vector<h265vps_t> VPSes;
		std::vector<h265sps_t> SPSes;
		std::vector<h265pps_t> PPSes;

		if (pmt->subtype == MEDIASUBTYPE_HVC1 || pmt->subtype == MEDIASUBTYPE_hvc1)
		{
			if (extralen < sizeof(HEVCDecoderConfigurationRecord))
			{
				return 0;
			}
			HEVCDecoderConfigurationRecord * config = reinterpret_cast<HEVCDecoderConfigurationRecord*>(extra);
			PropItem *hevcinfo = mtinfo->AddItem(new PropItem(_T("HEVC Decoder Configuration Record")));
			hevcinfo->AddItem(new PropItem(_T("Configuration Version"), config->configurationVersion));
			hevcinfo->AddItem(new PropItem(_T("general_profile_space"), config->general_profile_space));
			hevcinfo->AddItem(new PropItem(_T("general_tier_flag"), config->general_tier_flag));
			hevcinfo->AddItem(new PropItem(_T("general profile idc"), H265ProfileToString(config->general_profile_idc)));
			hevcinfo->AddItem(new PropItem(_T("general level idc"), H265LevelToString(config->general_level_idc)));
			hevcinfo->AddItem(new PropItem(_T("min_spatial_segmentation_idc"), config->min_spatial_segmentation_idc));
			hevcinfo->AddItem(new PropItem(_T("parallelismType"), config->parallelismType));
			hevcinfo->AddItem(new PropItem(_T("Chroma Format"), H265ChromaFormatToString(config->chromaFormat)));
			hevcinfo->AddItem(new PropItem(_T("bitDepthLumaMinus8"), config->bitDepthLumaMinus8));
			hevcinfo->AddItem(new PropItem(_T("bitDepthChromaMinus8"), config->bitDepthChromaMinus8));
			hevcinfo->AddItem(new PropItem(_T("avgFrameRate"), config->avgFrameRate));
			hevcinfo->AddItem(new PropItem(_T("constantFrameRate"), config->constantFrameRate));
			hevcinfo->AddItem(new PropItem(_T("numTemporalLayers"), config->numTemporalLayers));
			hevcinfo->AddItem(new PropItem(_T("temporalIdNested"), config->temporalIdNested));
			hevcinfo->AddItem(new PropItem(_T("lengthSizeMinusOne"), config->lengthSizeMinusOne));
			hevcinfo->AddItem(new PropItem(_T("numOfArrays"), config->numOfArrays));

			int offset = sizeof(HEVCDecoderConfigurationRecord);
			CBitStreamReader br(extra + offset, extralen - offset, false);

			for (uint8 i(0); i < config->numOfArrays; ++i)
			{
				if ((offset += 3) > extralen) return 0;
				br.SkipU(2);
				uint8 NAL_unit_type = br.ReadU(6);
				uint16 numNALUs = br.ReadU16();

				for (uint16 j(0); j < numNALUs; ++j)
				{
					if ((offset += 2) > extralen) return 0;
					uint16 length = br.ReadU16();
					if (offset + length > extralen) return 0;

					CBitStreamReader nalr(extra + offset, length);
					offset += length;
					
					// 7.3.1.2 NAL unit header syntax
					nalr.SkipU1(); // forbidden_zero_bit
					uint8 nal_unit_type = nalr.ReadU(6);
					uint8 nuh_layer_id = nalr.ReadU(6);
					uint8 nuh_temporal_id_plus1 = nalr.ReadU(3);

					switch (NAL_unit_type)
					{
					case kH265VPSNUT: { h265vps_t vps; CH265StructReader::ReadVPS(nalr, vps); VPSes.push_back(vps); } break;
					case kH265SPSNUT: { h265sps_t sps; CH265StructReader::ReadSPS(nalr, sps); SPSes.push_back(sps); } break;
					case kH265PPSNUT: { h265pps_t pps; CH265StructReader::ReadPPS(nalr, pps); PPSes.push_back(pps); } break;
					default: break;
					}
					br.SkipU8(length);
				}
			}
		}
		if (pmt->subtype == MEDIASUBTYPE_MC_H265)
		{
			// Annex B format
			CBitStreamReader br(extra, extralen);
			int lastNullBytes = 0;

			while (br.GetPos() < extralen - 4)
			{
				BYTE val = br.ReadU8();

				if (val == 0) lastNullBytes++;
				else if (val == 1 && lastNullBytes >= 3)
				{
					// 7.3.1.2 NAL unit header syntax
					br.SkipU1(); // forbidden_zero_bit
					uint8 nal_unit_type = br.ReadU(6);
					uint8 nuh_layer_id = br.ReadU(6);
					uint8 nuh_temporal_id_plus1 = br.ReadU(3);

					switch (nal_unit_type)
					{
					case kH265VPSNUT: { h265vps_t vps; CH265StructReader::ReadVPS(br, vps); VPSes.push_back(vps); } break;
					case kH265SPSNUT: { h265sps_t sps; CH265StructReader::ReadSPS(br, sps); SPSes.push_back(sps); } break;
					case kH265PPSNUT: { h265pps_t pps; CH265StructReader::ReadPPS(br, pps); PPSes.push_back(pps); } break;
					default: break;
					}

					br.SetPos(br.GetPos());

					lastNullBytes = 0;
				}
				else
					lastNullBytes = 0;
			}
		}
		if (pmt->subtype == MEDIASUBTYPE_HEVC || pmt->subtype == MEDIASUBTYPE_hevc || pmt->subtype == MEDIASUBTYPE_H265 || pmt->subtype == MEDIASUBTYPE_h265)
		{
			// NALU format
			CBitStreamReader br(extra, extralen, false);

			while (br.GetPos() < extralen - 4)
			{
				uint16 size = br.ReadU16();
				SSIZE_T pos = br.GetPos();
				// 7.3.1.2 NAL unit header syntax
				br.SkipU1(); // forbidden_zero_bit
				uint8 nal_unit_type = br.ReadU(6);
				uint8 nuh_layer_id = br.ReadU(6);
				uint8 nuh_temporal_id_plus1 = br.ReadU(3);

				switch (nal_unit_type)
				{
				case kH265VPSNUT: { h265vps_t vps; CH265StructReader::ReadVPS(br, vps); VPSes.push_back(vps); } break;
				case kH265SPSNUT: { h265sps_t sps; CH265StructReader::ReadSPS(br, sps); SPSes.push_back(sps); } break;
				case kH265PPSNUT: { h265pps_t pps; CH265StructReader::ReadPPS(br, pps); PPSes.push_back(pps); } break;
				default: break;
				}
				br.SetPos(pos + size);
			}
		}
		for (std::vector<h265vps_t>::const_iterator vps = VPSes.begin(); vps != VPSes.end(); ++vps)
		{
			FillH265VPSInfo(mtinfo->AddItem(new PropItem(_T("HEVC Video Parameter Set (VPS)"))), *vps);
		}
		for (std::vector<h265sps_t>::const_iterator sps = SPSes.begin(); sps != SPSes.end(); ++sps)
		{
			FillH265SPSInfo(mtinfo->AddItem(new PropItem(_T("HEVC Sequence Parameter Set (SPS)"))), *sps);
			if (sps->vui_parameters_present_flag)
			{
				FillH265VUIInfo(mtinfo->AddItem(new PropItem(_T("HEVC Video Usability Information (VUI)"))), sps->vui);
			}
		}
		for (std::vector<h265pps_t>::const_iterator pps = PPSes.begin(); pps != PPSes.end(); ++pps)
		{
			FillH265PPSInfo(mtinfo->AddItem(new PropItem(_T("HEVC Picture Parameter Set (PPS)"))), *pps);
		}
		return 0;
	}

    int GetExtradata_MPEGVideo(const CMediaType *pmt, PropItem *mtinfo)
	{
        if (pmt->pbFormat == NULL)
			return 0;

        int			extralen = 0;
		uint8*      extra = GetExtradataFromMediaType(pmt, extralen);

        bool isMpeg1 = false;

		// done with
		if (extralen <= 0) return 0;

        CBitStreamReader br(extra, extralen);
        
        // prefix lesen 0x000001b3
        UINT16 prefix = br.ReadU16();
        if (prefix != 0) return 0;
        prefix = br.ReadU8();
        if (prefix != 1) return 0;
        prefix = br.ReadU8();

        if (prefix == 0xB3) // MPEG1Seq
        {
            // http://www.fr-an.de/projects/01/01_01_02.htm
            PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Sequence Header")));
            
            UINT16 width = br.ReadU(12);
            shinfo->AddItem(new PropItem(_T("Width"), width));

            UINT16 height = br.ReadU(12);
            shinfo->AddItem(new PropItem(_T("Height"), height));

            // aspect
            UINT8 aspect = br.ReadU(4);
            if (isMpeg1)
            {
                static CString aspectValues[] =
                {
                    _T("forbidden (0)"),
                    _T("square pixels (1)"),
                    _T("0.6735 (2)"),
                    _T("16:9, 625 line, PAL (3)"),
                    _T("0.7615 (4)"),
                    _T("0.8055 (5)"),
                    _T("16:9, 525 line, NTSC (6)"),
                    _T("0.8935 (7)"),
                    _T("4:3, 625 line, PAL, CCIR601 (8)"),
                    _T("0.9815 (9)"),
                    _T("1.0255 (10)"),
                    _T("1.0695 (11)"),
                    _T("4:3, 525 line, NTSC, CCIR601 (12)"),
                    _T("1.1575 (13)"),
                    _T("1.2015 (14)"),
                    _T("reserved (15)")
                };
                shinfo->AddItem(new PropItem(_T("Aspect Ratio"), aspectValues[aspect]));
            }
            else
            {
                static CString aspectValues[] =
                {
                    _T("forbidden (0)"),
                    _T("1:1 square pixels (1)"),
                    _T("4:3 display (2)"),
                    _T("16:9 display (3)"),
                    _T("2.21:1 (4)")
                };

                CString arVal;
                if(aspect > 4)
                    arVal.Format(_T("reserved (%d)"), aspect);
                else
                    arVal = aspectValues[aspect];
                shinfo->AddItem(new PropItem(_T("Aspect Ratio"), arVal));
            }

            // Framerate
            UINT8 framerate = br.ReadU(4);
            static CString framerateValues[] =
            {
                _T("forbidden (0)"),
                _T("24000/1001.0 => 23.976 fps -- NTSC encapsulated film rate (1)"),
                _T("24.0 => Standard international cinema film rate (2)"),
                _T("25.0 => PAL (625/50) video frame rate (3)"),
                _T("30000/1001.0 => 29.97 fps -- NTSC video frame rate (4)"),
                _T("30.0 => NTSC drop-frame (525/60) video frame rate (5)"),
                _T("50.0 => double frame rate/progressive PAL (6)"),
                _T("60000/1001.0 => double frame rate NTSC (7)"),
                _T("60.0 => double frame rate drop-frame NTSC (8)")
            };
            CString frVal;
            if (framerate > 8)
                frVal.Format(_T("reserved (%d)"), framerate);
            else
                frVal = framerateValues[framerate];
            shinfo->AddItem(new PropItem(_T("Framerate"), frVal));

            // Bitrate
            UINT32 bitrate = br.ReadU(18);
            if (bitrate == 0x3FFFF)
                shinfo->AddItem(new PropItem(_T("Bitrate"), CString(_T("Variable (0x3FFFF)"))));
            else
            {
                bitrate = bitrate * 400;
                shinfo->AddItem(new PropItem(_T("Bitrate (Bit/s)"), bitrate));
            }

            // Marker
            UINT8 marker = br.ReadU1();
            if (!marker)
                shinfo->AddItem(new PropItem(_T("Marker"), CString(_T("0 (Error in Stream!)"))));
            else
                shinfo->AddItem(new PropItem(_T("Marker"), 1));

            // VBV
            UINT32 vbv = br.ReadU(10);
            vbv = vbv * 16;
            shinfo->AddItem(new PropItem(_T("VBV (kB)"), vbv));

            UINT8 cpf = br.ReadU1();
            shinfo->AddItem(new PropItem(_T("Constrained Parameter Flag"), cpf));

            // Intra Matrix
            UINT8 lim = br.ReadU1();
            if (lim)
                shinfo->AddItem(new PropItem(_T("Load Intra Matrix"), CString(_T("Use Standard (1)"))));
            else
            {
                shinfo->AddItem(new PropItem(_T("Load Intra Matrix"), CString(_T("Load (0)"))));
                CString matrix;
                for(int i=0;i<64;i++)
                {
                    UINT8 m = br.ReadU8();
                    matrix.AppendFormat(_T("%02X "), m);
                }
                shinfo->AddItem(new PropItem(_T("Intra Matrix"), matrix));
            }

            // Non Intra Matrix
            UINT8 lnim = br.ReadU1();
            if (lnim)
                shinfo->AddItem(new PropItem(_T("Load Non Intra Matrix"), CString(_T("Use Standard (1)"))));
            else
            {
                shinfo->AddItem(new PropItem(_T("Load Non Intra Matrix"), CString(_T("Load (0)"))));
                CString matrix;
                for(int i=0;i<64;i++)
                {
                    UINT8 m = br.ReadU8();
                    matrix.AppendFormat(_T("%02X "), m);
                }
                shinfo->AddItem(new PropItem(_T("Non Intra Matrix"), matrix));
            }

            if (!br.ByteAligned())
                br.SetPos(br.GetPos()+1);

            while (!br.IsEnd())
            {
                if (br.ReadU8() == 0)
                {
                    if (br.ReadU8() == 0)
                    {
                        if (br.ReadU8() == 1)
                        {
                            prefix = br.ReadU8();
                            break;
                        }
                    }
                }
            }
        }
        
        if (prefix == 0xB5) // MPEG2Seq
        {
            UINT8 id = br.ReadU(4);
            if(id == 1)
            {
                PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Sequenz Extension")));

                // Profile
                UINT8 profile = br.ReadU(4);
                static CString profileValues[] =
                {
                    _T("reserved (0)"),
                    _T("High Profile (1)"),
                    _T("Spatially Scalable Profile (2)"),
                    _T("SNR Scalable Profile (3)"),
                    _T("Main Profile (4)"),
                    _T("Simple Profile (5)"),
                };

                CString profileVal;
                if(profile > 5)
                    profileVal.Format(_T("Unknown (%d)"), profile);
                else
                    profileVal = profileValues[profile];
                shinfo->AddItem(new PropItem(_T("Profile"), profileVal));

                // Level
                shinfo->AddItem(new PropItem(_T("Level"), br.ReadU(4)));
                shinfo->AddItem(new PropItem(_T("Progressive Sequence"), br.ReadU1()));

                // Chroma
                UINT8 chroma = br.ReadU(2);
                static CString chromaValues[] =
                {
                    _T("0"),
                    _T("4:2:0 (1)"),
                    _T("4:2:2 (2)"),
                    _T("4:4:4 (3)")
                };
                shinfo->AddItem(new PropItem(_T("Chroma Format"), chromaValues[chroma]));

                shinfo->AddItem(new PropItem(_T("Width Extension"), br.ReadU(2)));
                shinfo->AddItem(new PropItem(_T("Height Extension"), br.ReadU(2)));
                shinfo->AddItem(new PropItem(_T("Bitrate Extension"), br.ReadU(12)));
                shinfo->AddItem(new PropItem(_T("Marker"), br.ReadU1()));
                shinfo->AddItem(new PropItem(_T("VBV Extension"), br.ReadU(8)));
                shinfo->AddItem(new PropItem(_T("Low Delay"), br.ReadU(1)));
                shinfo->AddItem(new PropItem(_T("Framerate Extension Numerator"), br.ReadU(2)));
                shinfo->AddItem(new PropItem(_T("Framerate Extension Denominator"), br.ReadU(5)));
            }
            else if (id == 2)
            {
                PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Sequenz Display Extension")));
            }
            else if (id == 3)
            {
                PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Quant Matrix Extension")));
            }
            else if (id == 4)
            {
                PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Copyright Extension")));
            }
            else if (id == 5)
            {
                PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Sequence Scalable Extension")));
            }
        }

		return 0;
	}

	int GetFourCC(DWORD fcc, CString &str)
	{
		BYTE		*b = (BYTE*)&fcc;
		int			i;

		// first check that the characters are reasonable
		for (i=0; i<4; i++) {
			if (b[i] >= 32 &&			// space
				b[i] <= 126)			// ~
			{
				// continue
			} else {
				// we can't make nice fourcc string
				return -1;
			}
		}

		CStringA	ansi_str;

		ansi_str = "";
		for (i=0; i<4; i++) {
			char	c = b[i];
			ansi_str += c;
		}

		str = ansi_str;
		return 0;
	}

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation



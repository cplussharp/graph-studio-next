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

		CString		date;
		date = time_cr.Format(_T("%d/%b/%Y  %H:%M:%S"));
		filedetails->AddItem(new PropItem(_T("Created"), date));
		date = time_mod.Format(_T("%d/%b/%Y  %H:%M:%S"));
		filedetails->AddItem(new PropItem(_T("Modified"), date));

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

	void CLSIDToString(const CLSID& clsid, CString &str)
	{
		LPOLESTR	ostr = NULL;
		StringFromCLSID(clsid, &ostr);
		if (ostr) {
			str = CString(ostr);
			CoTaskMemFree(ostr);
		}
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
			ULONG		len;
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
			ULONG		len;
			reg.QueryStringValue(_T(""), val, &len);
			name = val;

			ret = 0;
			reg.Close();
		}

		return ret;
	}

	int GetFilterDetails(CLSID clsid, PropItem *info)
	{
		info->AddItem(new PropItem(_T("CLSID"), clsid));

		CString name = _T("Unknown");
		GetObjectName(clsid, name);
		info->AddItem(new PropItem(_T("Object Name"), name));

		// load binary data
		CString		clsid_str;
		CLSIDToString(clsid, clsid_str);
		clsid_str = _T("\\CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\") + clsid_str;

		DSUtil::FilterTemplate		temp;
		bool						loaded = false;

		CRegKey		reg;
		if (reg.Open(HKEY_CLASSES_ROOT, clsid_str, KEY_READ) == ERROR_SUCCESS) {
			
			// get binary data
			ULONG		size;
			if (reg.QueryBinaryValue(_T("FilterData"), NULL, &size) == ERROR_SUCCESS) {
				BYTE *buf = (BYTE*)malloc(size+1);
				reg.QueryBinaryValue(_T("FilterData"), buf, &size);

				// parse data
				int ret = temp.Load((char*)buf, size);
				if (ret == 0) {
					loaded = true;

					CString		val;
					val.Format(_T("0x%08x"), temp.merit);
					info->AddItem(new PropItem(_T("Merit"), val));
					val.Format(_T("0x%08x"), temp.version);
					info->AddItem(new PropItem(_T("Version"), val));

				}

				free(buf);
			}
	
			reg.Close();
		}

		CString		filename;
		GetObjectFile(clsid, filename);
		PropItem	*fileinfo = new PropItem(_T("File"));

		if (GetFileDetails(filename, fileinfo) < 0) {
			delete fileinfo;
		} else {
			info->AddItem(fileinfo);
		}

		// registered pin details
		if (loaded) {
			PropItem	*pin_details = info->AddItem(new PropItem(_T("Registered Pins")));
			int cnt = temp.input_pins.GetCount() + temp.output_pins.GetCount();
			pin_details->AddItem(new PropItem(_T("Count"), cnt));

			int i, c;
			c = 0;
			for (i=0; i<temp.input_pins.GetCount(); i++) {
				DSUtil::PinTemplate	&pin = temp.input_pins[i];
				CString		name;
				name.Format(_T("Pin %d"), c);
				PropItem	*pininfo = pin_details->AddItem(new PropItem(name));
				GetPinTemplateDetails(&pin, pininfo);
				c++;
			}
			for (i=0; i<temp.output_pins.GetCount(); i++) {
				DSUtil::PinTemplate	&pin = temp.output_pins[i];
				CString		name;
				name.Format(_T("Pin %d"), c);
				PropItem	*pininfo = pin_details->AddItem(new PropItem(name));
				GetPinTemplateDetails(&pin, pininfo);
				c++;
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
				CString		mn, sn, mv, sv;
				mn.Format(_T("Major %d"), i);
				sn.Format(_T("Subtype %d"), i);

				GraphStudio::NameGuid(pin->major[i],mv,CgraphstudioApp::g_showGuidsOfKnownTypes);
				GraphStudio::NameGuid(pin->minor[i],sv,CgraphstudioApp::g_showGuidsOfKnownTypes);

				types->AddItem(new PropItem(mn, mv));
				types->AddItem(new PropItem(sn, sv));
			}
		}
		return 0;
	}

	int GetPinDetails(IPin *pin, PropItem *info)
	{
		PropItem	*group;

		group = info->AddItem(new PropItem(_T("Pin")));
		
		PIN_INFO	pi;
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

            // we can also parse out decoder specific info in some cases
            if (pmt->subtype == MEDIASUBTYPE_H264 || pmt->subtype == MEDIASUBTYPE_AVC1 || pmt->subtype == MEDIASUBTYPE_MC_H264 ) {
				GetExtradata_H264(pmt, mtinfo);
            } else if(pmt->subtype == MEDIASUBTYPE_MPEG2_VIDEO || pmt->subtype == MEDIASUBTYPE_MPEG1Video) {
                GetExtradata_MPEGVideo(pmt, mtinfo);
            }
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
		wfxinfo->AddItem(new PropItem(_T("wSamplesPerBlock"), (int)wfx->Samples.wSamplesPerBlock));
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
		v.Format(_T("0x%08x"), vih->dwPictAspectRatioX);	vihinfo->AddItem(new PropItem(_T("dwPictAspectRatioX"), v));
		v.Format(_T("0x%08x"), vih->dwPictAspectRatioY);	vihinfo->AddItem(new PropItem(_T("dwPictAspectRatioY"), v));
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

    int NALParser(CBitStreamReader& br, PropItem *mtinfo)
    {
        UINT8 NALType = br.ReadU8() & 0x1f;

        if(NALType == 7)
        {
            PropItem *spsinfo = mtinfo->AddItem(new PropItem(_T("H264 Sequence Parameter Set")));
            int profile = br.ReadU8();
            spsinfo->AddItem(new PropItem(_T("Profile IDC"), profile));
            
            RECT constsetflags; // use rect only to save and show the 4 values
            constsetflags.left = br.ReadU1();
            constsetflags.top = br.ReadU1();
            constsetflags.right = br.ReadU1();
            constsetflags.bottom = br.ReadU1();
            br.ReadU(4);    // Zero
            spsinfo->AddItem(new PropItem(_T("Constraint Set Flags"), constsetflags));

            spsinfo->AddItem(new PropItem(_T("Level IDC"), br.ReadU8()));

            spsinfo->AddItem(new PropItem(_T("Seq Parameter Set ID"), br.ReadUE()));

            if(profile == 100 || profile == 110 || profile == 122 || profile == 144)
            {
                int chroma = br.ReadUE();
                spsinfo->AddItem(new PropItem(_T("Chroma Format IDC"), chroma));
                if(chroma == 3)
                    spsinfo->AddItem(new PropItem(_T("Residual Colour Transform"), br.ReadU1()));

                spsinfo->AddItem(new PropItem(_T("BitDepth Luma (-8)"), br.ReadUE()));
                spsinfo->AddItem(new PropItem(_T("BitDepth Chroma (-8)"), br.ReadUE()));
                spsinfo->AddItem(new PropItem(_T("QPPrime Y Zero Transform Bypass"), br.ReadU1()));

                int seq_scaling_matrix = br.ReadU1();
                spsinfo->AddItem(new PropItem(_T("Seq Scaling Matrix Present"), seq_scaling_matrix));
                if(seq_scaling_matrix)
                {
                    // TODO 
                    // http://h264bitstream.svn.sourceforge.net/viewvc/h264bitstream/trunk/h264bitstream/h264_stream.c?revision=24&view=markup
                    return 0;
                }
            }

            spsinfo->AddItem(new PropItem(_T("Log2 Max Frame Num (-4)"), br.ReadUE()));
                
            int pic_order = br.ReadUE();
            spsinfo->AddItem(new PropItem(_T("Pic Order Cnt Type"), pic_order));
            if(pic_order == 0)
                spsinfo->AddItem(new PropItem(_T("Log2 Max Pic Order Cnt Lsb (-4)"), br.ReadUE()));
            else
            {
                spsinfo->AddItem(new PropItem(_T("Delta Pic Order Always Zero"), br.ReadU1()));
                spsinfo->AddItem(new PropItem(_T("Offset For Non Ref Pic"), br.ReadSE()));
                spsinfo->AddItem(new PropItem(_T("Offset For Top To Bottom Field"), br.ReadSE()));

                int num_ref_frames = br.ReadUE();
                spsinfo->AddItem(new PropItem(_T("Num Ref Frames In Pic Order Cnt Cycle"), num_ref_frames));
                if(num_ref_frames > 0)
                {
                    CString str = _T("");
                    for(int i=0; i<num_ref_frames; i++)
                    {
                        if(!str.IsEmpty())
                            str.Append(_T(", "));

                        str.AppendFormat(_T("%d"), br.ReadSE());
                    }

                    spsinfo->AddItem(new PropItem(_T("Offset For Ref Frames"), str));
                }
            }

            spsinfo->AddItem(new PropItem(_T("Num Ref Frames"), br.ReadUE()));
            spsinfo->AddItem(new PropItem(_T("Gaps In Frame Num Value Allowed"), br.ReadU1()));
            spsinfo->AddItem(new PropItem(_T("Pic Width In Mbs (-1)"), br.ReadUE()));
            spsinfo->AddItem(new PropItem(_T("Pic Height In Map Units (-1)"), br.ReadUE()));

            int frame_mbs_only = br.ReadU1();
            spsinfo->AddItem(new PropItem(_T("Frame Mbs Only"), frame_mbs_only));
            if(!frame_mbs_only)
                spsinfo->AddItem(new PropItem(_T("Mb Adaptive Frame Field"), br.ReadU1()));

            spsinfo->AddItem(new PropItem(_T("Direct 8x8 Inference"), br.ReadU1()));

            if(br.ReadU1())
            {
                RECT rect;
                rect.left = br.ReadUE();
                rect.right = br.ReadUE();
                rect.top = br.ReadUE();
                rect.bottom = br.ReadUE();

                spsinfo->AddItem(new PropItem(_T("Frame Crop (l,r,t,b)"), rect));
            }

            if(br.ReadU1())
            {
                spsinfo->AddItem(new PropItem(_T("VUI Parameters present"), 1));
                // TODO maybe later
                return 0;
            }
            else
                spsinfo->AddItem(new PropItem(_T("VUI Parameters present"), 0));
        }
        else if(NALType == 8)
        {
            PropItem *spsinfo = mtinfo->AddItem(new PropItem(_T("H264 Picture Parameter Set")));
            spsinfo->AddItem(new PropItem(_T("PPS ID"), br.ReadUE()));
            spsinfo->AddItem(new PropItem(_T("SPS ID"), br.ReadUE()));
            spsinfo->AddItem(new PropItem(_T("Entropy Coding Mode"), br.ReadU1()));
            spsinfo->AddItem(new PropItem(_T("Pic Order Present"), br.ReadU1()));

            int numSliceGroups = br.ReadUE();
            spsinfo->AddItem(new PropItem(_T("Num Slice Groups (-1)"), numSliceGroups));
            if(numSliceGroups > 0)
            {
                int mapType = br.ReadUE();
                spsinfo->AddItem(new PropItem(_T("Slice Group Map Type"), mapType));

                // TODO later
                return 0;
            }

            spsinfo->AddItem(new PropItem(_T("Num Ref Idx 10 Activ (-1)"), br.ReadUE()));
            spsinfo->AddItem(new PropItem(_T("Num Ref Idx 11 Activ (-1)"), br.ReadUE()));
            spsinfo->AddItem(new PropItem(_T("Weighted Pred"), br.ReadU1()));
            spsinfo->AddItem(new PropItem(_T("Weighted Bibred Idc"), br.ReadU(2)));
            spsinfo->AddItem(new PropItem(_T("Pic Init QP (-26)"), br.ReadSE()));
            spsinfo->AddItem(new PropItem(_T("Pic Init QS (-26)"), br.ReadSE()));
            spsinfo->AddItem(new PropItem(_T("Deblocking Filter Ctrl Present"), br.ReadU1()));
            spsinfo->AddItem(new PropItem(_T("Constrained Intra Pred"), br.ReadU1()));
            spsinfo->AddItem(new PropItem(_T("Redundant Pic Count Present"), br.ReadU1()));
            return 0;
        }

        return 0;
    }

    int GetExtradata_H264(const CMediaType *pmt, PropItem *mtinfo)
	{
        if (pmt->pbFormat == NULL
				|| pmt->formattype != FORMAT_MPEG2Video
				|| pmt->cbFormat < sizeof(MPEG2VIDEOINFO)) 
			return 0;

		const MPEG2VIDEOINFO * const m2vi = (MPEG2VIDEOINFO*)pmt->pbFormat;
		int			extralen = m2vi->cbSequenceHeader;
        uint8*      extra = (uint8*)m2vi->dwSequenceHeader;

		// done with
		if (extralen <= 0) return 0;

        CBitStreamReader br(extra, extralen);
        int lastNullBytes = 0;

        while(br.GetPos() < extralen-4)
        {
            BYTE val = br.ReadU8();

            if (val == 0) lastNullBytes++;
            else if (val == 1 && lastNullBytes >= 3)
            {
                NALParser(br, mtinfo);
                // zum vollen byte springen
                br.SetPos(br.GetPos());
                lastNullBytes = 0;
            }
            else
                lastNullBytes = 0;
        }

		return 0;
	}

    int GetExtradata_MPEGVideo(const CMediaType *pmt, PropItem *mtinfo)
	{
        if (pmt->pbFormat == NULL
				|| pmt->formattype != FORMAT_MPEG2Video
				|| pmt->cbFormat < sizeof(MPEG2VIDEOINFO)) 
			return 0;
        bool isMpeg1 = false;

		const MPEG2VIDEOINFO	* const m2vi = (MPEG2VIDEOINFO*)pmt->pbFormat;
		int			extralen = m2vi->cbSequenceHeader;
        uint8*      extra = (uint8*)m2vi->dwSequenceHeader;

		// done with
		if (extralen <= 0) return 0;

        CBitStreamReader br(extra, extralen);
        
        // prefix lesen 0x000001b3
        UINT16 prefix = br.ReadU16();
        if(prefix != 0) return 0;
        prefix = br.ReadU8();
        if(prefix != 1) return 0;
        prefix = br.ReadU8();

        if(prefix == 0xB3) // MPEG1Seq
        {
            // http://www.fr-an.de/projects/01/01_01_02.htm
            PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Sequence Header")));
            
            UINT16 width = br.ReadU(12);
            shinfo->AddItem(new PropItem(_T("Width"), width));

            UINT16 height = br.ReadU(12);
            shinfo->AddItem(new PropItem(_T("Height"), height));

            // aspect
            UINT8 aspect = br.ReadU(4);
            if(isMpeg1)
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
            if(framerate > 8)
                frVal.Format(_T("reserved (%d)"), framerate);
            else
                frVal = framerateValues[framerate];
            shinfo->AddItem(new PropItem(_T("Framerate"), frVal));

            // Bitrate
            UINT32 bitrate = br.ReadU(18);
            if(bitrate == 0x3FFFF)
                shinfo->AddItem(new PropItem(_T("Bitrate"), CString(_T("Variable (0x3FFFF)"))));
            else
            {
                bitrate = bitrate * 400;
                shinfo->AddItem(new PropItem(_T("Bitrate (Bit/s)"), bitrate));
            }

            // Marker
            UINT8 marker = br.ReadU1();
            if(!marker)
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
            if(lim)
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
            if(lnim)
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

            if(!br.ByteAligned())
                br.SetPos(br.GetPos()+1);

            while(!br.IsEnd())
            {
                if(br.ReadU8() == 0)
                {
                    if(br.ReadU8() == 0)
                    {
                        if(br.ReadU8() == 1)
                            prefix = br.ReadU8();
                            break;
                    }
                }
            }
        }
        
        if(prefix == 0xB5) // MPEG2Seq
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
            else if(id == 2)
            {
                PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Sequenz Display Extension")));
            }
            else if(id == 3)
            {
                PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Quant Matrix Extension")));
            }
            else if(id == 4)
            {
                PropItem *shinfo = mtinfo->AddItem(new PropItem(_T("MPEG Copyright Extension")));
            }
            else if(id == 5)
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



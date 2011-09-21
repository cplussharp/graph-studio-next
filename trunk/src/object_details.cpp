//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

namespace GraphStudio
{

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

	int CLSIDToString(CLSID clsid, CString &str)
	{
		LPOLESTR	ostr = NULL;
		StringFromCLSID(clsid, &ostr);
		if (ostr) {
			str = CString(ostr);
			CoTaskMemFree(ostr);
		}
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

	int GetPinTemplateDetails(DSUtil::PinTemplate *pin, PropItem *info)
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

				GraphStudio::NameGuid(pin->major[i],mv);
				GraphStudio::NameGuid(pin->minor[i],sv);

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
		if (pi.pFilter) pi.pFilter->Release();

		group->AddItem(new PropItem(_T("Name"), CString(pi.achName)));
		if (pi.dir == PINDIR_INPUT) {
			group->AddItem(new PropItem(_T("Direction"), CString(_T("PINDIR_INPUT"))));
		} else {
			group->AddItem(new PropItem(_T("Direction"), CString(_T("PINDIR_OUTPUT"))));
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
					GraphStudio::NameGuid(mt->majortype, maj_name);
					GraphStudio::NameGuid(mt->subtype,   sub_name);
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

		return 0;
	}

	int GetAllocatorDetails(ALLOCATOR_PROPERTIES *prop, PropItem *apinfo)
	{
		apinfo->AddItem(new PropItem(_T("cBuffers"), prop->cBuffers));
		apinfo->AddItem(new PropItem(_T("cbBuffer"), prop->cbBuffer));
		apinfo->AddItem(new PropItem(_T("cbAlign"), prop->cbAlign));
		apinfo->AddItem(new PropItem(_T("cbPrefix"), prop->cbPrefix));
		return 0;
	}

	int GetMediaTypeDetails(CMediaType *pmt, PropItem *mtinfo)
	{
		CString		id_name;

		GraphStudio::NameGuid(pmt->majortype,	id_name);		mtinfo->AddItem(new PropItem(_T("majortype"), id_name));
		GraphStudio::NameGuid(pmt->subtype,		id_name);		mtinfo->AddItem(new PropItem(_T("subtype"), id_name));
		GraphStudio::NameGuid(pmt->formattype,	id_name);		mtinfo->AddItem(new PropItem(_T("formattype"), id_name));

		mtinfo->AddItem(new PropItem(_T("bFixedSizeSamples"), pmt->bFixedSizeSamples == TRUE));
		mtinfo->AddItem(new PropItem(_T("bTemporalCompression"), pmt->bTemporalCompression == TRUE));
		mtinfo->AddItem(new PropItem(_T("lSampleSize"), (int)pmt->lSampleSize));
		mtinfo->AddItem(new PropItem(_T("cbFormat"), (int)pmt->cbFormat));

		if (pmt->formattype == FORMAT_WaveFormatEx) {
			WAVEFORMATEX	*wfx = (WAVEFORMATEX*)pmt->pbFormat;
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

		} else
		if (pmt->formattype == FORMAT_VideoInfo) {
			VIDEOINFOHEADER	*vih = (VIDEOINFOHEADER*)pmt->pbFormat;
			PropItem	*vihinfo = mtinfo->AddItem(new PropItem(_T("VIDEOINFOHEADER")));
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
		if (pmt->formattype == FORMAT_VideoInfo2) {
			VIDEOINFOHEADER2	*vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
			PropItem	*vihinfo = mtinfo->AddItem(new PropItem(_T("VIDEOINFOHEADER2")));
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
		if (pmt->formattype == FORMAT_MPEGVideo) {
			MPEG1VIDEOINFO		*mvi = (MPEG1VIDEOINFO*)pmt->pbFormat;
			PropItem	*mviinfo = mtinfo->AddItem(new PropItem(_T("MPEG1VIDEOINFO")));
			GetMpeg1VideoInfoDetails(mvi, mviinfo);
		} else
		if (pmt->formattype == FORMAT_MPEG2Video) {
			MPEG2VIDEOINFO		*mvi = (MPEG2VIDEOINFO*)pmt->pbFormat;
			PropItem	*mviinfo = mtinfo->AddItem(new PropItem(_T("MPEG2VIDEOINFO")));
			GetMpeg2VideoInfoDetails(mvi, mviinfo);
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

	int GetWaveFormatExtensibleDetails(WAVEFORMATEXTENSIBLE *wfx, PropItem *wfxinfo)
	{
		// read waveformatex info
		PropItem	*wfxi = wfxinfo->AddItem(new PropItem(_T("WAVEFORMATEX")));
		GetWaveFormatExDetails(&wfx->Format, wfxi);

		// and add WFExtensible
		wfxinfo->AddItem(new PropItem(_T("wSamplesPerBlock"), (int)wfx->Samples.wSamplesPerBlock));
		wfxinfo->AddItem(new PropItem(_T("dwChannelMask"), (int)wfx->dwChannelMask));

		CString		id_name;
		GraphStudio::NameGuid(wfx->SubFormat,	id_name);		
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

	int GetWaveFormatExDetails(WAVEFORMATEX *wfx, PropItem *wfxinfo)
	{
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

	int GetVideoInfoDetails(VIDEOINFOHEADER *vih, PropItem *vihinfo)
	{
		vihinfo->AddItem(new PropItem(_T("rcSource"), vih->rcSource));
		vihinfo->AddItem(new PropItem(_T("rcTarget"), vih->rcTarget));
		vihinfo->AddItem(new PropItem(_T("dwBitRate"), (int)vih->dwBitRate));
		vihinfo->AddItem(new PropItem(_T("dwBitErrorRate"), (int)vih->dwBitErrorRate));
		vihinfo->AddItem(new PropItem(_T("AvgTimePerFrame"), (__int64)vih->AvgTimePerFrame));

		PropItem	*bihinfo = vihinfo->AddItem(new PropItem(_T("BITMAPINFOHEADER")));
		GetBitmapInfoDetails(&vih->bmiHeader, bihinfo);
		return 0;
	}

	int GetVideoInfo2Details(VIDEOINFOHEADER2 *vih, PropItem *vihinfo)
	{
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

	int GetMpeg1VideoInfoDetails(MPEG1VIDEOINFO *mvi, PropItem *mviinfo)
	{
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

	int GetMpeg2VideoInfoDetails(MPEG2VIDEOINFO *mvi, PropItem *mviinfo)
	{
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

	int GetMpegLayer3InfoDetails(MPEGLAYER3WAVEFORMAT *mp3, PropItem *mp3info)
	{
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

	int GetMpeg1WaveFormatDetails(MPEG1WAVEFORMAT *wfx, PropItem *mpinfo)
	{
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

	int GetBitmapInfoDetails(BITMAPINFOHEADER *bih, PropItem *bihinfo)
	{
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

	int GetExtradata_AAC(CMediaType *pmt, PropItem *mtinfo)
	{
		if (pmt->formattype != FORMAT_WaveFormatEx) return 0;

		WAVEFORMATEX	*wfx = (WAVEFORMATEX*)pmt->pbFormat;
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


};



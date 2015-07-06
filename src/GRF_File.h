#pragma once

// Code to parse GRF files adapted from original C code by written Alessandro Angeli, see licence below

/*
	All source code provided on this web site is distribured according to the following BSD-style license.
	However, you are allowed to exclude clause 3 if and only if you redistribute the source or binary code under the GPL version 2.0 (see here) or 3.0 (see here).

	Copyright (c) 2010, Alessandro Angeli
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	1. Redistributions of source code must retain the above copyright
	   notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.
	3. All advertising materials mentioning features or use of this software
	   must display the following acknowledgement:
	   This product includes software developed by Alessandro Angeli.
	4. Neither the name of Alessandro Angeli nor the
	   names of its contributors may be used to endorse or promote products
	   derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY ALESSANDRO ANGELI ''AS IS'' AND ANY
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL ALESSANDRO ANGELI BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

struct GRF_Filter 
{
	GRF_Filter()
		: index(0)
		, clsid(GUID_NULL)
	{}

	int						index;
	CString					name;
	GUID					clsid;
	CString					source_filename;
	CString					sink_filename;
	CStringA				ipersiststream_data;	// can contain NULL characters

	CComPtr<IBaseFilter>	ibasefilter;
};

struct GRF_Connection
{
	GRF_Connection()
		: output_filter_index(0)
		, input_filter_index(0)
		, flags(0) {}

	int						output_filter_index;	// 1-based filter index
	CString					output_pin_id;
	int						input_filter_index;		// 1-based filter index
	CString					input_pin_id;
	int						flags;
	CMediaType				media_type;
};

class GRF_File
{
public:
	GRF_File()		{ Clear(); }
	void Clear();

	HRESULT Load(LPCWSTR fileName);

public:

	CArray<GRF_Filter>			grf_filters;
	CArray<GRF_Connection>		grf_connections;
	int							clock_flags;		// 0 for no clock, 1 for clock
	int							clock_index;		// index of filter implementing clock. If zero use default clock (CLSID of clock used rather than filter index is stored in the GRF file in this case)
};

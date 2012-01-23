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
	//	Some additional media types
	//
	//-------------------------------------------------------------------------

	// {000000FF-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_AAC =
	{ 0x000000ff, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

	// {000001FF-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_LATM_AAC =
	{ 0x000001ff, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
	
	// {726D6173-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_AMR =
	{ 0x726D6173, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

	// THESE are WMA SUBTYPES.
	// They are supposed to be WMA9, WMA9Pro, WMA9Lossless and WMA9Voice
	// Once I figure out which one is which I'll do an update

	// {00000160-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_WMA9_00 =
	{ 0x00000160, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
	
	// {00000161-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_WMA9_01 =
	{ 0x00000161, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

	// {00000162-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_WMA9_02 =
	{ 0x00000162, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

	// {00000163-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_WMA9_03 =
	{ 0x00000163, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };



    // from MainConcept Demo Codec SDK
    // {8D2D71CB-243F-45E3-B2D8-5FD7967EC09B}
    static const GUID MEDIASUBTYPE_MC_H264 =
	{ 0x8D2D71CB, 0x243F, 0x45E3, { 0xB2, 0xD8, 0x5F, 0xD7, 0x96, 0x7E, 0xC0, 0x9B } };




};

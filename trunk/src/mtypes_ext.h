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



	// from MainConcept Demo Codec SDK
	// {55B845A5-8169-4BE7-BA63-6C4C2C01266D}
	static const GUID MEDIASUBTYPE_MC_H265 =
	{ 0x55B845A5, 0x8169, 0x4bE7, { 0xBA, 0x63, 0x6C, 0x4C, 0x2C, 0x01, 0x26, 0x6D } };

	// {31637668-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_hvc1 =
	{ 0x31637668, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };

	// {31435648-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_HVC1 =
	{ 0x31435648, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };

	// {63766568-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_hevc =
	{ 0x63766568, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

	// {43564548-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_HEVC =
	{ 0x43564548, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

	// {35363268-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_h265 =
	{ 0x35363268, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

	// {35363248-0000-0010-8000-00AA00389B71}
	static const GUID MEDIASUBTYPE_H265 =
	{ 0x35363248, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };



    // from avcstrm.h
    // {ddcff71a-fc9f-4bd9-b90b-197b0d44ad94}
    static const GUID KSDATAFORMAT_SPECIFIER_DV_AVC =
	{ 0xddcff71aL, 0xfc9f, 0x4bd9, { 0xb9, 0xb, 0x19, 0x7b, 0xd, 0x44, 0xad, 0x94 } };

    // {f09dc377-6e51-4ec5-a0c4-cd7f39629880}
    static const GUID KSDATAFORMAT_SPECIFIER_AVC =
	{ 0xf09dc377L, 0x6e51, 0x4ec5, { 0xa0, 0xc4, 0xcd, 0x7f, 0x39, 0x62, 0x98, 0x80 } };

    // {97e218b1-1e5a-498e-a954-f962cfd98cde}
    static const GUID KSDATAFORMAT_SPECIFIER_61883_4 =
	{ 0x97e218b1L, 0x1e5a, 0x498e, { 0xa9, 0x54, 0xf9, 0x62, 0xcf, 0xd9, 0x8c, 0xde } };

GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

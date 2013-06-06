//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

CCrc32::CCrc32(void) :
    m_pdwCrc32Table(NULL)
{
    // This is the official polynomial used by CRC32 in PKZip.
	// Often times the polynomial shown reversed as 0x04C11DB7.
	DWORD dwPolynomial = 0xEDB88320;
	int i, j;

	m_pdwCrc32Table = new DWORD[256];

	DWORD dwCrc;
	for(i = 0; i < 256; i++)
	{
		dwCrc = i;
		for(j = 8; j > 0; j--)
		{
			if(dwCrc & 1)
				dwCrc = (dwCrc >> 1) ^ dwPolynomial;
			else
				dwCrc >>= 1;
		}
		m_pdwCrc32Table[i] = dwCrc;
	}
}

CCrc32::~CCrc32(void)
{
    if (m_pdwCrc32Table)
        delete [] m_pdwCrc32Table;
}


DWORD CCrc32::GetCrc32FromData(const BYTE* data, size_t size) const
{
    if (data == NULL || size == 0)
        return 0;

    DWORD dwCrc32 = 0xFFFFFFFF;

    for (size_t i = 0; i<size; i++)
        dwCrc32 = ((dwCrc32) >> 8) ^ m_pdwCrc32Table[(data[i]) ^ ((dwCrc32) & 0x000000FF)];

    dwCrc32 = ~dwCrc32;
    return dwCrc32;
}
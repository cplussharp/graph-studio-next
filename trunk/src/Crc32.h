//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#pragma once

// Create a CRC32 Checksum of data
// thanks to: http://www.codeproject.com/Articles/1671/CRC32-Generating-a-checksum-for-a-file

class CCrc32
{
public:
    CCrc32(void);
    ~CCrc32(void);

    DWORD GetCrc32FromData(const BYTE* data, size_t size) const;

private:
    DWORD* m_pdwCrc32Table;
};


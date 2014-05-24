//-----------------------------------------------------------------------------
//
//	Musepack Demuxer
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

GRAPHSTUDIO_NAMESPACE_START			// cf stdafx.h for explanation

	//-----------------------------------------------------------------------------
	//
	//	Bitstream class
	//
	//	podporuje citanie bitstreamu
	//
	//----------------------------------------------------------------------------

	const int32 Bitstream::EXP_GOLOMB_MAP[2][48] =
	{
		{
			47, 31, 15,  0, 23, 27, 29, 30,
			 7, 11, 13, 14, 39, 43, 45, 46,
			16,  3,  5, 10, 12, 19, 21, 26,
			28, 35, 37, 42, 44,  1,  2,  4,
			 8, 17, 18, 20, 24,  6,  9, 22,
			25, 32, 33, 34, 36, 40, 38, 41
		},
		{
			 0, 16,  1,  2,  4,  8, 32,  3,
			 5, 10, 12, 15, 47,  7, 11, 13,
			14,  6,  9, 31, 35, 37, 42, 44,
			33, 34, 36, 40, 39, 43, 45, 46,
			17, 18, 20, 24, 19, 21, 26, 28,
			23, 27, 29, 30, 22, 25, 38, 41
		}
	};

	const int32 Bitstream::EXP_GOLOMB_MAP_INV[2][48] =
	{
		{
			 3, 29, 30, 17, 31, 18, 37,  8,
			32, 38, 19,  9, 20, 10, 11,  2,
			16, 33, 34, 21, 35, 22, 39,  4,
			36, 40, 23,  5, 24,  6,  7,  1,
			41, 42, 43, 25, 44, 26, 46, 12,
			45, 47, 27, 13, 28, 14, 15,  0
		},
		{
			 0,  2,  3,  7,  4,  8, 17, 13,
			 5, 18,  9, 14, 10, 15, 16, 11,
			 1, 32, 33, 36, 34, 37, 44, 40,
			35, 45, 38, 41, 39, 42, 43, 19,
			 6, 24, 25, 20, 26, 21, 46, 28,
			27, 47, 22, 29, 23, 30, 31, 12 
		}
	};


	// Exp-Golomb Codes

	const int32 Bitstream::EXP_GOLOMB_SIZE[255] =
	{
		1, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7,
		9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
		11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
		11,11,11,11,11,11,11,11,11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
		13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
		13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
		13,13,13,13,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15
	};

	uint32 Bitstream::Get_UE()
	{
		int32 len = 0;
		NeedBits24();
		while (!UGetBits(1)) len++;
		NeedBits24();
		return (len == 0 ? 0 : (1<<len)-1 + UGetBits(len));
	}

	int32 Bitstream::Get_SE()
	{
		int32 len = 0;
		NeedBits24();
		while (!UGetBits(1)) len++;
		if (len == 0) return 0;
		NeedBits24();
		int32 val = (1 << len) | UGetBits(len);
		return (val&1) ? -(val>>1) : (val>>1);
	}

	int32 Bitstream::Get_ME(int32 mode)
	{
		// nacitame UE a potom podla mapovacej tabulky
		int32 codeNum = Get_UE();
		if (codeNum >= 48) return -1;		// chyba
		return EXP_GOLOMB_MAP[mode][codeNum];
	}

	int32 Bitstream::Get_TE(int32 range)
	{
		/* ISO/IEC 14496-10 - Section 9.1 */
		if (range > 1) {
			return Get_UE();
		} else {
			return (!UGetBits(1))&0x01;
		}
	}

	int32 Bitstream::Get_Golomb(int k)
	{
		int32 l=0;
		NeedBits();
		while (UBits(8) == 0) {
			l += 8;
			DumpBits(8);
			NeedBits();	
		}
		while (UGetBits(1) == 0) l++;
		NeedBits();
		return (l << k) | UGetBits(k);
	}




    int CBitStreamReader::StripEmulationBytes(UINT8* buf, SIZE_T bufLen)
    {
        if (buf == NULL || bufLen < 4) return 0;

        int strippedBytes = 0;
        int lastBytesNull = 0;
        for (SIZE_T i=0; i<bufLen; i++)
        {
            
            if (buf[i] == 0x03 && lastBytesNull == 2)
            {
                strippedBytes++;
                lastBytesNull = 0;
            }
            else
            {
                if (buf[i] == 0)
                    lastBytesNull++;
                else
                    lastBytesNull = 0;

                if (strippedBytes)
                    buf[i-strippedBytes] = buf[i];
            }
        }

        return strippedBytes;
    }

    CBitStreamReader::CBitStreamReader(const UINT8* buf, int size, bool skipEmulationBytes)
        : m_start(buf), m_p(buf), m_end(buf + size), m_bitsLeft(8), m_skipEmulationBytes(skipEmulationBytes)
    {
    }

    void CBitStreamReader::GotoNextByteIfNeeded()
    {
        if (m_bitsLeft == 0)
        {
            m_p++;
            if (GetPos() > 2 && m_skipEmulationBytes)
            {
                if (*(m_p-2) == 0 && *(m_p-1) == 0 && *m_p == 0x03)
                {
                    // skip emulation byte
                    m_p++;
                }
            }

            m_bitsLeft = 8;
        }
    }

    UINT32 CBitStreamReader::ReadU1()
    {
        UINT32 r = 0;
        if (IsEnd()) return 0;
        
        m_bitsLeft--;
        r = ((*(m_p)) >> m_bitsLeft) & 0x01;
    
        GotoNextByteIfNeeded();
    
        return r;
    }

    UINT32 CBitStreamReader::ReadU8()
    {
        UINT32 r = 0;
        if (IsEnd()) return 0;
        
        if (m_bitsLeft == 8)
        {
            // optimized reading when byte aligned
            r = *(m_p);
            m_bitsLeft = 0;

            GotoNextByteIfNeeded();
        }
        else
            r = ReadU(8);
    
        return r;
    }

    void CBitStreamReader::SkipU1()
    {
        if (IsEnd()) return;
        
        m_bitsLeft--;   
        GotoNextByteIfNeeded();
    }

    void CBitStreamReader::SkipU8(int n=1)
    {
        if (IsEnd()) return;

        int bitsLeft = m_bitsLeft;

        for (int i=0; i<n; i++)
        {
            m_bitsLeft = 0;
            GotoNextByteIfNeeded();
        }

        m_bitsLeft = bitsLeft;
    }
    
    UINT32 CBitStreamReader::ReadU(int n)
    {
        UINT32 r = 0;
        int i;
        for (i = 0; i < n; i++)
        {
            r |= ( ReadU1() << ( n - i - 1 ) );
        }
        return r;
    }

    void CBitStreamReader::SkipU(int n)
    {
        for (int i=0; i<n; i++)
            SkipU1();
    }

    UINT16 CBitStreamReader::ReadU16()
    {
        UINT16 r = ReadU(8) << 8;
        r |= ReadU8();
        return r;
    }

    UINT32 CBitStreamReader::ReadU32()
    {
        UINT32 r = ReadU(8) << 8;
        r |= ReadU8();
        r <<= 8;
        r |= ReadU8();
        r <<= 8;
        r |= ReadU8();
        return r;
    }
	
    UINT32 CBitStreamReader::ReadUE()
    {
	    UINT32 r = 0;
	    int i = 0;
	
	    while( ReadU1() == 0 && i < 32 && !IsEnd() )
	    {
	        i++;
	    }
	    r = ReadU(i);
	    r += (1 << i) - 1;
	    return r;
    }
	
    INT32 CBitStreamReader::ReadSE() 
    {
	    INT32 r = ReadUE();
	    if (r & 0x01)
	    {
	        r = (r+1)/2;
	    }
	    else
	    {
	        r = -(r/2);
	    }
	    return r;
    }

    UINT32 CBitStreamReader::PeekU1()
    {
        INT32 r = 0;
        if (IsEnd())
        {
            // whast is with m_skipEmulationBytes in this case?
            r = ((*(m_p)) >> (m_bitsLeft-1)) & 0x01;
        }
        return r;
    }


GRAPHSTUDIO_NAMESPACE_END			// cf stdafx.h for explanation

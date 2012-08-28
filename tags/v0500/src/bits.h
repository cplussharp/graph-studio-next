//-----------------------------------------------------------------------------
//
//	Bitstream classes.
//
//	Originally seen in Skal's mpeg-4 video codec. Modified by Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

// well.. I just like those
#ifndef uint8
	typedef unsigned char		uint8;
	typedef unsigned short		uint16;
	typedef unsigned int		uint32;
	typedef unsigned __int64	uint64;
	typedef __int8				int8;
	typedef __int16				int16;
	typedef __int32				int32;
	typedef __int64				int64;
#endif


namespace GraphStudio
{

	//-------------------------------------------------------------------------
	//
	//	Bitstream class
	//
	//-------------------------------------------------------------------------

	class Bitstream
	{
	public:
		uint32	bitbuf;				// bitbuffer
		uint8	*buf;				// byte buffer
		int32	bits;				// pocet bitov v bitbufferi 

	public:
		static const int32 EXP_GOLOMB_MAP[2][48];
		static const int32 EXP_GOLOMB_MAP_INV[2][48];
		static const int32 EXP_GOLOMB_SIZE[255];

	public:

		// Konstruktory
		Bitstream() : bitbuf(0), buf(NULL), bits(0) { };
		Bitstream(uint8 *b) : bitbuf(0), buf(b), bits(0) { };
		Bitstream(const Bitstream &b) : bitbuf(b.bitbuf), buf(b.buf), bits(b.bits) { };

		// Operator priradenia = kopia stavu bitstreamu
		Bitstream &operator =(const Bitstream &b) { bitbuf = b.bitbuf; buf = b.buf; bits = b.bits; return *this; };
		Bitstream &operator =(const uint8 *b) { bitbuf = 0; buf = (uint8*)b; bits = 0; return *this; };
		Bitstream &operator =(uint8 *b) { bitbuf = 0; buf = b; bits = 0; return *this; };

		// Resetovanie stavu
		inline void Init(const uint8 *b) { bitbuf = 0; buf = (uint8*)b; bits = 0; };
		inline void Init(uint8 *b) { bitbuf = 0; buf = b; bits = 0; };

		// Zistenie stavu bitstreamu
		inline int32 BitsLeft() { return bits; };
		inline uint32 BitBuf() { return bitbuf; };
		inline uint8 *Position() { return buf - (bits/8); };

		// Citanie z bitstreamu
		inline void DumpBits(int32 n) { bitbuf <<= n; bits -= n; };
		inline uint32 UBits(int32 n) { return (uint32)(bitbuf >> (32-n)); };
		inline uint32 UGetBits(int32 n) { uint32 val = (uint32)(bitbuf >> (32-n)); bitbuf <<= n; bits -= n; return val; };
		inline int32 SBits(int32 n) { return (int32)(bitbuf >> (32-n)); };
		inline int32 SGetBits(int32 n) { int32 val = (int32)(bitbuf >> (32-n)); bitbuf <<= n; bits -= n; return val; };
		inline void Markerbit() { DumpBits(1); }

		// Reading variable length size field for Musepack SV8
		inline int64 GetMpcSize() {
			int64 ret=0;
			uint8 tmp;
			do {
				NeedBits();
				tmp = UGetBits(8);
				ret = (ret<<7) | (tmp&0x7f);
			} while (tmp&0x80);
			return ret;
		}

		// AAC LATM
		inline int64 LatmGetValue() {
			NeedBits();
			uint8 bytesForValue = UGetBits(2);
			int64 value = 0;
			for (int i=0; i<=bytesForValue; i++) {
				NeedBits();
				value <<= 8;
				value |= UGetBits(8);
			}
			return value;
		}

		// Byte alignment
		inline int32 IsByteAligned() { return !(bits&0x07); };
		inline void ByteAlign() { if (bits&0x07) DumpBits(bits&0x07); };

		// Exp-Golomb Codes
		uint32 Get_UE();
		int32 Get_SE();
		int32 Get_ME(int32 mode);
		int32 Get_TE(int32 range);
		int32 Get_Golomb(int k);

		inline int32 Size_UE(uint32 val) 
		{
			if (val<255) return EXP_GOLOMB_SIZE[val];
			int32 isize=0;
			val++;
			if (val >= 0x10000) { isize+= 32;	val = (val >> 16)-1; }
			if (val >= 0x100)	{ isize+= 16;	val = (val >> 8)-1;  }
			return EXP_GOLOMB_SIZE[val] + isize;
		}

		inline int32 Size_SE(int32 val)				{ return Size_UE(val <= 0 ? -val*2 : val*2 - 1); }
		inline int32 Size_TE(int32 range, int32 v)  { if (range == 1) return 1; return Size_UE(v);	}

		// Loading bits...
		inline void NeedBits() { if (bits < 16) { bitbuf |= ((buf[0] << 8) | (buf[1])) << (16-bits); bits += 16; buf += 2; } };
		inline void NeedBits24() { while (bits<24) { bitbuf |= (buf[0] << (24-bits)); buf++; bits+= 8; } };
		inline void NeedBits32() { while (bits<32) { bitbuf |= (buf[0] << (24-bits)); buf++; bits+= 8; } };

		// Bitstream writing
		inline void PutBits(int32 val, int32 num) {
			bits += num;
			if (num < 32) val &= (1<<num)-1;
			if (bits > 32) {
				bits -= 32;
				bitbuf |= val >> (bits);
				*buf++ = ( bitbuf >> 24 ) & 0xff;
				*buf++ = ( bitbuf >> 16 ) & 0xff;
				*buf++ = ( bitbuf >>  8 ) & 0xff;
				*buf++ = ( bitbuf >>  0 ) & 0xff;
				bitbuf = val << (32 - bits);
			} else
			bitbuf |= val << (32 - bits);
		}
		inline void WriteBits()	{
			while (bits >= 8) {
				*buf++ = (bitbuf >> 24) & 0xff;
				bitbuf <<= 8;
				bits -= 8;
			}
		}
		inline void Put_ByteAlign_Zero() { int32 bl=(bits)&0x07; if (bl<8) { PutBits(0,8-bl); } WriteBits(); }
		inline void Put_ByteAlign_One() { int32 bl=(bits)&0x07; if (bl<8) {	PutBits(0xffffff,8-bl); } WriteBits(); }
	};


    class CBitStreamReader
    {
    public:
        CBitStreamReader(UINT8* buf, int size);
        inline UINT32 ByteAligned() const { return m_bitsLeft == 8 ? 1 : 0; }
        inline UINT32 IsEnd() const { return m_p >= m_end ? 1 : 0; }
        inline int GetPos() const { return (m_p - m_start); }
        inline void SetPos(int pos) { m_p = m_start + pos; m_bitsLeft = 8; }
        
        UINT32 ReadU(int n);
        UINT32 ReadU1();
        inline UINT32 ReadU8() {return ReadU(8);}
        UINT16 ReadU16();
        UINT32 ReadUE();
        INT32 ReadSE();

    private:
        UINT8* m_start;
        UINT8* m_p;
        UINT8* m_end;
        int m_bitsLeft;
    };

};

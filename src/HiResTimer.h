//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------
#pragma once

// Utility class for using high res performance counter
#ifndef HIGHRESTIMER
#define HIGHRESTIMER
class HighResTimer
{
private:
	LARGE_INTEGER	frequency;

    __int64 startNS;
    __int64 startRealtimeNS;

public:
	HighResTimer()
	{
		QueryPerformanceFrequency(&frequency);

        SYSTEMTIME sysTime;
        GetLocalTime(&sysTime);
        startNS = GetTimeNS();

        startRealtimeNS = (((sysTime.wHour * 60 + sysTime.wMinute) * 60 + sysTime.wSecond) * 1000 + sysTime.wMilliseconds);
        startRealtimeNS *= 1000* 1000;  // nanoseconds
	}

	__int64 GetTimeNS()
	{
		// we use high resolution counter to get time with
		// nanosecond precision
		LARGE_INTEGER		time;
		QueryPerformanceCounter(&time);

		// convert to nanoseconds
		return llMulDiv(time.QuadPart, 1000*1000*1000, frequency.QuadPart, 0);
	}

    __int64 GetRealtimeNS()
    {
        return startRealtimeNS + (GetTimeNS() - startNS);
    }
};
#endif
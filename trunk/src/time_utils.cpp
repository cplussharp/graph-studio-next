#include "stdafx.h"

#include "time_utils.h"


void MakeNiceTimeMS(int time_ms, CString &v)
{
	int		ms = time_ms%1000;	
	time_ms -= ms;
	time_ms /= 1000;

	int		h, m, s;
	h = time_ms / 3600;		time_ms -= h*3600;
	m = time_ms / 60;		time_ms -= m*60;
	s = time_ms;

	v.Format(_T("%.2d:%.2d:%.2d.%.3d"), h, m, s, ms);
}




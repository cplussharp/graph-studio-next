#include "stdafx.h"

#include "time_utils.h"


void MakeNiceTimeMS(LONGLONG time_ms, CString &v)
{
	LONGLONG ms = time_ms % 1000;	
	time_ms -= ms;
	time_ms /= 1000;

	LONGLONG h, m, s;
	h = time_ms / 3600;		time_ms -= h*3600;
	m = time_ms / 60;		time_ms -= m*60;
	s = time_ms;

	v.Format(_T("%.2d:%.2d:%.2d.%.3d"), (LONG) h, (LONG) m, (LONG) s, (LONG) ms);
}


// Code from http://stackoverflow.com/questions/1449805/how-to-format-a-number-from-1123456789-to-1-123-456-789-in-c
CString CommaFormat(__int64 n) 
{
	CString ret;

	__int64 n2 = 0;
	__int64 scale = 1;
	if (n < 0) {
		printf ("-");
		n = -n;
	}
	while (n >= 1000) {
		n2 = n2 + scale * (n % 1000);
		n /= 1000;
		scale *= 1000;
	}
	ret.Format(_T("%d"), (int)n);
	CString strGroup;
	while (scale != 1) {
		scale /= 1000;
		n = n2 / scale;
		n2 = n2  % scale;
		strGroup.Format(_T(",%03d"), (int)n);
		ret += strGroup;
	}
	return ret;
}

// Moved from StatisticsForm
CString GetCsvSeparator()
{
	TCHAR szSep[8];
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szSep, 8);
	if (szSep[0] == _T(','))
		return _T(";");
	return _T(",");
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

void CLSIDToString(const CLSID& clsid, CString &str)
{
	LPOLESTR	ostr = NULL;
	StringFromCLSID(clsid, &ostr);
	if (ostr) {
		str = ostr;
		CoTaskMemFree(ostr);
	}
}

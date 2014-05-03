#pragma once

void MakeNiceTimeMS(LONGLONG time_ms, CString &v);
CString CommaFormat(__int64 n);

// local information
CString GetCsvSeparator();

// Four-CC
int GetFourCC(DWORD fcc, CString &str);

// CLSID utils
void CLSIDToString(const CLSID& clsid, CString &str);

// The functions in here could do with being moved into different stand-alone utility modules which can be used from filter DLLs
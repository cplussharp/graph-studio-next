//-----------------------------------------------------------------------------
//
//	GraphStudioNext
//
//	Author : CPlusSharp
//
//-----------------------------------------------------------------------------

#pragma once

GRAPHSTUDIO_NAMESPACE_START

// Convert VARTYPE to string representation
CString VarTypeToString(VARTYPE vt);

// Convert string representation back to VARTYPE
VARTYPE StringToVarType(const CString& str);

GRAPHSTUDIO_NAMESPACE_END
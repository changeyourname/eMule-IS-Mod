//this file is part of eMule IS Mod
//Copyright (C) 2011 IS Mod Team
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "Base64Coding.h"
#include "StringConversion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString CBase64Coding::Encode( const CString& string )
{
	CStringA result_a = Encode(StrToUtf8(string));
	return OptUtf8ToStr(result_a);
}

CStringA CBase64Coding::Encode( const CStringA& string )
{
	int dst_len = Base64EncodeGetRequiredLength(string.GetLength());
	char* result = (char*)_alloca(dst_len+1);
	
	Base64Encode((BYTE*)(LPCSTR)string, string.GetLength(), result, &dst_len);
	result[dst_len] = '\0';

	return CStringA(result);
}

CString CBase64Coding::Decode( const CString& string )
{
	CStringA result_a = Decode(StrToUtf8(string));
	return OptUtf8ToStr(result_a);
}

CStringA CBase64Coding::Decode( const CStringA& string ) {
	int dst_len = Base64DecodeGetRequiredLength(string.GetLength());
	BYTE* result = (BYTE*)_alloca(dst_len+1);

	Base64Decode((LPCSTR)string, string.GetLength(), result, &dst_len);
	result[dst_len] = '\0';

	return CStringA(result);
}
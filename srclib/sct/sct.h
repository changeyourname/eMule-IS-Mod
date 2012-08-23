//this file is part of eMule IS Mod
//Copyright (C)2009 eMule IS Mod
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

#ifndef _SCT_H_
#define _SCT_H_

extern LPCTSTR g_szVersion;
extern LPCTSTR g_szEmuleVersion;
extern CString* g_pstrLogin;
extern CString* g_pstrPassword;

// old function
LPCTSTR __declspec(dllimport) GetSctVersion();

/*************************************************************************/

LPCTSTR __declspec(dllimport) __stdcall
Sct_GetVersion();

void __declspec(dllimport) __stdcall
Sct_SetEmuleVersion(LPCTSTR version);

void __declspec(dllimport) __stdcall
Sct_SetCredentials(void* login, void* password);

#endif // #ifndef _SCTCORE_H_
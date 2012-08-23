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

#include "stdafx.h"
#include "SctWnd.h"
#include "options.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE NEAR extensionDLL = { NULL, NULL };

LPCTSTR g_szVersion = SCT_VERSION;
LPCTSTR g_szEmuleVersion = NULL;
CString* g_pstrLogin = NULL;
CString* g_pstrPassword = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        // Extension DLL one-time initialization.
        if (!AfxInitExtensionModule(extensionDLL, hInstance))
            return 0;

        // create a new CDynLinkLibrary for this app
        #pragma warning(push)
        #pragma warning(disable:6211) //silence prefast warning because CDynLinkLibrary is cleaned up elsewhere
        new CDynLinkLibrary(extensionDLL);
        #pragma warning(pop)
    } else if (dwReason == DLL_PROCESS_DETACH ) {
		if(g_szEmuleVersion) free((void*)g_szEmuleVersion);
	}
    return 1;   // ok
}

/*************************************************************************/

// old deprecated function (for backwards compatibility)
LPCTSTR __declspec(dllexport) GetSctVersion()
{
    return SCT_VERSION;
}

/*************************************************************************/

LPCTSTR __declspec(dllexport) __stdcall
Sct_GetVersion()
{
	return g_szVersion;
}

void __declspec(dllexport) __stdcall
Sct_SetCredentials(void* login, void* password)
{
	g_pstrLogin = (CString*)login;
	g_pstrPassword = (CString*)password;
}

void __declspec(dllexport) __stdcall
Sct_SetEmuleVersion(LPCTSTR version)
{
	if(g_szEmuleVersion) free((void*)g_szEmuleVersion);
	g_szEmuleVersion = _tcsdup(version);
}




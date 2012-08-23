//this file is part of eMule IS Mod
//Copyright (C)2012 IS Mod Team
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <string>
#include <vector>
#include <curl/curl.h>
#include "emhelper.h"

namespace emActiveX
{

typedef HRESULT (__stdcall *FN_DllRegisterServer)(void);
typedef HRESULT (__stdcall *FN_DllUnregisterServer)(void);

HRESULT EMHELPER_API registerServer( LPCTSTR module )
{
	FN_DllRegisterServer __DllRegisterServer;
	
	HMODULE lib = LoadLibrary(module);
	if( NULL == lib ) return E_FAIL;
	
	__DllRegisterServer = (FN_DllRegisterServer)GetProcAddress(lib, "DllRegisterServer");
	if( NULL == __DllRegisterServer ) return E_FAIL;

	HRESULT result = __DllRegisterServer();
	FreeLibrary(lib); // TODO: check result?

	return result;
}

HRESULT EMHELPER_API unregisterServer( LPCTSTR module )
{
	FN_DllUnregisterServer __DllUnregisterServer;

	HMODULE lib = LoadLibrary(module);
	if( NULL == lib ) return E_FAIL;

	__DllUnregisterServer = (FN_DllRegisterServer)GetProcAddress(lib, "DllUnregisterServer");
	if( NULL == __DllUnregisterServer ) return E_FAIL;

	HRESULT result = __DllUnregisterServer();
	FreeLibrary(lib); // TODO: check result?

	return result;
}

};
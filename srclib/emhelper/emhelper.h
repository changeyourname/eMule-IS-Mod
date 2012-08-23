//this file is part of eMule IS Mod
//Copyright (C)2012 eMule IS Mod Team
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

#pragma once

#include <ostream>
#include <string>

#include "emapi.h"

namespace emCurl
{
bool EMHELPER_API httpRequestAsync( const std::string & url, const std::string & postData, std::ostream * outStream, HWND hwnd, UINT msg );
int EMHELPER_API httpRequest( const std::string & url, const std::string & postData, std::ostream & outStream );
const char EMHELPER_API *formatError( int code );
};

namespace emActiveX
{
HRESULT EMHELPER_API registerServer( LPCTSTR module );
HRESULT EMHELPER_API unregisterServer( LPCTSTR module );
};

namespace emUtils
{
std::wstring EMHELPER_API decodeHtmlEntities( const std::wstring &text );
bool EMHELPER_API shutdownWindows();
bool EMHELPER_API verifyEmbeddedSignature( const std::wstring &file );
};
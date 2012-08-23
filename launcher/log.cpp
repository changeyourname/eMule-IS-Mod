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

#include <time.h>
#include <sstream>
#include <codecvt>
#include "log.h"

using namespace std;

CLog logger;

CLog::CLog()
{
#ifdef ENABLE_LOGGER
    wstringstream logFile;
    logFile << L"D:\\emule" << time(nullptr) << L".log";
    stream_.open(logFile.str(), ios_base::app|ios_base::out);
    locale utf8_locale( locale(""), new codecvt_utf8<wchar_t, 0x10ffff, generate_header>);
    stream_.imbue(utf8_locale);
#endif
}

CLog::~CLog()
{
#ifdef ENABLE_LOGGER
    stream_.close();
#endif
}

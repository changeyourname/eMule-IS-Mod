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

#pragma once

#include <iostream>
#include <fstream>

//#define ENABLE_LOGGER 1

class CLog
{
public:
    CLog();
    ~CLog();
#ifdef ENABLE_LOGGER
    std::wostream & emit() { return stream_; }
#else
    std::wostream & emit() { return std::wclog; }
#endif
private:
    std::wofstream stream_;
};

extern CLog logger;



//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

#pragma once

#ifndef _T
#define _T(x)	x
#endif

#define _chSTR(x)		_T(#x)
#define chSTR(x)		_chSTR(x)

// *) Specify the version of emule only here with the following defines.
// *) When changing any of those version nr. defines you also have to rebuild the language DLLs.
//
// General format:
//	<major>.<minor>.<update>.<build>
//
// Fields:
//	<major>		major number (e.g. 0)
//	<minor>		minor number (e.g. 30)
//	<update>	update number (e.g. 0='a'  1='b'  2='c'  3='d'  4='e'  5='f' ...)
//	<build>		build number; currently not used
//
// Currently used:
//  <major>.<minor>.<update> is used for the displayed version (GUI) and the version check number
//	<major>.<minor>			 is used for the protocol(!) version
//
#define VERSION_MJR		0
#define VERSION_MIN		50
#define VERSION_UPDATE	0
#define VERSION_BUILD	4

// ismod [start]
#define IS_VERSION_MJR  0
#define IS_VERSION_MIN  26
#define IS_VERSION_UPD  2
#define IS_VERSION_BLD  59
#define _BETA

#define	IS_SZ_VERSION_NAME		chSTR(IS_VERSION_MJR) _T(".") \
								chSTR(IS_VERSION_MIN) _T(".") \
								chSTR(IS_VERSION_UPD) _T(".") \
								chSTR(IS_VERSION_BLD)
// ismod [end]

// NOTE: This version string is also used by the language DLLs!
#define	SZ_VERSION_NAME		chSTR(IS_VERSION_MJR) _T(".") chSTR(IS_VERSION_MIN) _T(".") chSTR(IS_VERSION_UPD)

class CAppVersion
{
public:
    explicit CAppVersion( const char * version );
    CAppVersion( uint32_t major, uint32_t minor, uint32_t update, uint32_t build );

    friend bool operator> (const CAppVersion & v1, const CAppVersion & v2 );

private:
    uint32_t m_major;
    uint32_t m_minor;
    uint32_t m_update;
    uint32_t m_build;
};

inline bool operator> (const CAppVersion & v1, const CAppVersion & v2 )
{
    return (v1.m_major > v2.m_major)
        || (v1.m_minor > v2.m_minor)
        || (v1.m_update > v2.m_update)
        || (v1.m_build > v2.m_build);
}

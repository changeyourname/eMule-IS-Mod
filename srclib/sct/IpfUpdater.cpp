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

#include "StdAfx.h"
#include <atlutil.h>
#include <Shlobj.h>
#include <sstream>
#include <emhelper/emhelper.h>
#include "IpfUpdater.h"
#include "tools.h"
#include "options.h"

#define EMULE_APPDIR        0
#define EMULE_CONFIGDIR     1

#define IPF_FILE		_T("ipfilter.dat")

static CString ShellGetFolderPath( int iCSIDL )
{
    CString strFolderPath;

    TCHAR szPath[MAX_PATH];
    if ( SHGetFolderPath(NULL, iCSIDL, NULL, SHGFP_TYPE_CURRENT, szPath) == S_OK )
        strFolderPath = szPath;

    return strFolderPath;
}

CIpfUpdater::CIpfUpdater(void)
{
}

CIpfUpdater::~CIpfUpdater(void)
{
}

void CIpfUpdater::DetectEmuleDirs()
{
    // get application dir
    TCHAR tchBuffer[MAX_PATH];
    ::GetModuleFileName(NULL, tchBuffer, _countof(tchBuffer));
    tchBuffer[_countof(tchBuffer) - 1] = _T('\0');
    LPTSTR pszFileName = _tcsrchr(tchBuffer, L'\\') + 1;
    strEmuleExeName = pszFileName;
    *pszFileName = L'\0';
    m_defaultDirs[EMULE_APPDIR] = tchBuffer; // slash

    // portable mode ( check for EMULE_APPDIR\config\preferences.ini file )
    CFileFind ff;
    bool portableMode = ff.FindFile(m_defaultDirs[EMULE_APPDIR] + L"config\\preferences.ini") != 0;
    ff.Close();
    if ( portableMode ) {
        m_defaultDirs[EMULE_CONFIGDIR] = m_defaultDirs[EMULE_APPDIR] + L"config\\"; // slash
    }
    // normal mode
    else {
        // there was a new API, but it's gone...
        //CString dir_appData = ShellGetFolderPath(CSIDL_APPDATA) + L"\\eMule IS Mod";
        CString dir_appDataLocal = ShellGetFolderPath(CSIDL_LOCAL_APPDATA) + L"\\eMule IS Mod";
        CString dir_documents = ShellGetFolderPath(CSIDL_PERSONAL);

        //::CreateDirectory(dir_appData, nullptr);
        ::CreateDirectory(dir_appDataLocal, nullptr);

        m_defaultDirs[EMULE_CONFIGDIR] = dir_appDataLocal + L"\\config\\";  // slash
    }
}

UINT AFX_CDECL CIpfUpdater::UpdateIpfThread(LPVOID /*lpParam*/)
{
	Sleep(10*1000); // подождем инициализации всякого всякого.
	
	CIpfUpdater _this;
	_this.DetectEmuleDirs();

	// download
	std::ostringstream data;
	int code = emCurl::httpRequest(SCT_IPF_URL, "", data);
	if ( 0 != code ) {
		return 1;
	}

	// save
	CFile newFile;
	if ( newFile.Open(_this.m_defaultDirs[EMULE_CONFIGDIR] + IPF_FILE, 
			          CFile::modeWrite|CFile::modeCreate|CFile::shareExclusive) == FALSE )
	{
		return FALSE;
	}
	
	std::string dataString = data.str();
    newFile.Write(dataString.c_str(), dataString.size());
	newFile.Close();

	// reload
	ShellExecute(0, 
		L"open",
		_this.m_defaultDirs[EMULE_APPDIR]+_this.strEmuleExeName,
		L"reloadipf",
		_this.m_defaultDirs[EMULE_APPDIR],
		SW_HIDE);

	return 0;
}


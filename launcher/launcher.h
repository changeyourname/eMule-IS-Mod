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

#include <string>
#include <vector>

#define LAUNCHER_UPDATE_KEY     L"/update"
#define LAUNCHER_APPDIR_KEY     L"/appdir"

class CLauncher {
public:
    CLauncher();

    const std::wstring currentDir() const { return currentDir_; }
    const std::wstring tempDir() const { return tempDir_; }
    
    void setAppDir( std::wstring dir ) { appDir_ = dir; }
    const std::wstring appDir() const { return appDir_; }

    int run();
    bool update();
    bool tryUpdate();
private:
    std::wstring currentDir_; // trailing backslash
    std::wstring tempDir_; // trailing backslash
    std::wstring appDir_;

    std::wstring lastError();
    HANDLE exec( const std::wstring &app, const std::vector<std::wstring> & args );
    static INT_PTR CALLBACK waiterDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    bool removeDir( std::wstring dir );
    std::wstring expandEnvironmentStrings( const std::wstring & source );
    bool fileVersionInfo( const std::wstring & file, VS_FIXEDFILEINFO & info );
};
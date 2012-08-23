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

#include <windows.h>
#include "log.h"
#include "launcher.h"

HINSTANCE g_hInstance;

int __stdcall wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd )
{
    g_hInstance = hInstance;

    CLauncher la;
    bool runUpdate = false;
    
    // parse command line args
    for ( int i=0; i<__argc; i++ ) {
        if ( 0 == wcscmp(__wargv[i], LAUNCHER_UPDATE_KEY) ) {
            logger.emit() << L"cmd key " LAUNCHER_UPDATE_KEY << std::endl;
            runUpdate = true;
        }
        else if ( 0 == wcscmp(__wargv[i], LAUNCHER_APPDIR_KEY) && (i+1)<__argc ) {
            logger.emit() << L"cmd key appdir: " << __wargv[i+1] << std::endl;
            la.setAppDir(__wargv[i+1]);
        }
    }

    // run update
    if ( runUpdate && !la.appDir().empty() ) {
        logger.emit() << L"Running update" << std::endl;
        la.update();
    }
    
    // try to update
    else if ( !runUpdate && la.tryUpdate() ) {
        logger.emit() << L"Try to run update" << std::endl;
        return 0;
    }
    
    // run application
    logger.emit() << L"Running application" << std::endl;
    int ret = la.run();

    return ret;
}
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
#include <sstream>
#include "log.h"
#include "launcher.h"
#include "resource.h"

#define _chStr(x)		L#x
#define chStr(x)		_chStr(x)

#define APP_VERSION_MAJOR   0
#define APP_VERSION_MINOR   26
#define APP_VERSION_UPDATE  2
#define APP_VERSION_BUILD   59

#define APP_VERSION         chStr(APP_VERSION_MAJOR)  L"."  \
                            chStr(APP_VERSION_MINOR)  L"." \
                            chStr(APP_VERSION_UPDATE) L"." \
                            chStr(APP_VERSION_BUILD)

#define UPDATE_FILE     L"emule-update.exe"
#define UPDATE_PARAM    L"/VERYSILENT"   
#define LAUNCHER_FILE   L"launcher.exe"
#define TEMP_PREFIX     L"_emuleismod_"
#define APP_NAME        L"eMule IS Mod"
#define APP_EXE         L"emule.exe"
#define APP_MUTEX       L"EMULE-ISMOD-{912C3CF8-AA86-4517-A649-200C6C7DD250}"

using namespace std;

extern HINSTANCE g_hInstance;

CLauncher::CLauncher()
{
    DWORD len;
    unique_ptr<wchar_t[]> ptr;

    // get current dir
    len = GetCurrentDirectoryW(0, nullptr);
    ptr.reset(new wchar_t[len]);
    GetCurrentDirectoryW(len, ptr.get());
    currentDir_ = ptr.get();

    // get temp dir
    len = GetTempPathW(0, nullptr);
    ptr.reset(new wchar_t[len]);
    GetTempPathW(len, ptr.get());
    tempDir_ = ptr.get();
    tempDir_ += TEMP_PREFIX L"\\";
}

/**
 * @brief try to initiate update sequence
 * @return zero on success
 */
int CLauncher::run()
{
    wstring app;
    // app dir specified, run appdir/launcher.exe
    if ( !appDir().empty() ) {
        app = appDir() + L"\\" LAUNCHER_FILE;
    } else {
        app = currentDir() + L"\\" APP_VERSION L"\\" APP_EXE;
    }

    vector<wstring> args;
    
    HANDLE hProcess = exec(app, args);

    if ( 0 == hProcess ) {
        wstring error = L"Ошибка запуска " APP_NAME L".\nПереустановите программу.(exec failed)";
        MessageBoxW(0, error.c_str(), L"Ошибка", MB_OK|MB_ICONERROR);
        return 1;
    }

    return 0;
}

bool CLauncher::tryUpdate()
{
    // skip update try if emule already running
    HANDLE appMutex = CreateMutexW(nullptr, FALSE, APP_MUTEX);
    bool emuleRunning = (::GetLastError() == ERROR_ALREADY_EXISTS ||::GetLastError() == ERROR_ACCESS_DENIED);
    if ( emuleRunning ) {
        return false;
        logger.emit() << APP_NAME L" already running. Skip update try." << endl;
    }
    CloseHandle(appMutex);

    // search update file in current dir
    if ( INVALID_FILE_ATTRIBUTES == GetFileAttributesW(UPDATE_FILE) ) {
        logger.emit() << UPDATE_FILE L" not found (" << lastError() << L")" << endl;
        return false;
    }

    // create temp directory
    CreateDirectoryW(tempDir().c_str(), nullptr);

    // move update to temp
    std::wstring dstFile = tempDir() + UPDATE_FILE;
    DeleteFileW(dstFile.c_str());
    if( !MoveFileW(UPDATE_FILE, dstFile.c_str()) ) {
        logger.emit() << L"Failed to move " << UPDATE_FILE L" to " << dstFile << L"(" << lastError() << L")" << endl;
        return false;
    }

    // copy launcher to temp directory
    dstFile = tempDir() + LAUNCHER_FILE;
    DeleteFileW(dstFile.c_str());
    if( !CopyFileW(LAUNCHER_FILE, dstFile.c_str(), FALSE) ) {
        logger.emit() << L"Failed to copy " LAUNCHER_FILE L" to " << dstFile << L"(" << lastError() << L")" << endl;
        return false;
    }

    // run launcher with special arguments
    vector<wstring> args;
    args.push_back(LAUNCHER_UPDATE_KEY);
    args.push_back(LAUNCHER_APPDIR_KEY);
    args.push_back(currentDir());

    HANDLE hProcess = exec(dstFile, args);
    if ( 0 == hProcess ) {
        return false;
    }

    return true;
}

bool CLauncher::update()
{
    wstring updateApp = currentDir() + L"\\" UPDATE_FILE; 

    vector<wstring> args;
    args.push_back(UPDATE_PARAM);
    
    HANDLE hProcess = exec(updateApp, args);
    if ( 0 == hProcess ) {
        return false;
    }

    DialogBoxParamW(g_hInstance, MAKEINTRESOURCEW(IDD_WAIT), 0, waiterDlgProc, (LPARAM)hProcess);

    WaitForSingleObject(hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(hProcess, &exitCode);
    CloseHandle(hProcess);

    // 0 - InnoSetup successful exit code
    if ( 0 != exitCode ) {
        logger.emit() << L"Update package failed (exit code:" << exitCode << L")" << endl; 
        return false;
    }

    // remove old app dir, if not the same version
    VS_FIXEDFILEINFO verInfo;
    if ( fileVersionInfo(updateApp, verInfo) ) {
        const DWORD productVersionMS = MAKELONG(APP_VERSION_MINOR, APP_VERSION_MAJOR);
        const DWORD productVersionLS = MAKELONG(APP_VERSION_BUILD, APP_VERSION_UPDATE);
        if ( (productVersionLS != verInfo.dwProductVersionLS) || (productVersionMS != verInfo.dwProductVersionMS) ) {
            wstring oldappDir( L"%LOCALAPPDATA%\\" APP_NAME L"\\application\\" APP_VERSION );
            oldappDir = expandEnvironmentStrings(oldappDir);
            if ( !removeDir(oldappDir) ) {
                logger.emit() << L"Failed to delete old application files (" << oldappDir << L")" << endl; 
            }
        }
    }
    
    return true;
}

INT_PTR CALLBACK CLauncher::waiterDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( uMsg ) {
        case WM_INITDIALOG:
            SetWindowLongPtrW(hwndDlg, GWLP_USERDATA, lParam);
            SetWindowTextW(hwndDlg, L"Обновление...");
            SetDlgItemTextW(hwndDlg, IDC_WAIT_LABEL, L"Обновление программы, пожалуйста подождите...");
            SetTimer(hwndDlg, 0 , 1000, nullptr); // check every second

            return FALSE; // no focus change
            
        case WM_TIMER: {
            HANDLE hProcess = (HANDLE)GetWindowLongPtrW(hwndDlg, GWLP_USERDATA);
            if( WAIT_TIMEOUT != WaitForSingleObject(hProcess, 0) ) {
                KillTimer(hwndDlg, 0);
                EndDialog(hwndDlg, 0);
            }
            return FALSE; 
        }

        default:
            return FALSE; 
    }
}

/**
 * @brief wrapper for GetLastError() and FormatMessage()
 * @return last error text presentation
 */
wstring CLauncher::lastError()
{
    DWORD errorCode = GetLastError();
    wstring error;
    wchar_t * buffer = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&buffer, 0, nullptr);
    error = buffer;
    LocalFree(buffer);
    error.resize(error.size()-2);
    return error;
}

/**
 * @brief executes app executable in same working dir
 * @param app application to run
 * @param args command line arguments
 * @return process handle on success, zero instead
 */
HANDLE CLauncher::exec( const wstring & app, const vector<wstring> & args )
{
    // work dir
    wstring appDir = app.substr(0, app.find_last_of(L'\\'));

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    // prepare
    wstringstream argsStream;
    argsStream << L"\"" << app << L"\"";
    for ( auto it = args.begin(); it != args.end(); ++it ) {
        // TODO: replace all " with \"
        if ( wstring::npos != it->find(L' ') )
            argsStream << L" \"" << *it << L"\"";
        else
            argsStream << L" " <<  *it ;
    }
    wstring argsString = argsStream.str();
    unique_ptr<wchar_t[]> argsMutable(new wchar_t[argsString.size()+1]);
    wcscpy_s(argsMutable.get(), argsString.size()+1, argsString.c_str());

    BOOL ret = CreateProcessW(app.c_str(), argsMutable.get(), nullptr, nullptr, FALSE, 0, nullptr, appDir.c_str(), &si, &pi);

    if ( !ret ) {
        logger.emit() << L"ERROR: Run:\"" << app << L"\", args:\"" << argsString << L"\", error:\"" << lastError() << L"\"" << endl;
        return 0;
    }
    
    CloseHandle(pi.hThread);
    CloseHandle(si.hStdError);
    CloseHandle(si.hStdInput);
    CloseHandle(si.hStdOutput);

    logger.emit() << L"Run:\"" << app << L"\", Args:\"" << argsString << L"\"" << endl;
    
    return pi.hProcess;             
}

/**
 * @brief remove directory recursive
 * @param dir   target path
 * @return true on success
 */
bool CLauncher::removeDir( wstring dir )
{
    // extra null character at the end '\0\0'
    dir.resize(dir.size()+1); 

    SHFILEOPSTRUCTW fileOp = {
        NULL,
        FO_DELETE,
        dir.c_str(),
        nullptr,
        FOF_NOCONFIRMATION |
        FOF_NOERRORUI |
        FOF_SILENT,
        false,
        nullptr,
        nullptr
    };

    return SHFileOperationW(&fileOp) == 0;
}

/**
 * @brief expand string with environment variables (ex. %SystemRoot% => 'C:\Windows')
 * @param source    string
 * @return result string
 */
wstring CLauncher::expandEnvironmentStrings( const wstring & source )
{
    size_t len = ExpandEnvironmentStringsW(source.c_str(), nullptr, 0);
    unique_ptr<wchar_t[]> buf(new wchar_t[len+1]);
    ExpandEnvironmentStringsW(source.c_str(), buf.get(), len);
    wstring result = buf.get();

    return result;
}

/**
 * @brief tries to get VS_FIXEDFILEINFO from target file
 * @param file  target file  path
 * @param info  version structure
 * @return true on success
 */
bool CLauncher::fileVersionInfo( const wstring & file, VS_FIXEDFILEINFO & info )
{
    DWORD verHandle = NULL;
    size_t len = GetFileVersionInfoSizeW(file.c_str(), &verHandle);
    unique_ptr<BYTE[]> verData(new BYTE[len]);
    if ( GetFileVersionInfoW(file.c_str(), verHandle, len, verData.get()) ) {
        LPVOID lpBuffer = nullptr;
        if ( VerQueryValueW(verData.get(), L"\\", &lpBuffer, &len) ) {
            if ( len > 0 ) {
                VS_FIXEDFILEINFO * verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                if ( 0xfeef04bd == verInfo->dwSignature ) {
                    CopyMemory(&info, verInfo, sizeof(info));
                    return true;
                }
            } 
        }
    }     

    return false;
}